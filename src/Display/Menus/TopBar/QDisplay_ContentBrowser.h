#pragma once

#include <imgui.h>
#include <filesystem>

#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include <imgui_stdlib.h>
#include "Indexer/MetaData.h"

class QDisplay_ContentBrowser : public QDisplay_Base {

private:
  // Content Browser Config
  std::unique_ptr<Image> m_directory_icon;
  std::unique_ptr<Image> m_file_icon;
  std::unique_ptr<Image> m_preview_image;

  const char *c_base_content_directory = "data/output";
  std::filesystem::path m_current_directory;
  std::filesystem::path m_selected_file;

  // Index & search fields
  std::string m_searchString;
  std::pair<meta_node, metadata> m_xmpData;
  std::set<std::string> m_filteredPaths;

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

  void searchPanel() {
    ImGui::InputText("filter", &m_searchString);
    if (ImGui::Button("Search")) {
      QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Searching for: ", m_searchString);
      m_filteredPaths = m_stableManager->searchIndex(m_searchString);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
      m_filteredPaths.clear();
      m_searchString.clear();
    }
  }

  void loadImageXMPData(const std::string &filepath) { m_xmpData = XMP::GetInstance().readFile(filepath); }

  void metadataPanel() {
    if (!m_selected_file.empty()) {

      // Get XMP data of select image
      for (auto &i : m_xmpData.second.m_map) {
        ImGui::Separator();
        ImGui::Text(i.first.c_str(), 1);
        ImGui::Separator();

        ImGui::Indent();
        {
          ImGui::Text("%s\n", i.second.c_str());
          if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
            ImGui::OpenPopup(i.first.c_str());
          }

          if (ImGui::BeginPopup(i.first.c_str())) {
            if (ImGui::Selectable("Copy")) {
              ImGui::SetClipboardText(i.second.c_str());
            }
            ImGui::EndPopup();
          }
        }
        ImGui::Unindent();
      }
    }
  }

  void previewPanel() {
    if (!m_selected_file.empty() && m_preview_image->textured) {
      ImGui::Image((void *)(intptr_t)m_preview_image->m_texture,
                   ImVec2(m_preview_image->m_width, m_preview_image->m_height));
    }
  }

  void selectImage(const std::filesystem::path &path) {
    m_selected_file = path.filename();

    // Create new texture
    m_preview_image.reset();
    m_preview_image = std::unique_ptr<Image>(new Image(512, 512, "preview"));
    m_preview_image->loadFromImage(path.string());
    m_preview_image->textured = true;

    loadImageXMPData(path.string());
  }

  void contentBrowser() {
    ImGui::BeginChild("Content-Browser");

    if (m_current_directory != std::filesystem::path(c_base_content_directory) && m_filteredPaths.empty()) {
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

    // Check if this file is in our filters
    if (!m_filteredPaths.empty()) {
      // Our set contains full file paths (including data directory)
      for (auto &path : m_filteredPaths) {
        ImGui::PushID(path.c_str());
        Image icon = *m_file_icon;

        // Create texture
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton((void *)(intptr_t)icon.m_texture, {thumbnailSize, thumbnailSize}, {1, 0}, {0, 1});
        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          selectImage(path);
          m_stableManager->useImage(path);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          selectImage(path);
        }

        ImGui::TextWrapped("%s", path.c_str());

        ImGui::NextColumn();
        ImGui::PopID();
      }

    } else {

      for (auto &directoryEntry : std::filesystem::directory_iterator(m_current_directory)) {
        const auto &path = directoryEntry.path();

        // skip this item if it's not an image or directory
        if (directoryEntry.is_regular_file() && (path.extension() != ".png" && path.extension() != ".jpg")) {
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
            m_stableManager->useImage(path.string());
          }
        }
        if (!directoryEntry.is_directory() && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          selectImage(path);
        }

        ImGui::TextWrapped("%s", filenameString.c_str());

        ImGui::NextColumn();
        ImGui::PopID();
      }
    }

    ImGui::EndChild();
  }

  virtual void render() {
    if (m_isOpen) {

      ImGui::Begin("Content Browser");

      static float w = 300.0f;
      static float h = 400.0f;

      // Child window 1, search & filterwindow
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
      ImGui::BeginChild("child1", ImVec2(w, h), true);
      ImGui::Text("Search & Filter: ");
      searchPanel();

      ImGui::InvisibleButton("hsplitter", ImVec2(-1, 8.0f));
      if (ImGui::IsItemActive())
        h += ImGui::GetIO().MouseDelta.y;

      ImGui::Text("Metadata: ");
      metadataPanel();
      ImGui::EndChild();

      ImGui::SameLine();
      ImGui::InvisibleButton("vsplitter", ImVec2(8.0f, h));
      if (ImGui::IsItemActive())
        w += ImGui::GetIO().MouseDelta.x;
      ImGui::SameLine();

      // Child window 2, content browser / search
      ImGui::BeginChild("child3", ImVec2(0, h), true);
      previewPanel();
      ImGui::EndChild();

      ImGui::InvisibleButton("hsplitter", ImVec2(-1, 8.0f));
      if (ImGui::IsItemActive())
        h += ImGui::GetIO().MouseDelta.y;

      // Child window 3, content browser
      ImGui::BeginChild("child4", ImVec2(0, 0), true);
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