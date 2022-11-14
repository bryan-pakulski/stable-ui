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
    glm::mat4 m_viewProjectionMatrix;
    glm::vec3 m_position;
    float m_rotation = 0.0f;
    float m_zoom = 1.0f;

    std::pair<int, int> m_screen{};   // Screen size

public:
    Camera(GLFWwindow *w);
    ~Camera();

    // Mouse positions for dragging across screen
    glm::vec2 prev_mouse;
    glm::vec2 cur_mouse;

    void updateLogic();
    void updateVisual();

    void moveCameraPosition(float x, float y);
    void recalculateViewMatrix();
    glm::mat4 getViewProjectionMatrix() { return m_viewProjectionMatrix; }
    glm::mat4 getProjectionMatrix() { return m_projectionMatrix; }
    glm::mat4 getViewMatrix() { return m_viewMatrix; }
};