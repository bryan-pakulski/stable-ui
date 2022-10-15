#pragma once

#include <fstream>
#include <imgui.h>

#include "../../Display/ErrorHandler.h"
#include "../../QLogger.h"
#include "../../Rendering/RenderManager.h"
#include "../../Rendering/objects/Helper.h"
#include "../../config.h"
#include "../QDisplay_Base.h"

class QDisplay_MainWindow : public QDisplay_Base {
  GLuint m_image_texture = 0;
  int m_image_width = 0;
  int m_image_height = 0;

  char m_prompt[CONFIG::PROMPT_LENGTH_LIMIT];

public:
  // Initialise render manager reference
  QDisplay_MainWindow(RenderManager *rm) : QDisplay_Base(rm) {}

  void promptHelper() {
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::Begin("Prompt Designer");
    ImGui::InputText("prompt", m_prompt, CONFIG::PROMPT_LENGTH_LIMIT);
    ImGui::End();
  }

  virtual void render() {

    promptHelper();

    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::Begin("Paint Studio");

    if (m_image_texture) {
      ImGui::Text("size = %d x %d", m_image_width, m_image_height);
      ImGui::Image((void *)(intptr_t)m_image_texture,
                   ImVec2(m_image_width, m_image_height));
    }

    if (ImGui::Button("Load")) {

      bool ret = LoadTextureFromFile("test.jpg", &m_image_texture,
                                     &m_image_width, &m_image_height);
      if (!ret) {
        ErrorHandler::GetInstance().setError("Failed to load texture file");
        QLogger::GetInstance().Log(LOGLEVEL::ERR,
                                   "Failed to load texture file");
      } else {
        QLogger::GetInstance().Log(LOGLEVEL::INFO, "loaded texture file");
      }
    }

    if (ImGui::Button("Save")) {
    }

    ImGui::End();
  }
};