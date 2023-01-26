#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>

#include "../../../Display/ErrorHandler.h"
#include "../../../QLogger.h"
#include "../../../Rendering/StableManager.h"
#include "../../../Config/config.h"
#include "../../QDisplay_Base.h"

namespace fs = std::filesystem;

class QDisplay_Text2Image : public QDisplay_Base {

  // Window variables & flags
  char *m_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT.get()]();
  char *m_negative_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT.get()]();
  std::string m_selectedSampler = "DDIM";
  std::vector<listItem> m_samplerList;
  int m_width = 512;
  int m_height = 512;
  int m_steps = 70;
  int m_seed = 0;
  double m_cfg = 7.5;

  std::unique_ptr<Image> m_image = 0;
  int m_renderState = EXECUTION_STATE::PENDING;

public:
  // Initialise render manager references
  QDisplay_Text2Image(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_prompt[0] = 0;
    m_negative_prompt[0] = 0;

    listItem i{.m_name = "DDIM"};
    listItem j{.m_name = "PLMS"};
    m_samplerList.push_back(i);
    m_samplerList.push_back(j);
  }

  std::string getLatestFile() {
    std::string outfile = "";
    fs::file_time_type write_time;

    try {
      for (const auto &entry : fs::directory_iterator("data" + CONFIG::OUTPUT_DIRECTORY.get() + "/" +
                                                      m_stableManager->getActiveCanvas()->m_name)) {
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
    m_stableManager->textToImage(m_prompt, m_negative_prompt, m_selectedSampler, 1, m_steps, m_cfg, m_seed, m_width,
                                 m_height, m_image->renderState);
  }

  void imageWindow() {
    ImGui::BeginChild("GL");
    if (m_image) {
      // Generate option only available whilst a image isn't pending
      if (m_image->renderState != EXECUTION_STATE::LOADING) {
        if (ImGui::Button("Generate")) {
          renderImage();
        }
      } else {
        ImGui::Text("Generating image...");
      }

      // Once image is marked as rendered display on screen
      if (m_image->renderState == EXECUTION_STATE::SUCCESS) {
        ImGui::Text("image width: %d image height:%d", m_image->m_width, m_image->m_height);
        if (ImGui::Button("Send to Canvas")) {
          // Send image to be rendered on canvas at selection coordinates
          m_stableManager->sendImageToCanvas(*m_image);
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
    ImGui::BeginChild("Prompt");
    ImGui::InputTextMultiline("prompt", m_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
    ImGui::EndChild();

    ImGui::NextColumn();

    ImGui::BeginChild("Negative Prompt");
    ImGui::InputTextMultiline("negative prompt", m_negative_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
    ImGui::EndChild();
  }

  void promptConfig() {
    ImGui::BeginChild("Prompt Config");
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

    if (ImGui::BeginCombo("Sampler", m_selectedSampler.c_str(), ImGuiComboFlags_NoArrowButton)) {
      for (auto &item : m_samplerList) {
        if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
          m_selectedSampler = item.m_name;
        }
        if (item.m_isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    ImGui::InputInt("steps", &m_steps);
    ImGui::InputInt("seed", &m_seed);
    ImGui::InputDouble("cfg scale", &m_cfg, 0.1);

    ImGui::EndChild();
  }

  virtual void render() {
    ImGui::Columns(4);
    ImGui::SetColumnOffset(1, 320.0f);
    ImGui::SetColumnOffset(2, 640.0f);
    ImGui::SetColumnOffset(3, 960.0f);
    { promptHelper(); }
    ImGui::NextColumn();
    { promptConfig(); }
    ImGui::NextColumn();
    { imageWindow(); }
  }
};