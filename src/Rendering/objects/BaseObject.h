#pragma once

#include "QLogger.h"

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
  BaseObject(std::pair<int, int> pixelCoords);

  virtual ~BaseObject();

  virtual void updateLogic() = 0;
  virtual void updateVisual() = 0;

  // Read shader file
  static std::string readShader(const char *filePath);

  // Link shaders
  void linkShaders(unsigned int vertexShader, unsigned int fragmentShader, int &success);

  // Set shader buffers
  void setShaderBuffers(float *vertices, int sv, unsigned int *indices, int si);

  // Set matrix coordinates for projection
  void setMat4(std::string uniformName, glm::mat4x4 mat);
};

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
    QLogger::GetInstance().Log(LOGLEVEL::ERR,
                               "BaseObject::initVertexShader ERROR::VERTEX::SHADER::COMPILATION_FAILED\n", errorInfo);
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
    QLogger::GetInstance().Log(
        LOGLEVEL::ERR, "BaseObject::initFragmentShader ERROR::FRAGMENT::SHADER::COMPILATION_FAILED\n", errorInfo);
  }

  return fragmentShader;
}