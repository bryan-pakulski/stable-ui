#pragma once

#include "Config/structs.h"
#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Config/config.h"
#include "Helpers/GLHelper.h"
#include "Display/QDisplay_Base.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"
#include "StableManager.h"

class QDisplay_ContextMenu : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_ContextMenu(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  virtual void render() {
    if (ImGui::BeginPopup("context menu")) {
      {
        if (*m_renderManager->getPaintPipelineStatus() != Q_RENDER_STATE::RENDERING &&
            (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::LOADED)) {
          if (ImGui::Selectable("Outpaint")) {
            m_renderManager->paintSelection(true);
          }

          if (ImGui::Selectable("Capture to buffer")) {
            m_renderManager->captureBuffer();
          }
        }

        if (ImGui::Selectable("Info")) {
          // TODO: get info???
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
