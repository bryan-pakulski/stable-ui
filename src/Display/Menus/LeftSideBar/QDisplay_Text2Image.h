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
  }

  virtual void render() {

    // Generate option only available whilst a image isn't pending
    if ((m_renderManager->getImgPipelineStatus() != Q_RENDER_STATE::RENDERING)) {
      static const ImVec4 currentColor{0, 0.5f, 0, 1.0f};

      ImGui::PushStyleColor(ImGuiCol_Button, currentColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, currentColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentColor);

      if (ImGui::Button("Generate", ImVec2(150, 40))) {
        renderImage();
      }
      ImGui::PopStyleColor(3);
    } else if (m_renderManager->getImgPipelineStatus() == Q_RENDER_STATE::RENDERING) {
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
    imageWindow();
  }

private:
  std::shared_ptr<pipelineConfig> m_config;
  bool m_randomSeed = true;

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
    StableManager::GetInstance().textToImage(*m_config, m_renderManager->getTxtPipelineStatus());
  }

  void imageWindow() {
    if (ImGui::CollapsingHeader("Preview")) {
      if (m_image) {
        // Once image is marked as rendered display on screen
        if (m_renderManager->getImgPipelineStatus() == Q_RENDER_STATE::RENDERED) {
          ImGui::Text("image width: %d image height:%d", m_image->m_width, m_image->m_height);
          if (ImGui::Button("Send to Canvas")) {
            // Send image to be rendered on canvas at selection coordinates
            m_renderManager->sendImageToCanvas(*m_image);
          }

          // Retrieve texture file
          if (!m_image->textured) {
            m_image->loadFromImage(StableManager::GetInstance().getLatestFile(
                CONFIG::OUTPUT_DIRECTORY.get() + "/" + m_renderManager->getActiveCanvas()->m_name));
            m_image->textured = true;
          }

          ImGui::Image((void *)(intptr_t)m_image->m_texture, ImVec2(m_image->m_width * 0.3, m_image->m_height * 0.3));
        }
      }
    }
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
    }
  }
};