#include "StableManager.h"

#include "Client/Commands.h"
#include "Client/StableClient.h"
#include "Config/config.h"
#include "Indexer/MetaData.h"
#include "Display/QDisplay.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"
#include <functional>

StableManager::StableManager() : m_indexer(CONFIG::CRAWLER_PATH.get()) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::StableManager initialized");

  // Intialise zmq server within docker to receive commands from client
  launchSDModelServer();

  m_renderManager = std::shared_ptr<RenderManager>(new RenderManager(*QDisplay::GetInstance().getWindow()));

  // TODO: make a command queue
}

StableManager::~StableManager() {}

void StableManager::launchSDModelServer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::launchSDModelServer starting up SD Model Server...");

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
  m_modelState = Q_MODEL_STATUS::LOADING;

  commands::loadModelToMemory cmd = commands::loadModelToMemory{modelConfig};

  m_Thread = std::thread(
      std::bind(&StableClient::loadModelToMemory, &StableClient::GetInstance(), cmd, std::ref(m_modelState)));
  m_Thread.detach();
}

void StableManager::releaseSDModel() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::releaseSDModel releasing model from server...");

  m_Thread = std::thread(std::bind(&StableClient::releaseModel, &StableClient::GetInstance(), m_modelState));
  m_Thread.detach();
}
// Text to Image, render result to canvas
void StableManager::textToImage(std::string prompt, std::string negativePrompt, std::string &samplerName, int nIter,
                                int steps, double cfg, int seed, int width, int height, int &renderState) {
  commands::textToImage cmd = commands::textToImage{
      m_model,     prompt, width, height, negativePrompt, m_renderManager->getActiveCanvas()->m_name,
      samplerName, nIter,  steps, cfg,    seed,           CONFIG::OUTPUT_DIRECTORY.get()};

  renderState = Q_RENDER_STATE::RENDERING;

  m_Thread =
      std::thread(std::bind(&StableClient::textToImage, &StableClient::GetInstance(), cmd, std::ref(renderState)));
  m_Thread.detach();
}

// Image to Image, render result to canvas
void StableManager::imageToImage(std::string &imgPath, std::string &prompt, std::string &negativePrompt,
                                 std::string &samplerName, int nIter, int steps, double cfg, double strength, int seed,
                                 int &renderState) {
  commands::imageToImage cmd =
      commands::imageToImage{m_model, prompt,      negativePrompt, m_renderManager->getActiveCanvas()->m_name,
                             imgPath, samplerName, nIter,          steps,
                             cfg,     strength,    seed,           CONFIG::OUTPUT_DIRECTORY.get()};

  renderState = Q_RENDER_STATE::RENDERING;

  m_Thread =
      std::thread(std::bind(&StableClient::imageToImage, &StableClient::GetInstance(), cmd, std::ref(renderState)));
  m_Thread.detach();
}

std::string StableManager::getLatestFile(const std::string &path) {
  std::string outfile = "";
  std::filesystem::file_time_type lastWrite;

  try {
    for (const auto &entry : fs::directory_iterator(path)) {
      if (entry.is_regular_file()) {
        if (outfile == "") {
          lastWrite = entry.last_write_time();
          outfile = entry.path().string();
        } else if (lastWrite < entry.last_write_time()) {
          lastWrite = entry.last_write_time();
          outfile = entry.path().string();
        }
      }
    }
  } catch (const fs::filesystem_error &err) {
    ErrorHandler::GetInstance().setError("Failed to parse path for new files");
    QLogger::GetInstance().Log(LOGLEVEL::ERR, err.what());
  }

  return outfile;
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