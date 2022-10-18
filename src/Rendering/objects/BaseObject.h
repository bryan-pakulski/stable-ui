#pragma once

#include "../../QLogger.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

class BaseObject {

protected:
  std::pair<int, int> pixelCoords;
  unsigned int VAO{}, VBO{}, EBO{};
  unsigned int shaderProgram{};
  GLFWwindow *m_window = 0;

public:
  BaseObject(std::pair<int, int> pixelCoords) : pixelCoords{pixelCoords} {
    QLogger::GetInstance().Log(LOGLEVEL::INFO, "Object initialized at", pixelCoords.first, pixelCoords.second);
  }

  virtual ~BaseObject() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
  }

  virtual void updateLogic() = 0;
  virtual void updateVisual() = 0;

  // Read shader file
  static std::string readShader(const char *filePath) {
    std::ifstream fileStream(filePath, std::ios::in);

    if (!fileStream.is_open()) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "Could not read file, file doesn't exist: ", filePath);
      return "";
    }

    std::stringstream fileBuffer;
    fileBuffer.str(std::string());
    fileBuffer << fileStream.rdbuf();

    return fileBuffer.str();
  }

  // Linear convert X/Y coordinates to local coordinates (-1, 1.0)
  static float getLC(const int &pixelCoord, const int &max) { return (((float)pixelCoord / (float)max) * 2) - 1; }

  // Initialise vertex shader for GLFW
  static unsigned int initVertexShader(const char *vertexShaderSource, int &success) {
    char errorInfo[512] = "";
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(vertexShader, 512, nullptr, errorInfo);
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "ERROR::VERTEX::SHADER::COMPILATION_FAILED\n", errorInfo);
    }

    return vertexShader;
  }

  // Initialise fragment shader for GLFW
  static unsigned int initFragmentShader(const char *fragmentShaderSource, int &success) {
    char errorInfo[512] = "";
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(fragmentShader, 512, nullptr, errorInfo);
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "ERROR::FRAGMENT::SHADER::COMPILATION_FAILED\n", errorInfo);
    }

    return fragmentShader;
  }

  // Link shaders
  void linkShaders(unsigned int vertexShader, unsigned int fragmentShader, int &success) {
    char errorInfo[512] = "";
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
      glGetProgramInfoLog(shaderProgram, 512, nullptr, errorInfo);
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "ERROR::PROGRAM::LINKING_FAILED\n", errorInfo);
    }

    // delete shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  // Set shader buffers
  void setShaderBuffers(float *vertices, int sv, unsigned int *indices, int si) {
    // Set vertex buffer object and vertex array object and element buffer
    // objects
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind vertex array object
    glBindVertexArray(VAO);

    // bind vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sv, vertices, GL_STATIC_DRAW);

    // bind element buffer objects
    // EBO is stored in the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, si, indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // unbind the VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
};