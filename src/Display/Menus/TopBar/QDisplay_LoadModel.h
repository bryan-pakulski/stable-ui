// Load model into sd_model_server#pragma once

#include <imgui.h>
#include "../../../Display/ErrorHandler.h"
#include "../../QDisplay_Base.h"

#include <imgui_stdlib.h>

#include <filesystem>

class QDisplay_LoadModel : public QDisplay_Base {

private:
  std::vector<listItem> m_ModelList;

  std::string m_selected_model = "";
  std::string m_selected_hash = "";
  bool visible = false;

  void clear() {
    m_ModelList.clear();
    m_selected_model = "";
    m_selected_hash = "";
  }

  void reloadFiles() {
    // Load models list
    try {
      static YAML::Node configFile = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
      YAML::Node models = configFile["models"];
      for (YAML::const_iterator it = models.begin(); it != models.end(); ++it) {
        listItem i{.m_name = it->second["name"].as<std::string>(), .m_key = it->first.as<std::string>()};
        m_ModelList.push_back(i);
      }
    } catch (YAML::Exception) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "Failed to parse yaml file: ", CONFIG::MODELS_CONFIGURATION_FILE.get());
      return;
    }
  }

  // Attempt to load model
  void loadModel() {
    // Retrieve root node and dump back to file
    YAML::Node node, _baseNode = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
    YAML::Node model = _baseNode["models"][m_selected_hash];

    m_stableManager->attachModel(model, m_selected_hash);
    visible = false;
  }

public:
  QDisplay_LoadModel(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  void openWindow() {
    clear();
    reloadFiles();
    visible = true;
  }

  virtual void render() {
    if (visible) {
      ImGui::Begin("Load Model");
      if (ImGui::BeginCombo("model config", m_selected_model.c_str(), ImGuiComboFlags_NoArrowButton)) {
        for (auto &item : m_ModelList) {
          if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
            m_selected_model = item.m_name;
            m_selected_hash = item.m_key;
          }
          if (item.m_isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      if (m_selected_hash != "default" && m_selected_hash != "") {
        if (ImGui::Button("Load Model")) {
          loadModel();
        }
        ImGui::SameLine();
      }

      if (ImGui::Button("Cancel")) {
        visible = false;
      }

      ImGui::End();
    }
  }
};