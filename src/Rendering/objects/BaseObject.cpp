#include "BaseObject.h"

BaseObject::BaseObject(std::pair<int, int> pixelCoords) : pixelCoords{pixelCoords} {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Object initialized at", pixelCoords.first, pixelCoords.second);
}

BaseObject::~BaseObject() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shaderProgram);
}

// Read shader file
std::string BaseObject::readShader(const char *filePath) {
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

// Link shaders
void BaseObject::linkShaders(unsigned int vertexShader, unsigned int fragmentShader, int &success) {
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
void BaseObject::setShaderBuffers(float *vertices, int sv, unsigned int *indices, int si) {
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

// Set matrix coordinates for projection
void BaseObject::setMat4(std::string uniformName, glm::mat4x4 mat) {
  int location = glGetUniformLocation(shaderProgram, uniformName.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}