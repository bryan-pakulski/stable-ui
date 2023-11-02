#pragma once

#include <imgui.h>
#include <filesystem>

#include "Config/model_config.h"
#include "Config/structs.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include <imgui_stdlib.h>

#include "yaml-cpp/emittermanip.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

namespace fs = std::filesystem;

class QDisplay_ConfigureModel : public QDisplay_Base {
public:
  QDisplay_ConfigureModel(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  void openWindow() {
    m_isOpen = true;
    reloadFiles();
  };

  virtual void render() {
    // Render configure modules popup
    configureModelPopup();
  }

private:
  ModelConfig m_modelConfig;
  std::vector<listItem> m_ModelList;
  std::vector<listItem> m_ModelConfigList;
  std::vector<listItem> m_VAEList;
  std::vector<listItem> m_VAEConfigList;
  std::vector<listItem> m_SchedulerList = {{.m_name = "pndm"},  {.m_name = "lms"},       {.m_name = "heun"},
                                           {.m_name = "euler"}, {.m_name = "ancestral"}, {.m_name = "dpm"},
                                           {.m_name = "ddim"}};

private:
  void clear() {
    m_modelConfig = ModelConfig{};
    m_ModelList.clear();
    m_ModelConfigList.clear();
    m_VAEList.clear();
    m_VAEConfigList.clear();
  }

  void reloadFiles() {
    // Load model config files
    try {
      m_ModelConfigList.push_back(listItem{.m_name = "CLEAR"});
      for (const auto &entry : fs::directory_iterator(CONFIG::MODEL_CONFIGURATIONS_DIRECTORY.get())) {
        listItem i{.m_name = entry.path().filename().string()};
        m_ModelConfigList.push_back(i);
      }
    } catch (const fs::filesystem_error &err) {
      ErrorHandler::GetInstance().setError("Configuration Error", "MODEL_CONFIGURATIONS_DIRECTORY config not set!");
    }

    // Load models list
    try {
      static YAML::Node configFile = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
      YAML::Node models = configFile["models"];
      for (YAML::const_iterator it = models.begin(); it != models.end(); ++it) {
        listItem i{.m_name = it->second["name"].as<std::string>(), .m_key = it->first.as<std::string>()};
        m_ModelList.push_back(i);
      }
    } catch (const YAML::Exception &err) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "QDisplay_ConfigureModel::reloadFiles Failed to parse yaml file: ",
                                 CONFIG::MODELS_CONFIGURATION_FILE.get(), err.what());
      return;
    }

    // Load VAE List
    try {
      m_VAEList.push_back(listItem{.m_name = "CLEAR"});
      for (const auto &entry : fs::directory_iterator(CONFIG::VAE_FOLDER_PATH.get())) {
        if (entry.path().filename().string() != ".placeholder") {
          listItem i{.m_name = entry.path().filename().string()};
          m_VAEList.push_back(i);
        }
      }
    } catch (const fs::filesystem_error &err) {
      ErrorHandler::GetInstance().setError("Configuration Error", "VAE_FOLDER_PATH config not set!");
    }

    // Load VAE Config List
    try {
      m_VAEConfigList.push_back(listItem{.m_name = "CLEAR"});
      for (const auto &entry : fs::directory_iterator(CONFIG::VAE_CONFIG_FOLDER_PATH.get())) {
        if (entry.path().filename().string() != ".placeholder") {
          listItem i{.m_name = entry.path().filename().string()};
          m_VAEConfigList.push_back(i);
        }
      }
    } catch (const fs::filesystem_error &err) {
      ErrorHandler::GetInstance().setError("Configuration Error", "VAE_CONFIG_FOLDER_PATH config not set!");
    }
  }

  void configureModelPopup() {
    if (m_isOpen) {
      ImGui::Begin("Configure Models");

      ImGui::Text("Select a model to configure");
      if (ImGui::BeginCombo("model", m_modelConfig.name.c_str(), ImGuiComboFlags_NoArrowButton)) {
        for (auto &item : m_ModelList) {
          if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
            m_modelConfig = MODEL_CONFIG::loadModelConfig(item.m_key);
          }
          if (item.m_isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      if (m_modelConfig.name != "") {

        // Model config file
        if (ImGui::BeginCombo("Model Configuration File", m_modelConfig.config.c_str(),
                              ImGuiComboFlags_NoArrowButton)) {
          for (auto &item : m_ModelConfigList) {
            if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
              if (item.m_name == "CLEAR") {
                m_modelConfig.config = "";
              } else {
                m_modelConfig.config = "/models/configs/models/" + item.m_name;
              }
            }
            if (item.m_isSelected) {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }

        // VAE path
        if (ImGui::BeginCombo("Custom VAE", m_modelConfig.vae.c_str(), ImGuiComboFlags_NoArrowButton)) {
          for (auto &item : m_VAEList) {
            if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
              if (item.m_name == "CLEAR") {
                m_modelConfig.vae = "";
              } else {
                m_modelConfig.vae = "/models/vae/" + item.m_name;
              }
            }
            if (item.m_isSelected) {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }

        // VAE Config
        if (m_modelConfig.vae != "") {
          if (ImGui::BeginCombo("VAE Config", m_modelConfig.vae_config.c_str(), ImGuiComboFlags_NoArrowButton)) {
            for (auto &item : m_VAEConfigList) {
              if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
                if (item.m_name == "CLEAR") {
                  m_modelConfig.vae_config = "";
                } else {
                  m_modelConfig.vae_config = "/models/configs/vae/" + item.m_name;
                }
              }
              if (item.m_isSelected) {
                ImGui::SetItemDefaultFocus();
              }
            }
            ImGui::EndCombo();
          }

          ImGui::Checkbox("Convert VAE to Diffusers", &m_modelConfig.convert_vae);
        }

        // Scheduler
        if (ImGui::BeginCombo("Scheduler", m_modelConfig.scheduler.c_str(), ImGuiComboFlags_NoArrowButton)) {
          for (auto &item : m_SchedulerList) {
            if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
              m_modelConfig.scheduler = item.m_name;
            }
            if (item.m_isSelected) {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }

        ImGui::Separator();

        // Trigger words
        ImGui::InputText("Trigger Words: ", &m_modelConfig.trigger_prompt);

        ImGui::Separator();
        ImGui::Text("Optimisations");
        ImGui::Separator();

        // TODO: mouse over popups with context information
        ImGui::Checkbox("Enable TF32", &m_modelConfig.enable_tf32);
        ImGui::Checkbox("Torch16 weights", &m_modelConfig.enable_t16);
        ImGui::Checkbox("Enable VAE tiling", &m_modelConfig.enable_vaeTiling);
        ImGui::Checkbox("Enable VAE slicing", &m_modelConfig.enable_vaeSlicing);
        ImGui::Checkbox("Enable sequential CPU offload", &m_modelConfig.enable_seqCPUOffload);
        ImGui::Checkbox("Enable CPU Model offload", &m_modelConfig.enable_cpu_offload);

        // Save available once we fill in mandatory information
        if (m_modelConfig.config != "") {
          if (ImGui::Button("Save")) {
            m_isOpen = false;
            MODEL_CONFIG::saveModelConfig(m_modelConfig);
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
};