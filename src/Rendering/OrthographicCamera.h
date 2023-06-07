#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <GLFW/glfw3.h>

class OrthographicCamera {
public:
  glm::ivec2 m_screen = {0, 0};
  glm::vec3 m_position = {0.0f, 0.0f, 1.0f};
  glm::vec3 m_offset;
  float m_rotation = 0.0f;
  glm::vec2 m_prevMouse = {0.0f, 0.0f};

  float m_zoom = 2.5f;
  float m_zoomSpeed = 0.05f;
  const float c_defaultZoom = 1.0f;
  const glm::vec2 c_zoomLimits = {0.5f, 5.0f};

public:
  OrthographicCamera(GLFWwindow *w);

  void updateLogic();
  void updateVisual();

  void onMousePressed(glm::vec2 position) { m_prevMouse = position; }

  void moveCamera(glm::ivec2 mouse) {
    // find the difference between new position, and last, in pixels
    int offsetX = mouse.x - m_prevMouse.x;
    int offsetY = mouse.y - m_prevMouse.y;

    // update mouse pos
    m_prevMouse.x = mouse.x;
    m_prevMouse.y = mouse.y;

    // get as ratio +/- 1
    float dx = ((float)offsetX) / m_screen.x;
    float dy = ((float)offsetY) / m_screen.y;

    // now move camera by offset (might need to multiply by 2 here?)
    m_position.x += round(m_offset.x * dx * 2);
    m_position.y -= round(m_offset.y * dy * 2);
  }

  void SetRotation(float rotation) {
    m_rotation = rotation;
    RecalculateViewMatrix();
  }

  glm::ivec2 screenToGlobalCoordinates(glm::ivec2 position);

  const glm::mat4 &GetProjectionMatrix() const { return m_projectionMatrix; }
  const glm::mat4 &GetViewMatrix() const { return m_viewMatrix; }
  const glm::mat4 &GetViewProjctionMatrix() const { return m_viewProjectionMatrix; }

private:
  void RecalculateViewMatrix();
  void RecalculateProjection();
  void RecalculateViewProjectionMatrix();

private:
  glm::mat4 m_projectionMatrix;
  glm::mat4 m_viewMatrix;
  glm::mat4 m_viewProjectionMatrix;

  GLFWwindow *m_window;
};