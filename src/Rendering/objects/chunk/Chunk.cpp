#include "Chunk.h"

Chunk::Chunk(std::shared_ptr<Image> im, std::shared_ptr<Camera> c, int x, int y, int id)
    : m_image{im}, m_coordinates{std::pair<int, int>(x, y)}, BaseObject(std::pair<int, int>(x, y)) {
  int success = 0;
  m_camera = std::shared_ptr<Camera>(c);
  m_image->loadFromImage(m_image->m_image_source.c_str(), true);

  // Vertex data
  float vertices[] = {
      // positions        // colors         // texture coords
      (float)m_image->m_width / 2,  (float)m_image->m_height / 2,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      (float)m_image->m_width / 2,  -(float)m_image->m_height / 2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -(float)m_image->m_width / 2, -(float)m_image->m_height / 2, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -(float)m_image->m_width / 2, (float)m_image->m_height / 2,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
  };

  // Index buffer // Element Buffer Objects (EBO)
  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  std::string vShaderSource = readShader("data/shaders/Base_V.glsl");
  std::string fShaderSource = readShader("data/shaders/Base_F.glsl");

  unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), success);
  unsigned int fragmentShader = initFragmentShader(fShaderSource.c_str(), success);

  std::shared_ptr<shader> sh = std::shared_ptr<shader>(new shader);

  linkShaders(vertexShader, fragmentShader, success, sh);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices), sh);

  createShader(sh, "chunk");

  if (!success) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Chunk::Chunk Error creating new chunk");
  }
}

Chunk::~Chunk() {}

// Check if grid is currently visible based on world coordinates and window size
bool Chunk::visible(const std::pair<int, int> &windowCoords, const std::pair<int, int> &windowSize) {
  // TODO: use simple box boundary check to see if 512x512 grid is within the window (offsetting the window for world
  // coordinates)
  if (m_coordinates.first < windowCoords.first + windowSize.first &&
      m_coordinates.first + m_image->m_width > windowCoords.first &&
      m_coordinates.second < windowCoords.second + windowSize.second &&
      m_coordinates.second + m_image->m_height > windowCoords.second) {
    return true;
  } else {
    return true;
  }
}

void Chunk::updateLogic() {}

// Render onto screen, offset based on world coordinates & window size
void Chunk::updateVisual() {
  if (m_renderFlag) {
    glUseProgram(getShader("chunk")->shaderProgram);

    // View code
    setMat4("view", m_camera->getViewMatrix(), "chunk");
    setMat4("projection", m_camera->getProjectionMatrix(), "chunk");

    // Model projection code
    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(m_coordinates.first, m_coordinates.second, 0.0f)) * // translation
        glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                             // rotation
        glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));                                     // scale
    setMat4("model", model, "chunk");

    // Update texture information
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, m_image->m_texture);

    glBindVertexArray(getShader("chunk")->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  }
}