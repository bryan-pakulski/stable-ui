#pragma once

#include "Indexer/Indexer.h"
#include "Rendering/RenderManager.h"

#include <memory>
#include <vector>

struct model {
  std::string name;
  std::string hash;
  std::string path;
};

class StableManager {
public:
  int m_modelLoaded = Q_EXECUTION_STATE::PENDING;
  model m_model;

  static StableManager &GetInstance() {
    static StableManager s_stableManager;
    return s_stableManager;
  }

  // Prohibit external replication constructs
  StableManager(StableManager const &) = delete;

  // Prohibit external assignment operations
  void operator=(StableManager const &) = delete;

  std::shared_ptr<RenderManager> getRenderManager();

  // Search index for files that match our search term
  std::set<std::string> searchIndex(const std::string &searchTerm);

  // Main update loop
  void update();

  /*
  MODEL SERVER INTERACTION
  */

  // Attach model to server
  void attachModel(YAML::Node model, std::string &hash, std::string &precision);
  // Return model state
  int getModelState();

private:
  std::shared_ptr<RenderManager> m_renderManager;
  Indexer m_indexer;

  explicit StableManager();
  ~StableManager();

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};