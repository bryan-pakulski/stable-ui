#pragma once

#include "glm/fwd.hpp"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <utility>
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"

class Camera {

  GLFWwindow *m_window;

  glm::mat4 m_projectionMatrix;
  glm::mat4 m_viewMatrix;

  // Camera speed is proportional to zoom level
  glm::vec2 m_cameraSpeed = {0.5f, 2.0f};

  void recalculateViewMatrix();

public:
  Camera(GLFWwindow *w);
  ~Camera();

  // Mouse positions for dragging across screen
  glm::vec2 prev_mouse;
  glm::vec2 cur_mouse;

  // Helper to convert screenspace coordinates to global coordinates
  glm::vec2 screenToGlobalCoordinates(float x, float y);

  float m_zoom = 1.0f;
  const float c_defaultZoom = 1.0f;
  std::pair<float, float> c_zoom_minmax{0.05f, 5.0f};
  float m_zoomSpeed = 0.05f;

  glm::vec3 m_position;
  std::pair<int, int> m_screen{}; // Screen size

  void updateLogic();
  void updateVisual();

  void moveCameraPosition(float x, float y);
  glm::mat4 getProjectionMatrix() { return m_projectionMatrix; }
  glm::mat4 getViewMatrix() { return m_viewMatrix; }

  std::pair<int, int> getScreenSize();
};