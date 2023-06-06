#pragma once

#include <climits>
#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <filesystem>

#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"
#include "Config/config.h"
#include "Display/QDisplay_Base.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "StableManager.h"

class QDisplay_Image2Image : public QDisplay_Base {

  // Window variables & flags
  std::string m_prompt;
  std::string m_negativePrompt;
  std::string m_selectedSampler = "pndm";
  std::vector<listItem> m_samplerList = {
      {.m_name = "ddim"},      {.m_name = "ddiminverse"}, {.m_name = "ddpm"},           {.m_name = "deis"},
      {.m_name = "dpmsmulti"}, {.m_name = "dpmssingle"},  {.m_name = "eulerancestral"}, {.m_name = "euler"},
      {.m_name = "heun"},      {.m_name = "kdpm2"},       {.m_name = "kdpm2ancestral"}, {.m_name = "lms"},
      {.m_name = "pndm"},      {.m_name = "unipc"}};

  int m_steps = 35;
  int m_seed = 0;
  int m_nIter = 1;
  float m_strength = 0.5;
  double m_cfg = 7.5;

  std::unique_ptr<GLImage> m_image = 0;
  GLImage m_preview;

public:
  // Initialise render manager references
  QDisplay_Image2Image(std::shared_ptr<RenderManager> rm, GLFWwindow *w)
      : QDisplay_Base(rm, w), m_preview{GLImage(512, 512, "preview")} {}

  void baseImagePreview() {
    if (ImGui::CollapsingHeader("Base Image")) {
      if (m_renderManager->getImage() != "" && m_preview.m_image_source != m_renderManager->getImage()) {
        m_preview.loadFromImage(m_renderManager->getImage());
      } else {
        ImGui::Image((void *)(intptr_t)m_preview.m_texture, ImVec2(m_preview.m_width * 0.3, m_preview.m_height * 0.3));
      }
    }
  }

  void renderImage() {
    m_image.reset();
    m_image = std::unique_ptr<GLImage>(
        new GLImage(CONFIG::IMAGE_SIZE_X_LIMIT.get(), CONFIG::IMAGE_SIZE_Y_LIMIT.get(), "img2img"));
    std::string path = m_renderManager->getImage();

    int seed = m_seed;
    if (m_seed == 0) {
      seed = rand() % INT_MAX + 1;
    }
    StableManager::GetInstance().imageToImage(path, m_prompt, m_negativePrompt, m_selectedSampler, m_nIter, m_steps,
                                              m_cfg, m_strength, seed, m_image->renderState);
  }

  void renderPreview() {
    if (ImGui::CollapsingHeader("Render Preview")) {
      if (m_image) {
        // Once image is marked as rendered display on screen
        if (m_image->renderState == Q_RENDER_STATE::RENDERED) {
          ImGui::Text("image width: %d image height:%d", m_image->m_width, m_image->m_height);
          if (ImGui::Button("Send to Canvas")) {
            // Send image to be rendered on canvas at selection coordinates
            m_renderManager->sendImageToCanvas(*m_image);
          }

          // Retrieve latest redered file
          if (!m_image->textured) {
            m_image->loadFromImage(StableManager::GetInstance().getLatestFile(
                CONFIG::OUTPUT_DIRECTORY.get() + "/" + m_renderManager->getActiveCanvas()->m_name));
            m_image->textured = Q_RENDER_STATE::RENDERED;
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
      ImGui::InputTextMultiline("##prompt", &m_prompt, ImVec2(320, CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get()));

      ImGui::Text("Negative Prompt");
      ImGui::InputTextMultiline("##negative prompt", &m_negativePrompt,
                                ImVec2(320, CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get()));
    }
  }

  void promptConfig() {
    if (ImGui::CollapsingHeader("Gen Config")) {

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
      ImGui::SliderFloat("Strength", &m_strength, 0.0, 1.0, "%.2f");
      ImGui::InputDouble("cfg scale", &m_cfg, 0.1);
      ImGui::InputInt("Iterations", &m_nIter);
    }
  }

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
    baseImagePreview();
    promptHelper();
    promptConfig();
    renderPreview();
  }
};