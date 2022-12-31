#include "Camera.h"

#include <imgui.h>
#include <string>

Camera::Camera(GLFWwindow *w) : m_window(w) {
  glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

  m_position = glm::vec3(0.0f);

  // Get the aspect ratio by dividing the width and height of the viewport
  float aspectRatio = (float)m_screen.first / (float)m_screen.second;

  // Generate the orthographic projection matrix
  m_projectionMatrix = glm::ortho(-m_screen.first * aspectRatio, m_screen.second * aspectRatio, (float)-m_screen.first,
                                  (float)m_screen.second, -1.0f, 1.0f);

  m_viewMatrix = glm::mat4(1.0f);
  m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

Camera::~Camera() {}

// Offset camera
void Camera::moveCameraPosition(float x, float y) {
  m_position.x -= (x * m_cameraSpeed);
  m_position.y -= (y * m_cameraSpeed);
}

// Recalculate matrices
void Camera::recalculateViewMatrix() {
  glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

  glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position) *
                        glm::rotate(glm::mat4(1.0f), m_rotation, glm::vec3(0, 0, 1)) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(m_zoom));

  // Don't apply view matrix for 2d rendering
  m_viewMatrix = glm::inverse(transform);
  float aspectRatio = (float)m_screen.first / (float)m_screen.second;
  m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

void Camera::updateLogic() {
  if (m_zoom < c_zoom_minmax.first) {
    m_zoom = c_zoom_minmax.first;
  } else if (m_zoom > c_zoom_minmax.second) {
    m_zoom = c_zoom_minmax.second;
  }
}

void Camera::updateVisual() {
  // Set up the orthographic projection matrix
  recalculateViewMatrix();

  float aspectRatio = (float)m_screen.first / (float)m_screen.second;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, m_screen.first, m_screen.second);
  m_projectionMatrix = glm::ortho(-m_screen.first * aspectRatio, m_screen.second * aspectRatio, (float)-m_screen.first,
                                  (float)m_screen.second, -1.0f, 1.0f);

  // Set the modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

std::pair<int, int> Camera::getScreenSize() { return m_screen; }