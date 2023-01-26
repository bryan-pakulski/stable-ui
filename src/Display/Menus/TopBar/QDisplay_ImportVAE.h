#pragma once

#include <imgui.h>
#include "../../../Display/ErrorHandler.h"
#include "../../../ThirdParty/imgui/imfilebrowser.h"
#include "../../QDisplay_Base.h"

#include <imgui_stdlib.h>

#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

class QDisplay_ImportVAE : public QDisplay_Base {

private:
  ImGui::FileBrowser fileDialog;
  bool shared_bool = false;
  std::shared_ptr<bool> m_saving = std::make_shared<bool>(shared_bool);

  void clear() { fileDialog.ClearSelected(); }

  // Save model configuration to model config file
  static void saveVAEConfiguration(std::string vae_path, std::shared_ptr<bool> m_saving) {
    std::filesystem::path modelPath(vae_path);

    // Copy model file
    try {
      fs::copy_file(modelPath, "data/models/vae/" + modelPath.filename().string());
    } catch (fs::filesystem_error const &ex) {
      ErrorHandler::GetInstance().setError(ex.what());
    }

    *m_saving = false;
  }

public:
  void openWindow() { fileDialog.Open(); };

  QDisplay_ImportVAE(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    fileDialog.SetTitle("Import Models");
    fileDialog.SetTypeFilters({".ckpt", ".safetensor"});

    *m_saving = false;
  }

  virtual void render() {
    // File browser management
    fileDialog.Display();
    if (fileDialog.HasSelected()) {
      *m_saving = true;
      std::thread t(saveVAEConfiguration, fileDialog.GetSelected().string(), m_saving);
      t.detach();
      clear();
    }

    if (*m_saving) {
      ImGui::Begin("Saving Model Configuration");
      ImGui::Text("Saving model configuration and copying to docker image, please wait...");
      ImGui::End();
    }
  }
};