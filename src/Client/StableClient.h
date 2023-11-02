#pragma once
#include <string>
#include "Client/Commands.h"
#include "Config/config.h"
#include "Helpers/States.h"
#include "ThirdParty/cppzmq/zmq.hpp"

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
  void releaseModel();
  void loadModelToMemory(commands::loadModelToMemory command);
  void textToImage(commands::textToImage command);
  void imageToImage(commands::imageToImage command);
  void outpainting(commands::outpainting command);

private:
  zmq::context_t m_ctx;
  zmq::socket_t m_socket;
  zmq::socket_t m_heartbeatSocket;
  int m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::PENDING;

  std::mutex m_mutex;

  std::string m_addr = "tcp://" + CONFIG::DOCKER_IP_ADDRESS.get() + ":5555";
  std::string m_heartbeat_addr = "tcp://" + CONFIG::DOCKER_IP_ADDRESS.get() + ":5556";
  std::string s_failedResponse = "FAILED";

private:
  std::string sendMessage(const std::string &message);

  StableClient();
  ~StableClient() {}
};