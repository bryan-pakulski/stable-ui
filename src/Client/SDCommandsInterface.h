#pragma once

#include "Config/config.h"
#include "Config/structs.h"
#include "Helpers/QLogger.h"

#include "Client/Commands.h"
#include "Client/StableClient.h"
#include "Helpers/States.h"

#include <memory>
#include <vector>

// Singleton class implementation
// Runs on seperate thread for non-blocking operations
class SDCommandsInterface {
private:
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
  void releaseSDModelServer();
  void attachModelToServer(ModelConfig model, int &state);

  // Commands that can be used by modules
  void textToImage(commands::textToImage command, int &renderState);

  void imageToImage(commands::imageToImage command, int &renderState);
};