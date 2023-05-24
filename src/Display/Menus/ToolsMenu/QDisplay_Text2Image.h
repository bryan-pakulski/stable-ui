#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>

#include "Display/ErrorHandler.h"
#include "QLogger.h"
#include "Rendering/StableManager.h"
#include "Config/config.h"
#include "Display/QDisplay_Base.h"

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
  int m_renderState = Q_EXECUTION_STATE::PENDING;

public:
  // Initialise render manager references
  QDisplay_Text2Image(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_prompt[0] = 0;
    m_negative_prompt[0] = 0;

    listItem i{.m_name = "DDIM"};
    listItem j{.m_name = "PLMS"};
    listItem k{.m_name = "UNIPC"};
    m_samplerList.push_back(i);
    m_samplerList.push_back(j);
    m_samplerList.push_back(k);
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
    } catch (const fs::filesystem_error &err) {
      ErrorHandler::GetInstance().setConfigError(CONFIG::OUTPUT_DIRECTORY, "OUTPUT_DIRECTORY");
      QLogger::GetInstance().Log(LOGLEVEL::ERR, err.what());
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
    if (ImGui::CollapsingHeader("Preview")) {
      if (m_image) {
        // Once image is marked as rendered display on screen
        if (m_image->renderState == Q_EXECUTION_STATE::SUCCESS) {
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
      }
    }
  }

  // Prompt
  void promptHelper() {
    if (ImGui::CollapsingHeader("Prompts")) {
      ImGui::Text("Prompt");
      ImGui::InputTextMultiline("##prompt", m_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
      ImGui::Text("Negative Prompt");
      ImGui::InputTextMultiline("##neg prompt", m_negative_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
    }
  }

  void promptConfig() {
    if (ImGui::CollapsingHeader("Gen Config")) {
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
    }
  }

  virtual void render() {

    // Generate option only available whilst a image isn't pending
    if ((m_image && m_image->renderState != Q_EXECUTION_STATE::LOADING) || !m_image) {
      static const ImVec4 currentColor{0, 0.5f, 0, 1.0f};

      ImGui::PushStyleColor(ImGuiCol_Button, currentColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, currentColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentColor);

      if (ImGui::Button("Generate", ImVec2(150, 40))) {
        renderImage();
      }
      ImGui::PopStyleColor(3);
    } else if (m_image && m_image->renderState == Q_EXECUTION_STATE::LOADING) {
      static const ImVec4 currentColor{0.5f, 0, 0, 1.0f};
      ImGui::PushStyleColor(ImGuiCol_Button, currentColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, currentColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentColor);

      if (ImGui::Button("Cancel", ImVec2(150, 40))) {
        // TODO: cut render short?
      }
      ImGui::PopStyleColor(3);
    }

    promptHelper();
    promptConfig();
    imageWindow();
  }
};