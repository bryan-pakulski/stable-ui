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
void StableManager::attachModel(YAML::Node model, std::string &hash, std::string &precision) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "StableManager::attachModel Attaching model: ", model["name"].as<std::string>(),
                             " to Stable Diffusion Docker Server");
  // Optional parameters
  std::string vae = "";
  if (model["vae"]) {
    vae = model["vae"].as<std::string>();
  }

  // Build model struct
  m_model.name = model["name"].as<std::string>();
  m_model.hash = hash;
  m_model.path = model["path"].as<std::string>();

  SDCommandsInterface::GetInstance().attachModelToServer(
      model["path"].as<std::string>(), model["config"].as<std::string>(), vae, precision, m_modelLoaded);
}

int StableManager::getModelState() { return m_modelLoaded; }
void StableManager::setModelState(int state) { m_modelLoaded = state; }
model StableManager::getModel() { return m_model; }

// Search our inverted index for a term
std::set<std::string> StableManager::searchIndex(const std::string &searchTerm) {
  std::set<std::string> results;

  std::set<meta_node> data = m_indexer.find(searchTerm);
  for (auto &node : data) {
    results.insert(node.m_filepath);
  }

  return results;
}