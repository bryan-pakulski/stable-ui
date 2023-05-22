#pragma once

#include <imgui.h>
#include <filesystem>

#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include <imgui_stdlib.h>

class QDisplay_ContentBrowser : public QDisplay_Base {

private:
  // Content Browser Config
  const char *c_base_content_directory = "data/output";
  std::filesystem::path m_current_directory;
  std::filesystem::path m_selected_file;
  std::string m_filepath;
  std::unique_ptr<Image> m_directory_icon;
  std::unique_ptr<Image> m_file_icon;
  std::unique_ptr<Image> m_preview_image;

public:
  QDisplay_ContentBrowser(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    // Initialise content browser
    m_directory_icon = std::unique_ptr<Image>(new Image(256, 256, "dir_icon"));
    m_file_icon = std::unique_ptr<Image>(new Image(256, 256, "file_icon"));
    m_directory_icon->loadFromImage("data/images/directory_icon.png");
    m_file_icon->loadFromImage("data/images/file_icon.png");
    m_current_directory = std::filesystem::path(c_base_content_directory);
  }

  void openWindow() { m_isOpen = true; }

  void previewPanel() {
    if (!m_selected_file.empty() && m_preview_image->textured) {
      ImGui::Image((void *)(intptr_t)m_preview_image->m_texture,
                   ImVec2(m_preview_image->m_width * 0.4, m_preview_image->m_height * 0.4));
    }
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

  void contentBrowser() {
    ImGui::BeginChild("Content-Browser");

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
          // Send image to be used for further processing
          selectImage(path);
          m_stableManager->useImage(path);
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

  virtual void render() {
    if (m_isOpen) {

      ImGui::Begin("Content Browser");

      static float w = 200.0f;
      static float h = 300.0f;

      // Child window 1, search & filterwindow
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
      ImGui::BeginChild("child1", ImVec2(w, h), true);
      ImGui::Text("Search & Filter: ");
      ImGui::EndChild();

      ImGui::SameLine();
      ImGui::InvisibleButton("vsplitter", ImVec2(8.0f, h));
      if (ImGui::IsItemActive())
        w += ImGui::GetIO().MouseDelta.x;
      ImGui::SameLine();

      // Child window 2, content browser / search
      ImGui::BeginChild("child2", ImVec2(0, h), true);
      previewPanel();
      ImGui::EndChild();

      ImGui::InvisibleButton("hsplitter", ImVec2(-1, 8.0f));
      if (ImGui::IsItemActive())
        h += ImGui::GetIO().MouseDelta.y;

      // Child window 3, content browser
      ImGui::BeginChild("child3", ImVec2(0, 0), true);
      contentBrowser();
      ImGui::EndChild();
      ImGui::PopStyleVar();

      if (ImGui::Button("Close")) {
        m_isOpen = false;
      }

      ImGui::End();
    }
  }
};