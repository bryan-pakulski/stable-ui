#include "RenderManager.h"
#include "Config/config.h"
#include "GLFW/glfw3.h"
#include "Rendering/OrthographicCamera.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "Indexer/MetaData.h"

// Initialise render manager
RenderManager::RenderManager(GLFWwindow &w) : m_window{w} {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager::RenderManager RenderManager initialized");

  // Allow access to camera variable through static callback function
  glfwSetWindowUserPointer(&m_window, (void *)this);

  // Create objects
  m_camera = std::shared_ptr<OrthographicCamera>(new OrthographicCamera(&m_window));
  m_selection = std::shared_ptr<Selection>(new Selection(glm::ivec2{0, 0}, &m_window, m_camera));

  // Initialise selection buffer texture
  m_selectionBuffer = std::shared_ptr<GLImage>(new GLImage(m_selection->m_size.x, m_selection->m_size.y, "buffer"));

  createCanvas(0, 0, "default");
}

// Destructor, destroy remaining instances
RenderManager::~RenderManager() {}

// Main update function, checks for object cap before calling instance logic /
// render loops
void RenderManager::update() {
  logicLoop();
  renderLoop();
}

// Update logic on instanced objects
void RenderManager::logicLoop() {
  m_camera->updateLogic();

  for (auto &canvas : m_canvas) {
    canvas->updateLogic();
  }

  m_selection->updateLogic();
}

// Update visuals on instanced objects
void RenderManager::renderLoop() {
  m_camera->updateVisual();

  getActiveCanvas()->updateVisual();
  getActiveCanvas()->renderImages();

  m_selection->updateVisual();
}

// Set capture render buffer flag, check for out of bounds condition and raise error if required
void RenderManager::captureBuffer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager::captureBuffer capturing to texture, selection at: ",
                             m_selection->getPosition().x, m_selection->getPosition().y);

  std::vector<RGBAPixel> pixels =
      getActiveCanvas()->getPixelsAtSelection(m_selection->getPosition(), m_selection->m_size);
  m_selectionBuffer->resize(m_selection->m_size.x, m_selection->m_size.y);

  // Create the texture buffer with correct format, internal format and properties
  glBindTexture(GL_TEXTURE_2D, m_selectionBuffer->m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_selectionBuffer->m_width, m_selectionBuffer->m_width, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderManager::saveBuffer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager::saveBuffer saving to data/output/buffer.png");
  GLHELPER::SaveTextureToFile("data/output/buffer.png", &m_selectionBuffer->m_texture, m_selectionBuffer->m_width,
                              m_selectionBuffer->m_height);
}

void RenderManager::outpaintSelection() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "RenderManager::outpaintSelection outpainting coordinates: ", m_selection->getPosition().x,
                             m_selection->getPosition().y);

  // Get image as base64 string
  captureBuffer();
  std::string b64Image = GLHELPER::textureToBase64String(&m_selectionBuffer->m_texture, m_selectionBuffer->m_width,
                                                         m_selectionBuffer->m_height);

  QLogger::GetInstance().Log(LOGLEVEL::INFO, b64Image);

  // TODO: process pixels and send to outpainting pipeline
  // TODO: Use PIL Processesing to create mask for outpainting https: // note.nkmk.me/en/python-pillow-composite/
}

// Make a canvas active
void RenderManager::selectCanvas(int id) {
  if (getActiveCanvas()) {
    m_canvas[m_activeId]->m_active = false;
  }

  m_activeId = id;
  m_canvas[m_activeId]->m_active = true;
}

// Create new canvas object & return a reference
std::shared_ptr<Canvas> RenderManager::createCanvas(int x, int y, const std::string &name) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager::createCanvas Creating new canvas");
  m_canvas.emplace_back(new Canvas(glm::ivec2{x, y}, name, &m_window, m_camera));
  selectCanvas(m_canvas.size() - 1);
  return m_canvas.back();
}

// Get active canvas
std::shared_ptr<Canvas> RenderManager::getActiveCanvas() {
  if (m_canvas.size() > 0) {
    return m_canvas[m_activeId];
  } else {
    return nullptr;
  }
}

// Select an image to use as a base for generation
void RenderManager::useImage(std::string path) { m_baseImage = path; }
const std::string RenderManager::getImage() { return m_baseImage; }

// Get image from canvas, based on selection coordinates
void RenderManager::sendImageToCanvas(GLImage &im) {
  m_canvas[m_activeId]->createImage(std::shared_ptr<GLImage>(new GLImage(im)), m_selection->getPosition());
}

// Mouse movement callback
void RenderManager::mouse_cursor_callback(GLFWwindow *window, double xposIn, double yposIn) {
  RenderManager *rm;
  rm = (RenderManager *)glfwGetWindowUserPointer(window);

  // Move camera view
  if (rm->m_cameraDrag) {
    rm->m_camera->moveCamera(glm::vec2{xposIn, yposIn});
  }

  // Catch selection coordinates
  if (rm->m_selection->m_dragging) {
    rm->m_selection->UpdateDrag(glm::vec2{xposIn, yposIn});
  }
}

// Mouse button callback function for dragging camera and interacting with canvas
void RenderManager::mouse_btn_callback(GLFWwindow *window, int button, int action, int mods) {
  // Check if the click was inside an ImGui window or popup
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }

  RenderManager *rm;
  rm = (RenderManager *)glfwGetWindowUserPointer(window);

  if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    if (GLFW_PRESS == action) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);

      rm->m_cameraDrag = true;
      rm->m_camera->onMousePressed(glm::vec2{xpos, ypos});
    } else if (GLFW_RELEASE == action) {
      rm->m_cameraDrag = false;
    }
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    rm->m_selection->m_dragging = true;
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    rm->m_selection->m_dragging = false;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    rm->m_contextWindowVisible = true;
  }
}

void RenderManager::mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  // Check if the click was inside an ImGui window or popup
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }

  RenderManager *rm = (RenderManager *)glfwGetWindowUserPointer(window);
  rm->m_camera->m_zoom -= yoffset * rm->m_camera->m_zoomSpeed;
}

// Callback to log GL errors
void RenderManager::GLFWErrorCallBack(int, const char *err_str) { QLogger::GetInstance().Log(LOGLEVEL::ERR, err_str); }

void GLAPIENTRY RenderManager::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                               const GLchar *message, const void *userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}