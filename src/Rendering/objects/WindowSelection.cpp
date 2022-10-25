#include "WindowSelection.h"


WindowSelection::~WindowSelection() {}

WindowSelection::WindowSelection(std::pair<int, int> pc, GLFWwindow *w) : BaseObject(pc), m_pc{pc} {
    int success;
    m_window = w;

    glfwGetFramebufferSize(m_window, &screen.first, &screen.second);

    float vertices[] = {
        // positions        // colors         // texture coords
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
        0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    };

    // Index buffer // Element Buffer Objects (EBO)
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    std::string vShaderSource = readShader("data/shaders/WindowSelection_V.glsl");
    std::string fShaderSource = readShader("data/shaders/WindowSelection_F.glsl");

    unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), success);
    unsigned int fragmentShader = initFragmentShader(fShaderSource.c_str(), success);

    linkShaders(vertexShader, fragmentShader, success);
    setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices));

    GLHELPER::LoadTextureFromFile("data/images/selection.png", &m_texture, &m_width, &m_height);
}

void WindowSelection::updateLogic() {
    glfwGetFramebufferSize(m_window, &screen.first, &screen.second);
    glfwGetCursorPos(m_window, &m_mousePosX, &m_mousePosY);
    m_mousePosX /= screen.second;
    m_mousePosY /= -screen.first;
}

void WindowSelection::updateVisual() {
    glUseProgram(shaderProgram);

    // Update texture information
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Transformation code
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::scale(trans, glm::vec3(m_scale, m_scale, m_scale));
    trans = glm::translate(trans, glm::vec3(m_mousePosX, m_mousePosY, 0.0f));
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    // Draw code
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}