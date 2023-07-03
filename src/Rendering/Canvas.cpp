#include "Canvas.h"
#include "Display/ErrorHandler.h"
#include <chrono>
#include <memory>

Canvas::~Canvas() {}

Canvas::Canvas(glm::ivec2 position, const std::string &name, GLFWwindow *w, std::shared_ptr<OrthographicCamera> c)
    : BaseObject(position), m_name{name} {
  int success;
  m_window = w;
  m_camera = std::shared_ptr<OrthographicCamera>(c);

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

  // Create default layer
  createLayer(glm::ivec2{2560, 1440}, "Base Layer", true);
}

// i.e. 512x768 causes issues
// Retrieve raw pixel data from all images that fall space between two world space coordinates
// If the mask value is set we won't actually retrieve pixel data but instead create a mask for our sd pipeline to use
std::vector<RGBAPixel> Canvas::getPixelsAtSelection(glm::ivec2 position, glm::ivec2 size, bool mask) {

  // Raw pixel data, treat as 2d array
  std::vector<RGBAPixel> pixels(size.x * size.y);
  std::vector<RGBAPixel> selectionPixels;

  // We are assuming that our m_editorGrid stays sorted, which should be true as we only emplace data at the end of the
  // vector
  for (auto &layer : m_editorGrid) {

    LocalIntersect intersect =
        GLHELPER::GetLocalIntersectSourceDest(position, size, layer->getPosition(), layer->c_size);

    if (intersect.collision && layer->m_renderFlag) {

      // Iterate over the pixels in the intersection rectangle and extract the relative pixels from the image into
      // its own vector, note that openGL textures are indexed from the bottom left opposed to our coordinates which
      // index from the top left
      if (!mask) {
        std::vector<RGBAPixel> selectionPixels = layer->getPixelsAtSelection(intersect.sourceCoordinates);
      }

      // Copy the selectionPixels to our main image
      int i = 0;
      for (int y = intersect.destinationCoordinates.y; y < intersect.destinationCoordinates.w; y++) {
        for (int x = intersect.destinationCoordinates.x; x < intersect.destinationCoordinates.z; x++) {
          int index = y * size.x + x;

          if (index > size.x * size.y || index < 0) {
            QLogger::GetInstance().Log(LOGLEVEL::ERR, "Invalid buffer captured! expected max size ", size.x * size.y,
                                       " got out of bounds index at", index);
            ErrorHandler::GetInstance().setError("Invalid image buffer captured!");
            return pixels;
          } else {
            if (!mask) {
              pixels[index] = selectionPixels.at(i);
            } else {
              pixels[index] = RGBAPixel{0x00, 0x00, 0x00, 0xFF};
            }
          }

          i++;
        }
      }
    }
  }

  // Inverse Y
  pixels = GLHELPER::FlipMatrixY(pixels, size.x, size.y);
  return pixels;
}

// Clear out pixel data in our layer, ignore images
void Canvas::eraseSelection(glm::ivec2 position, glm::ivec2 size) {
  for (auto &layer : m_editorGrid) {
    layer->eraseSelection(position, size);
  }
}

void Canvas::updateLogic() {
  for (auto &layer : m_editorGrid) {
    layer->updateLogic();
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
              -m_camera->m_position.y);
  glUniform1f(glGetUniformLocation(getShader("background_grid")->shaderProgram, "zoom"), m_camera->m_zoom);

  glBindVertexArray(getShader("background_grid")->VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Canvas::renderImages() {
  // Check which chunks are in view and should be rendered
  for (auto &layer : m_editorGrid) {
    layer->updateVisual();
  }
}

// TODO: Create a new grid chunk object/s based on provided image & coordinates
void Canvas::createImage(int layerId, std::shared_ptr<GLImage> image, glm::ivec2 position) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "Canvas::createImage Creating new image chunk at coordinates: ", position.x, position.y,
                             "on canvas: ", m_name);
  m_editorGrid[layerId]->addImage(Image(image, m_camera, position));
}