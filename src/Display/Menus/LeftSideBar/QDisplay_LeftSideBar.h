#pragma once

#include <fstream>
#include <imgui.h>
#include "imgui_internal.h"
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

  void update_dock() {
    dock_id = ImGui::GetID(c_dockName.c_str());

    dock_size.x = ImGui::DockBuilderGetNode(dock_id)->Size.x;
    dock_size.y = m_windowSize.second;

    ImGui::DockBuilderSetNodeSize(dock_id, dock_size);
    ImGui::DockBuilderSetNodePos(dock_id, dock_pos);
    ImGui::DockBuilderFinish(dock_id);
  }

  void dock_init() {

    // Create main dock node
    dock_id = ImGui::GetID(c_dockName.c_str());

    ImGui::DockBuilderRemoveNode(dock_id); // Clear any preexisting layouts associated with the ID we just chose
    ImGui::DockBuilderAddNode(dock_id);    // Create a new dock node to use

    dock_size.y = m_windowSize.second;

    ImGui::DockBuilderSetNodeSize(dock_id, dock_size);
    ImGui::DockBuilderSetNodePos(dock_id, dock_pos);

    ImGuiID dock_upper = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 1.0f, nullptr, &dock_id);
    ImGuiID dock_lower = ImGui::DockBuilderSplitNode(dock_upper, ImGuiDir_Down, 0.5f, nullptr, &dock_upper);

    ImGui::DockBuilderDockWindow(m_Text2ImageWindow->m_windowName.c_str(), dock_upper);
    ImGui::DockBuilderDockWindow(m_Image2ImageWindow->m_windowName.c_str(), dock_upper);
    ImGui::DockBuilderDockWindow(m_PaintingWindow->m_windowName.c_str(), dock_upper);
    ImGui::DockBuilderDockWindow(m_PreviewWindow->m_windowName.c_str(), dock_lower);

    ImGui::DockBuilderFinish(dock_id);

    m_dock_init = false;
  }

  virtual void render() {
    getWindowSize(m_windowSize);

    if (m_dock_init) {
      dock_init();
    }

    update_dock();

    if (dock_size.x > CONFIG::IMGUI_TOOLS_WINDOW_MIN_WIDTH.get()) {
      m_Text2ImageWindow->render();
      m_Image2ImageWindow->render();
      m_PaintingWindow->render();
      m_PreviewWindow->render();
    } else {
      m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoTabBar;
      ImGui::SetNextWindowClass(&m_window_class);
      ImGui::Begin(m_PreviewWindow->m_windowName.c_str(), 0, ImGuiWindowFlags_NoMove);
      ImGui::End();
    }
  }

private:
  std::unique_ptr<QDisplay_Text2Image> m_Text2ImageWindow;
  std::unique_ptr<QDisplay_Image2Image> m_Image2ImageWindow;
  std::unique_ptr<QDisplay_Painting> m_PaintingWindow;
  std::unique_ptr<QDisplay_Preview> m_PreviewWindow;

  std::pair<int, int> m_windowSize{};

  const std::string c_dockName = "dock_left_panel";
  ImGuiID dock_id;
  ImVec2 dock_pos{0, CONFIG::IMGUI_TOP_WINDOW_HEIGHT.get()};
  ImVec2 dock_size{CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), (float)CONFIG::WINDOW_HEIGHT.get()};
  bool m_dock_init = true;
};