#include "Camera.h"

#include <imgui.h>
#include <string>

Camera::Camera(GLFWwindow *w) : m_window(w) {
  glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

  // Position in world space
  m_position = glm::vec3(0.0f, 0.0f, m_zoom);

  // Generate the orthographic projection matrix
  m_projectionMatrix =
      glm::ortho(m_position.x - (m_screen.first * m_zoom) / 2.0f, m_position.x + (m_screen.first * m_zoom) / 2.0f,
                 m_position.y + (m_screen.second * m_zoom) / 2.0f, m_position.y - (m_screen.second * m_zoom) / 2.0f,
                 -10.0f, 10.0f);

  m_viewMatrix = glm::lookAt(m_position,                               // Camera is at coordinates, in World Space
                             glm::vec3(m_position.x, m_position.y, 0), // and looks ahead
                             glm::vec3(0, 1.0f, 0));                   // Head is up (set to 0,-1,0 to look upside-down)
}

Camera::~Camera() {}

// Offset camera
void Camera::moveCameraPosition(float x, float y) {
  m_position.x += (-x * m_cameraSpeed);
  m_position.y += (-y * m_cameraSpeed);
}

// Recalculate matrices
void Camera::recalculateViewMatrix() {
  glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

  // Don't apply view matrix for 2d rendering
  m_viewMatrix = glm::lookAt(m_position,                               // Camera is at coordinates, in World Space
                             glm::vec3(m_position.x, m_position.y, 0), // and looks ahead
                             glm::vec3(0, 1, 0));                      // Head is up (set to 0,-1,0 to look upside-down)
}

glm::vec2 Camera::screenToGlobalCoordinates(float x, float y) {
  // Step 1: Normalize coordinates
  float normX = (2.0f * x) / m_screen.first - 1.0f;
  float normY = 1.0f - (2.0f * y) / m_screen.second;

  // Step 2 and 3: Calculate distances from camera position
  glm::vec2 distFromCamera =
      glm::vec2((normX * m_screen.first * m_zoom) / 2.0f, (normY * m_screen.second * m_zoom) / 2.0f);

  // Step 4: Add camera position
  return distFromCamera + glm::vec2(m_position.x, m_position.y);
}

void Camera::updateLogic() {
  if (m_zoom < c_zoom_minmax.first) {
    m_zoom = c_zoom_minmax.first;
  } else if (m_zoom > c_zoom_minmax.second) {
    m_zoom = c_zoom_minmax.second;
  }

  m_position.z = m_zoom;
}

void Camera::updateVisual() {
  // Set up the orthographic projection matrix
  recalculateViewMatrix();

  glMatrixMode(GL_PROJECTION);
  glOrtho(0, m_screen.first, m_screen.second, 0.0, -1.0, 1.0);

  glLoadIdentity();
  glViewport(0, 0, m_screen.first, m_screen.second);

  m_projectionMatrix =
      glm::ortho(m_position.x - (m_screen.first * m_zoom) / 2.0f, m_position.x + (m_screen.first * m_zoom) / 2.0f,
                 m_position.y + (m_screen.second * m_zoom) / 2.0f, m_position.y - (m_screen.second * m_zoom) / 2.0f,
                 -10.0f, 10.0f);

  // Set the modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

std::pair<int, int> Camera::getScreenSize() { return m_screen; }