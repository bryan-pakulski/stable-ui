#pragma once

#include "Config/config.h"
#include "Helpers/QLogger.h"
#include "Helpers/GLHelper.h"
#include "Client/Heartbeat.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"

#include "QDisplay_SaveFile.h"
#include "QDisplay_LoadFile.h"
#include "QDisplay_ConfigureModel.h"
#include "QDisplay_ImportModel.h"
#include "QDisplay_ImportVAE.h"
#include "QDisplay_LoadModel.h"
#include "QDisplay_PluginsWindow.h"
#include "QDisplay_ContentBrowser.h"
#include "QDisplay_NewCanvas.h"
#include "QDisplay_LogWindow.h"

#include "StableManager.h"

#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <memory>

class QDisplay_TopBar : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_TopBar(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_saveFileWindow = std::unique_ptr<QDisplay_SaveFile>(new QDisplay_SaveFile(rm, w));
    m_loadFileWindow = std::unique_ptr<QDisplay_LoadFile>(new QDisplay_LoadFile(rm, w));
    m_configureModelWindow = std::unique_ptr<QDisplay_ConfigureModel>(new QDisplay_ConfigureModel(rm, w));
    m_importModelWindow = std::unique_ptr<QDisplay_ImportModel>(new QDisplay_ImportModel(rm, w));
    m_importVAEWindow = std::unique_ptr<QDisplay_ImportVAE>(new QDisplay_ImportVAE(rm, w));
    m_loadModelWindow = std::unique_ptr<QDisplay_LoadModel>(new QDisplay_LoadModel(rm, w));
    m_pluginsWindow = std::unique_ptr<QDisplay_PluginsWindow>(new QDisplay_PluginsWindow(rm, w));
    m_contentBrowserWindow = std::unique_ptr<QDisplay_ContentBrowser>(new QDisplay_ContentBrowser(rm, w));
    m_newCanvasPopup = std::unique_ptr<QDisplay_NewCanvas>(new QDisplay_NewCanvas(rm, w));

    // Load images
    m_docker_connected_icon = std::unique_ptr<GLImage>(new GLImage(32, 32, "connected_icon"));
    m_docker_disconnected_icon = std::unique_ptr<GLImage>(new GLImage(32, 32, "disconnected_icon"));

    m_docker_connected_icon->loadFromImage("data/images/connected.png");
    m_docker_disconnected_icon->loadFromImage("data/images/disconnected.png");
  }

  /*
   * Main Menu renderer, contains logic for showing additional display items
   */
  virtual void render() {
    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::BeginMenu("File")) {

        if (ImGui::MenuItem("New Project")) {
          m_newCanvasPopup->openWindow();
        }

        if (ImGui::MenuItem("Save Project")) {
          m_saveFileWindow->openWindow();
        }

        if (ImGui::MenuItem("Load Project")) {
          m_loadFileWindow->openWindow();
        }

        if (ImGui::MenuItem("Content Browser")) {
          m_contentBrowserWindow->m_isOpen = true;
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Model")) {

        if (ImGui::MenuItem("Import New Model")) {
          m_importModelWindow->openWindow();
        }

        if (ImGui::MenuItem("Import New VAE")) {
          m_importVAEWindow->openWindow();
        }

        if (ImGui::MenuItem("Load Model To Memory")) {
          m_loadModelWindow->openWindow();
        }

        if (ImGui::MenuItem("Configure Models")) {
          m_configureModelWindow->openWindow();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Plugins")) {
        m_pluginsWindow->menus();

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Debug")) {

        if (ImGui::MenuItem("Release model from memory")) {
          StableManager::GetInstance().releaseSDModel();
        }

        if (ImGui::MenuItem("Restart SD Server")) {
          StableManager::GetInstance().launchSDModelServer();
        }

        if (ImGui::MenuItem("Open Log")) {
          logFileOpen = true;
        }

        ImGui::EndMenu();
      }

      ImGui::Separator();
      if (ImGui::MenuItem(std::string("Project - " + m_renderManager->getActiveCanvas()->m_name).c_str())) {
        selectCanvasOpen = true;
      }

      // Docker connection state
      ImGui::Separator();
      {
        GLImage icon = Heartbeat::GetInstance().getState() ? *m_docker_connected_icon : *m_docker_disconnected_icon;
        ImGui::Image((void *)(intptr_t)icon.m_texture, {c_dockerIconSize, c_dockerIconSize}, {1, 0}, {0, 1});
        std::string dockerState = Heartbeat::GetInstance().getState() ? "Docker Connected" : "Docker Disconnected";
        ImGui::MenuItem(dockerState.c_str());
      }

      ImGui::EndMainMenuBar();
    }

    // These will only render if their corresponding flags are set
    QDisplay_LogFile();
    QDisplay_SelectCanvasOpen();

    // Render additional windows
    m_saveFileWindow->render();
    m_loadFileWindow->render();
    m_configureModelWindow->render();
    m_importModelWindow->render();
    m_importVAEWindow->render();
    m_loadModelWindow->render();
    m_pluginsWindow->render();
    m_contentBrowserWindow->render();
    m_newCanvasPopup->render();
  }

