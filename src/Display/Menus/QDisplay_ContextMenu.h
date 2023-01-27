#pragma once

#include "../../../Display/ErrorHandler.h"
#include "../../../QLogger.h"
#include "../../../Config/config.h"
#include "../../../Rendering/Helper.h"
#include "../QDisplay_Base.h"

class QDisplay_ContextMenu : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_ContextMenu(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  virtual void render() {
    if (ImGui::BeginPopup("context menu")) {
      {
        if (ImGui::Selectable("Send to buffer")) {
          m_stableManager->m_selection->captureBuffer();
        }
        if (ImGui::Selectable("Info")) {
        }
        ImGui::EndPopup();
        m_stableManager->m_contextWindowVisible = false;
      }
    }

    setPopup();
  }

  void setPopup() {
    if (m_stableManager->m_contextWindowVisible) {
      ImGui::OpenPopup("context menu");
    }
  }

private:
};
