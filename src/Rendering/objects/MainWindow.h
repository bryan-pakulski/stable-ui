#pragma once

#include <cmath>

#include "../../QLogger.h"
#include "BaseObject.h"

class MainWindow : public BaseObject {
private:
  std::pair<int, int> m_pc{};   // Pixel Coordinates (top left)
  std::pair<int, int> m_size{}; // Size of display
  std::pair<int, int> screen{}; // Screen size

  // Reference to texture for main window
  GLuint *m_texture_id = 0;

public:
  MainWindow(std::pair<int, int> pc, std::pair<int, int>, GLFWwindow *w)
      : BaseObject(pc), m_pc{pc} {
    int success;
    m_window = w;

    glfwGetFramebufferSize(m_window, &screen.first, &screen.second);

    // Set vertex data
    float vertices[] = {
        // positions        // colors         // texture coords
        1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
        1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    };

    // Index buffer // Element Buffer Objects (EBO)
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    std::string vShaderSource = readShader("data/shaders/MainWindow_V.glsl");
    std::string fShaderSource = readShader("data/shaders/MainWindow_F.glsl");

    unsigned int vertexShader =
        initVertexShader(vShaderSource.c_str(), success);
    unsigned int fragmentShader =
        initFragmentShader(fShaderSource.c_str(), success);

    linkShaders(vertexShader, fragmentShader, success);
    setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices));
  }

  void updateLogic() override {
    glfwGetFramebufferSize(m_window, &screen.first, &screen.second);
  }

  void updateVisual() override {
    glUseProgram(shaderProgram);

    // Update texture information
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, *m_texture_id);

    // Transformation code
    glm::mat4 trans = glm::mat4(1.0f);
    unsigned int transformLoc =
        glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  }

  void setMainWindowTexture(GLuint *id) { m_texture_id = id; }

  virtual ~MainWindow() {}
};