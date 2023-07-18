#pragma once

#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Config/config.h"
#include "Helpers/GLHelper.h"
#include "Display/QDisplay_Base.h"
#include "imgui.h"
#include "imgui_internal.h"

class QDisplay_SelectionTools : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_SelectionTools(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {

    m_windowName = "selection_tools";
  }

  virtual void render() {
    ImGui::Begin(m_windowName.c_str());

    selectionPreview();

    ImGui::End();
  }

private:
  std::pair<int, int> m_windowSize{};

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
};