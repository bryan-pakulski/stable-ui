#pragma once

#include <fstream>
#include <imgui.h>

#include "../../../Config/config.h"
#include "../../../QLogger.h"
#include "../../../Rendering/Helper.h"

#include "../../../Display/ErrorHandler.h"
#include "../../QDisplay_Base.h"
#include "QDisplay_ConfigureModel.h"
#include "QDisplay_ImportModel.h"
#include "QDisplay_ImportVAE.h"
#include "QDisplay_LoadModel.h"

class QDisplay_TopBar : public QDisplay_Base {

private:
  // Window triggers
  bool newFileOpen = false;
  bool logFileOpen = false;
  bool loadFileOpen = false;
  bool selectCanvasOpen = false;
  bool configureModelOpen = false;

  std::unique_ptr<QDisplay_ConfigureModel> m_configureModelWindow;
  std::unique_ptr<QDisplay_ImportModel> m_importModelWindow;
  std::unique_ptr<QDisplay_ImportVAE> m_importVAEWindow;
  std::unique_ptr<QDisplay_LoadModel> m_loadModelWindow;

  // Log config
  std::ifstream logStream;
  std::stringstream logFileBuffer;
  bool logUpdated = true;
  clock_t lastLogReadTime;

  // New file config
  char m_canvasName[256] = "new";

  /*
   * Popup for displaying log file output
   */
  void QDisplay_LogFile() {

    if (logFileOpen) {
      ImGui::SetNextWindowBgAlpha(0.9f);
      ImGui::SetNextWindowSize(ImVec2(CONFIG::IMGUI_LOG_WINDOW_WIDTH.get(), CONFIG::IMGUI_LOG_WINDOW_HEIGHT.get()));
      ImGui::Begin("Log");

      if (ImGui::Button("Close")) {
        logFileOpen = false;
      }

      ImGui::SameLine();

      if (ImGui::Button("Clear")) {
        logStream.close();
        QLogger::GetInstance().resetLog();
      }

      ImGui::Separator();

      ImGui::BeginChild("ScrollingLog");

      // Only update text if required
      if (QLogger::GetInstance().m_LAST_WRITE_TIME != lastLogReadTime) {
        logStream.open(QLOGGER_LOGFILE, std::ios::in);

        logFileBuffer.clear();
        logFileBuffer.str(std::string());

        lastLogReadTime = QLogger::GetInstance().m_LAST_WRITE_TIME;
        logFileBuffer << logStream.rdbuf();
        logStream.close();
        logUpdated = true;
      }

      ImGui::TextUnformatted(logFileBuffer.str().c_str());

      // Move to bottom of screen
      if (logUpdated) {
        ImGui::SetScrollY(ImGui::GetScrollMaxY() + ImGui::GetStyle().ItemSpacing.y * 2);
        logUpdated = false;
      }
      ImGui::EndChild();

      ImGui::End();
    }
  }

  void QDisplay_NewFile() {
    if (newFileOpen) {
      ImGui::OpenPopup("NEW_FILE");
      if (ImGui::BeginPopupModal("NEW_FILE")) {

        ImGui::InputText("canvas name", m_canvasName, 256);

        if (ImGui::Button("Create Canvas")) {
          m_stableManager->createCanvas(0, 0, std::string(m_canvasName));
          newFileOpen = false;
        }

        if (ImGui::Button("Cancel")) {
          newFileOpen = false;
        }
        ImGui::EndPopup();
      }
    }
  }

  void QDisplay_LoadFile() {
    if (loadFileOpen) {

      // Todo: Load file

      loadFileOpen = false;
    }
  }

  void QDisplay_SelectCanvasOpen() {
    if (selectCanvasOpen) {
      ImGui::OpenPopup("SELECTCANVAS");
      if (ImGui::BeginPopupModal("SELECTCANVAS")) {
        if (ImGui::BeginListBox("Canvas")) {
          for (auto &item : m_stableManager->m_canvas) {
            const char *item_name = item->m_name.c_str();
            int index = std::addressof(item) - std::addressof(m_stableManager->m_canvas.front());
            const bool is_selected = index == m_stableManager->m_activeId;

            if (ImGui::Selectable(item_name, is_selected)) {
              m_stableManager->selectCanvas(index);
              selectCanvasOpen = false;
            }

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected) {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndListBox();
        }
      }
      if (ImGui::Button("close")) {
        selectCanvasOpen = false;
      }
      ImGui::EndPopup();
    }
  }

public:
  // Initialise render manager reference
  QDisplay_TopBar(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_configureModelWindow = std::unique_ptr<QDisplay_ConfigureModel>(new QDisplay_ConfigureModel(rm, w));
    m_importModelWindow = std::unique_ptr<QDisplay_ImportModel>(new QDisplay_ImportModel(rm, w));
    m_importVAEWindow = std::unique_ptr<QDisplay_ImportVAE>(new QDisplay_ImportVAE(rm, w));
    m_loadModelWindow = std::unique_ptr<QDisplay_LoadModel>(new QDisplay_LoadModel(rm, w));
  }

  /*
   * Main Menu renderer, contains logic for showing additional display items
   */
  virtual void render() {
    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::BeginMenu("File")) {

        if (ImGui::MenuItem("New Canvas")) {
          newFileOpen = true;
        }

        if (ImGui::MenuItem("Load Canvas")) {
          loadFileOpen = true;
        }

        if (ImGui::MenuItem("Import Model")) {
          m_importModelWindow->openWindow();
        }

        if (ImGui::MenuItem("Import VAE")) {
          m_importVAEWindow->openWindow();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {

        if (ImGui::MenuItem("Select Canvas")) {
          selectCanvasOpen = true;
        }

        if (ImGui::MenuItem("Load Model To Memory")) {
          m_loadModelWindow->openWindow();
        }

        if (ImGui::MenuItem("Configure Models")) {
          m_configureModelWindow->openWindow();
        }

        if (ImGui::MenuItem("Open Log")) {
          logFileOpen = true;
        }

        ImGui::EndMenu();
      }

      ImGui::MenuItem(m_stableManager->getActiveCanvas()->m_name.c_str());

      ImGui::EndMainMenuBar();
    }

    // These will only render if their corresponding flags are set
    QDisplay_LogFile();
    QDisplay_NewFile();
    QDisplay_LoadFile();
    QDisplay_SelectCanvasOpen();

    // Render additional windows
    m_configureModelWindow->render();
    m_importModelWindow->render();
    m_importVAEWindow->render();
    m_loadModelWindow->render();
  }
};