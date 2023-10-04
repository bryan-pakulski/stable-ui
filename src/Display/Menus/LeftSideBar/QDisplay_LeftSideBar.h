#pragma once

#include "imgui_internal.h"
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <memory>

#include "Config/config.h"
#include "Display/ErrorHandler.h"
#include "Display/Menus/LeftSideBar/QDisplay_NodeWindow.h"
#include "Display/QDisplay_Base.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"
#include "StableManager.h"

class QDisplay_LeftSideBar : public QDisplay_Base {

public:
  // Initialise render manager references
  QDisplay_LeftSideBar(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {

    // Initialise sub menus
    m_NodeWindow = std::unique_ptr<QDisplay_NodeWindow>(new QDisplay_NodeWindow(rm, w));
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

    ImGui::DockBuilderRemoveNode(dock_id); // Clear any preexisting layouts associated with the ID we
                                           // just chose
    ImGui::DockBuilderAddNode(dock_id);    // Create a new dock node to use

    dock_size.y = m_windowSize.second;

    ImGui::DockBuilderSetNodeSize(dock_id, dock_size);
    ImGui::DockBuilderSetNodePos(dock_id, dock_pos);

    ImGuiID dock_upper = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 1.0f, nullptr, &dock_id);

    ImGui::DockBuilderDockWindow(m_NodeWindow->m_windowName.c_str(), dock_upper);

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
      m_NodeWindow->render();
    } else {
      m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoTabBar;
      ImGui::SetNextWindowClass(&m_window_class);
      ImGui::Begin(c_dockName.c_str());
      if (ImGui::Button("->")) {
        dock_pos = {0, CONFIG::IMGUI_TOP_WINDOW_HEIGHT.get()};
        dock_size = {CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), (float)CONFIG::WINDOW_HEIGHT.get()};
        dock_init();
      }
      ImGui::End();
    }
  }

private:
  std::unique_ptr<QDisplay_NodeWindow> m_NodeWindow;
  std::pair<int, int> m_windowSize{};

  const std::string c_dockName = "dock_left_panel";
  ImGuiID dock_id;
  ImVec2 dock_pos{0, CONFIG::IMGUI_TOP_WINDOW_HEIGHT.get()};
  ImVec2 dock_size{CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), (float)CONFIG::WINDOW_HEIGHT.get()};
  bool m_dock_init = true;
};