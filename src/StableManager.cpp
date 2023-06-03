#include "StableManager.h"

#include "Client/SDCommandsInterface.h"
#include "Config/config.h"
#include "Indexer/MetaData.h"
#include "Display/QDisplay.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"

#include <memory>

// Initialise render manager
StableManager::StableManager() : m_indexer(CONFIG::CRAWLER_PATH.get()) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::StableManager StableManager initialized");

  // Intialise zmq server within docker to receive commands from client
  SDCommandsInterface::GetInstance().launchSDModelServer();
  m_renderManager = std::shared_ptr<RenderManager>(new RenderManager(*QDisplay::GetInstance().getWindow()));
}

// Destructor, destroy remaining instances
StableManager::~StableManager() {}

std::shared_ptr<RenderManager> StableManager::getRenderManager() { return m_renderManager; }

void StableManager::update() { m_renderManager->update(); }

// Send Command to attach a model on the docker server
void StableManager::attachModel(ModelConfig model) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::attachModel Attaching model: ", model.name,
                             " to Stable Diffusion Docker Server");
  m_model = model;

  SDCommandsInterface::GetInstance().attachModelToServer(model, m_modelLoaded);
}

int StableManager::getModelState() { return m_modelLoaded; }
void StableManager::setModelState(int state) { m_modelLoaded = state; }
ModelConfig StableManager::getLoadedModel() { return m_model; }

// Search our inverted index for a term
std::set<std::string> StableManager::searchIndex(const std::string &searchTerm) {
  std::set<std::string> results;

  std::set<meta_node> data = m_indexer.find(searchTerm);
  for (auto &node : data) {
    results.insert(node.m_filepath);
  }

  return results;
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