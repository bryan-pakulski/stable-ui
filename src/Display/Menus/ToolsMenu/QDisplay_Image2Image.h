#pragma once

#include <fstream>
#include <imgui.h>
#include <filesystem>

#include "../../../Display/ErrorHandler.h"
#include "../../../QLogger.h"
#include "../../../Rendering/StableManager.h"
#include "../../../Config/config.h"
#include "../../QDisplay_Base.h"
#include "../../../Rendering/objects/image/Image.h"

namespace fs = std::filesystem;

class QDisplay_Image2Image : public QDisplay_Base {

  // Window variables & flags
  char *m_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT.get()]();
  char *m_negative_prompt = new char[CONFIG::PROMPT_LENGTH_LIMIT.get()]();
  int m_steps = 70;
  int m_seed = 0;
  float m_strength = 0.5;
  double m_cfg = 7.5;

  std::string m_selectedSampler = "DDIM";
  std::vector<listItem> m_samplerList;

  std::unique_ptr<Image> m_image = 0;
  std::unique_ptr<Image> m_image_mask = 0;

public:
  // Initialise render manager references
  QDisplay_Image2Image(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_prompt[0] = 0;
    m_negative_prompt[0] = 0;

    // Sampler menu
    listItem i{.m_name = "DDIM"};
    m_samplerList.push_back(i);
  }

  std::string getLatestFile() {
    std::string outfile = "";

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
        new Image(CONFIG::IMAGE_SIZE_X_LIMIT.get(), CONFIG::IMAGE_SIZE_Y_LIMIT.get(), "img2img"));
    std::string path = m_stableManager->getImage();
    m_stableManager->imageToImage(path, m_prompt, m_negative_prompt, m_selectedSampler, 1, m_steps, m_cfg, m_strength,
                                  m_seed, m_image->renderState);
  }

  void imageWindow() {
    if (ImGui::CollapsingHeader("Render")) {
      if (m_image) {
        // Once image is marked as rendered display on screen
        if (m_image->renderState == Q_EXECUTION_STATE::SUCCESS) {
          ImGui::Text("image width: %d image height:%d", m_image->m_width, m_image->m_height);
          if (ImGui::Button("Send to Canvas")) {
            // Send image to be rendered on canvas at selection coordinates
            m_stableManager->sendImageToCanvas(*m_image);
          }

          // Retrieve latest redered file
          if (!m_image->textured == Q_EXECUTION_STATE::SUCCESS) {
            m_image->loadFromImage(getLatestFile());
            m_image->textured = Q_EXECUTION_STATE::SUCCESS;
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
      ImGui::InputTextMultiline("##negative prompt", m_negative_prompt, CONFIG::PROMPT_LENGTH_LIMIT.get());
    }
  }

  void promptConfig() {
    if (ImGui::CollapsingHeader("Gen Config")) {

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
      ImGui::SliderFloat("Strength", &m_strength, 0.0, 1.0, "%.2f");
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