#pragma once

#include <fstream>
#include <imgui.h>

#include "../../QLogger.h"
#include "../../Rendering/RenderManager.h"

/*
 * TXT 2 IMG Renderer
 *
 */
inline void QDisplay_Img2Img() {

  ImGui::SetNextWindowBgAlpha(0.9f);
  ImGui::Begin("Image to Image");

  ImGui::TextUnformatted("Text Prompt:");

  if (ImGui::Button("Generate")) {
  }

  ImGui::End();
}