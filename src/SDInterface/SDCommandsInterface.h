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
  std::thread m_Thread;
  std::vector<std::unique_ptr<base_type>> *arguments = new std::vector<std::unique_ptr<base_type>>;
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

  // Commands that can be used by modules
  void textToImage(std::string sdModelPath, std::string prompt, std::string negative_prompt, int samples, int steps,
                   double cfg, int seed, int width, int height, int &renderState);

  void imageToImage(std::string path, std::string prompt, std::string negative_prompt, int samples, int steps,
                    double strength, int seed, int &renderState);
};