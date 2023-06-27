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
        if (m_outpaintStatus != Q_RENDER_STATE::RENDERING &&
            (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::LOADED))
          if (ImGui::Selectable("Outpaint")) {
            m_renderManager->captureBuffer();
            std::string b64Image = GLHELPER::textureToBase64String(&m_renderManager->getBuffer()->m_texture,
                                                                   m_renderManager->getBuffer()->m_width,
                                                                   m_renderManager->getBuffer()->m_height);

            std::string b64Mask = GLHELPER::textureToBase64String(&m_renderManager->getMask()->m_texture,
                                                                  m_renderManager->getMask()->m_width,
                                                                  m_renderManager->getMask()->m_height);

            // TODO: process pixels and send to outpainting pipeline
            // TODO: Use PIL Processesing to create mask for outpainting https: //
            // note.nkmk.me/en/python-pillow-composite/
            StableManager::GetInstance().outpaint(b64Image, b64Mask, *m_renderManager->getPipeline(PIPELINE::PAINT),
                                                  m_outpaintStatus);
          }

        if (ImGui::Selectable("Capture to buffer")) {
          m_renderManager->captureBuffer();
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

private:
  int m_outpaintStatus = Q_RENDER_STATE::UNRENDERED;
};
