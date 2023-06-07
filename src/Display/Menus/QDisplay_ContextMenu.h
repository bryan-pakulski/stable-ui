#pragma once

#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Config/config.h"
#include "Helpers/GLHelper.h"
#include "Display/QDisplay_Base.h"

class QDisplay_ContextMenu : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_ContextMenu(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  virtual void render() {
    // TODO: Make Popup menus a static definition
    if (ImGui::BeginPopup("context menu")) {
      {
        // TODO: add flags to make sure we can only use these options when models are loaded etc...
        if (ImGui::Selectable("Capture to buffer")) {
          m_renderManager->captureBuffer();
        }
        if (ImGui::Selectable("Outpaint")) {
          m_renderManager->outpaintSelection();
        }
        if (ImGui::Selectable("Info")) {
        }
        ImGui::EndPopup();
        m_renderManager->m_contextWindowVisible = false;
      }
    }

    setPopup();
  }

  void setPopup() {
    if (m_renderManager->m_contextWindowVisible) {
      ImGui::OpenPopup("context menu");
    }
  }
};
