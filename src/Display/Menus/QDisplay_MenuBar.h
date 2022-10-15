#pragma once

#include <fstream>
#include <imgui.h>

#include "../../QLogger.h"
#include "../QDisplay_Base.h"

class QDisplay_MenuBar : public QDisplay_Base {

private:
  // Window triggers
  bool newFileOpen = false;
  bool logFileOpen = false;

  // Log config
  std::ifstream logStream;
  std::stringstream logFileBuffer;
  bool logUpdated = true;
  clock_t lastLogReadTime;

  // New file config
  int x = 1920;
  int y = 1080;

  /*
   * Popup for displaying log file output
   */
  void QDisplay_LogFile() {

    if (logFileOpen) {
      ImGui::SetNextWindowBgAlpha(0.9f);
      ImGui::Begin("Log");

      // Only update text if required
      if (QLogger::GetInstance().m_LAST_WRITE_TIME != lastLogReadTime) {
        logStream.open(QLOGGER_LOGFILE, std::ios::in);

        logFileBuffer.clear();
        logFileBuffer.str(std::string());

        lastLogReadTime = QLogger::GetInstance().m_LAST_WRITE_TIME;
        logFileBuffer << logStream.rdbuf();
        logUpdated = true;
      }

      ImGui::TextUnformatted(logFileBuffer.str().c_str());

      // Move to bottom of screen
      if (logUpdated) {
        ImGui::SetScrollY(ImGui::GetScrollMaxY() +
                          ImGui::GetStyle().ItemSpacing.y * 2);
        logUpdated = false;
      }

      if (ImGui::Button("Close")) {
        logFileOpen = false;
      }

      if (ImGui::Button("Clear")) {
        logStream.close();
        QLogger::GetInstance().resetLog();
      }

      logStream.close();
      ImGui::End();
    }
  }

  void QDisplay_NewFile() {
    if (newFileOpen) {
      ImGui::OpenPopup("NEW_FILE");
      if (ImGui::BeginPopupModal("NEW_FILE")) {

        ImGui::InputInt("canvas width", &x);
        ImGui::InputInt("canvas height", &y);

        if (ImGui::Button("Create Canvas")) {
          m_renderManager->createCanvas(x, y);
          newFileOpen = false;
        }

        if (ImGui::Button("Cancel")) {
          newFileOpen = false;
        }
        ImGui::EndPopup();
      }
    }
  }

public:
  // Initialise render manager reference
  QDisplay_MenuBar(RenderManager *rm) : QDisplay_Base(rm) {}
  /*
   * Main Menu renderer, contains logic for showing additional display items
   *
   * @param dEngine - reference to DataEngine
   */
  virtual void render() {

    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::BeginMenu("File")) {

        if (ImGui::MenuItem("New File")) {
          newFileOpen = true;
        }

        if (ImGui::MenuItem("Open Log")) {
          logFileOpen = true;
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    // These will only render if their corresponding flags are set
    QDisplay_LogFile();
    QDisplay_NewFile();
  }
};