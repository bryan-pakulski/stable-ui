#pragma once

#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <filesystem>

#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"
#include "Config/config.h"
#include "Config/structs.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "Display/QDisplay_Base.h"
#include "StableManager.h"

class QDisplay_Text2Image : public QDisplay_Base {

public:
  // Initialise render manager references
  QDisplay_Text2Image(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_config = rm->getPipeline(PIPELINE::TXT);
    m_windowName = "txt2img";

    reloadFiles();
  }

  virtual void render() {

    ImGui::Begin(m_windowName.c_str());

    if (StableManager::GetInstance().getModelState() != Q_MODEL_STATUS::LOADED) {
      ImGui::Text("Load Model to memory first!");
    } else {
      // Generate option only available whilst a image isn't pending
      if ((StableManager::GetInstance().getRenderState() != Q_RENDER_STATE::RENDERING)) {
        static const ImVec4 currentColor{0, 0.5f, 0, 1.0f};

        ImGui::PushStyleColor(ImGuiCol_Button, currentColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, currentColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentColor);

        if (ImGui::Button("Generate", ImVec2(150, 40))) {
          renderImage();
        }
        ImGui::PopStyleColor(3);
      } else if (StableManager::GetInstance().getRenderState() == Q_RENDER_STATE::RENDERING) {
        static const ImVec4 currentColor{0.5f, 0, 0, 1.0f};
        ImGui::PushStyleColor(ImGuiCol_Button, currentColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, currentColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentColor);

        if (ImGui::Button("Cancel", ImVec2(150, 40))) {
          // TODO: cut render short?
        }
        ImGui::PopStyleColor(3);
      }

      promptHelper();
      promptConfig();
    }

    ImGui::End();
  }

private:
  std::shared_ptr<pipelineConfig> m_config;
  bool m_randomSeed = true;
  std::string m_refinerModel;
  std::vector<listItem> m_refinerModelList;

  std::vector<listItem> m_samplerList = {
      {.m_name = "ddim"},      {.m_name = "ddiminverse"}, {.m_name = "ddpm"},           {.m_name = "deis"},
      {.m_name = "dpmsmulti"}, {.m_name = "dpmssingle"},  {.m_name = "eulerancestral"}, {.m_name = "euler"},
      {.m_name = "heun"},      {.m_name = "kdpm2"},       {.m_name = "kdpm2ancestral"}, {.m_name = "lms"},
      {.m_name = "pndm"},      {.m_name = "unipc"}};

  std::unique_ptr<GLImage> m_image = 0;

private:
  void renderImage() {
    m_image.reset();
    m_image = std::unique_ptr<GLImage>(
        new GLImage(CONFIG::IMAGE_SIZE_X_LIMIT.get(), CONFIG::IMAGE_SIZE_Y_LIMIT.get(), "txt2img"));

    if (m_randomSeed) {
      m_config->seed = rand() % INT_MAX + 1;
    }
    StableManager::GetInstance().textToImage(*m_config);
  }

  // Prompt
  void promptHelper() {
    if (ImGui::CollapsingHeader("Prompts", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("Prompt");
      ImGui::InputTextMultiline("##prompt", &m_config->prompt, ImVec2(ImGui::GetWindowContentRegionWidth(), 240));

      ImGui::Text("Negative Prompt");
      ImGui::InputTextMultiline("##negative prompt", &m_config->negative_prompt,
                                ImVec2(ImGui::GetWindowContentRegionWidth(), 240));
    }
  }

  void reloadFiles() {
    // Load models list
    try {
      static YAML::Node configFile = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
      YAML::Node models = configFile["models"];
      for (YAML::const_iterator it = models.begin(); it != models.end(); ++it) {
        if (it->second["name"].as<std::string>() != "default") {
          listItem i{.m_name = it->second["name"].as<std::string>(), .m_key = it->first.as<std::string>()};
          m_refinerModelList.push_back(i);
        }
      }
    } catch (const YAML::Exception &) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "QDisplay_LoadModel::reloadFiles Failed to parse yaml file: ",
                                 CONFIG::MODELS_CONFIGURATION_FILE.get());
      return;
    }
  }

  // Refiner configuration
  void selectRefiner() {
    if (ImGui::BeginCombo("model config", m_refinerModel.c_str(), ImGuiComboFlags_NoArrowButton)) {
      for (auto &item : m_refinerModelList) {
        if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
          m_refinerModel = item.m_name;
          m_config->refiner_model_hash = item.m_key;
        }
        if (item.m_isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
  }

  void promptConfig() {
    if (ImGui::CollapsingHeader("Gen Config", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Width control
      ImGui::SliderInt("width", &m_config->width, 1, CONFIG::IMAGE_SIZE_X_LIMIT.get());
      if (ImGui::BeginPopupContextItem("width")) {
        ImGui::InputInt("value", &m_config->width);
        if (ImGui::MenuItem("Reset to default: 512"))
          m_config->width = 512;
        ImGui::EndPopup();
      }

      // Height control
      ImGui::SliderInt("height", &m_config->height, 1, CONFIG::IMAGE_SIZE_Y_LIMIT.get());
      if (ImGui::BeginPopupContextItem("height")) {
        ImGui::InputInt("value", &m_config->height);
        if (ImGui::MenuItem("Reset to default: 512"))
          m_config->height = 512;
        ImGui::EndPopup();
      }

      if (ImGui::BeginCombo("Sampler", m_config->sampler.c_str(), ImGuiComboFlags_NoArrowButton)) {
        for (auto &item : m_samplerList) {
          if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
            m_config->sampler = item.m_name;
          }
          if (item.m_isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      ImGui::InputInt("steps", &m_config->steps);
      if (!m_randomSeed) {
        ImGui::InputInt("seed", &m_config->seed);
      }
      ImGui::Checkbox("Randomize Seed", &m_randomSeed);
      ImGui::InputDouble("cfg scale", &m_config->cfg, 0.1);
      ImGui::InputInt("Iterations", &m_config->iterations);
      ImGui::Checkbox("Use Refiner", &m_config->use_refiner);

      if (m_config->use_refiner) {
        selectRefiner();
      }
    }
  }
};