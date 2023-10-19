#pragma once

#include "Config/model_config.h"
#include "Config/structs.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include "Helpers/hash.h"

#include <imgui.h>
#include "ThirdParty/imgui/imfilebrowser.h"
#include <imgui_stdlib.h>
#include "yaml-cpp/emittermanip.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

class QDisplay_ImportModel : public QDisplay_Base {

public:
  QDisplay_ImportModel(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    fileDialog.SetTitle("Import Models");
    fileDialog.SetTypeFilters({".ckpt", ".safetensors"});

    *m_saving = false;
  }

  void openWindow() { fileDialog.Open(); };

  virtual void render() {
    // File browser management
    fileDialog.Display();
    if (fileDialog.HasSelected()) {
      *m_saving = true;
      std::thread t(saveModelConfiguration, fileDialog.GetSelected().string(), m_saving);
      t.detach();
      clear();
    }

    if (*m_saving) {
      ImGui::Begin("Saving Model");
      ImGui::Text("Copying model to docker image, please wait...");
      ImGui::End();
    }
  }

private:
  ImGui::FileBrowser fileDialog;
  bool shared_bool = false;
  std::shared_ptr<bool> m_saving = std::make_shared<bool>(shared_bool);

private:
  void clear() { fileDialog.ClearSelected(); }

  // Save model configuration to model config file
  static void saveModelConfiguration(std::string filepath, std::shared_ptr<bool> m_saving) {
    ModelConfig m_modelConfig;

    std::filesystem::path modelPath(filepath);

    m_modelConfig.name = modelPath.filename().string();
    m_modelConfig.path = "/models/" + modelPath.filename().string();
    m_modelConfig.hash = getFileHash(filepath.c_str());

    try {
      fs::copy_file(modelPath, "data/models/" + modelPath.filename().string());
      MODEL_CONFIG::saveModelConfig(m_modelConfig);
    } catch (fs::filesystem_error const &err) {
      ErrorHandler::GetInstance().setError("Model Configuration Error", err.what());
    }

    *m_saving = false;
  }
};