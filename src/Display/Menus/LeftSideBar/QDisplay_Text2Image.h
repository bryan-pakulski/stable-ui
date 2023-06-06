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
#include "Rendering/objects/GLImage/GLImage.h"
#include "Display/QDisplay_Base.h"
#include "StableManager.h"

class QDisplay_Text2Image : public QDisplay_Base {

public:
  // Initialise render manager references
  QDisplay_Text2Image(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  virtual void render() {

    // Generate option only available whilst a image isn't pending
    if ((m_image && m_image->renderState != Q_RENDER_STATE::RENDERING) || !m_image) {
      static const ImVec4 currentColor{0, 0.5f, 0, 1.0f};

      ImGui::PushStyleColor(ImGuiCol_Button, currentColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, currentColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentColor);

      if (ImGui::Button("Generate", ImVec2(150, 40))) {
        renderImage();
      }
      ImGui::PopStyleColor(3);
    } else if (m_image && m_image->renderState == Q_RENDER_STATE::RENDERING) {
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
  // Window variables & flags
  std::string m_prompt;
  std::string m_negativePrompt;
  std::string m_selectedSampler = "pndm";
  std::vector<listItem> m_samplerList = {
      {.m_name = "ddim"},      {.m_name = "ddiminverse"}, {.m_name = "ddpm"},           {.m_name = "deis"},
      {.m_name = "dpmsmulti"}, {.m_name = "dpmssingle"},  {.m_name = "eulerancestral"}, {.m_name = "euler"},
      {.m_name = "heun"},      {.m_name = "kdpm2"},       {.m_name = "kdpm2ancestral"}, {.m_name = "lms"},
      {.m_name = "pndm"},      {.m_name = "unipc"}};

  int m_width = 512;
  int m_height = 512;
  int m_steps = 35;
  int m_seed = 0;
  int m_nIter = 1;
  double m_cfg = 7.5;

  std::unique_ptr<GLImage> m_image = 0;

private:
  void renderImage() {
    m_image.reset();
    m_image = std::unique_ptr<GLImage>(
        new GLImage(CONFIG::IMAGE_SIZE_X_LIMIT.get(), CONFIG::IMAGE_SIZE_Y_LIMIT.get(), "txt2img"));

    int seed = m_seed;
    if (m_seed == 0) {
      seed = rand() % INT_MAX + 1;
    }
    StableManager::GetInstance().textToImage(m_prompt, m_negativePrompt, m_selectedSampler, m_nIter, m_steps, m_cfg,
                                             seed, m_width, m_height, m_image->renderState);
  }

  void imageWindow() {
    if (ImGui::CollapsingHeader("Preview")) {
      if (m_image) {
        // Once image is marked as rendered display on screen
        if (m_image->renderState == Q_RENDER_STATE::RENDERED) {
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
    if (ImGui::CollapsingHeader("Prompts")) {
      ImGui::Text("Prompt");
      ImGui::InputTextMultiline("##prompt", &m_prompt);
      ImGui::Text("Negative Prompt");
      ImGui::InputTextMultiline("##neg prompt", &m_negativePrompt);
    }
  }

  void promptConfig() {
    if (ImGui::CollapsingHeader("Gen Config")) {
      // Width control
      ImGui::SliderInt("width", &m_width, 1, CONFIG::IMAGE_SIZE_X_LIMIT.get());
      if (ImGui::BeginPopupContextItem("width")) {
        ImGui::InputInt("value", &m_width);
        if (ImGui::MenuItem("Reset to default: 512"))
          m_width = 512;
        ImGui::EndPopup();
      }

      // Height control
      ImGui::SliderInt("height", &m_height, 1, CONFIG::IMAGE_SIZE_Y_LIMIT.get());
      if (ImGui::BeginPopupContextItem("height")) {
        ImGui::InputInt("value", &m_height);
        if (ImGui::MenuItem("Reset to default: 512"))
          m_height = 512;
        ImGui::EndPopup();
      }

      if (ImGui::BeginCombo("Sampler", m_selectedSampler.c_str(), ImGuiComboFlags_NoArrowButton)) {
        for (auto &item : m_samplerList) {
          if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
            m_selectedSampler = item.m_name;
          }
          if (item.m_isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      ImGui::InputInt("steps", &m_steps);
      ImGui::InputInt("seed", &m_seed);
      ImGui::InputDouble("cfg scale", &m_cfg, 0.1);
      ImGui::InputInt("Iterations", &m_nIter);
    }
  }
};