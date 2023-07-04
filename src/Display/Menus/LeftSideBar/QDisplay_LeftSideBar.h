#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>
#include <memory>

#include "Helpers/States.h"
#include "StableManager.h"
#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Rendering/RenderManager.h"
#include "Config/config.h"
#include "Display/QDisplay_Base.h"
#include "Display/Menus/LeftSideBar/QDisplay_Text2Image.h"
#include "Display/Menus/LeftSideBar/QDisplay_Image2Image.h"
#include "Display/Menus/LeftSideBar/QDisplay_Painting.h"
#include "Display/Menus/LeftSideBar/QDisplay_Preview.h"

class QDisplay_LeftSideBar : public QDisplay_Base {

public:
  // Initialise render manager references
  QDisplay_LeftSideBar(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {

    // Initialise sub menus
    m_Text2ImageWindow = std::unique_ptr<QDisplay_Text2Image>(new QDisplay_Text2Image(rm, w));
    m_Image2ImageWindow = std::unique_ptr<QDisplay_Image2Image>(new QDisplay_Image2Image(rm, w));
    m_PaintingWindow = std::unique_ptr<QDisplay_Painting>(new QDisplay_Painting(rm, w));
    m_PreviewWindow = std::unique_ptr<QDisplay_Preview>(new QDisplay_Preview(rm, w));

    // Set drag drop callback
    glfwSetDropCallback(w, drop_callback);
  }

  // Drag Drop callback for use with img2img
  static void drop_callback(GLFWwindow *window, int count, const char **paths) {
    // TODO: do something with drag/drop operations
  }

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
    if (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::LOADED) {
      {
        if (ImGui::Button("text")) {
          m_activeTab = tabs::TXT2IMG;
        }
        ImGui::SameLine();
        if (ImGui::Button("image")) {
          m_activeTab = tabs::IMG2IMG;
        }
        ImGui::SameLine();
        if (ImGui::Button("painting")) {
          m_activeTab = tabs::PAINTING;
        }
      }
      ImGui::Separator();

      if (m_activeTab == tabs::TXT2IMG) {
        m_Text2ImageWindow->render();
      }
      if (m_activeTab == tabs::IMG2IMG) {
        m_Image2ImageWindow->render();
      }
      if (m_activeTab == tabs::PAINTING) {
        m_PaintingWindow->render();
      }

      ImGui::Separator();
      m_PreviewWindow->render();

    } else if (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::NONE_LOADED) {
      ImGui::Text("Please import and load a model first!");
    } else if (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::LOADING) {
      ImGui::Text("Loading model to memory..");
    } else if (StableManager::GetInstance().getModelState() == Q_MODEL_STATUS::FAILED) {
      ImGui::Text("Model failed to load, please check logs or reload");
    }

    ImGui::Separator();

    ImGui::End();
  }

private:
  struct tabs {
    static const int TXT2IMG = 0;
    static const int IMG2IMG = 1;
    static const int PAINTING = 2;
    static const int TRAINING = 3;
  };

  std::unique_ptr<QDisplay_Text2Image> m_Text2ImageWindow;
  std::unique_ptr<QDisplay_Image2Image> m_Image2ImageWindow;
  std::unique_ptr<QDisplay_Painting> m_PaintingWindow;
  std::unique_ptr<QDisplay_Preview> m_PreviewWindow;

  // Window Options
  const std::string c_windowName = "Helper Window";
  std::pair<int, int> m_windowSize{};
  int m_activeTab = tabs::TXT2IMG;
};