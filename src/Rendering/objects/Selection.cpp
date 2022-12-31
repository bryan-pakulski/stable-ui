#include "Selection.h"

Selection::~Selection() {}

Selection::Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c) : BaseObject(coords) {
  int success;
  m_window = w;
  m_camera = std::shared_ptr<Camera>(c);

  glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);
  m_coords.first = 0.0f;
  m_coords.second = 0.0f;

  // Vertex data
  float vertices[] = {
      // positions        // colors         // texture coords
      100.0f,  100.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      100.0f,  -100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -100.0f, -100.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -100.0f, 100.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
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
}

std::pair<int, int> Selection::getCoordinates() { return m_coords; }

// Set drag start coordinates
void Selection::dragStart(int x, int y) {
  m_listening = true;
  m_dragCoords.first.first = x;
  m_dragCoords.first.second = y;
}

// Set drag end coordinates
void Selection::dragStop(int x, int y) {
  m_listening = false;
  m_dragCoords.second.first = x;
  m_dragCoords.second.second = y;
}

// Basic check to see if we actually have performed a dragging action
bool Selection::isDragged() {
  m_listening = false;
  // Check if the mouse was dragged
  int xdiff = abs(m_dragCoords.second.first - m_dragCoords.first.first);
  int ydiff = abs(m_dragCoords.second.second - m_dragCoords.first.second);
  if (xdiff > c_drag_trigger && ydiff > c_drag_trigger) {
    return true;
  } else {
    return false;
  }
}

// Updates the selection texture buffer with a selection of given screen coordinates
void Selection::makeSelection() {

  // Make selection from screen buffer
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Making selection at coordinates: ", m_dragCoords.first.first,
                             m_dragCoords.first.second, "and", m_dragCoords.second.first, m_dragCoords.second.second);

  // Reset drag values
  m_dragCoords.first = {0, 0};
  m_dragCoords.second = {0, 0};
}

void Selection::updateLogic() {
  // TODO: Check if selection was updated? if so trigger texture to be regenerated?
}

void Selection::updateVisual() {
  glUseProgram(shaderProgram);

  // View code
  setMat4("viewProjection", m_camera->getViewProjectionMatrix());

  // Model code
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_coords.first, m_coords.second, 0.0f)) * // translation
                    glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                   // rotation
                    glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));                           // scale
  setMat4("model", model);

  // Update texture information
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}