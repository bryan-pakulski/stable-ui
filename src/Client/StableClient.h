#pragma once
#include <string>
#include "../Config/config.h"
#include "../ThirdParty/cppzmq/zmq.hpp"

// Singleton class implementation, functions are called on seperate threads, mutex lock on socket access
class StableClient {
public:
  static StableClient &GetInstance() {
    static StableClient s_client;
    return s_client;
  }

  // Prohibit external replication constructs
  StableClient(StableClient const &) = delete;
  // Prohibit external assignment operations
  void operator=(StableClient const &) = delete;

  // Commands list
  void heartbeat(int &state);
  void releaseMemory(int &state);
  void loadModelToMemory(std::string ckpt_path, std::string config_path, std::string vae_path, std::string precision,
                         int &state);

  void textToImage(std::string sdModelPath, std::string &canvasName, std::string prompt, std::string negative_prompt,
                   std::string &samplerName, int batch_size, int steps, double cfg, int seed, int width, int height,
                   int &renderState);
  void imageToImage(int &state);

private:
  zmq::context_t m_ctx;
  zmq::socket_t m_socket;
  zmq::socket_t m_heartbeatSocket;

  std::mutex m_mutex;

  std::string m_addr = "tcp://" + CONFIG::DOCKER_IP_ADDRESS.get() + ":5555";
  std::string m_heartbeat_addr = "tcp://" + CONFIG::DOCKER_IP_ADDRESS.get() + ":5556";

  std::string sendMessage(const std::string &message, int &state);

  StableClient();
  ~StableClient();
};