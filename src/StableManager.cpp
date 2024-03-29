#include "StableManager.h"

#include "Client/Commands.h"
#include "Client/StableClient.h"
#include "Config/config.h"
#include "Config/structs.h"
#include "Indexer/MetaData.h"
#include "Display/QDisplay.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"
#include <functional>

StableManager::StableManager() : m_indexer(CONFIG::CRAWLER_PATH.get()) {
  QLogger::GetInstance().Log(LOGLEVEL::TRACE, "StableManager::StableManager");

  // Intialise zmq server within docker to receive commands from client
  launchSDModelServer();

  m_renderManager = std::shared_ptr<RenderManager>(new RenderManager(*QDisplay::GetInstance().getWindow()));

  // TODO: make a command queue
}

StableManager::~StableManager() {}

void StableManager::launchSDModelServer() {
  QLogger::GetInstance().Log(LOGLEVEL::TRACE, "StableManager::launchSDModelServer");

  // Launch SD Server inside docker on startup
#ifdef _WIN32
  std::string commandStr = "data\\scripts\\start_sd_server.bat";
  system(commandStr.c_str());
#else
  system("./data/scripts/start_sd_server.sh");
#endif
}

// Send Command to attach a model on the docker server
void StableManager::attachModel(ModelConfig modelConfig) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::attachModel Attaching model: ", modelConfig.name,
                             " to Stable Diffusion Docker Server");
  m_model = modelConfig;
  setModelState(Q_MODEL_STATUS::LOADING);

  commands::loadModelToMemory cmd = commands::loadModelToMemory{modelConfig};

  m_Thread = std::thread(std::bind(&StableClient::loadModelToMemory, &StableClient::GetInstance(), cmd));
  m_Thread.detach();
}

void StableManager::releaseSDModel() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::releaseSDModel releasing model from server...");

  m_Thread = std::thread(std::bind(&StableClient::releaseModel, &StableClient::GetInstance()));
  m_Thread.detach();
}
// Text to Image, save and preview result
void StableManager::textToImage(pipelineConfig &config) {
  commands::textToImage cmd = commands::textToImage{m_model,
                                                    config.prompt,
                                                    config.width,
                                                    config.height,
                                                    config.negative_prompt,
                                                    m_renderManager->getActiveCanvas()->m_name,
                                                    config.sampler,
                                                    config.iterations,
                                                    config.steps,
                                                    config.cfg,
                                                    config.seed,
                                                    CONFIG::OUTPUT_DIRECTORY.get()};

  setRenderState(Q_RENDER_STATE::RENDERING);

  m_Thread = std::thread(std::bind(&StableClient::textToImage, &StableClient::GetInstance(), cmd));
  m_Thread.detach();
}

// Image to Image, save and preview
void StableManager::imageToImage(std::string &imgPath, pipelineConfig &config) {
  commands::imageToImage cmd = commands::imageToImage{m_model,
                                                      config.prompt,
                                                      config.negative_prompt,
                                                      m_renderManager->getActiveCanvas()->m_name,
                                                      imgPath,
                                                      config.sampler,
                                                      config.iterations,
                                                      config.steps,
                                                      config.cfg,
                                                      config.strength,
                                                      config.seed,
                                                      CONFIG::OUTPUT_DIRECTORY.get()};

  setRenderState(Q_RENDER_STATE::RENDERING);

  m_Thread = std::thread(std::bind(&StableClient::imageToImage, &StableClient::GetInstance(), cmd));
  m_Thread.detach();
}

// Outpaint, render result to canvas
void StableManager::outpaint(std::string &imgData, std::string &imgMask, pipelineConfig &config) {
  commands::outpainting cmd = commands::outpainting{m_model,
                                                    config.prompt,
                                                    config.negative_prompt,
                                                    m_renderManager->getActiveCanvas()->m_name,
                                                    config.width,
                                                    config.height,
                                                    imgData,
                                                    imgMask,
                                                    config.sampler,
                                                    config.iterations,
                                                    config.steps,
                                                    config.cfg,
                                                    config.strength,
                                                    config.seed,
                                                    CONFIG::OUTPUT_DIRECTORY.get()};

  setRenderState(Q_RENDER_STATE::RENDERING);

  m_Thread = std::thread(std::bind(&StableClient::outpainting, &StableClient::GetInstance(), cmd));
  m_Thread.detach();
}

// Search our inverted index for a term
std::set<std::string> StableManager::searchIndex(const std::string &searchTerm) {
  std::set<std::string> results;

  std::set<meta_node> data = m_indexer.find(searchTerm);
  for (auto &node : data) {
    results.insert(node.m_filepath);
  }

  return results;
}