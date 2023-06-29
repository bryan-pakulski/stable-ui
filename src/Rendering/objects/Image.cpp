#include "Rendering/objects/Image.h"
#include "Helpers/QLogger.h"

Image::Image(std::shared_ptr<GLImage> im, std::shared_ptr<OrthographicCamera> c, glm::ivec2 position)
    : BaseObject(position), m_image{im} {
  int success = 0;
  m_camera = std::shared_ptr<OrthographicCamera>(c);
  m_image->loadFromImage(m_image->m_image_source.c_str(), true);

  // Vertex data
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

  std::string vShaderSource = readShader("data/shaders/Base_V.glsl");
  std::string fShaderSource = readShader("data/shaders/Base_F.glsl");

  unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), success);
  unsigned int fragmentShader = initFragmentShader(fShaderSource.c_str(), success);

  std::shared_ptr<shader> sh = std::shared_ptr<shader>(new shader);

  linkShaders(vertexShader, fragmentShader, success, sh);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices), sh);

  createShader(sh, "image");

  if (!success) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Image::Image Error creating new image");
  }
}

// Check if an image intersects with x1,y1, x2,y2
bool Image::intersects(const glm::ivec2 &l1, const glm::ivec2 &r1, const glm::ivec2 &l2, const glm::ivec2 &r2) {

  QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Image::intersects, checking intersection of [", l1.x, l1.y, r1.x, r1.y,
                             "] and [", l2.x, l2.y, r2.x, r2.y, "]");

  bool wp = std::min(r1.x, r2.x) > std::max(l1.x, l2.x);
  bool hp = std::min(r1.y, r2.y) > std::max(l1.y, l2.y);

  if (wp && hp) {
    QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Image::intersects, found intersecting image!");
    return true;
  } else {
    return false;
  }
}

// Render onto screen, offset based on world coordinates & window size
void Image::updateVisual() {
  if (m_renderFlag) {
    glUseProgram(getShader("image")->shaderProgram);

    // View code
    setMat4("view", m_camera->GetViewMatrix(), "image");
    setMat4("projection", m_camera->GetProjectionMatrix(), "image");

    // Model projection code
    // Offset position x2 to account for the centering effect
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, 0.0f)) *     // translation
                      glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                  // rotation
                      glm::scale(glm::mat4(1.0f), glm::vec3(m_image->m_width, m_image->m_height, 1.0f)); // scale
    setMat4("model", model, "image");

    // Update texture information
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, m_image->m_texture);

    glBindVertexArray(getShader("image")->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  }
}