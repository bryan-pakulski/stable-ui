#include "Camera.h"

#include <imgui.h>
#include <string>

Camera::Camera(GLFWwindow *w) : m_window(w) {
    glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

    m_focusPosition.x = 0.0f;
    m_focusPosition.y = 0.0f;
}

Camera::~Camera() {

}

// Offset camera
void Camera::moveCameraPosition(float x, float y) {
    m_focusPosition.x += x;
    m_focusPosition.y += y;
}

glm::mat4x4 Camera::GetProjectionMatrix() {
    glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

    float left = m_focusPosition.x - m_screen.first / 2.0f;
    float right = m_focusPosition.x + m_screen.first / 2.0f;
    float top = m_focusPosition.y - m_screen.second / 2.0f;
    float bottom = m_focusPosition.y + m_screen.second / 2.0f;

    glm::mat4x4 orthoMatrix = glm::ortho(left, right, top, bottom, 0.001f, 100.0f);
    // glm::mat4x4 zoomMatrix = glm::scale(m_zoom);

    // return orthoMatrix * zoomMatrix;
    return orthoMatrix;
}

void Camera::updateLogic() {

}

void Camera::updateVisual() {
    // Debug menu to view camera coordinates
    ImGui::Begin("Camera");
    
    ImGui::Text("Camera X: %s", std::to_string(m_focusPosition.x).c_str());
    ImGui::Text("Camera Y: %s", std::to_string(m_focusPosition.y).c_str());

    ImGui::End();
}