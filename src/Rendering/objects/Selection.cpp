#include "Selection.h"
#include "Rendering/OrthographicCamera.h"
#include <vector>

Selection::~Selection() {}

Selection::Selection(glm::ivec2 position, GLFWwindow *w, std::shared_ptr<OrthographicCamera> c) : BaseObject(position) {
  int shaderInitSuccess;
  m_window = w;
  m_camera = std::shared_ptr<OrthographicCamera>(c);

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

  std::shared_ptr<shader> sh = std::shared_ptr<shader>(new shader);

  std::string vShaderSource = readShader("data/shaders/Selection_V.glsl");
  std::string fShaderSource = readShader("data/shaders/Selection_F.glsl");

  unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), shaderInitSuccess);
  unsigned int fragmentShader = initFragmentShader(fShaderSource.c_str(), shaderInitSuccess);

  linkShaders(vertexShader, fragmentShader, shaderInitSuccess, sh);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices), sh);

  createShader(sh, "selection");
}

void Selection::UpdateDrag(glm::vec2 position) {
  m_position = m_camera->screenToGlobalCoordinates(glm::vec2{std::round(position.x), std::round(position.y)});

  if (m_snap) {
    m_position.x = roundUp(m_position.x, m_pixelSnap);
    m_position.y = roundUp(m_position.y, m_pixelSnap);
  }
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
                    glm::scale(glm::mat4(1.0f), glm::vec3(m_size.x, m_size.y, 1.0f));               // scale
  setMat4("model", model, "selection");

  glLineWidth(1.0f);
  glBindVertexArray(getShader("selection")->VAO);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
  glBindVertexArray(0);
}