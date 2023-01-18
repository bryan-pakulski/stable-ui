#pragma once

#include "../Config/config.h"
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
  bool serverFinished = false;

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
  void attachModelToServer();

  // Commands that can be used by modules
  void textToImage(std::string prompt, std::string negative_prompt, int samples, int steps, double cfg, int seed,
                   int width, int height, bool &finishedFlag, std::string model_name, bool half_precision);

  void imageToImage(std::string path, std::string prompt, std::string negative_prompt, int samples, int steps,
                    double strength, int seed, bool &finishedFlag, std::string model_name, bool half_precision);
};