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
#include "QDisplay_ContentBrowser.h"
#include "QDisplay_CanvasTools.h"

class QDisplay_RightSideBar : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_RightSideBar(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_contentBrowserWindow = std::unique_ptr<QDisplay_ContentBrowser>(new QDisplay_ContentBrowser(rm, w));
    CanvasToolsWindow = std::unique_ptr<QDisplay_CanvasTools>(new QDisplay_CanvasTools(rm, w));
  }

  virtual void render() {
    getWindowSize(m_windowSize);

    ImGui::SetNextWindowPos(
        ImVec2(m_windowSize.first - CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), CONFIG::IMGUI_TOP_WINDOW_HEIGHT.get()));
    ImGui::SetNextWindowSize(ImVec2(CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), float(m_windowSize.second)));

    ImGui::SetNextWindowSizeConstraints(ImVec2(CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), float(m_windowSize.second)),
                                        ImVec2(CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get(), float(m_windowSize.second)));

    ImGui::Begin(c_windowName.c_str(), 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // Rendering menus
    if (ImGui::CollapsingHeader("Content Browser")) {
      m_contentBrowserWindow->render();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Tools", ImGuiTreeNodeFlags_DefaultOpen)) {
      CanvasToolsWindow->render();
    }

    ImGui::Separator();

    ImGui::End();
  }

private:
  std::unique_ptr<QDisplay_ContentBrowser> m_contentBrowserWindow;
  std::unique_ptr<QDisplay_CanvasTools> CanvasToolsWindow;

  // Window Options
  const std::string c_windowName = "Tools";
  std::pair<int, int> m_windowSize{};
};
