#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <GLFW/glfw3.h>

class OrthographicCamera {
public:
  glm::ivec2 m_screen = {0, 0};
  glm::vec3 m_position = {0.0f, 0.0f, 1.0f};
  float m_rotation = 0.0f;

  float m_zoom = 0.5f;
  float m_zoomSpeed = 0.05f;
  const float c_defaultZoom = 1.0f;
  const glm::vec2 c_zoomLimits = {0.05f, 5.0f};

public:
  OrthographicCamera(GLFWwindow *w);

  void updateLogic();
  void updateVisual();

  void SetPosition(glm::vec3 position) {
    m_position = position;
    RecalculateViewMatrix();
  }

  void OffsetPosition(glm::vec2 offset) {
    m_position.x += (-offset.x);
    m_position.y += (-offset.y);
    RecalculateViewMatrix();
  }

  void SetRotation(float rotation) {
    m_rotation = rotation;
    RecalculateViewMatrix();
  }

  glm::vec2 screenToGlobalCoordinates(float x, float y);

  const glm::mat4 &GetProjectionMatrix() const { return m_projectionMatrix; }
  const glm::mat4 &GetViewMatrix() const { return m_viewMatrix; }
  const glm::mat4 &GetViewProjctionMatrix() const { return m_viewProjectionMatrix; }

private:
  void RecalculateViewMatrix();
  void RecalculateProjection();

private:
  glm::mat4 m_projectionMatrix;
  glm::mat4 m_viewMatrix;
  glm::mat4 m_viewProjectionMatrix;

  GLFWwindow *m_window;
};