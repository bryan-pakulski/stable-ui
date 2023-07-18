#pragma once

#include "Config/structs.h"
#include "Helpers/States.h"
#include "Indexer/Indexer.h"
#include "Rendering/RenderManager.h"

#include <memory>
#include <vector>

/*
  This class is responsible for owning the Indexing engine as well as the RenderManager
  The main functionality provided is a singleton stateful interface to the model server running on docker
*/

class StableManager {
public:
  std::vector<std::string> m_latestFiles;

public:
  static StableManager &GetInstance() {
    static StableManager s_stableManager;
    return s_stableManager;
  }

  // Prohibit external replication constructs
  StableManager(StableManager const &) = delete;
  // Prohibit external assignment operations
  void operator=(StableManager const &) = delete;

  std::shared_ptr<RenderManager> getRenderManager() { return m_renderManager; }

  // Search index for files that match our search term
  std::set<std::string> searchIndex(const std::string &searchTerm);

  // Main update loop
  void update() { m_renderManager->update(); }

  /*
  DOCKER SERVER INTERACTION
  */

  void launchSDModelServer();
  void releaseSDModel();
  void attachModel(ModelConfig model);
  const ModelConfig &getLoadedModel() { return m_model; }

  // TODO: Make these functions thread safe, basic mutex lock should suffice
  int getModelState() { return m_modelState; }
  void setModelState(int state) { m_modelState = state; }
  void setRenderState(int state) { m_renderState = state; }
  int getRenderState() { return m_renderState; }

  void textToImage(pipelineConfig &config);
  void imageToImage(std::string &imgPath, pipelineConfig &config);
  void outpaint(std::string &imgData, std::string &imgMask, pipelineConfig &config);

  // Retrieve last modified file/s that the indexer was able to locate after a manual run
  void getLatestFiles(bool collectLatestFiles = false) { m_latestFiles = m_indexer.forceUpdate(collectLatestFiles); }

private:
  std::shared_ptr<RenderManager> m_renderManager;

  int m_modelState = Q_MODEL_STATUS::NONE_LOADED;
  int m_renderState = Q_RENDER_STATE::UNRENDERED;
  int m_dockerState = Q_COMMAND_EXECUTION_STATE::NONE;

  ModelConfig m_model;
  Indexer m_indexer;

  std::thread m_Thread;

private:
  explicit StableManager();
  ~StableManager();

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};