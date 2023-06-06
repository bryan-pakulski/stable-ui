#include "SDCommandsInterface.h"
#include <functional>

#include "Client/Commands.h"
#include "StableManager.h"
#include "Helpers/States.h"
#include <memory>

SDCommandsInterface::SDCommandsInterface() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SDCommandsInterface::SDCommandsInterface initialising");
  StableClient::GetInstance();
}

SDCommandsInterface::~SDCommandsInterface() {}

// Release SD Model Server data from gpu memory
void SDCommandsInterface::releaseSDModelServer() {}
// Connect a new model to SD Server
void SDCommandsInterface::attachModelToServer(ModelConfig model, int &state) {}

// Calls text to image command from client -> sd model server
void SDCommandsInterface::textToImage(commands::textToImage command, int &renderState) {}

void SDCommandsInterface::imageToImage(commands::imageToImage command, int &renderState) {
  renderState = Q_RENDER_STATE::RENDERING;

  m_Thread =
      std::thread(std::bind(&StableClient::imageToImage, &StableClient::GetInstance(), command, std::ref(renderState)));
  m_Thread.detach();
}