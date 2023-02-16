#include "Canvas.h"

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
      16000.0f,  16000.0f,  0.0f, 1.0f, 0.0f, 0.0f, 160.0f, 160.0f, // top right
      16000.0f,  -16000.0f, 0.0f, 0.0f, 1.0f, 0.0f, 160.0f, 0.0f,   // bottom right
      -16000.0f, -16000.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,   0.0f,   // bottom left
      -16000.0f, 16000.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f,   160.0f  // top left
  };

  // Index buffer // Element Buffer Objects (EBO)
  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  std::string vShaderSource = readShader("data/shaders/MainWindow_V.glsl");
  std::string fShaderSource = readShader("data/shaders/MainWindow_F.glsl");

  unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), success);
  unsigned int fragmentShader = initFragmentShader(fShaderSource.c_str(), success);

  linkShaders(vertexShader, fragmentShader, success);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices));

  int imgX = 0;
  int imgY = 0;
  GLHELPER::LoadTextureFromFile("data/images/uv_grid.png", &m_texture_id, &imgX, &imgY, true);
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
  glUseProgram(shaderProgram);

  // View code
  setMat4("viewProjection", m_camera->getViewProjectionMatrix());

  // Model code, default canvas scale is 16,000 x 16,000 pixels
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)) *                // translation
                    glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * // rotation
                    glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));         // scale
  setMat4("model", model);

  // Update texture information
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

  // Check which chunks are in view and should be rendered
  for (auto &chunk : m_editorGrid) {
    if (chunk->visible(m_coords, m_screen)) {
      chunk->updateVisual();
    }
  }
}

// Set Canvas Texture
void Canvas::setTexture(GLuint *id) { m_texture_id = *id; }

// TODO: Build a texture based on the visible grid chunks
void Canvas::updateMainWindowTexture() {}

// TODO: Create a new grid chunk object/s based on provided image & coordinates
void Canvas::createChunk(std::shared_ptr<Image> image, std::pair<int, int> chunk_coordinates) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "Canvas::createChunk Creating new image chunk at coordinates: ", chunk_coordinates.first,
                             chunk_coordinates.second, "on canvas: ", m_name);
  m_editorGrid.emplace_back(
      new Chunk(image, m_camera, chunk_coordinates.first, chunk_coordinates.second, m_editorGrid.size()));
}

void Canvas::deleteChunk(int index) { m_editorGrid.erase(m_editorGrid.begin() + index); }

// Chunk visibility control
void Canvas::hideChunk(int index) { m_editorGrid[index]->m_renderFlag = false; }

void Canvas::showChunk(int index) { m_editorGrid[index]->m_renderFlag = true; }