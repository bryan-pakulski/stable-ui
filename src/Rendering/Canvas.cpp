#include "Canvas.h"
#include "Display/ErrorHandler.h"
#include "Helpers/QLogger.h"
#include "Helpers/GLHelper.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include <chrono>

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
}

// Retrieve raw pixel data from all images that fall space between two world space coordinates
std::vector<RGBAPixel> Canvas::getPixelsAtSelection(glm::ivec2 position, glm::ivec2 size) {

  // Raw pixel data, treat as 2d array
  std::vector<RGBAPixel> pixels(size.x * size.y);

  // We are assuming that our m_editorGrid stays sorted, which should be true as we only emplace data at the end of the
  // vector
  for (auto &image : m_editorGrid) {

    // Convert the origin as center to top left for simpler calculation
    glm::ivec2 l1 = {(position.x - (size.x / 2)), (position.y + (size.y / 2))};
    glm::ivec2 r1 = {(position.x + (size.x / 2)), (position.y - (size.y / 2))};

    glm::ivec2 l2 = {(image->getPosition().x - image->m_image->m_width / 2),
                     (image->getPosition().y + image->m_image->m_height / 2)};
    glm::ivec2 r2 = {(image->getPosition().x + image->m_image->m_width / 2),
                     (image->getPosition().y - image->m_image->m_height / 2)};

    if (image->intersects(l1, r1, l2, r2) && image->m_renderFlag) {

      // Calculate intersection rectangle
      // As these fall on the cartesian plane, these can be negative!
      int leftX = std::max(l1.x, l2.x);
      int rightX = std::min(r1.x, r2.x);
      int topY = std::min(l1.y, l2.y);
      int bottomY = std::max(r1.y, r2.y);

      int width = rightX - leftX;
      int height = topY - bottomY;

      // Bind image texture and read pixel data for our selected area
      glBindTexture(GL_TEXTURE_2D, image->m_image->m_texture);

      std::vector<RGBAPixel> imgPixels(image->m_image->m_width * image->m_image->m_height);
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgPixels.data());

      QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Canvas::getPixelsAtSelection Intersection Rect: {", leftX, topY, "}",
                                 "{", rightX, bottomY, "}");

      // TODO: Iterate over the pixels in the intersection rectangle and extract the relative pixels from imgPixels into
      // pixels vector, note that openGL textures are indexed from the bottom left opposed to our coordinates which
      // index from the top left
      for (int x = leftX - l2.x; x < (leftX - l2.x) + width; x++) {
        for (int y = l2.y - topY; y < l2.y - bottomY; y++) {
          int index = (y * size.y) + x;

          int xoffset = l1.x - leftX;
          int yoffset = l2.y - topY;
          int offsetIndex = ((y - yoffset) * size.y) + (x - xoffset);

          if (offsetIndex >= size.x * size.y || offsetIndex < 0) {
            QLogger::GetInstance().Log(LOGLEVEL::ERR, "Invalid buffer captured! expected max size ", size.x * size.y,
                                       " got out of bounds index at", offsetIndex);
            ErrorHandler::GetInstance().setError("Invalid image buffer captured!");

            return pixels;
            glBindTexture(GL_TEXTURE_2D, 0);
          } else {
            pixels[offsetIndex] = imgPixels[index];
          }
        }
      }

      pixels = GLHELPER::Rotate1DSquareMatrixClockwise<RGBAPixel>(pixels);
      pixels = GLHELPER::Rotate1DSquareMatrixClockwise<RGBAPixel>(pixels);

      for (int i = 0; i < size.y - 1; i++) {
        std::reverse(&pixels.at(i * size.x), &pixels.at(i * size.x + size.x)); // reverse row
      }

      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }

  return pixels;
}

void Canvas::updateLogic() {

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
              -m_camera->m_position.y);
  glUniform1f(glGetUniformLocation(getShader("background_grid")->shaderProgram, "zoom"), m_camera->m_zoom);

  glBindVertexArray(getShader("background_grid")->VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Canvas::renderImages() {
  // Check which chunks are in view and should be rendered
  for (auto &image : m_editorGrid) {
    image->updateVisual();
  }
}

// TODO: Create a new grid chunk object/s based on provided image & coordinates
void Canvas::createImage(std::shared_ptr<GLImage> image, glm::ivec2 position) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "Canvas::createImage Creating new image chunk at coordinates: ", position.x, position.y,
                             "on canvas: ", m_name);
  m_editorGrid.emplace_back(new Image(image, m_camera, position));
}