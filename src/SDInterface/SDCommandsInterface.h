#pragma once

#include "../config.h"
#include "../QLogger.h"

#include "py/SnakeHandler.h"

#include <memory>
#include <vector>

// Singleton class implementation
// Runs on seperate thread for non-blocking operations
class SDCommandsInterface {
private:
  std::unique_ptr<SnakeHandler> m_py_handle;
  std::thread m_Thread;
  std::vector<std::unique_ptr<base_type>> *arguments = new std::vector<std::unique_ptr<base_type>>;

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

  void textToImage(std::string prompt, int samples, int steps, int seed, int width, int height, bool &finishedFlag, std::string model_name);

  void imageToImage();
};