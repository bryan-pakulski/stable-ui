#include "StableClient.h"
#include "Commands.h"
#include "Display/ErrorHandler.h"
#include "Helpers/States.h"
#include "Helpers/QLogger.h"
#include "ThirdParty/cppzmq/zmq.hpp"
#include <sstream>

StableClient::StableClient() {

  m_ctx = zmq::context_t{1};

  // Initialise general communication socket
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableClient::StableClient Connecting main zmq interface", m_addr);
  m_socket = zmq::socket_t(m_ctx, zmq::socket_type::req);
  m_socket.connect(m_addr);

  // Initialise heartbeat socket for keepalive status
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableClient::StableClient Connecting heartbeat zmq interface",
                             m_heartbeat_addr);
  m_heartbeatSocket = zmq::socket_t(m_ctx, zmq::socket_type::req);
  m_heartbeatSocket.connect(m_heartbeat_addr);
  m_heartbeatSocket.set(zmq::sockopt::rcvtimeo, 5000);
}

StableClient::~StableClient() {
  // TODO: close gets stuck if no connection to docker server is managed
  m_socket.unbind(m_addr);
  m_heartbeatSocket.unbind(m_heartbeat_addr);

  zmq_close(&m_socket);
  zmq_close(&m_heartbeatSocket);
  zmq_ctx_destroy(&m_ctx);
}

// Heartbeat ping/pong to determine server status
void StableClient::heartbeat(int &state) {
  commands::heartbeat cmd = commands::heartbeat();
  zmq::message_t msg = zmq::message_t(cmd.getCommandString());
  zmq::message_t pong;

  try {
    m_heartbeatSocket.send(msg, zmq::send_flags::none);

    m_heartbeatSocket.recv(pong, zmq::recv_flags::none);
    state = HEARTBEAT_STATE::ALIVE;
  } catch (const zmq::error_t &err) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, err.what());
    state = HEARTBEAT_STATE::DEAD;
    return;
  }
}

// Reloads sd server in docker (release any models in memory)
void StableClient::releaseMemory() {
  commands::restartServer cmd = commands::restartServer();
  std::string msg = sendMessage(cmd.getCommandString());
}

// Load a stable diffusion model into memory in preperation for running inference commands
void StableClient::loadModelToMemory(std::string ckpt_path, std::string config_path, std::string vae_path,
                                     std::string precision, int &state) {

  commands::loadModelToMemory cmd = commands::loadModelToMemory{ckpt_path, config_path, vae_path, precision};
  std::string msg = sendMessage(cmd.getCommandString());
  if (m_dockerCommandStatus == Q_COMMAND_EXECUTION_STATE::SUCCESS) {
    state = Q_MODEL_STATUS::LOADED;
  } else {
    state = Q_MODEL_STATUS::FAILED;
  }
}

// Text to image
void StableClient::textToImage(std::string hash, std::string outDir, std::string &canvasName, std::string prompt,
                               std::string negative_prompt, std::string &samplerName, int batch_size, int steps,
                               double cfg, int seed, int width, int height, int &renderState) {

  commands::textToImage cmd = commands::textToImage(hash, prompt, width, height, negative_prompt, canvasName,
                                                    samplerName, batch_size, 1, steps, cfg, seed, outDir);

  std::string msg = sendMessage(cmd.getCommandString());
  if (m_dockerCommandStatus == Q_COMMAND_EXECUTION_STATE::SUCCESS) {
    renderState = Q_RENDER_STATE::RENDERED;
  } else {
    renderState = Q_RENDER_STATE::UNRENDERED;
  }
}

// Image to image
void StableClient::imageToImage(std::string hash, std::string outDir, std::string &prompt, std::string &negative_prompt,
                                std::string &canvas_name, std::string &img_path, std::string &sampler_name,
                                int batch_size, int n_iter, int steps, double cfg_scale, double strength, int seed,
                                int &renderState) {
  commands::imageToImage cmd =
      commands::imageToImage(hash, prompt, negative_prompt, canvas_name, img_path, sampler_name, batch_size, n_iter,
                             steps, cfg_scale, strength, seed, outDir);

  std::string msg = sendMessage(cmd.getCommandString());
  if (m_dockerCommandStatus == Q_COMMAND_EXECUTION_STATE::SUCCESS) {
    renderState = Q_RENDER_STATE::RENDERED;
  } else {
    renderState = Q_RENDER_STATE::UNRENDERED;
  }
}

// Send message to ZMQ server listening on m_socket
std::string StableClient::sendMessage(const std::string &message) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::PENDING;

  m_socket.send(zmq::buffer(message));

  zmq::message_t recv;
  zmq::recv_result_t r = m_socket.recv(recv, zmq::recv_flags::none);

  if (recv.str() == "FAILED" || !r.has_value()) {
    m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::FAILED;
    ErrorHandler::GetInstance().setError("Docker command failed: " + message);
  } else {
    m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::SUCCESS;
  }

  return recv.str();
}