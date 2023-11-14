#pragma once

#include "Helpers/QLogger.h"
#include "Helpers/Serializer.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

struct shader {
  unsigned int VAO{}, VBO{}, EBO{};
  unsigned int shaderProgram{};

  ~shader() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
  }
};

class BaseObject : public Serialisable {

public:
  BaseObject(glm::ivec2 m_position);

  virtual ~BaseObject() {}

  virtual void updateLogic() = 0;
  virtual void updateVisual() = 0;

  // Read shader file
  static std::string readShader(const char *filePath);

  // Link shaders
  void linkShaders(unsigned int vertexShader, unsigned int fragmentShader, int &success, std::shared_ptr<shader> sh);

  // Set shader buffers
  void setShaderBuffers(float *vertices, int sv, unsigned int *indices, int si, std::shared_ptr<shader> sh);

  // Create shader
  void createShader(std::shared_ptr<shader> sh, std::string name);

  // Get shader
  std::shared_ptr<shader> getShader(std::string name);

  // Get Position
  glm::ivec2 &getPosition() { return m_position; };

  // Set matrix coordinates for projection
  void setMat4(std::string uniformName, glm::mat4x4 mat, std::string shaderName);

  // Check if image intersects a given selection space
  bool intersects(const glm::ivec2 &l1, const glm::ivec2 &r1, const glm::ivec2 &l2, const glm::ivec2 &r2);

protected:
  glm::ivec2 m_position;
  std::map<std::string, std::shared_ptr<shader>> m_shaders;
  GLFWwindow *m_window = 0;
};

// Initialise vertex shader for GLFW
static unsigned int initVertexShader(const char *vertexShaderSource, int &success) {
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    GLint length = 0;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &length);
    std::string errorLog(length, ' ');
    glGetShaderInfoLog(vertexShader, length, &length, &errorLog[0]);
    QLogger::GetInstance().Log(LOGLEVEL::ERR,
                               "BaseObject::initVertexShader ERROR::VERTEX::SHADER::COMPILATION_FAILED\n", errorLog);
  }

  return vertexShader;
}

// Initialise fragment shader for GLFW
static unsigned int initFragmentShader(const char *fragmentShaderSource, int &success) {
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    GLint length = 0;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length);
    std::string errorLog(length, ' ');
    glGetShaderInfoLog(fragmentShader, length, &length, &errorLog[0]);
    QLogger::GetInstance().Log(
        LOGLEVEL::ERR, "BaseObject::initFragmentShader ERROR::FRAGMENT::SHADER::COMPILATION_FAILED\n", errorLog);
  }

  return fragmentShader;
}