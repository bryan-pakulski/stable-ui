#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>

#include "../../Display/ErrorHandler.h"
#include "../../QLogger.h"
#include "../../Rendering/RenderManager.h"
#include "../../Config/config.h"
#include "../QDisplay_Base.h"
#include "QDisplay_Text2Image.h"
#include "QDisplay_Image2Image.h"

class QDisplay_MainWindow : public QDisplay_Base {

  std::unique_ptr<QDisplay_Text2Image> Text2ImageWindow;
  std::unique_ptr<QDisplay_Image2Image> Image2ImageWindow;

  // Window Options
  const std::string c_windowName = "Helper Window";
  std::pair<int, int> m_windowSize {};
  const float c_windowHeight = 320.0f;
  int tab = 0;

public:
  // Initialise render manager references
  QDisplay_MainWindow(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    Text2ImageWindow = std::unique_ptr<QDisplay_Text2Image>(new QDisplay_Text2Image(rm, w));
    Image2ImageWindow = std::unique_ptr<QDisplay_Image2Image>(new QDisplay_Image2Image(rm, w));

    // Set drag drop callback
    glfwSetDropCallback(w, drop_callback);
  }

  // Drag Drop callback for use with img2img
  static void drop_callback(GLFWwindow* window, int count, const char** paths) {
    
  }

  virtual void render() {
    getWindowSize(m_windowSize);

    // Bottom helper bar
    ImGui::SetNextWindowPos(ImVec2(0, float(m_windowSize.second) - c_windowHeight));
    ImGui::SetNextWindowSize(ImVec2(float(m_windowSize.first), c_windowHeight));

    ImGui::SetNextWindowSizeConstraints(ImVec2(float(m_windowSize.first), 0.0f), ImVec2(float(m_windowSize.first), float(m_windowSize.second)));
    
    ImGui::Begin(c_windowName.c_str(), 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse );

    // Tabbed menu
    {
      ImGui::SameLine();
      if (ImGui::Button("Text To Image")) {
        tab = 0;
      }
      ImGui::SameLine();
      if (ImGui::Button("Image To Image")) {
        tab = 1;
      }
      ImGui::SameLine();
      if (ImGui::Button("Textual Inversion")) {
        tab = 2;
      }
    }
    ImGui::Separator();

    if (tab == 0) {
      Text2ImageWindow->render();
    }
    if (tab == 1) {
      Image2ImageWindow->render();
    }

    ImGui::End();
  }
};