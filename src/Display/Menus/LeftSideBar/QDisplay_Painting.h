#pragma once

#include <imgui.h>
#include <imgui_stdlib.h>

#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"
#include "Config/config.h"
#include "Display/QDisplay_Base.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "StableManager.h"

class QDisplay_Painting : public QDisplay_Base {

public:
  // Initialise render manager references
  QDisplay_Painting(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_config = rm->getPipeline(PIPELINE::PAINT);
    m_windowName = "painting";

    // When using inpainting strength must be set to 1.0 otherwise we may get unexpected results using our mask
    m_config->strength = 1.0f;
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

      ImGui::Separator();
      promptConfig();
      ImGui::Separator();
      promptHelper();
      ImGui::Separator();
      selectionPreview();
      ImGui::Separator();
    }

    ImGui::End();
  }

private:
  std::shared_ptr<pipelineConfig> m_config;
  bool m_randomSeed = true;

  std::vector<listItem> m_samplerList = {
      {.m_name = "ddim"},      {.m_name = "ddiminverse"}, {.m_name = "ddpm"},           {.m_name = "deis"},
      {.m_name = "dpmsmulti"}, {.m_name = "dpmssingle"},  {.m_name = "eulerancestral"}, {.m_name = "euler"},
      {.m_name = "heun"},      {.m_name = "kdpm2"},       {.m_name = "kdpm2ancestral"}, {.m_name = "lms"},
      {.m_name = "pndm"},      {.m_name = "unipc"}};

private:
  void selectionPreview() {
    ImGui::SliderInt("width", &m_renderManager->m_selection->m_size.x, 1, CONFIG::IMAGE_SIZE_X_LIMIT.get());
    if (ImGui::BeginPopupContextItem("Selection Width")) {
      ImGui::InputInt("value", &m_renderManager->m_selection->m_size.x);
      if (ImGui::MenuItem("Reset to default: 512"))
        m_renderManager->m_selection->m_size.x = 512;
      ImGui::EndPopup();
    }

    ImGui::SliderInt("height", &m_renderManager->m_selection->m_size.y, 1, CONFIG::IMAGE_SIZE_Y_LIMIT.get());
    if (ImGui::BeginPopupContextItem("Selection Height")) {
      ImGui::InputInt("value", &m_renderManager->m_selection->m_size.y);
      if (ImGui::MenuItem("Reset to default: 512"))
        m_renderManager->m_selection->m_size.y = 512;
      ImGui::EndPopup();
    }

    ImGui::Separator();

    ImGui::Checkbox("Snap to grid", &m_renderManager->m_selection->m_snap);

    if (m_renderManager->m_selection->m_snap) {
      ImGui::SliderInt("Selection Snap", &m_renderManager->m_selection->m_pixelSnap, 1, 512);
    }

    // TODO: add send to img2img

    ImGui::Text("Buffer");
    if (ImGui::Button("Save buffer to file")) {
      m_renderManager->saveBuffer();
    }

    ImGui::Separator();

    float scaleFactor = ImGui::GetWindowContentRegionWidth() / (float)m_renderManager->getBuffer()->m_width;

    ImGui::Image((void *)(intptr_t)m_renderManager->getBuffer()->m_texture,
                 ImVec2(m_renderManager->getBuffer()->m_width * scaleFactor,
                        m_renderManager->getBuffer()->m_height * scaleFactor));

    ImGui::Separator();

    ImGui::Image(
        (void *)(intptr_t)m_renderManager->getMask()->m_texture,
        ImVec2(m_renderManager->getMask()->m_width * scaleFactor, m_renderManager->getMask()->m_height * scaleFactor));

    ImGui::Separator();
  }

  void renderImage() {

    if (m_randomSeed) {
      m_config->seed = rand() % INT_MAX + 1;
    }

    m_renderManager->paintSelection(false);
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
      ImGui::SliderFloat("Strength", &m_config->strength, 0.0, 1.0, "%.2f");
      ImGui::InputDouble("cfg scale", &m_config->cfg, 0.1);
      ImGui::InputInt("Iterations", &m_config->iterations);
    }
  }
};