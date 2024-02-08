#pragma once

#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Config/config.h"
#include "Helpers/GLHelper.h"
#include "Display/QDisplay_Base.h"
#include "imgui.h"
#include "imgui_internal.h"

class QDisplay_CanvasTools : public QDisplay_Base {
public:
  // Initialise render manager reference
  QDisplay_CanvasTools(std::shared_ptr<RenderManager> rm, GLFWwindow *w) : QDisplay_Base(rm, w) {
    m_visible_icon = std::unique_ptr<GLImage>(new GLImage(32, 32, "visible_icon"));
    m_hidden_icon = std::unique_ptr<GLImage>(new GLImage(32, 32, "hidden_icon"));

    m_visible_icon->loadFromImage("data/images/visible.png");
    m_hidden_icon->loadFromImage("data/images/hidden.png");

    m_windowName = "canvas_tools";
  }

  virtual void render() {
    ImGui::Begin(m_windowName.c_str(), 0, ImGuiWindowFlags_NoMove);

    ImGui::Text("Camera");
    cameraHelper();
    ImGui::Separator();

    ImGui::Text("Layer Tools");
    layerHelper();
    ImGui::Separator();

    ImGui::Text("Debug Info");
    debugInfo();
    ImGui::Separator();
    ImGui::End();
  }

private:
  std::pair<int, int> m_windowSize{};

  // Layer options
  int m_selectedImageIndex = -1;
  std::unique_ptr<GLImage> m_visible_icon;
  std::unique_ptr<GLImage> m_hidden_icon;
  float c_visibilityIconSize = 12.0f;

private:
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
    if (ImGui::Button("Create Layer")) {
      // TODO: create a popup to set layer size & name
      m_renderManager->getActiveCanvas()->createLayer(glm::ivec2{1920, 1080}, "New Layer", false);
    }
    if (ImGui::BeginListBox("Layers")) {
      for (auto &layer : m_renderManager->getActiveCanvas()->m_editorGrid) {
        const char *item_name = layer->m_name.c_str();
        int index = std::addressof(layer) - std::addressof(m_renderManager->getActiveCanvas()->m_editorGrid.front());
        const bool is_selected = index == m_renderManager->getActiveLayer();

        // Visibility icons
        ImGui::PushID(static_cast<const void *>(&*layer)); // We need a unique identifier for images to allow ImGui to
                                                           // perform interaction, hence using memory location
        GLImage icon = layer->m_renderFlag ? *m_visible_icon : *m_hidden_icon;
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton((void *)(intptr_t)icon.m_texture, {c_visibilityIconSize, c_visibilityIconSize}, {1, 0},
                           {0, 1});
        ImGui::PopStyleColor();

        // Toggle item visibility flag
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          layer->m_renderFlag = !layer->m_renderFlag;
        }
        ImGui::SameLine();

        if (ImGui::Selectable(item_name, is_selected)) {
          m_renderManager->setActiveLayer(index);
        }

        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
        ImGui::PopID();
      }

      ImGui::EndListBox();
    }

    if (m_renderManager->getActiveLayer() != 0) {
      if (ImGui::Button("Delete Layer")) {
        m_renderManager->getActiveCanvas()->deleteLayer(m_renderManager->getActiveLayer());
        m_renderManager->setActiveLayer(m_renderManager->getActiveLayer() - 1);
      }
    }

    ImGui::Separator();
    ImGui::Text("Layer Content");
    layerContentHelper(m_renderManager->getActiveLayer());

    // Image in layer control
    if (m_selectedImageIndex != -1) {
      ImGui::Text("Layer Image");

      ImGui::InputInt("img x", &m_renderManager->getActiveCanvas()
                                    ->m_editorGrid[m_renderManager->getActiveLayer()]
                                    ->getImages()[m_selectedImageIndex]
                                    .getPosition()
                                    .x);
      ImGui::InputInt("img y", &m_renderManager->getActiveCanvas()
                                    ->m_editorGrid[m_renderManager->getActiveLayer()]
                                    ->getImages()[m_selectedImageIndex]
                                    .getPosition()
                                    .y);

      if (ImGui::Button("Merge Image with Layer")) {
        m_renderManager->getActiveCanvas()->m_editorGrid[m_renderManager->getActiveLayer()]->mergeImageFromStack(
            m_selectedImageIndex);
        m_selectedImageIndex -= 1;
      }

      if (ImGui::Button("Delete Image")) {
        m_renderManager->getActiveCanvas()->m_editorGrid[m_renderManager->getActiveLayer()]->deleteImage(
            m_selectedImageIndex);
        m_selectedImageIndex -= 1;
      }
    }
  }

  // Layer content Helper
  void layerContentHelper(int layerId) {
    if (ImGui::BeginListBox("Layer Content")) {
      if (m_renderManager->getActiveCanvas()->m_editorGrid.size() > layerId) {
        for (auto &image : m_renderManager->getActiveCanvas()->m_editorGrid.at(layerId)->getImages()) {
          const char *item_name = image.m_image->m_image_source.c_str();
          int index = std::addressof(image) -
                      std::addressof(m_renderManager->getActiveCanvas()->m_editorGrid.at(layerId)->getImages().front());
          const bool is_selected = index == m_selectedImageIndex;

          // Visibility icons
          ImGui::PushID(static_cast<const void *>(&image)); // We need a unique identifier for images to allow ImGui to
                                                            // perform interaction, hence using memory location
          GLImage icon = image.m_renderFlag ? *m_visible_icon : *m_hidden_icon;
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
          ImGui::ImageButton((void *)(intptr_t)icon.m_texture, {c_visibilityIconSize, c_visibilityIconSize}, {1, 0},
                             {0, 1});
          ImGui::PopStyleColor();

          // Toggle item visibility flag
          if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            image.m_renderFlag = !image.m_renderFlag;
          }
          ImGui::SameLine();

          if (ImGui::Selectable(item_name, is_selected)) {
            m_selectedImageIndex = index;
          }

          if (is_selected) {
            ImGui::SetItemDefaultFocus();
          }
          ImGui::PopID();
        }
      }

      ImGui::EndListBox();
    }
  }
};