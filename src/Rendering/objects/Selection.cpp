#include "Selection.h"

Selection::~Selection() {}

Selection::Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c) : BaseObject(coords) {
  int success;
  m_window = w;
  m_scale = 100.0f;
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
  unsigned int fragmentShader =  initFragmentShader(fShaderSource.c_str(), success);

  linkShaders(vertexShader, fragmentShader, success);
  setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices));

  int imgX = 0;
  int imgY = 0;
  GLHELPER::LoadTextureFromFile("data/images/selection.png", &m_texture_id, &imgX, &imgY, true);
}

std::pair<int, int> Selection::getCoordinates() {
    return m_coords;
}

void Selection::updateLogic() { 
    // Get updated screen size
    glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second); 
}

void Selection::updateVisual() {
    glUseProgram(shaderProgram);

    // View code
    setMat4("viewProjection", m_camera->getViewProjectionMatrix());

    // Model code
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_coords.first, m_coords.second, 0.0f)) *     // translation
            glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                                 // rotation
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));                                   // scale
    setMat4("model", model);

    // Update texture information
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

}