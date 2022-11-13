#pragma once

#include "glm/fwd.hpp"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <utility>
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"

class Camera {

    GLFWwindow *m_window;
    glm::vec2 m_focusPosition;
    float m_zoom = 1.0;
    std::pair<int, int> m_screen{};   // Screen size

public:
    Camera(GLFWwindow *w);
    ~Camera();

    glm::vec2 prev_mouse;
    glm::vec2 cur_mouse;

    void updateLogic();
    void updateVisual();

    void moveCameraPosition(float x, float y);
    glm::mat4x4 GetProjectionMatrix();
};