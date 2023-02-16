#pragma once

#include "../../../Display/ErrorHandler.h"
#include "../../../QLogger.h"
#include "../../../Config/config.h"
#include "../../../Rendering/Helper.h"
#include "../../QDisplay_Base.h"

class QDisplay_CanvasTools : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_CanvasTools(std::shared_ptr<StableManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_visible_icon = std::unique_ptr<Image>(new Image(32, 32, "visible_icon"));
    m_hidden_icon = std::unique_ptr<Image>(new Image(32, 32, "hidden_icon"));

    m_visible_icon->loadFromImage("data/images/visible.png");
    m_hidden_icon->loadFromImage("data/images/hidden.png");
  }

  virtual void render() {
    ImGui::Text("Camera");
    cameraHelper();
    ImGui::Separator();

    ImGui::Text("Layer Helper");
    layerHelper();
    ImGui::Separator();

    ImGui::Text("Selection");
    selectionPreview();
    ImGui::Separator();
  }

private:
  // Window Options
  const std::string c_windowName = "Tools Window";
  std::pair<int, int> m_windowSize{};

  // Layer options
  int m_selectedLayerIndex = -1;
  std::unique_ptr<Image> m_visible_icon;
  std::unique_ptr<Image> m_hidden_icon;
  float c_visibilityIconSize = 12.0f;

  // Selection preview
  void selectionPreview() {
    // TODO: add send to img2img
    if (ImGui::Button("Save Buffer to tmp")) {
      m_stableManager->m_selection->saveBuffer();
    }
    ImGui::Image(
        (void *)(intptr_t)m_stableManager->m_selection->m_selection_texture_buffer,
        ImVec2(m_stableManager->m_selection->m_size.first * 0.4, m_stableManager->m_selection->m_size.second * 0.4));
  }

  // Camera helper
  void cameraHelper() {
    // Debug menu to view camera coordinates
    ImGui::Text("Camera X: %s", std::to_string(m_stableManager->m_camera->m_position.x).c_str());
    ImGui::Text("Camera Y: %s", std::to_string(m_stableManager->m_camera->m_position.y).c_str());

    // Almost all widgets return true when their value changes
    ImGui::SliderFloat("Zoom", &m_stableManager->m_camera->m_zoom, 1.0f, 0.05f, "");
    if (ImGui::BeginPopupContextItem("Zoom")) {
      if (ImGui::MenuItem("Reset")) {
        m_stableManager->m_camera->m_zoom = 0.5f;
      }
      ImGui::EndPopup();
    }
  }

  // Layer Code
  void layerHelper() {
    if (ImGui::BeginListBox("Layers")) {
      for (auto &item : m_stableManager->getActiveCanvas()->m_editorGrid) {
        const char *item_name = item->m_image->m_image_source.c_str();
        int index = std::addressof(item) - std::addressof(m_stableManager->getActiveCanvas()->m_editorGrid.front());
        const bool is_selected = index == m_selectedLayerIndex;

        // Visibility icons
        ImGui::PushID(static_cast<const void *>(&*item)); // We need a unique identifier for images to allow ImGui to
                                                          // perform interaction, hence using memory location
        Image icon = item->m_renderFlag ? *m_visible_icon : *m_hidden_icon;
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton((void *)(intptr_t)icon.m_texture, {c_visibilityIconSize, c_visibilityIconSize}, {1, 0},
                           {0, 1});
        ImGui::PopStyleColor();

        // Toggle item visibility flag
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          item->m_renderFlag = !item->m_renderFlag;
        }
        ImGui::SameLine();

        if (ImGui::Selectable(item_name, is_selected)) {
          m_selectedLayerIndex = index;
        }

        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
        ImGui::PopID();
      }

      ImGui::EndListBox();
    }

    if (m_selectedLayerIndex != -1) {
      ImGui::Text("Layer Tools");

      // TODO: scaling is not consistent with the coordinates of other resources, look to bring together all resources
      // under the same coordinate format
      ImGui::InputInt("img x",
                      &m_stableManager->getActiveCanvas()->m_editorGrid[m_selectedLayerIndex]->m_coordinates.first);
      ImGui::InputInt("img y",
                      &m_stableManager->getActiveCanvas()->m_editorGrid[m_selectedLayerIndex]->m_coordinates.second);

      if (ImGui::Button("Delete Layer")) {
        m_stableManager->getActiveCanvas()->deleteChunk(m_selectedLayerIndex);
        m_selectedLayerIndex = -1;
      }
    }
  }
};