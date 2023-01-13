#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>

#include "../../../Display/ErrorHandler.h"
#include "../../../QLogger.h"
#include "../../../Rendering/RenderManager.h"
#include "../../../Config/config.h"
#include "../../QDisplay_Base.h"

namespace fs = std::filesystem;

class QDisplay_Text2Image : public QDisplay_Base {

  // Window variables & flags
  char *m_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT.get()]();
  char *m_negative_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT.get()]();
  int m_width = 512;
  int m_height = 512;
  int m_steps = 70;
  int m_seed = 0;
  double m_cfg = 7.5;
  bool m_half_precision = false;

  std::string m_selected_model = "";
  std::string m_ckpt_path = "";
  std::vector<listItem> m_ckpt_files;
  std::unique_ptr<Image> m_image = 0;
  bool finishedRendering = false;

public:
  // Initialise render manager references
  QDisplay_Text2Image(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_prompt[0] = 0;
    m_negative_prompt[0] = 0;
    reloadModelFiles();
  }

  void reloadModelFiles() {
    // Load model files
    try {
      for (const auto &entry : fs::directory_iterator(CONFIG::MODELS_DIRECTORY.get())) {
        listItem i{.m_name = entry.path().filename().string()};
        m_ckpt_files.push_back(i);
      }
    } catch (fs::filesystem_error) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::MODELS_DIRECTORY, "MODELS_DIRECTORY");
    }
  }

  std::string getLatestFile() {
    std::string outfile = "";
    fs::file_time_type write_time;

    try {
      for (const auto &entry : fs::directory_iterator("data" + CONFIG::OUTPUT_DIRECTORY.get() + "/txt2img")) {
        if (entry.is_regular_file()) {
          outfile = entry.path().string();
        }
      }
    } catch (fs::filesystem_error) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::OUTPUT_DIRECTORY, "OUTPUT_DIRECTORY");
    }

    return outfile;
  }

  void renderImage() {
    m_image.reset();
    m_image = std::unique_ptr<Image>(
        new Image(CONFIG::IMAGE_SIZE_X_LIMIT.get(), CONFIG::IMAGE_SIZE_Y_LIMIT.get(), "txt2img"));
    m_image->rendered = false;
    m_renderManager->textToImage(m_prompt, m_negative_prompt, 1, m_steps, m_cfg, m_seed, m_width, m_height,
                                 m_image->rendered, m_selected_model, m_half_precision);
  }

  void imageWindow() {
    ImGui::BeginChild("GL");
    if (m_image) {

      // Generate option only available whilst a image isn't pending
      if (m_image->rendered) {
        if (ImGui::Button("Generate")) {
          renderImage();
        }
      } else {
        ImGui::Text("Generating image...");
      }

      // Once image is marked as rendered display on screen
      if (m_image->rendered) {
        ImGui::Text("image width: %d image height:%d", m_image->m_width, m_image->m_height);
        if (ImGui::Button("Send to Canvas")) {
          // Send image to be rendered on canvas at selection coordinates
          m_renderManager->sendImageToCanvas(*m_image);
        }

        // Retrieve texture file
        if (!m_image->textured) {
          m_image->loadFromImage(getLatestFile());
          m_image->textured = true;
        }

        ImGui::Image((void *)(intptr_t)m_image->m_texture, ImVec2(m_image->m_width * 0.3, m_image->m_height * 0.3));
      }
    } else {
      if (ImGui::Button("Generate")) {
        renderImage();
      }
    }

    ImGui::EndChild();
  }

  // Prompt
  void promptHelper() {
    ImGui::BeginChild("Prompt Helper");
    ImGui::InputTextMultiline("prompt", m_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
    ImGui::InputTextMultiline("negative prompt", m_negative_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
    ImGui::EndChild();
  }

  void promptConfig() {
    ImGui::BeginChild("Prompt Config");

    ImGui::Checkbox("half precision", &m_half_precision);

    // Width control
    ImGui::SliderInt("width", &m_width, 1, CONFIG::IMAGE_SIZE_X_LIMIT.get());
    if (ImGui::BeginPopupContextItem("width")) {
      ImGui::InputInt("value", &m_width);
      if (ImGui::MenuItem("Reset to default: 512"))
        m_width = 512;
      ImGui::EndPopup();
    }

    // Height control
    ImGui::SliderInt("height", &m_height, 1, CONFIG::IMAGE_SIZE_Y_LIMIT.get());
    if (ImGui::BeginPopupContextItem("height")) {
      ImGui::InputInt("value", &m_height);
      if (ImGui::MenuItem("Reset to default: 512"))
        m_height = 512;
      ImGui::EndPopup();
    }

    ImGui::InputInt("steps", &m_steps);
    ImGui::InputInt("seed", &m_seed);
    ImGui::InputDouble("cfg scale", &m_cfg, 0.1);

    if (ImGui::BeginCombo("models", m_selected_model.c_str(), ImGuiComboFlags_NoArrowButton)) {
      for (auto &item : m_ckpt_files) {
        if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
          m_selected_model = item.m_name;
        }
        if (item.m_isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    if (ImGui::Button("Reload Models")) {
      reloadModelFiles();
    }

    ImGui::EndChild();
  }

  virtual void render() {
    ImGui::Columns(3);
    ImGui::SetColumnOffset(1, 420.0f);
    ImGui::SetColumnOffset(2, 740.0f);
    { promptHelper(); }
    ImGui::NextColumn();
    { promptConfig(); }
    ImGui::NextColumn();
    { imageWindow(); }
  }
};