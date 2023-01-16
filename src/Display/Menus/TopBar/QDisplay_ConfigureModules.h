#pragma once

#include <imgui.h>
#include "../../QDisplay_Base.h"

class QDisplay_ConfigureModules : public QDisplay_Base {

private:
  bool m_isOpen = false;

  void configureModulesPopup() {
    if (m_isOpen) {
      ImGui::Begin("Configure Modules");

      if (ImGui::Button("Close")) {
        m_isOpen = false;
      } else if (ImGui::Button("Save")) {
        m_isOpen = false;
      }
      ImGui::End();
    }
  }

public:
  void openWindow() { m_isOpen = true; };

  QDisplay_ConfigureModules(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  virtual void render() {
    // Render configure modules popup
    configureModulesPopup();
  }
};