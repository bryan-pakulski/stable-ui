#include "SDCommandsInterface.h"
#include <functional>

#include "StableManager.h"
#include "Helpers/States.h"
#include <memory>

SDCommandsInterface::SDCommandsInterface() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SDCommandsInterface::SDCommandsInterface initialising");
  StableClient::GetInstance();
}

SDCommandsInterface::~SDCommandsInterface() {}

// Starts up SD Model Server
void SDCommandsInterface::launchSDModelServer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SDCommandsInterface::launchSDModelServer starting up SD Model Server...");

  // Launch SD Server inside docker on startup
#ifdef _WIN32
  std::string commandStr = "data\\scripts\\start_sd_server.bat";
  system(commandStr.c_str());
#else
  system("./data/scripts/start_sd_server.sh");
#endif
}

// Release SD Model Server data from gpu memory
void SDCommandsInterface::releaseSDModelServer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "SDCommandsInterface::restartSDModelServer shutting down SD Model Server...");
  StableManager::GetInstance().setModelState(Q_MODEL_STATUS::NONE_LOADED);
  m_Thread = std::thread(std::bind(&StableClient::releaseMemory, &StableClient::GetInstance()));
  m_Thread.detach();
}
// Connect a new model to SD Server
void SDCommandsInterface::attachModelToServer(std::string ckpt_path, std::string config_path, std::string vae_path,
                                              std::string precision, int &state) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SDCommandsInterface::attachModelToServer loading model to memory...");
  state = Q_MODEL_STATUS::LOADING;

  m_Thread = std::thread(std::bind(&StableClient::loadModelToMemory, &StableClient::GetInstance(), ckpt_path,
                                   config_path, vae_path, precision, std::ref(state)));
  m_Thread.detach();
}

// Calls text to image command from client -> sd model server
void SDCommandsInterface::textToImage(std::string &canvasName, std::string prompt, std::string negative_prompt,
                                      std::string &samplerName, int batch_size, int steps, double cfg, int seed,
                                      int width, int height, int &renderState) {
  std::string outDir = CONFIG::OUTPUT_DIRECTORY.get();
  renderState = Q_RENDER_STATE::RENDERING;

  model mdl = StableManager::GetInstance().getModel();

  m_Thread = std::thread(std::bind(&StableClient::textToImage, &StableClient::GetInstance(), mdl.hash, outDir,
                                   canvasName, prompt, negative_prompt, samplerName, batch_size, steps, cfg, seed,
                                   width, height, std::ref(renderState)));
  m_Thread.detach();
}

void SDCommandsInterface::imageToImage(std::string &canvasName, std::string &imgPath, std::string &prompt,
                                       std::string &negative_prompt, std::string &samplerName, int batch_size,
                                       int steps, double cfg, double strength, int seed, int &renderState) {
  std::string outDir = CONFIG::OUTPUT_DIRECTORY.get();
  renderState = Q_RENDER_STATE::RENDERING;

  model mdl = StableManager::GetInstance().getModel();

  m_Thread = std::thread(std::bind(&StableClient::imageToImage, &StableClient::GetInstance(), mdl.hash, outDir, prompt,
                                   negative_prompt, canvasName, imgPath, samplerName, batch_size, 1, steps, cfg,
                                   strength, seed, std::ref(renderState)));
  m_Thread.detach();
}