#pragma once

#include "../Config/config.h"
#include "../QLogger.h"

#include "py/SnakeHandler.h"
#include "../Helpers/States.h"

#include <memory>
#include <vector>

// Singleton class implementation
// Runs on seperate thread for non-blocking operations
class SDCommandsInterface {
private:
  std::unique_ptr<SnakeHandler> m_py_handle;
  std::shared_ptr<PyArgs> m_arguments;
  std::thread m_Thread;
  int m_dockerState = EXECUTION_STATE::PENDING;

  SDCommandsInterface();
  ~SDCommandsInterface();

public:
  static SDCommandsInterface &GetInstance() {
    static SDCommandsInterface s_ci;
    return s_ci;
  }

  // Prevent replication
  SDCommandsInterface(SDCommandsInterface const &) = delete;
  void operator=(SDCommandsInterface const &) = delete;

  // SD Server Specific Commands
  void launchSDModelServer();
  void terminateSDModelServer();
  void attachModelToServer(std::string ckpt_path, std::string config_path, std::string vae_path, std::string precision,
                           int &modelLoadState);
  void heartbeat(int &heartbeatState);

  // Commands that can be used by modules
  void textToImage(std::string sdModelPath, std::string &canvasName, std::string prompt, std::string negative_prompt,
                   std::string &samplerName, int samples, int steps, double cfg, int seed, int width, int height,
                   int &renderState);

  void imageToImage(std::string &sdModelPath, std::string &canvasName, std::string &imgPath, std::string &prompt,
                    std::string &negative_prompt, std::string &samplerName, int batch_size, int steps, double cfg,
                    double strength, int seed, int &renderState);
};