private:
  // Window triggers
  bool logFileOpen = false;
  bool selectCanvasOpen = false;

  std::unique_ptr<QDisplay_SaveFile> m_saveFileWindow;
  std::unique_ptr<QDisplay_LoadFile> m_loadFileWindow;
  std::unique_ptr<QDisplay_NewCanvas> m_newCanvasPopup;
  std::unique_ptr<QDisplay_ConfigureModel> m_configureModelWindow;
  std::unique_ptr<QDisplay_ImportModel> m_importModelWindow;
  std::unique_ptr<QDisplay_ImportVAE> m_importVAEWindow;
  std::unique_ptr<QDisplay_LoadModel> m_loadModelWindow;
  std::unique_ptr<QDisplay_PluginsWindow> m_pluginsWindow;
  std::unique_ptr<QDisplay_ContentBrowser> m_contentBrowserWindow;

  // Docker status icons
  std::unique_ptr<GLImage> m_docker_connected_icon;
  std::unique_ptr<GLImage> m_docker_disconnected_icon;
  float c_dockerIconSize = 15.0f;

  // Log config
  std::ifstream logStream;
  std::stringstream logFileBuffer;
  bool logUpdated = true;
  clock_t lastLogReadTime;

  // New file config
  std::string m_canvasName = "new";

private:
  /*
   * Popup for displaying log file output
   */
  void QDisplay_LogFile() {

    if (logFileOpen) {
      ImGui::SetNextWindowBgAlpha(0.9f);
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
        logStream.open(CONFIG::LOG_FILE.get(), std::ios::in);

        logFileBuffer.clear();
        logFileBuffer.str(std::string());

        lastLogReadTime = QLogger::GetInstance().m_LAST_WRITE_TIME;
        logFileBuffer << logStream.rdbuf();
        logStream.close();
        logUpdated = true;
      }

      // ImGui::TextWrapped("%s", logFileBuffer.str().c_str());
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

  void QDisplay_SelectCanvasOpen() {
    if (selectCanvasOpen) {

      ImGui::OpenPopup("Select Canvas");
      if (ImGui::BeginPopupModal("Select Canvas")) {
        if (ImGui::BeginListBox("Canvas")) {
          for (auto &item : m_renderManager->m_canvas) {
            const char *item_name = item->m_name.c_str();
            int index = std::addressof(item) - std::addressof(m_renderManager->m_canvas.front());
            const bool is_selected = index == m_renderManager->m_activeId;

            if (ImGui::Selectable(item_name, is_selected)) {
              m_renderManager->selectCanvas(index);
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

      ImGui::Separator();
      ImGui::Text("Configuration: ");
      ImGui::InputText("Project Name:", &m_renderManager->getActiveCanvas()->m_name);
      ImGui::Separator();

      if (ImGui::Button("okay")) {
        selectCanvasOpen = false;
      }
      ImGui::SameLine();
      if (ImGui::Button("close")) {
        selectCanvasOpen = false;
      }
      ImGui::EndPopup();
    }
  }
};