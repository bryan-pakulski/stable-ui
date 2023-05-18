#pragma once

#include <imgui.h>
#include <filesystem>

#include "../../../Display/ErrorHandler.h"
#include "../../QDisplay_Base.h"
#include <imgui_stdlib.h>

#include "yaml-cpp/emittermanip.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

namespace fs = std::filesystem;

class QDisplay_ConfigureModel : public QDisplay_Base {

private:
  std::string m_selected_model = "";
  std::string m_selected_hash = "";
  std::string m_selected_model_config = "";

  std::vector<listItem> m_ModelList;
  std::vector<listItem> m_ModelConfigList;
  std::vector<listItem> m_VAEList;
  std::string m_triggerWords = "";
  std::string m_selected_vae = "";

  void clear() {
    m_ModelList.clear();
    m_ModelConfigList.clear();
    m_VAEList.clear();

    m_selected_model = "";
    m_selected_hash = "";
    m_triggerWords = "";
    m_selected_vae = "";
  }

  void reloadFiles() {
    // Load model config files
    try {
      for (const auto &entry : fs::directory_iterator(CONFIG::MODEL_CONFIGURATIONS_DIRECTORY.get())) {
        listItem i{.m_name = entry.path().filename().string()};
        m_ModelConfigList.push_back(i);
      }
    } catch (const fs::filesystem_error &err) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::MODEL_CONFIGURATIONS_DIRECTORY,
                                                 "MODEL_CONFIGURATIONS_DIRECTORY");
      QLogger::GetInstance().Log(LOGLEVEL::ERR, err.what());
    }

    // Load models list
    try {
      static YAML::Node configFile = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
      YAML::Node models = configFile["models"];
      for (YAML::const_iterator it = models.begin(); it != models.end(); ++it) {
        if (it->second["name"].as<std::string>() != "default") {
          listItem i{.m_name = it->second["name"].as<std::string>(), .m_key = it->first.as<std::string>()};
          m_ModelList.push_back(i);
        }
      }
    } catch (const YAML::Exception &) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "QDisplay_ConfigureModel::reloadFiles Failed to parse yaml file: ",
                                 CONFIG::MODELS_CONFIGURATION_FILE.get());
      return;
    }

    // Load VAE List
    try {
      for (const auto &entry : fs::directory_iterator(CONFIG::VAE_FOLDER_PATH.get())) {
        listItem i{.m_name = entry.path().filename().string()};
        m_VAEList.push_back(i);
      }
    } catch (const fs::filesystem_error &err) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::VAE_FOLDER_PATH, "VAE_FOLDER_PATH");
      QLogger::GetInstance().Log(LOGLEVEL::ERR, err.what());
    }
  }

  // Save model configuration to model config file
  static void saveModelConfiguration(std::string model_name, std::string model_config_file, std::string hash,
                                     std::string trigger_words, std::string vae) {
    // Build yaml node to attach to model configuration file
    YAML::Node model_node;
    model_node["config"] = "/models/configs/" + model_config_file;
    model_node["name"] = model_name;
    model_node["path"] = "/models/" + model_name;
    model_node["trigger_prompt"] = trigger_words;
    if (vae != "") {
      model_node["vae"] = "/models/vae/" + vae;
    }

    // Retrieve root node and dump back to file
    YAML::Node node, _baseNode = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
    _baseNode["models"][hash] = model_node;
    std::ofstream fout(CONFIG::MODELS_CONFIGURATION_FILE.get());
    fout << _baseNode;
  }

  void configureModelPopup() {
    if (m_isOpen) {
      ImGui::Begin("Configure Models");

      ImGui::Text("Select a model to configure");
      if (ImGui::BeginCombo("model", m_selected_model.c_str(), ImGuiComboFlags_NoArrowButton)) {
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

      if (m_selected_model != "") {
        // Model config file
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

        if (ImGui::BeginCombo("Custom VAE", m_selected_vae.c_str(), ImGuiComboFlags_NoArrowButton)) {
          for (auto &item : m_VAEList) {
            if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
              m_selected_vae = item.m_name;
            }
            if (item.m_isSelected) {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }

        ImGui::InputText("Trigger Words: ", &m_triggerWords);

        // Save available once we fill in information
        if (m_selected_model_config != "") {
          if (ImGui::Button("Save")) {
            m_isOpen = false;
            saveModelConfiguration(m_selected_model, m_selected_model_config, m_selected_hash,
                                   std::string(m_triggerWords), m_selected_vae);
            clear();
          }
          ImGui::SameLine();
        }
      }

      if (ImGui::Button("Cancel")) {
        m_isOpen = false;
        clear();
      }
      ImGui::End();
    }
  }

public:
  void openWindow() {
    m_isOpen = true;
    reloadFiles();
  };

  QDisplay_ConfigureModel(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  virtual void render() {
    // Render configure modules popup
    configureModelPopup();
  }
};