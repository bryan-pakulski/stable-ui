#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>

#include "Helpers/States.h"
#include "StableManager.h"
#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Rendering/RenderManager.h"
#include "Config/config.h"
#include "Display/QDisplay_Base.h"
#include "QDisplay_Text2Image.h"
#include "QDisplay_Image2Image.h"

class QDisplay_LeftSideBar : public QDisplay_Base {

  struct tabs {
    static const int TXT2IMG = 0;
    static const int IMG2IMG = 1;
    static const int TRAINING = 2;
  };

  std::unique_ptr<QDisplay_Text2Image> Text2ImageWindow;
  std::unique_ptr<QDisplay_Image2Image> Image2ImageWindow;

  // Window Options
  const std::string c_windowName = "Helper Window";
  std::pair<int, int> m_windowSize{};
  int activeTab = tabs::TXT2IMG;

public:
  // Initialise render manager references
  QDisplay_LeftSideBar(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {

    // Initialise sub menus
    Text2ImageWindow = std::unique_ptr<QDisplay_Text2Image>(new QDisplay_Text2Image(rm, w));
    Image2ImageWindow = std::unique_ptr<QDisplay_Image2Image>(new QDisplay_Image2Image(rm, w));

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
      if (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::LOADED) {
        {
          if (ImGui::Button("inference")) {
            activeTab = tabs::TXT2IMG;
          }
          ImGui::SameLine();
          if (ImGui::Button("img2img")) {
            activeTab = tabs::IMG2IMG;
          }
          ImGui::SameLine();
          if (ImGui::Button("training")) {
            activeTab = tabs::TRAINING;
          }
        }
        ImGui::Separator();

        if (activeTab == tabs::TXT2IMG) {
          Text2ImageWindow->render();
        }
        if (activeTab == tabs::IMG2IMG) {
          Image2ImageWindow->render();
        }
      } else if (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::NONE_LOADED) {
        ImGui::Text("Please import and load a model first!");
      } else if (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::LOADING) {
        ImGui::Text("Loading model to memory..");
      } else if (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::FAILED) {
        ImGui::Text("Model failed to load, please check logs or reload");
      }
    }

    ImGui::Separator();

    ImGui::End();
  }
};