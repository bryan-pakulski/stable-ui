#include "Camera.h"

#include <imgui.h>
#include <string>

Camera::Camera(GLFWwindow *w) : m_window(w) {
    glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

    m_position = glm::vec3(0.0f);

    // Camera bounds
    float left   = -m_screen.first / 2.0f;
    float right  =  m_screen.first / 2.0f;
    float top    = -m_screen.second / 2.0f;
    float bottom =  m_screen.second / 2.0f;

    m_projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    m_viewMatrix = glm::mat4(1.0f);
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

Camera::~Camera() {

}

// Offset camera
void Camera::moveCameraPosition(float x, float y) {
    m_position.x -= (x * m_cameraSpeed);
    m_position.y -= (y * m_cameraSpeed);

    recalculateViewMatrix();
}

// Recalculate matrices
void Camera::recalculateViewMatrix() {
    glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position) * 
        glm::rotate(glm::mat4(1.0f), m_rotation, glm::vec3(0, 0, 1)) * 
        glm::scale(glm::mat4(1.0f), glm::vec3(m_zoom));

    m_viewMatrix = glm::inverse(transform);
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

}

void Camera::updateLogic() {

}

void Camera::updateVisual() {

}