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

  bool m_draw_mask_window = false;
  std::unique_ptr<Image> m_image = 0;
  std::unique_ptr<Image> m_image_mask = 0;
  bool finishedRendering = false;

  // Content Browser Config
  const char *c_base_content_directory = "data/output";
  std::filesystem::path m_current_directory;
  std::filesystem::path m_selected_file;
  std::string m_filepath;
  std::unique_ptr<Image> m_directory_icon;
  std::unique_ptr<Image> m_file_icon;
  std::unique_ptr<Image> m_preview_image;

public:
  // Initialise render manager references
  QDisplay_Image2Image(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_prompt[0] = 0;
    m_negative_prompt[0] = 0;

    // Initialise content browser
    m_directory_icon = std::unique_ptr<Image>(new Image(256, 256, "dir_icon"));
    m_file_icon = std::unique_ptr<Image>(new Image(256, 256, "file_icon"));
    m_directory_icon->loadFromImage("data/images/directory_icon.png");
    m_file_icon->loadFromImage("data/images/file_icon.png");
    m_current_directory = std::filesystem::path(c_base_content_directory);

    // Sampler menu
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
        new Image(CONFIG::IMAGE_SIZE_X_LIMIT.get(), CONFIG::IMAGE_SIZE_Y_LIMIT.get(), "img2img"));
    m_stableManager->imageToImage(m_filepath, m_prompt, m_negative_prompt, m_selectedSampler, 1, m_steps, m_cfg,
                                  m_strength, m_seed, m_image->renderState);
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
        if (!m_image->textured == EXECUTION_STATE::SUCCESS) {
          m_image->loadFromImage(getLatestFile());
          m_image->textured = EXECUTION_STATE::SUCCESS;
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

  void selectImage(const std::filesystem::path &path) {
    m_selected_file = path.filename();

    // Create new texture
    m_preview_image.reset();
    m_preview_image = std::unique_ptr<Image>(new Image(512, 512, "preview"));
    m_preview_image->loadFromImage(m_current_directory.string() + "/" + m_selected_file.string());

    // Path must be relative to docker, remove local data prefix
    m_filepath = m_current_directory.string() + "/" + m_selected_file.string();
    m_filepath.replace(m_filepath.find("data"), sizeof("data") - 1, "");

    m_preview_image->textured = true;
  }

  // Drag drop window
  void contentBrowser() {
    ImGui::BeginChild("Content Browser");
    ImGui::Text("Content Browser");

    if (m_current_directory != std::filesystem::path(c_base_content_directory)) {
      if (ImGui::Button("<-")) {
        m_current_directory = m_current_directory.parent_path();
      }
    }

    static float padding = 6.0f;
    static float thumbnailSize = 64.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1)
      columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    for (auto &directoryEntry : std::filesystem::directory_iterator(m_current_directory)) {
      const auto &path = directoryEntry.path();

      // skip this item if it's not an image or directory
      if (!directoryEntry.is_directory() && (path.extension() != ".png" && path.extension() != ".jpg")) {
        continue;
      }

      std::string filenameString = path.filename().string();

      ImGui::PushID(filenameString.c_str());
      Image icon = directoryEntry.is_directory() ? *m_directory_icon : *m_file_icon;

      // Create texture
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
      ImGui::ImageButton((void *)(intptr_t)icon.m_texture, {thumbnailSize, thumbnailSize}, {1, 0}, {0, 1});
      ImGui::PopStyleColor();

      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (directoryEntry.is_directory()) {
          m_current_directory /= path.filename();
        } else {
          // Send image to be rendered on canvas at selection coordinates
          selectImage(path);
          m_stableManager->sendImageToCanvas(*m_preview_image);
        }
      }
      if (!directoryEntry.is_directory() && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        selectImage(path);
      }

      ImGui::TextWrapped("%s", filenameString.c_str());

      ImGui::NextColumn();
      ImGui::PopID();
    }

    ImGui::Columns(1);

    ImGui::EndChild();
  }

  void drawMaskWindow() {
    if (m_draw_mask_window) {
      ImGui::Begin("GLMask");
      ImGui::Text("Masking");

      // We will draw over the top of this with our mask
      if (!m_selected_file.empty() && m_preview_image->textured) {
        ImGui::Image((void *)(intptr_t)m_preview_image->m_texture,
                     ImVec2(m_preview_image->m_width, m_preview_image->m_height));
      }

      // Capture mouse input???
      m_image_mask->drawMaskToTexture(1, 1, 4.0f);

      if (ImGui::Button("Save Mask")) {
        m_draw_mask_window = false;
      }
      ImGui::SameLine();
      if (ImGui::Button("Reset Mask")) {
        m_image_mask.reset();
        m_image_mask =
            std::unique_ptr<Image>(new Image(m_preview_image->m_width, m_preview_image->m_height, "image_mask"));
      }
      ImGui::SameLine();
      if (ImGui::Button("Close")) {
        m_draw_mask_window = false;
      }
      ImGui::End();
    }
  }

  // Preview window for files selected in content browser
  void previewWindow() {
    ImGui::BeginChild("Browser Preview");
    ImGui::Text("Preview");
    if (!m_selected_file.empty() && m_preview_image->textured) {
      if (ImGui::Button("Draw Mask")) {
        if (!m_image_mask) {
          m_image_mask =
              std::unique_ptr<Image>(new Image(m_preview_image->m_width, m_preview_image->m_height, "image_mask"));
        }
        m_draw_mask_window = true;
      }

      ImGui::Image((void *)(intptr_t)m_preview_image->m_texture,
                   ImVec2(m_preview_image->m_width * 0.4, m_preview_image->m_height * 0.4));
    }
    ImGui::EndChild();
  }

  // Mask preview
  void maskPreviewWindow() {
    ImGui::BeginChild("Mask Preview");
    ImGui::Text("Mask Preview");

    if (m_image_mask) {
      if (ImGui::Button("Delete Mask")) {
        m_image_mask.reset();
      } else {
        ImGui::Image((void *)(intptr_t)m_image_mask->m_texture,
                     ImVec2(m_image_mask->m_width * 0.4, m_image_mask->m_height * 0.4));
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
    ImGui::Text("Prompt Config");

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
    ImGui::EndChild();
  }

  virtual void render() {
    ImGui::Columns(7);
    ImGui::SetColumnOffset(1, 320.0f);
    ImGui::SetColumnOffset(2, 640.0f);
    ImGui::SetColumnOffset(3, 960.0f);
    { contentBrowser(); }
    ImGui::NextColumn();
    { previewWindow(); }
    ImGui::NextColumn();
    { maskPreviewWindow(); }
    ImGui::NextColumn();
    { promptHelper(); }
    ImGui::NextColumn();
    { promptConfig(); }
    ImGui::NextColumn();
    { imageWindow(); }

    // Only render if flags are enabled
    drawMaskWindow();
  }
};