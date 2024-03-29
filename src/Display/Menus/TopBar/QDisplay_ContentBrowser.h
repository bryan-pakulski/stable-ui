#pragma once

#include "Config/config.h"
#include "StableManager.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include "Indexer/MetaData.h"

#include <imgui.h>
#include "imgui_internal.h"
#include <filesystem>
#include <imgui_stdlib.h>

class QDisplay_ContentBrowser : public QDisplay_Base {

public:
  QDisplay_ContentBrowser(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    // Initialise content browser
    m_directory_icon = std::unique_ptr<GLImage>(new GLImage(256, 256, "dir_icon"));
    m_file_icon = std::unique_ptr<GLImage>(new GLImage(256, 256, "file_icon"));
    m_directory_icon->loadFromImage("data/images/directory_icon.png");
    m_file_icon->loadFromImage("data/images/file_icon.png");
    m_current_directory = std::filesystem::path(c_base_content_directory);
  }

  void dock_init() {

    // Create main dock node
    dock_id = ImGui::GetID(c_dockName.c_str());

    ImGui::DockBuilderRemoveNode(dock_id); // Clear any preexisting layouts associated with the ID we just chose
    ImGui::DockBuilderAddNode(dock_id);    // Create a new dock node to use

    ImGui::DockBuilderSetNodeSize(dock_id, dock_size);
    ImGui::DockBuilderSetNodePos(dock_id, dock_pos);

    ImGuiID dock_search = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Up, 0.2f, nullptr, &dock_id);
    ImGuiID dock_directory = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.8f, nullptr, &dock_id);
    ImGuiID dock_Preview = ImGui::DockBuilderSplitNode(dock_directory, ImGuiDir_Right, 0.5f, nullptr, &dock_directory);
    ImGuiID dock_XMP = ImGui::DockBuilderSplitNode(dock_directory, ImGuiDir_Down, 0.5f, nullptr, &dock_directory);
    ImGuiID dock_close = ImGui::DockBuilderSplitNode(dock_XMP, ImGuiDir_Down, 0.2f, nullptr, &dock_XMP);

    ImGui::DockBuilderDockWindow("content_top", dock_search);
    ImGui::DockBuilderDockWindow("content_directory", dock_directory);
    ImGui::DockBuilderDockWindow("content_xmp", dock_XMP);
    ImGui::DockBuilderDockWindow("content_preview", dock_Preview);
    ImGui::DockBuilderDockWindow("content_close", dock_close);

    ImGui::DockBuilderFinish(dock_id);

    m_dock_init = false;
  }

  virtual void render() {

    if (m_dock_init) {
      dock_init();
    }

    if (m_isOpen) {

      topPanel();
      contentBrowserPanel();
      previewPanel();
      metadataPanel();

      m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
      ImGui::SetNextWindowClass(&m_window_class);
      ImGui::Begin("content_close");
      if (ImGui::Button("close")) {
        m_isOpen = false;
      }
      ImGui::End();
    }
  }

private:
  const std::string c_dockName = "content_browser";
  ImGuiID dock_id;
  ImVec2 dock_pos{(float)CONFIG::WINDOW_WIDTH.get() / 2, (float)CONFIG::WINDOW_HEIGHT.get() / 2};
  ImVec2 dock_size{800, 600};
  bool m_dock_init = true;

  // Content Browser Config
  std::unique_ptr<GLImage> m_directory_icon;
  std::unique_ptr<GLImage> m_file_icon;
  std::unique_ptr<GLImage> m_preview_image;

  const char *c_base_content_directory = "data/output";
  std::filesystem::path m_current_directory;
  std::filesystem::path m_selected_file;

  // Index & search fields
  std::string m_searchString;
  std::pair<meta_node, metadata> m_xmpData;
  std::set<std::string> m_filteredPaths;

