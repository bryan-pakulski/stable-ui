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

  void textToImage(std::string prompt, int samples, int steps, int seed, int width, int height);

  void imageToImage();
};