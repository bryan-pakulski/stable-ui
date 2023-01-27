#pragma once

#include <imgui.h>
#include "../../../Helpers/SHA1.h"
#include "../../../Display/ErrorHandler.h"
#include "../../../ThirdParty/imgui/imfilebrowser.h"
#include "../../QDisplay_Base.h"

#include <imgui_stdlib.h>

#include "yaml-cpp/emittermanip.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

class QDisplay_ImportModel : public QDisplay_Base {

private:
  ImGui::FileBrowser fileDialog;
  bool shared_bool = false;
  std::shared_ptr<bool> m_saving = std::make_shared<bool>(shared_bool);

  std::string m_selected_model_config = "";
  std::string m_selected_module = "";
  std::string m_selected_vae = "";
  std::string m_triggerWords = "";
  std::vector<listItem> m_ModulesList;
  std::vector<listItem> m_ModelConfigList;
  std::vector<listItem> m_VAEList;

  void clear() {
    fileDialog.ClearSelected();
    m_ModulesList.clear();
    m_ModelConfigList.clear();
    m_VAEList.clear();

    m_triggerWords = "";
    m_selected_model_config = "";
    m_selected_module = "";
    m_selected_vae = "";
  }

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
    try {
      static YAML::Node configFile = YAML::LoadFile(CONFIG::MODULES_CONFIGURATION_FILE.get());
      YAML::Node modules = configFile["modules"];
      for (YAML::const_iterator it = modules.begin(); it != modules.end(); ++it) {
        listItem i{.m_name = it->first.as<std::string>()};
        m_ModulesList.push_back(i);
      }
    } catch (YAML::Exception) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR,
                                 "Failed to parse yaml file: ", CONFIG::MODULES_CONFIGURATION_FILE.get());
      return;
    }

    // Load VAE List
    try {
      for (const auto &entry : fs::directory_iterator(CONFIG::VAE_FOLDER_PATH.get())) {
        listItem i{.m_name = entry.path().filename().string()};
        m_VAEList.push_back(i);
      }
    } catch (fs::filesystem_error) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::VAE_FOLDER_PATH, "VAE_FOLDER_PATH");
    }
  }

  // Save model configuration to model config file
  static void saveModelConfiguration(std::string filepath, std::string module_name, std::string model_config_file,
                                     std::shared_ptr<bool> m_saving, std::string trigger_words, std::string vae) {
    std::filesystem::path modelPath(filepath);

    // Build yaml node to attach to model configuration file
    YAML::Node model_node;
    std::string hash = getFileHash(filepath.c_str());
    model_node["working_dir"] = "/modules/" + module_name;
    model_node["config"] = "/models/configs/" + model_config_file;
    model_node["name"] = modelPath.filename().string();
    model_node["path"] = "/models/" + modelPath.filename().string();
    model_node["trigger_prompt"] = trigger_words;
    if (vae != "") {
      model_node["vae"] = "/models/vae/" + vae;
    }

    // Retrieve root node and dump back to file
    YAML::Node node, _baseNode = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
    _baseNode["models"][hash] = model_node;
    std::ofstream fout(CONFIG::MODELS_CONFIGURATION_FILE.get());
    fout << _baseNode;

    // Copy model file
    try {
      fs::copy_file(modelPath, "data/models/" + modelPath.filename().string());
    } catch (fs::filesystem_error const &ex) {
      ErrorHandler::GetInstance().setError(ex.what());
    }

    *m_saving = false;
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

    if (ImGui::BeginCombo("Module Runner", m_selected_module.c_str(), ImGuiComboFlags_NoArrowButton)) {
      for (auto &item : m_ModulesList) {
        if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
          m_selected_module = item.m_name;
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

    if (ImGui::Button("Cancel")) {
      clear();
    } else if (m_selected_module != "" && m_selected_model_config != "" && !*m_saving) {
      ImGui::SameLine();
      if (ImGui::Button("Save")) {
        // Save on a seperate thread so we don't block application, use the m_saving shared pointer as a basic lock to
        // track completion
        *m_saving = true;
        std::thread t(saveModelConfiguration, fileDialog.GetSelected().string(), m_selected_module,
                      m_selected_model_config, m_saving, std::string(m_triggerWords), m_selected_vae);
        t.detach();
        clear();
      }
    }

    ImGui::End();
  }

public:
  void openWindow() {
    fileDialog.Open();
    reloadFiles();
  };

  QDisplay_ImportModel(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    fileDialog.SetTitle("Import Models");
    fileDialog.SetTypeFilters({".ckpt", ".safetensors"});

    *m_saving = false;
  }

  virtual void render() {
    // File browser management
    fileDialog.Display();
    if (fileDialog.HasSelected()) {
      configureModelPopup();
    }

    if (*m_saving) {
      ImGui::Begin("Saving Model Configuration");
      ImGui::Text("Saving model configuration and copying to docker image, please wait...");
      ImGui::End();
    }
  }
};