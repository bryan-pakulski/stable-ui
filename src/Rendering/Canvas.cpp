#include "Canvas.h"
#include <chrono>

Canvas::~Canvas() {}

Canvas::Canvas(std::pair<int, int> coords, const std::string &name, GLFWwindow *w,
               std::shared_ptr<OrthographicCamera> c)
    : BaseObject(coords), m_coords{coords}, m_name{name} {
  int success;
  m_window = w;
  m_camera = std::shared_ptr<OrthographicCamera>(c);

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
  std::shared_ptr<shader> gridShader = std::shared_ptr<shader>(new shader);
  std::shared_ptr<shader> fboShader = std::shared_ptr<shader>(new shader);

  std::string vBackgroundShaderSource = readShader("data/shaders/StarField_V.glsl");
  std::string fBackgroundShaderSource = readShader("data/shaders/StarField_F.glsl");
  unsigned int backgroundVertexShader = initVertexShader(vBackgroundShaderSource.c_str(), success);
  unsigned int backgroundFragmentShader = initFragmentShader(fBackgroundShaderSource.c_str(), success);
  linkShaders(backgroundVertexShader, backgroundFragmentShader, success, backgroundShader);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices), backgroundShader);
  createShader(backgroundShader, "background");

  std::string vGridShaderSource = readShader("data/shaders/Grid_V.glsl");
  std::string fGridShaderSource = readShader("data/shaders/Grid_F.glsl");
  unsigned int gridVertexShader = initVertexShader(vGridShaderSource.c_str(), success);
  unsigned int gridFragmentShader = initFragmentShader(fGridShaderSource.c_str(), success);
  linkShaders(gridVertexShader, gridFragmentShader, success, gridShader);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices), gridShader);
  createShader(gridShader, "background_grid");
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
  if (CONFIG::STAR_FIELD.get()) {
    renderStarField();
  } else {
    renderGrid();
  }
}

void Canvas::renderStarField() {
  glUseProgram(getShader("background")->shaderProgram);

  // View code
  setMat4("view", m_camera->GetViewMatrix(), "background");
  setMat4("projection", m_camera->GetProjectionMatrix(), "background");

  glm::mat4 model =
      glm::translate(glm::mat4(1.0f), glm::vec3(m_camera->m_position.x, m_camera->m_position.y, 1.0f)) * // translation
      glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                                  // rotation
      glm::scale(glm::mat4(1.0f), glm::vec3(m_camera->m_screen.x, m_camera->m_screen.y, 1.0f));          // scale
  setMat4("model", model, "background");

  // Set screen size, camera coords and time
  glUniform2f(glGetUniformLocation(getShader("background")->shaderProgram, "iResolution"), m_camera->m_screen.x,
              m_camera->m_screen.y);
  glUniform2f(glGetUniformLocation(getShader("background")->shaderProgram, "iMouse"), -m_camera->m_position.x,
              -m_camera->m_position.y);

  // TODO: increment time until we hit the max value for a float, then decrement to zero and repeat
  m_time += 0.01;
  glUniform1f(glGetUniformLocation(getShader("background")->shaderProgram, "iTime"), m_time);

  glBindVertexArray(getShader("background")->VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Canvas::renderGrid() {
  glUseProgram(getShader("background_grid")->shaderProgram);

  // View code
  setMat4("view", m_camera->GetViewMatrix(), "background_grid");
  setMat4("projection", m_camera->GetProjectionMatrix(), "background_grid");

  glm::mat4 model =
      glm::translate(glm::mat4(1.0f), glm::vec3(m_camera->m_position.x, m_camera->m_position.y, 0.0f)) * // translation
      glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                                  // rotation
      glm::scale(glm::mat4(1.0f), glm::vec3(m_camera->m_screen.x * m_camera->m_zoom,
                                            m_camera->m_screen.y * m_camera->m_zoom, 1.0f)); // scale
  setMat4("model", model, "background_grid");

  glUniform2f(glGetUniformLocation(getShader("background_grid")->shaderProgram, "iResolution"), m_camera->m_screen.x,
              m_camera->m_screen.y);
  glUniform2f(glGetUniformLocation(getShader("background_grid")->shaderProgram, "iPos"), -m_camera->m_position.x,
              m_camera->m_position.y);
  glUniform1f(glGetUniformLocation(getShader("background_grid")->shaderProgram, "zoom"), m_camera->m_zoom);

  glBindVertexArray(getShader("background_grid")->VAO);
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