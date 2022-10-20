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
  Canvas *m_canvas = 0;
  bool finishedRendering = false;

public:
  // Initialise render manager references
  QDisplay_MainWindow(RenderManager *rm) : QDisplay_Base(rm) {}

  void canvasWindow() {
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::Begin("Canvas");

    if (!m_canvas) {
      ImGui::TextUnformatted("Generate Text to image here, right click to copy contents "
                             "into selection box");
    }

    if (m_canvas) {
      ImGui::Text("canvas width: %d canvas height:%d", m_canvas->c_width, m_canvas->c_height);
      ImGui::Image((void *)(intptr_t)m_canvas->m_canvas, ImVec2(m_canvas->c_width, m_canvas->c_height));
    } else {
      ImGui::Text("...");
    }

    // Generate new canvas
    if (ImGui::Button("Generate")) {
      delete m_canvas;
      finishedRendering = false;
      m_canvas = new Canvas(CONFIG::CANVAS_SIZE_X_LIMIT, CONFIG::CANVAS_SIZE_Y_LIMIT, "generated");
      m_renderManager->textToImage(*m_canvas, "A cat with a hat", 1, 80, 1234567, 512, 512, finishedRendering);
    }

    // TODO: loading bar, once finished load image into GLuint
    if (finishedRendering) {
      ImGui::Text("Canvas here..");
    } else {
      ImGui::Text("Please wait while rendering completes...");
    }

    ImGui::End();
  }

  void promptHelper() {
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::Begin("Prompt Designer");
    ImGui::InputText("prompt", m_prompt, CONFIG::PROMPT_LENGTH_LIMIT);
    ImGui::End();
  }

  virtual void render() {

    canvasWindow();
    promptHelper();
  }
};