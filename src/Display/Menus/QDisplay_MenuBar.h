#pragma once

#include <fstream>
#include <imgui.h>

#include "../../Display/ErrorHandler.h"
#include "../../QLogger.h"
#include "../../Rendering/Helper.h"
#include "../QDisplay_Base.h"

class QDisplay_MenuBar : public QDisplay_Base {

private:
  // Window triggers
  bool newFileOpen = false;
  bool logFileOpen = false;
  bool loadFileOpen = false;
  bool selectCanvasOpen = false;
  bool layerListOpen = false;

  // Layer list config
  int m_selectedLayerIndex = -1;

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
        ImGui::SetScrollY(ImGui::GetScrollMaxY() + ImGui::GetStyle().ItemSpacing.y * 2);
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

        ImGui::InputText("canvas name", m_canvasName, 256);

        if (ImGui::Button("Create Canvas")) {
          m_renderManager->createCanvas(0, 0, std::string(m_canvasName));
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
          for (auto &item : m_renderManager->m_canvas) {
            const char *item_name = item->m_name.c_str();
            int index = std::addressof(item) - std::addressof(m_renderManager->m_canvas.front());
            const bool is_selected = index == m_renderManager->m_activeId;

            if (ImGui::Selectable(item_name, is_selected)) {
              m_renderManager->selectCanvas(index);
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

  // Lists all images applied to canvas, allows details modification i.e. coordinates, rotation etc...
  void QDisplay_LayerList() {
    if (layerListOpen) {
      ImGui::SetNextWindowBgAlpha(0.9f);
      ImGui::Begin("Layer Manager");

      if (ImGui::BeginListBox("Layers")) {
        for (auto &item : m_renderManager->getActiveCanvas()->m_editorGrid) {
          const char *item_name = item->m_image->m_image_source.c_str();
          int index = std::addressof(item) - std::addressof(m_renderManager->getActiveCanvas()->m_editorGrid.front());
          const bool is_selected = index == m_selectedLayerIndex;

          // TODO: add visible / hidden button for each row, rendering code to be updated to support hidden flag

          if (ImGui::Selectable(item_name, is_selected)) {
            m_selectedLayerIndex = index;
          }

          // Set the initial focus when opening the combo (scrolling +
          // keyboard navigation focus)
          if (is_selected) {
            ImGui::SetItemDefaultFocus();
          }
        }

        ImGui::EndListBox();
      }

      if (m_selectedLayerIndex != -1) {
        ImGui::Text("Layer Tools");


        if (ImGui::Button("Delete Layer")) {
          m_renderManager->getActiveCanvas()->deleteChunk(m_selectedLayerIndex);
          m_selectedLayerIndex = -1;
        }
      }

      if (ImGui::Button("Close")) {
        m_selectedLayerIndex = 0;
        layerListOpen = false;
      }

      ImGui::End();
    }
  }

public:
  // Initialise render manager reference
  QDisplay_MenuBar(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {}

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

        if (ImGui::MenuItem("Load File")) {
          loadFileOpen = true;
        }

        if (ImGui::MenuItem("Open Log")) {
          logFileOpen = true;
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {

        if (ImGui::MenuItem("Select Canvas")) {
          selectCanvasOpen = true;
        }

        if (ImGui::MenuItem("View Layers")) {
          layerListOpen = true;
        }

        ImGui::EndMenu();
      }

      ImGui::MenuItem(m_renderManager->getActiveCanvas()->m_name.c_str());

      ImGui::EndMainMenuBar();
    }

    // These will only render if their corresponding flags are set
    QDisplay_LogFile();
    QDisplay_NewFile();
    QDisplay_LoadFile();
    QDisplay_SelectCanvasOpen();
    QDisplay_LayerList();
  }
};