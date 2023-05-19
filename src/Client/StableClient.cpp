#include "StableClient.h"
#include "Commands.h"
#include "../Helpers/States.h"
#include "../QLogger.h"
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

StableClient::~StableClient() {}

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

    // TODO: reset socket
    m_heartbeatSocket.disconnect(m_heartbeat_addr);
    m_heartbeatSocket = zmq::socket_t(m_ctx, zmq::socket_type::req);
    m_heartbeatSocket.connect(m_heartbeat_addr);

    return;
  }
}

// Reloads sd server in docker (release any models in memory)
void StableClient::releaseMemory(int &state) {
  commands::restartServer cmd = commands::restartServer();
  std::string msg = sendMessage(cmd.getCommandString(), state);
}

// Load a stable diffusion model into memory in preperation for running inference commands
void StableClient::loadModelToMemory(std::string ckpt_path, std::string config_path, std::string vae_path,
                                     std::string precision, int &state) {

  commands::loadModelToMemory cmd = commands::loadModelToMemory{ckpt_path, config_path, vae_path, precision};
  std::string msg = sendMessage(cmd.getCommandString(), state);
  state = Q_EXECUTION_STATE::SUCCESS;
}

// Text to image
void StableClient::textToImage(std::string outDir, std::string &canvasName, std::string prompt,
                               std::string negative_prompt, std::string &samplerName, int batch_size, int steps,
                               double cfg, int seed, int width, int height, int &renderState) {

  commands::textToImage cmd = commands::textToImage(prompt, width, height, negative_prompt, canvasName, samplerName,
                                                    batch_size, 1, steps, cfg, seed, outDir);

  std::string msg = sendMessage(cmd.getCommandString(), renderState);
  renderState = Q_EXECUTION_STATE::SUCCESS;
}

// Image to image
void StableClient::imageToImage(int &state) {}

// Send message to ZMQ server listening on m_socket
std::string StableClient::sendMessage(const std::string &message, int &state) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_socket.send(zmq::buffer(message));

  zmq::message_t recv;
  m_socket.recv(recv, zmq::recv_flags::none);

  return recv.str();
}