#pragma once

#include <imgui.h>
#include "Display/ErrorHandler.h"
#include "ThirdParty/imgui/imfilebrowser.h"
#include "Display/QDisplay_Base.h"

#include <imgui_stdlib.h>

class QDisplay_LoadFile : public QDisplay_Base {
public:
  QDisplay_LoadFile(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    fileDialog.SetTitle("Load File");
    fileDialog.SetTypeFilters({".sdui"});
  }

  void openWindow() { fileDialog.Open(); };

  virtual void render() {
    fileDialog.Display();
    if (fileDialog.HasSelected()) {
      m_renderManager->loadCanvas(fileDialog.GetSelected().string());
      clear();
    }
  }

private:
  ImGui::FileBrowser fileDialog;

private:
  void clear() { fileDialog.ClearSelected(); }
};