private:
  void loadImageXMPData(const std::string &filepath) { m_xmpData = XMP::GetInstance().readFile(filepath); }

  void selectImage(const std::filesystem::path &path) {
    m_selected_file = path.filename();

    // Create new texture
    m_preview_image.reset();
    m_preview_image = std::unique_ptr<GLImage>(new GLImage(512, 512, "preview"));
    m_preview_image->loadFromImage(path.string());
    m_preview_image->textured = true;

    loadImageXMPData(path.string());
  }

  void topPanel() {
    m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGui::SetNextWindowClass(&m_window_class);
    ImGui::Begin("content_top");

    ImGui::InputText("filter", &m_searchString);
    if (ImGui::Button("Lookup")) {
      QLogger::GetInstance().Log(LOGLEVEL::DBG4, "Searching for: ", m_searchString);
      m_filteredPaths = StableManager::GetInstance().searchIndex(m_searchString);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
      m_filteredPaths.clear();
      m_searchString.clear();
    }
    ImGui::End();
  }

  void metadataPanel() {
    m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&m_window_class);
    ImGui::Begin("content_xmp");

    if (!m_selected_file.empty()) {

      // Get XMP data of select image
      for (auto &i : m_xmpData.second.m_map) {
        ImGui::Separator();
        ImGui::Text(i.first.c_str(), 1);
        ImGui::Separator();

        ImGui::Indent();
        {
          ImGui::TextWrapped("%s\n", i.second.c_str());
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
    ImGui::End();
  }

  void previewPanel() {
    m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&m_window_class);
    ImGui::Begin("content_preview");

    if (!m_selected_file.empty() && m_preview_image->textured) {
      {
        if (ImGui::Button("Send to img2img")) {
          m_renderManager->useImage(m_preview_image->m_image_source);
        }
        ImGui::SameLine();
        if (ImGui::Button("Send to Canvas")) {
          m_renderManager->sendImageToCanvas(*m_preview_image);
        }
      }
      ImGui::Separator();
      ImGui::Image((void *)(intptr_t)m_preview_image->m_texture,
                   ImVec2(m_preview_image->m_width * 0.3, m_preview_image->m_height * 0.3));
    }

    ImGui::End();
  }

  void contentBrowserPanel() {
    m_window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&m_window_class);
    ImGui::Begin("content_directory");

    if (m_current_directory != std::filesystem::path(c_base_content_directory) && m_filteredPaths.empty()) {
      if (ImGui::Button("<-")) {
        m_current_directory = m_current_directory.parent_path();
      }
    }

    static float padding = 6.0f;
    static float thumbnailSize = 46.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x - (thumbnailSize);
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1)
      columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    // Check if this file is in our filters
    if (!m_filteredPaths.empty()) {
      // Our set contains full file paths (including data directory)
      for (auto &path : m_filteredPaths) {
        ImGui::PushID(path.c_str());
        GLImage icon = *m_file_icon;

        // Create texture
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton((void *)(intptr_t)icon.m_texture, {thumbnailSize, thumbnailSize}, {1, 0}, {0, 1});
        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          selectImage(path);
          m_renderManager->useImage(path);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          selectImage(path);
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
          ImGui::OpenPopup(path.c_str());
        }

        if (ImGui::BeginPopup(path.c_str())) {
          if (ImGui::Selectable("Delete")) {
            // TODO: delete file
          }
          ImGui::EndPopup();
        }

        ImGui::TextWrapped("%s", path.c_str());

        ImGui::NextColumn();
        ImGui::PopID();
      }

    } else {

      for (auto &directoryEntry : std::filesystem::directory_iterator(m_current_directory)) {
        const auto &path = directoryEntry.path();

        // skip this item if it's not an image or directory
        if (directoryEntry.is_regular_file() &&
            (path.extension() != ".png" && path.extension() != ".jpg" && path.extension() != ".jpeg")) {
          continue;
        }

        std::string filenameString = path.filename().string();

        ImGui::PushID(filenameString.c_str());
        GLImage icon = directoryEntry.is_directory() ? *m_directory_icon : *m_file_icon;

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
            m_renderManager->useImage(path.string());
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

    ImGui::End();
  }
};