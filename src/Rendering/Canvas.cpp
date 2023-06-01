#include "Canvas.h"
#include <chrono>

Canvas::~Canvas() {}

Canvas::Canvas(std::pair<int, int> coords, const std::string &name, GLFWwindow *w, std::shared_ptr<Camera> c)
    : BaseObject(coords), m_coords{coords}, m_name{name} {
  int success;
  m_window = w;
  m_camera = std::shared_ptr<Camera>(c);

  glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

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

  std::shared_ptr<shader> backgroundShader = std::shared_ptr<shader>(new shader);
  std::shared_ptr<shader> fboShader = std::shared_ptr<shader>(new shader);

  std::string vBackgroundShaderSource = readShader("data/shaders/MainWindow_V.glsl");
  std::string fBackgroundShaderSource = readShader("data/shaders/MainWindow_F.glsl");
  std::string vFBOShaderSource = readShader("data/shaders/Base_V.glsl");
  std::string fFBOShaderSource = readShader("data/shaders/Base_F.glsl");

  unsigned int backgroundVertexShader = initVertexShader(vBackgroundShaderSource.c_str(), success);
  unsigned int backgroundFragmentShader = initFragmentShader(fBackgroundShaderSource.c_str(), success);
  unsigned int fboVertexShader = initVertexShader(vFBOShaderSource.c_str(), success);
  unsigned int fboFragmentShader = initFragmentShader(fFBOShaderSource.c_str(), success);

  linkShaders(backgroundVertexShader, backgroundFragmentShader, success, backgroundShader);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices), backgroundShader);
  linkShaders(fboVertexShader, fboFragmentShader, success, fboShader);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices), fboShader);

  createShader(backgroundShader, "background");
  createShader(fboShader, "fbo");
}

void Canvas::updateLogic() {
  // Get updated screen size
  glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

  // Check which chunks are in view and should be rendered
  for (auto &chunk : m_editorGrid) {
    chunk->updateLogic();
  }
}

void Canvas::updateVisual() {

  // TODO: make background configurable i.e. grid / star system etc...
  if (CONFIG::STAR_FIELD.get() == 1) {
    renderBackground();
  }

  // Render FBO texture on top of background
  glUseProgram(getShader("fbo")->shaderProgram);

  setMat4("view", m_camera->getViewMatrix(), "fbo");
  setMat4("projection", m_camera->getProjectionMatrix(), "fbo");

  glm::mat4 model =
      glm::translate(glm::mat4(1.0f), glm::vec3(m_camera->m_position.x, m_camera->m_position.y, 0.0f)) * // translation
      glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                                  // rotation
      glm::scale(glm::mat4(1.0f), glm::vec3(m_camera->m_screen.first * m_camera->m_zoom,
                                            m_camera->m_screen.second * m_camera->m_zoom, 1.0f)); // scale
  setMat4("model", model, "fbo");

  glBindVertexArray(getShader("fbo")->VAO);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);
  glBindVertexArray(getShader("background")->VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Canvas::renderBackground() {
  glUseProgram(getShader("background")->shaderProgram);

  // View code
  setMat4("view", m_camera->getViewMatrix(), "background");
  setMat4("projection", m_camera->getProjectionMatrix(), "background");

  glm::mat4 model =
      glm::translate(glm::mat4(1.0f), glm::vec3(m_camera->m_position.x, m_camera->m_position.y, 0.0f)) * // translation
      glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                                  // rotation
      glm::scale(glm::mat4(1.0f), glm::vec3(m_camera->m_screen.first * m_camera->m_zoom,
                                            m_camera->m_screen.second * m_camera->m_zoom, 1.0f)); // scale
  setMat4("model", model, "background");

  // Set screen size, camera coords and time
  glUniform2f(glGetUniformLocation(getShader("background")->shaderProgram, "iResolution"), m_camera->m_screen.first,
              m_camera->m_screen.second);
  glUniform2f(glGetUniformLocation(getShader("background")->shaderProgram, "iMouse"), -m_camera->m_position.x,
              -m_camera->m_position.y);

  // TODO: increment time until we hit the max value for a float, then decrement to zero and repeat
  m_time += 0.01;
  glUniform1f(glGetUniformLocation(getShader("background")->shaderProgram, "iTime"), m_time);

  glBindVertexArray(getShader("background")->VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Canvas::renderImages() {
  // Check which chunks are in view and should be rendered
  for (auto &chunk : m_editorGrid) {
    if (chunk->visible(m_coords, m_screen)) {
      chunk->updateVisual();
    }
  }
}

// Set Canvas Texture
void Canvas::setTexture(GLuint *id) { m_texture_id = *id; }

// TODO: Create a new grid chunk object/s based on provided image & coordinates
void Canvas::createImage(std::shared_ptr<Image> image, std::pair<int, int> chunk_coordinates) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "Canvas::createImage Creating new image chunk at coordinates: ", chunk_coordinates.first,
                             chunk_coordinates.second, "on canvas: ", m_name);
  m_editorGrid.emplace_back(
      new Chunk(image, m_camera, chunk_coordinates.first, chunk_coordinates.second, m_editorGrid.size()));
}

void Canvas::deleteChunk(int index) { m_editorGrid.erase(m_editorGrid.begin() + index); }

// Chunk visibility control
void Canvas::hideChunk(int index) { m_editorGrid[index]->m_renderFlag = false; }

void Canvas::showChunk(int index) { m_editorGrid[index]->m_renderFlag = true; }