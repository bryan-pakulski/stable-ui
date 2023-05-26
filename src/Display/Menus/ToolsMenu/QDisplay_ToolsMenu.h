#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>

#include "StableManager.h"
#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Rendering/RenderManager.h"
#include "Config/config.h"
#include "Display/QDisplay_Base.h"
#include "QDisplay_CanvasTools.h"
#include "QDisplay_Text2Image.h"
#include "QDisplay_Image2Image.h"

class QDisplay_ToolsMenu : public QDisplay_Base {

  std::unique_ptr<QDisplay_Text2Image> Text2ImageWindow;
  std::unique_ptr<QDisplay_Image2Image> Image2ImageWindow;
  std::unique_ptr<QDisplay_CanvasTools> CanvasToolsWindow;

  // Window Options
  const std::string c_windowName = "Helper Window";
  std::pair<int, int> m_windowSize{};
  int tab = 0;

public:
  // Initialise render manager references
  QDisplay_ToolsMenu(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {

    // Initialise sub menus
    Text2ImageWindow = std::unique_ptr<QDisplay_Text2Image>(new QDisplay_Text2Image(rm, w));
    Image2ImageWindow = std::unique_ptr<QDisplay_Image2Image>(new QDisplay_Image2Image(rm, w));
    CanvasToolsWindow = std::unique_ptr<QDisplay_CanvasTools>(new QDisplay_CanvasTools(rm, w));

    // Set drag drop callback
    glfwSetDropCallback(w, drop_callback);
  }

  // Drag Drop callback for use with img2img
  static void drop_callback(GLFWwindow *window, int count, const char **paths) {}

  virtual void render() {
    getWindowSize(m_windowSize);

    // Bottom helper bar
    ImGui::SetNextWindowPos(ImVec2(0, CONFIG::IMGUI_TOP_WINDOW_HEIGHT.get()));
    ImGui::SetNextWindowSize(ImVec2(CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), float(m_windowSize.second)));

    ImGui::SetNextWindowSizeConstraints(ImVec2(CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), float(m_windowSize.second)),
                                        ImVec2(CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), float(m_windowSize.second)));

    ImGui::Begin(c_windowName.c_str(), 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // Tabbed menu

    // Rendering menus
    if (ImGui::CollapsingHeader("Rendering")) {
      if (StableManager::GetInstance().getModelState() == Q_EXECUTION_STATE::SUCCESS) {
        {
          if (ImGui::Button("txt2img")) {
            tab = 0;
          }
          ImGui::SameLine();
          if (ImGui::Button("img2img")) {
            tab = 1;
          }
          ImGui::SameLine();
          if (ImGui::Button("training")) {
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
      } else if (StableManager::GetInstance().getModelState() == Q_EXECUTION_STATE::PENDING) {
        ImGui::Text("Please import and load a model first!");
      } else if (StableManager::GetInstance().getModelState() == Q_EXECUTION_STATE::LOADING) {
        ImGui::Text("Please wait for model to finish loading...");
      } else if (StableManager::GetInstance().getModelState() == Q_EXECUTION_STATE::FAILED) {
        ImGui::Text("Model failed to load, please check application logs");
      }
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Tools")) {
      CanvasToolsWindow->render();
    }

    ImGui::End();
  }
};