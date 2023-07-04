#pragma once

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

class QDisplay_Preview : public QDisplay_Base {

public:
  // Initialise render manager references
  QDisplay_Preview(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {

    m_image = std::unique_ptr<GLImage>(new GLImage(512, 512, "preview"));
  }

  virtual void render() { renderPreview(); }

private:
  std::unique_ptr<GLImage> m_image = 0;
  bool m_previewFlag = false;
  int m_previewIndex = 0;

  void renderPreview() {
    if (ImGui::CollapsingHeader("Render Preview")) {

      // Get latest files once rendering is finished
      if (StableManager::GetInstance().getRenderState() == Q_RENDER_STATE::RENDERED && !m_previewFlag) {
        m_previewFlag = true;
      }

      // Reset files once rendering starts
      if (StableManager::GetInstance().getRenderState() == Q_RENDER_STATE::RENDERING && m_previewFlag) {
        m_previewFlag = false;
        m_previewIndex = 0;
      }

      // Preview images that we have found
      if (m_previewFlag && StableManager::GetInstance().m_latestFiles.size() > 0) {
        if (m_image->m_image_source != StableManager::GetInstance().m_latestFiles.at(m_previewIndex)) {
          m_image->loadFromImage(StableManager::GetInstance().m_latestFiles.at(m_previewIndex));
        }

        if (m_image) {
          if (m_previewIndex > 0) {
            if (ImGui::Button("Previous")) {
              m_previewIndex -= 1;
            }
          }
          if (m_previewIndex < StableManager::GetInstance().m_latestFiles.size() - 1) {

            if (m_previewIndex > 0) {
              ImGui::SameLine();
            }

            if (ImGui::Button("Next")) {
              m_previewIndex += 1;
            }
          }
          ImGui::Separator();
          ImGui::Text("image width: %d image height:%d", m_image->m_width, m_image->m_height);
          if (ImGui::Button("Send to Canvas")) {
            // Send image to be rendered on canvas at selection coordinates
            m_renderManager->sendImageToCanvas(*m_image);
          }
          ImGui::Image((void *)(intptr_t)m_image->m_texture, ImVec2(m_image->m_width * 0.3, m_image->m_height * 0.3));
        }
      }
    }
  }
};
