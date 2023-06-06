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
  int getModelState() { return m_modelState; }
  void setModelState(int state) { m_modelState = state; }
  const ModelConfig &getLoadedModel() { return m_model; }

  void setRenderState(int state);
  int getRenderState() { return m_renderState; }
  void textToImage(std::string prompt, std::string negative_prompt, std::string &samplerName, int nIter, int steps,
                   double cfg, int seed, int width, int height, int &renderState);

  void imageToImage(std::string &imgPath, std::string &prompt, std::string &negativePrompt, std::string &samplerName,
                    int nIter, int steps, double cfg, double strength, int seed, int &renderState);

  // Retrieve last modified file from path
  std::string getLatestFile(const std::string &path);

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