#pragma once

#include <imnodes.h>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "Config/config.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include "Rendering/RenderManager.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "StableManager.h"

class QDisplay_NodeWindow : public QDisplay_Base {

public:
  // Initialise render manager references
  QDisplay_NodeWindow(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

  virtual void render() {
    m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::Begin("node editor");

    ImNodes::BeginNodeEditor();

    ImNodes::BeginNode(hardcoded_node_id);
    ImGui::Dummy(ImVec2(80.0f, 45.0f));
    ImNodes::EndNode();

    ImNodes::EndNodeEditor();

    ImGui::End();
  }

private:
  const int hardcoded_node_id = 1;
};
