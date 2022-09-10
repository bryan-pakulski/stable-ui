#pragma once

#include <fstream>
#include <imgui.h>

#include "../../QLogger.h"
#include "../../Rendering/RenderManager.h"

std::ifstream logStream;
std::stringstream logFileBuffer;
bool logFileOpen = false;
bool logUpdated = true;
clock_t lastLogReadTime;

/*
 * Popup for displaying log file output
 */
inline void QDisplay_LogFile() {

  if (logFileOpen) {
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::Begin("Log");

    // Only update text if required
    if (QLogger::GetInstance().LAST_WRITE_TIME != lastLogReadTime) {
      logStream.open(QLOGGER_LOGFILE, std::ios::in);

      logFileBuffer.clear();
      logFileBuffer.str(std::string());

      lastLogReadTime = QLogger::GetInstance().LAST_WRITE_TIME;
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

/*
 * Main Menu renderer, contains logic for showing additional display items
 *
 * @param dEngine - reference to DataEngine
 */
inline void QDisplay_MainMenu() {

  if (ImGui::BeginMainMenuBar()) {

    if (ImGui::BeginMenu("File")) {

      if (ImGui::MenuItem("Open Log")) {
        // Set log flags
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
}