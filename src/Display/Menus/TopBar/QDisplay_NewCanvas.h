#pragma once

#include "StableManager.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"

#include <imgui.h>
#include <imgui_stdlib.h>

class QDisplay_NewCanvas : public QDisplay_Base {
public:
  QDisplay_NewCanvas(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  void openWindow() { m_isOpen = true; }

  virtual void render() {
    if (m_isOpen) {
      ImGui::OpenPopup("NEW_FILE");
      if (ImGui::BeginPopupModal("NEW_FILE")) {

        ImGui::InputText("canvas name", &m_canvasName);

        if (ImGui::Button("Create Canvas")) {
          m_renderManager->createCanvas(0, 0, m_canvasName);
          m_isOpen = false;
        }

        if (ImGui::Button("Cancel")) {
          m_isOpen = false;
        }
        ImGui::EndPopup();
      }
    }
  }

private:
  std::string m_canvasName = "New Canvas";
};