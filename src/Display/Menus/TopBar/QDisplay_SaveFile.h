#pragma once

#include <imgui.h>
#include "Display/ErrorHandler.h"
#include "ThirdParty/imgui/imfilebrowser.h"
#include "Display/QDisplay_Base.h"

#include <imgui_stdlib.h>

class QDisplay_SaveFile : public QDisplay_Base {
public:
  QDisplay_SaveFile(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    fileDialog.SetTitle("Save File");
    fileDialog.SetTypeFilters({".sdui"});
  }

  void openWindow() { fileDialog.Open(); };

  virtual void render() {
    fileDialog.Display();
    if (fileDialog.HasSelected()) {
      m_renderManager->saveCanvas(fileDialog.GetSelected().string());
      clear();
    }
  }

private:
  ImGui::FileBrowser fileDialog;

private:
  void clear() { fileDialog.ClearSelected(); }
};