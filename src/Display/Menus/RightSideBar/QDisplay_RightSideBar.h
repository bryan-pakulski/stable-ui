#pragma once

#include <fstream>
#include <imgui.h>
#include "imgui_internal.h"
#include <filesystem>

#include "Helpers/States.h"
#include "StableManager.h"
#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Rendering/RenderManager.h"
#include "Config/config.h"
#include "Display/QDisplay_Base.h"
#include "Display/Menus/RightSideBar/QDisplay_CanvasTools.h"
#include "Display/Menus/RightSideBar/QDisplay_SelectionTools.h"

class QDisplay_RightSideBar : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_RightSideBar(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    CanvasToolsWindow = std::unique_ptr<QDisplay_CanvasTools>(new QDisplay_CanvasTools(rm, w));
    SelectionToolsWindow = std::unique_ptr<QDisplay_SelectionTools>(new QDisplay_SelectionTools(rm, w));
  }

  void update_dock() {

    dock_id = ImGui::GetID(c_dockName.c_str());

    dock_size.x = ImGui::DockBuilderGetNode(dock_id)->Size.x;
    dock_size.y = m_windowSize.second;
    dock_pos.x = m_windowSize.first - dock_size.x;
    dock_pos.y = CONFIG::IMGUI_TOP_WINDOW_HEIGHT.get();

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

    ImGuiID dock_upper = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Up, 0.6f, nullptr, &dock_id);
    ImGuiID dock_lower = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.4f, nullptr, &dock_id);

    ImGui::DockBuilderDockWindow(CanvasToolsWindow->m_windowName.c_str(), dock_upper);
    ImGui::DockBuilderDockWindow(SelectionToolsWindow->m_windowName.c_str(), dock_lower);

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
      CanvasToolsWindow->render();
      SelectionToolsWindow->render();
    } else {
      m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoTabBar;
      ImGui::SetNextWindowClass(&m_window_class);
      ImGui::Begin(CanvasToolsWindow->m_windowName.c_str());
      if (ImGui::Button("<-")) {
        dock_pos = {CONFIG::WINDOW_WIDTH.get() - CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(),
                    CONFIG::IMGUI_TOP_WINDOW_HEIGHT.get()};
        dock_size = {CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), (float)CONFIG::WINDOW_HEIGHT.get()};
        dock_init();
      }
      ImGui::End();
    }
  }

private:
  std::unique_ptr<QDisplay_CanvasTools> CanvasToolsWindow;
  std::unique_ptr<QDisplay_SelectionTools> SelectionToolsWindow;
  std::pair<int, int> m_windowSize{};

  const std::string c_dockName = "dock_tools";
  ImGuiID dock_id;
  ImVec2 dock_pos{CONFIG::WINDOW_WIDTH.get() - CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(),
                  CONFIG::IMGUI_TOP_WINDOW_HEIGHT.get()};
  ImVec2 dock_size{CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), (float)CONFIG::WINDOW_HEIGHT.get()};
  bool m_dock_init = true;
};
