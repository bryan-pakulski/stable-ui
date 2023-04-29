#include "Camera.h"

#include <algorithm>
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

  // Adjust camera speed based on how far zoomed in we are, ranging from min/max camera speed
  float cameraSpeed = (m_zoom / c_zoom_minmax.second) * m_cameraSpeed.y;
  if (cameraSpeed < m_cameraSpeed.x) {
    cameraSpeed = m_cameraSpeed.x;
  }

  m_position.x += (-x * cameraSpeed);
  m_position.y += (-y * cameraSpeed);
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
  float normX = 2.0f * x / m_screen.first - 1.0f;
  float normY = 2.0f * y / m_screen.second - 1.0f;

  // HOMOGENEOUS SPACE
  glm::vec4 screenPos = glm::vec4(normX, normY, -1.0f, 1.0f);

  // Projection/Eye Space
  glm::mat4 ProjectView = getProjectionMatrix() * getViewMatrix();
  glm::mat4 viewProjectionInverse = inverse(ProjectView);
  glm::vec4 worldPos = viewProjectionInverse * screenPos;
  return glm::vec2(worldPos.x, worldPos.y);
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