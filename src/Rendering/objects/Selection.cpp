#include "Selection.h"
#include <vector>

Selection::~Selection() {}

Selection::Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c) : BaseObject(coords) {
  int success;
  m_window = w;
  m_camera = std::shared_ptr<Camera>(c);
  m_position = glm::vec3(0.0f);

  glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);
  // Vertex data
  float vertices[] = {
      // positions        // colors         // texture coords
      512.0f,  512.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      512.0f,  -512.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -512.0f, -512.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -512.0f, 512.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
  };

  // Index buffer // Element Buffer Objects (EBO)
  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  std::string vShaderSource = readShader("data/shaders/Selection_V.glsl");
  std::string fShaderSource = readShader("data/shaders/Selection_F.glsl");

  unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), success);
  unsigned int fragmentShader = initFragmentShader(fShaderSource.c_str(), success);

  linkShaders(vertexShader, fragmentShader, success);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices));

  GLHELPER::LoadTextureFromFile("data/images/selection.png", &m_texture_id, &m_size.first, &m_size.second, true);

  // Initialise selection buffer texture
  glGenTextures(1, &m_selection_texture_buffer);
  // glBindTexture(GL_TEXTURE_2D, m_selection_texture_buffer);
}

std::pair<int, int> Selection::getCoordinates() { return std::pair<int, int>{m_position.x, -m_position.y}; }

// Offset camera
void Selection::moveSelectionPosition(float x, float y) {
  m_position.x -= -(x);
  m_position.y -= -(y);
}

void Selection::updateLogic() {
  // TODO: Check if selection was updated? if so trigger texture to be regenerated?
}

void Selection::updateVisual() {
  glUseProgram(shaderProgram);

  // View code
  setMat4("viewProjection", m_camera->getViewProjectionMatrix());

  // Model code
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *    // translation
                    glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * // rotation
                    glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));         // scale
  setMat4("model", model);

  glUniform2f(glGetUniformLocation(shaderProgram, "offset"), (float)m_position.x, (float)m_position.y);
  glUniform2f(glGetUniformLocation(shaderProgram, "uViewportSize"), (float)m_camera->getScreenSize().first,
              (float)m_camera->getScreenSize().second);
  // Update texture information
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Selection::captureBuffer() {
  glBindTexture(GL_TEXTURE_2D, m_selection_texture_buffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<int>(m_size.first / m_camera->m_zoom),
               static_cast<int>(m_size.second / m_camera->m_zoom), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Copy the window contents to the texture
  int windowWidth, windowHeight;
  glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);
  glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_position.x,
                   windowHeight - (m_position.y + m_size.second), // bottom-left corner of selection
                   static_cast<int>(m_size.first / m_camera->m_zoom),
                   static_cast<int>(m_size.second / m_camera->m_zoom), // scaled width and height of selection
                   0);

  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "Selection::captureBuffer Capturing framebuffer details\n\twindow size: ", windowWidth,
                             windowHeight, "\n\tselection position: ", m_position.x, m_position.y,
                             "\n\tcamera position: ", m_camera->m_position.x, m_camera->m_position.y);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Selection::saveBuffer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Selection::saveBuffer Saving selection buffer to file ",
                             "data/output/buffer.png");
  GLHELPER::SaveTextureToFile("data/output/buffer.png", &m_selection_texture_buffer, m_size.first, m_size.second);
}