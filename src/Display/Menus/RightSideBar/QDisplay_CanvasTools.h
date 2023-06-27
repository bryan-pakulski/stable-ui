#pragma once

#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Config/config.h"
#include "Helpers/GLHelper.h"
#include "Display/QDisplay_Base.h"
#include "imgui.h"

class QDisplay_CanvasTools : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_CanvasTools(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_visible_icon = std::unique_ptr<GLImage>(new GLImage(32, 32, "visible_icon"));
    m_hidden_icon = std::unique_ptr<GLImage>(new GLImage(32, 32, "hidden_icon"));

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

    ImGui::Text("Debug Info");
    debugInfo();
    ImGui::Separator();
  }

private:
  // Window Options
  const std::string c_windowName = "Tools Window";
  std::pair<int, int> m_windowSize{};

  // Layer options
  int m_selectedLayerIndex = -1;
  std::unique_ptr<GLImage> m_visible_icon;
  std::unique_ptr<GLImage> m_hidden_icon;
  float c_visibilityIconSize = 12.0f;

private:
  // Selection preview
  void selectionPreview() {

    ImGui::SliderInt("Selection X", &m_renderManager->m_selection->m_size.x, 0, 1024);
    if (ImGui::BeginPopupContextItem("Selection X")) {
      if (ImGui::MenuItem("Reset")) {
        m_renderManager->m_selection->m_size.x = 512;
      }
      ImGui::EndPopup();
    }

    ImGui::SliderInt("Selection Y", &m_renderManager->m_selection->m_size.y, 0, 1024);
    if (ImGui::BeginPopupContextItem("Selection Y")) {
      if (ImGui::MenuItem("Reset")) {
        m_renderManager->m_selection->m_size.y = 512;
      }
      ImGui::EndPopup();
    }

    ImGui::Checkbox("Snap to grid", &m_renderManager->m_selection->m_snap);

    if (m_renderManager->m_selection->m_snap) {
      ImGui::SliderInt("Selection Snap", &m_renderManager->m_selection->m_pixelSnap, 1, 512);
    }

    // TODO: add send to img2img
    if (ImGui::Button("Save buffer to file")) {
      m_renderManager->saveBuffer();
    }
    ImGui::Image((void *)(intptr_t)m_renderManager->getBuffer()->m_texture,
                 ImVec2(m_renderManager->getBuffer()->m_width * 0.4, m_renderManager->getBuffer()->m_height * 0.4));
    ImGui::Image((void *)(intptr_t)m_renderManager->getMask()->m_texture,
                 ImVec2(m_renderManager->getMask()->m_width * 0.4, m_renderManager->getMask()->m_height * 0.4));
  }

  void debugInfo() {
    // Debug menu to view camera coordinates
    ImGui::Text("Camera X: %s", std::to_string(m_renderManager->m_camera->m_position.x).c_str());
    ImGui::Text("Camera Y: %s", std::to_string(m_renderManager->m_camera->m_position.y).c_str());
    ImGui::Text("Camera Zoom: %s", std::to_string(m_renderManager->m_camera->m_zoom).c_str());
  }

  // Camera helper
  void cameraHelper() {
    // Almost all widgets return true when their value changes
    ImGui::SliderFloat("Zoom", &m_renderManager->m_camera->m_zoom, m_renderManager->m_camera->c_zoomLimits.x,
                       m_renderManager->m_camera->c_zoomLimits.y, "");
    if (ImGui::BeginPopupContextItem("Zoom")) {
      if (ImGui::MenuItem("Reset")) {
        m_renderManager->m_camera->m_zoom = m_renderManager->m_camera->c_defaultZoom;
      }
      ImGui::EndPopup();
    }
  }

  // Layer Code
  void layerHelper() {
    if (ImGui::BeginListBox("Layers")) {
      for (auto &item : m_renderManager->getActiveCanvas()->m_editorGrid) {
        const char *item_name = item->m_image->m_image_source.c_str();
        int index = std::addressof(item) - std::addressof(m_renderManager->getActiveCanvas()->m_editorGrid.front());
        const bool is_selected = index == m_selectedLayerIndex;

        // Visibility icons
        ImGui::PushID(static_cast<const void *>(&*item)); // We need a unique identifier for images to allow ImGui to
                                                          // perform interaction, hence using memory location
        GLImage icon = item->m_renderFlag ? *m_visible_icon : *m_hidden_icon;
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
                      &m_renderManager->getActiveCanvas()->m_editorGrid[m_selectedLayerIndex]->getPosition().x);
      ImGui::InputInt("img y",
                      &m_renderManager->getActiveCanvas()->m_editorGrid[m_selectedLayerIndex]->getPosition().y);

      if (ImGui::Button("Delete Layer")) {
        m_renderManager->getActiveCanvas()->deleteImage(m_selectedLayerIndex);
        m_selectedLayerIndex = -1;
      }
    }
  }
};