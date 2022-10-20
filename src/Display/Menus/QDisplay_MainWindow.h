#pragma once

#include <fstream>
#include <imgui.h>

#include "../../Display/ErrorHandler.h"
#include "../../QLogger.h"
#include "../../Rendering/RenderManager.h"
#include "../../config.h"
#include "../QDisplay_Base.h"

class QDisplay_MainWindow : public QDisplay_Base {
  // Initialise empty prompt
  char *m_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT]();
  int m_width = 512;
  int m_height = 512;
  int m_steps = 50;
  int m_seed = 0;

  Canvas *m_canvas = 0;
  bool finishedRendering = false;

public:
  // Initialise render manager references
  QDisplay_MainWindow(RenderManager *rm) : QDisplay_Base(rm) {
    m_prompt[0] = 0;
  }

  void renderCanvas() {
    delete m_canvas;
    m_canvas = new Canvas(CONFIG::CANVAS_SIZE_X_LIMIT, CONFIG::CANVAS_SIZE_Y_LIMIT, "generated");
    m_canvas->rendered = false;
    m_renderManager->textToImage(*m_canvas, m_prompt, 1, m_steps, m_seed, m_width, m_height, m_canvas->rendered);
  }

  void canvasWindow() {
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::Begin("Canvas");

    if (m_canvas) {

      // Generate option only available whilst a canvas isn't pending
      if (m_canvas->rendered) {
        if (ImGui::Button("Generate")) {
          renderCanvas();
        }
      }

      // Once canvas is marked as rendered display on screen
      if (m_canvas->rendered) {
        ImGui::Text("canvas width: %d canvas height:%d", m_canvas->m_width, m_canvas->m_height);
        ImGui::Image((void *)(intptr_t)m_canvas->m_canvas, ImVec2(m_canvas->m_width, m_canvas->m_height));
      }
    } else {
      if (ImGui::Button("Generate")) {
        renderCanvas();
      }
    }
    ImGui::End();
  }

  void promptHelper() {
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::Begin("Prompt Designer");
    ImGui::InputTextMultiline("prompt", m_prompt, CONFIG::PROMPT_LENGTH_LIMIT);
    ImGui::SliderInt("width", &m_width, 0, CONFIG::CANVAS_SIZE_X_LIMIT);
    ImGui::SliderInt("height", &m_height, 0, CONFIG::CANVAS_SIZE_Y_LIMIT);
    ImGui::InputInt("steps", &m_steps);
    ImGui::InputInt("seed", &m_seed);

    ImGui::End();
  }

  virtual void render() {
    canvasWindow();
    promptHelper();
  }
};