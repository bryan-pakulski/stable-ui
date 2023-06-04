#include "Selection.h"
#include "Rendering/OrthographicCamera.h"
#include <vector>

Selection::~Selection() {}

Selection::Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<OrthographicCamera> c)
    : BaseObject(coords) {
  int success;
  m_window = w;
  m_camera = std::shared_ptr<OrthographicCamera>(c);
  m_position = glm::vec3(0.0f);

  // Vertex data
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

  std::shared_ptr<shader> sh = std::shared_ptr<shader>(new shader);

  std::string vShaderSource = readShader("data/shaders/Selection_V.glsl");
  std::string fShaderSource = readShader("data/shaders/Selection_F.glsl");

  unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), success);
  unsigned int fragmentShader = initFragmentShader(fShaderSource.c_str(), success);

  linkShaders(vertexShader, fragmentShader, success, sh);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices), sh);

  createShader(sh, "selection");

  // Initialise selection buffer texture
  glGenTextures(1, &m_selection_texture_buffer);
}

std::pair<int, int> Selection::getCoordinates() { return std::pair<int, int>{m_position.x, m_position.y}; }

void Selection::startCapture(float x, float y) { m_captureInProgress = true; }

void Selection::updateCapture(float x, float y) {
  glm::vec2 convertedCoords = m_camera->screenToGlobalCoordinates(x, y);

  // Round coordinates to nearest multiple of snapping distance
  int roundedX = static_cast<int>(std::round(convertedCoords.x / m_pixelSnap)) * m_pixelSnap;
  int roundedY = static_cast<int>(std::round(convertedCoords.y / m_pixelSnap)) * m_pixelSnap;

  // Assign to m_position
  m_position.x = static_cast<float>(roundedX);
  m_position.y = -static_cast<float>(roundedY);
}

void Selection::updateLogic() {
  // TODO: Check if selection was updated? if so trigger texture to be regenerated?
}

void Selection::updateVisual() {
  glUseProgram(getShader("selection")->shaderProgram);

  // View code
  setMat4("view", m_camera->GetViewMatrix(), "selection");

  // Projection code
  setMat4("projection", m_camera->GetProjectionMatrix(), "selection");

  // Model code
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, 0.0f)) *  // translation
                    glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * // rotation
                    glm::scale(glm::mat4(1.0f), glm::vec3(m_size.first, m_size.second, 1.0f));      // scale
  setMat4("model", model, "selection");

  // Render the square
  // Set the line width
  glLineWidth(1.0f);

  // Draw the square
  glBindVertexArray(getShader("selection")->VAO);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
  glBindVertexArray(0);
}

void Selection::captureBuffer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Selection::captureBuffer Capturing at screenspace coords: ", m_position.x,
                             m_position.y, "\n\tWorld space coords: ", m_position.x, m_position.y,
                             "Capture size: ", m_size.first, m_size.second);

  // Allocate memory block size of pixel count
  std::vector<unsigned char> pixels(m_size.first * m_size.second * 4);

  // Read pixels starting from position with specified width, height, format and type
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glReadPixels(m_position.x, m_position.y, m_size.first, m_size.second, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  // Check for errors here

  // Create the texture buffer with correct format, internal format and properties
  glGenTextures(1, &m_selection_texture_buffer);
  glBindTexture(GL_TEXTURE_2D, m_selection_texture_buffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_size.first, m_size.second, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  // If filtering is requred:
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Selection::saveBuffer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Selection::saveBuffer Saving selection buffer to file ",
                             "data/output/buffer.png");
  GLHELPER::SaveTextureToFile("data/output/buffer.png", &m_selection_texture_buffer, m_size.first, m_size.second);
}