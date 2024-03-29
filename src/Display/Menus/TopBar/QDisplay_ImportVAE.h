#pragma once

#include <imgui.h>
#include "Display/ErrorHandler.h"
#include "ThirdParty/imgui/imfilebrowser.h"
#include "Display/QDisplay_Base.h"

#include <imgui_stdlib.h>

#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

class QDisplay_ImportVAE : public QDisplay_Base {
public:
  QDisplay_ImportVAE(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    fileDialog.SetTitle("Import Models");
    fileDialog.SetTypeFilters({".ckpt", ".safetensors", ".pth", ".bin", ".vae"});

    *m_saving = false;
  }

  void openWindow() { fileDialog.Open(); };

  virtual void render() {
    // File browser management
    fileDialog.Display();
    if (fileDialog.HasSelected()) {
      *m_saving = true;
      std::thread t(saveVAE, fileDialog.GetSelected().string(), m_saving);
      t.detach();
      clear();
    }

    if (*m_saving) {
      ImGui::Begin("Saving Model Configuration");
      ImGui::Text("Saving model configuration and copying to docker image, please wait...");
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
  static void saveVAE(std::string vae_path, std::shared_ptr<bool> m_saving) {
    std::filesystem::path modelPath(vae_path);

    // Copy model file
    try {
      fs::copy_file(modelPath, "data/models/vae/" + modelPath.filename().string());
    } catch (fs::filesystem_error const &err) {
      ErrorHandler::GetInstance().setError("VAE Configuration Error", err.what());
    }

    *m_saving = false;
  }
};