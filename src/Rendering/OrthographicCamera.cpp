#include "OrthographicCamera.h"
#include "Helpers/QLogger.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

OrthographicCamera::OrthographicCamera(GLFWwindow *w) : m_viewMatrix(1.0f), m_window(w) {
  glfwGetFramebufferSize(m_window, &m_screen.x, &m_screen.y);
  QLogger::GetInstance().Log(LOGLEVEL::TRACE, "OrthographicCamera::OrthographicCamera");

  m_offset = glm::vec3{-m_screen.x / 2.0 * m_zoom, -m_screen.y / 2.0 * m_zoom, 0.0f};

  RecalculateViewMatrix();
  RecalculateProjection();
  RecalculateViewProjectionMatrix();
}

void OrthographicCamera::RecalculateProjection() {
  m_projectionMatrix = glm::ortho(0.0f, m_screen.x * m_zoom, 0.0f, m_screen.y * m_zoom, -1.0f, 1.0f);
}

void OrthographicCamera::RecalculateViewMatrix() {
  m_offset = glm::vec3{(-m_screen.x / 2.0) * m_zoom, (-m_screen.y / 2.0) * m_zoom, 0.0f};

  glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position + m_offset) *
                        glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0, 0, 1.0f));

  m_viewMatrix = glm::inverse(transform);
}

void OrthographicCamera::RecalculateViewProjectionMatrix() {
  m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

glm::ivec2 OrthographicCamera::screenToGlobalCoordinates(glm::ivec2 position) {
  RecalculateViewProjectionMatrix();

  // Step 1: Normalize coordinates, account for inverted y
  float normX = 2.0f * position.x / m_screen.x - 1.0f;
  float normY = -(2.0f * position.y / m_screen.y - 1.0f);

  // HOMOGENEOUS SPACE
  glm::vec4 screenPos = glm::vec4(normX, normY, -1.0f, 1.0f);

  // Projection/Eye Space
  glm::mat4 viewProjectionInverse = glm::inverse(m_viewProjectionMatrix);
  glm::vec4 worldPos = viewProjectionInverse * screenPos;

  return glm::ivec2(round(worldPos.x), round(worldPos.y));
}

void OrthographicCamera::updateLogic() {
  if (m_zoom < c_zoomLimits.x) {
    m_zoom = c_zoomLimits.x;
  } else if (m_zoom > c_zoomLimits.y) {
    m_zoom = c_zoomLimits.y;
  }
}

void OrthographicCamera::updateVisual() {
  glfwGetFramebufferSize(m_window, &m_screen.x, &m_screen.y);

  RecalculateProjection();
  RecalculateViewMatrix();
}