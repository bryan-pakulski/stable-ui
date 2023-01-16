#pragma once

#include <imgui.h>
#include <filesystem>
#include "../../../Display/ErrorHandler.h"
#include "../../../ThirdParty/imgui/imfilebrowser.h"
#include "../../QDisplay_Base.h"

namespace fs = std::filesystem;

class QDisplay_ConfigureModel : public QDisplay_Base {

private:
  ImGui::FileBrowser fileDialog;

  std::string m_selected_model_config = "";
  std::vector<listItem> m_ModulesList;
  std::vector<listItem> m_ModelConfigList;

  void reloadFiles() {
    // Load model config files
    try {
      for (const auto &entry : fs::directory_iterator(CONFIG::MODEL_CONFIGURATIONS_DIRECTORY.get())) {
        listItem i{.m_name = entry.path().filename().string()};
        m_ModelConfigList.push_back(i);
      }
    } catch (fs::filesystem_error) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::MODEL_CONFIGURATIONS_DIRECTORY,
                                                 "MODEL_CONFIGURATIONS_DIRECTORY");
    }

    // Load modules list from yaml
    // TODO: open file using yaml-cpp, read each module name and append to listItem
  }

  void configureModelPopup() {
    ImGui::Begin("Configure Model");

    ImGui::Text("%s", fileDialog.GetSelected().c_str());

    if (ImGui::BeginCombo("model config", m_selected_model_config.c_str(), ImGuiComboFlags_NoArrowButton)) {
      for (auto &item : m_ModelConfigList) {
        if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
          m_selected_model_config = item.m_name;
        }
        if (item.m_isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    if (ImGui::Button("Cancel")) {
      fileDialog.ClearSelected();
    } else if (ImGui::Button("Save")) {
      fileDialog.ClearSelected();
    }

    ImGui::End();
  }

public:
  void openWindow() {
    fileDialog.Open();
    reloadFiles();
  };

  QDisplay_ConfigureModel(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    fileDialog.SetTitle("Import Models");
    fileDialog.SetTypeFilters({".ckpt", ".safetensor"});
  }

  virtual void render() {
    // File browser management
    fileDialog.Display();
    if (fileDialog.HasSelected()) {
      configureModelPopup();
    }
  }
};