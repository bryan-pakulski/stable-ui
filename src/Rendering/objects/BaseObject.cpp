#include "BaseObject.h"
#include <sstream>

BaseObject::BaseObject(glm::ivec2 position) : m_position{position} {
  QLogger::GetInstance().Log(LOGLEVEL::DBG3, "BaseObject::BaseObject Object initialized at", position.x, position.y);
}

// Read shader file
std::string BaseObject::readShader(const char *filePath) {
  std::ifstream fileStream(filePath, std::ios::in);

  if (!fileStream.is_open()) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR,
                               "BaseObject::readShader Could not read file, file doesn't exist: ", filePath);
    return "";
  }

  std::stringstream fileBuffer;
  fileBuffer.str(std::string());
  fileBuffer << fileStream.rdbuf();

  return fileBuffer.str();
}

// Link shaders
void BaseObject::linkShaders(unsigned int vertexShader, unsigned int fragmentShader, int &success,
                             std::shared_ptr<shader> sh) {
  sh->shaderProgram = glCreateProgram();

  glAttachShader(sh->shaderProgram, vertexShader);
  glAttachShader(sh->shaderProgram, fragmentShader);
  glLinkProgram(sh->shaderProgram);
  glGetProgramiv(sh->shaderProgram, GL_LINK_STATUS, &success);

  if (!success) {
    GLint length = 0;
    glGetShaderiv(sh->shaderProgram, GL_INFO_LOG_LENGTH, &length);
    std::string errorLog(length, ' '); // Resize and fill with space character
    glGetProgramInfoLog(sh->shaderProgram, length, &length, &errorLog[0]);
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "BaseObject::linkShaders ERROR::PROGRAM::LINKING_FAILED\n", errorLog);
  }

  // delete shaders after linking
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

// Set shader buffers
void BaseObject::setShaderBuffers(float *vertices, int sv, unsigned int *indices, int si, std::shared_ptr<shader> sh) {
  // Set vertex buffer object and vertex array object and element buffer
  // objects
  glGenVertexArrays(1, &sh->VAO);
  glGenBuffers(1, &sh->VBO);
  glGenBuffers(1, &sh->EBO);

  // bind vertex array object
  glBindVertexArray(sh->VAO);

  // bind vertex buffer object
  glBindBuffer(GL_ARRAY_BUFFER, sh->VBO);
  glBufferData(GL_ARRAY_BUFFER, sv, vertices, GL_STATIC_DRAW);

  // bind element buffer objects
  // EBO is stored in the VAO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sh->EBO);
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

// Set matrix coordinates for projection
void BaseObject::setMat4(std::string uniformName, glm::mat4x4 mat, std::string shaderName) {
  int location = glGetUniformLocation(m_shaders.at(shaderName)->shaderProgram, uniformName.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

// Create shader
void BaseObject::createShader(std::shared_ptr<shader> sh, std::string name) { m_shaders.emplace(name, sh); }

// Get shader
std::shared_ptr<shader> BaseObject::getShader(std::string name) { return m_shaders.at(name); }

// Check if an image intersects with x1,y1, x2,y2
bool BaseObject::intersects(const glm::ivec2 &l1, const glm::ivec2 &r1, const glm::ivec2 &l2, const glm::ivec2 &r2) {

  QLogger::GetInstance().Log(LOGLEVEL::DBG2, "BaseObject::intersects, checking intersection of [", l1.x, l1.y, r1.x,
                             r1.y, "] and [", l2.x, l2.y, r2.x, r2.y, "]");

  bool wp = std::min(r1.x, r2.x) > std::max(l1.x, l2.x);
  bool hp = std::min(r1.y, r2.y) > std::max(l1.y, l2.y);

  if (wp && hp) {
    QLogger::GetInstance().Log(LOGLEVEL::DBG2, "Image::intersects, found intersecting image!");
    return true;
  } else {
    return false;
  }
}