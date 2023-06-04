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
  m_selection = std::shared_ptr<Selection>(new Selection(std::pair<int, int>(0, 0), &m_window, m_camera));

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

  // TODO: capture raw pixel data based on selection coordinates and chunks
  if (m_captureBuffer == true) {
    m_selection->captureBuffer();
    m_captureBuffer = false;
  }

  getActiveCanvas()->updateVisual();
  getActiveCanvas()->renderImages();

  m_selection->updateVisual();
}

// Set capture render buffer flag, check for out of bounds condition and raise error if required
void RenderManager::captureBuffer() {
  m_captureBuffer = true;
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager::captureBuffer Setting capture buffer flag to ",
                             m_captureBuffer);
}

// Generate img2img from the selection buffer, check for out of bounds condition and raise error if required
void RenderManager::genFromSelection() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "RenderManager::genFromSelection Generating img2img from selection on canvas ");

  // TODO: call img2img with selection buffer
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
  m_canvas.emplace_back(new Canvas(std::pair<int, int>{x, y}, name, &m_window, m_camera));
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
void RenderManager::sendImageToCanvas(Image &im) {
  m_canvas[m_activeId]->createImage(std::shared_ptr<Image>(new Image(im)), m_selection->getCoordinates());
}

// Text to Image, render result to canvas
void RenderManager::textToImage(std::string prompt, std::string negative_prompt, std::string &samplerName, int samples,
                                int steps, double cfg, int seed, int width, int height, int &renderState) {
  SDCommandsInterface::GetInstance().textToImage(getActiveCanvas()->m_name, prompt, negative_prompt, samplerName,
                                                 samples, steps, cfg, seed, width, height, renderState);
}

// Image to Image, render result to canvas
void RenderManager::imageToImage(std::string &imgPath, std::string prompt, std::string negative_prompt,
                                 std::string &samplerName, int samples, int steps, double cfg, double strength,
                                 int seed, int &renderState) {
  SDCommandsInterface::GetInstance().imageToImage(getActiveCanvas()->m_name, imgPath, prompt, negative_prompt,
                                                  samplerName, samples, steps, cfg, strength, seed, renderState);
}

// Mouse movement callback
void RenderManager::mouse_cursor_callback(GLFWwindow *window, double xposIn, double yposIn) {
  RenderManager *rm;
  rm = (RenderManager *)glfwGetWindowUserPointer(window);

  // Move camera view
  if (rm->m_cameraDrag) {
    rm->m_camera->SetPosition(glm::vec3(xposIn - rm->m_prev_mouse.x, yposIn - rm->m_prev_mouse.y, 1.0f));
    rm->m_prev_mouse.x = xposIn;
    rm->m_prev_mouse.y = yposIn;
  }

  // Catch selection coordinates
  if (rm->m_selection->m_captureInProgress) {
    rm->m_selection->updateCapture(xposIn, yposIn);
  }

  rm->m_prev_mouse.x = xposIn;
  rm->m_prev_mouse.y = yposIn;
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
      rm->m_prev_mouse.x = rm->m_prev_mouse.x;
      rm->m_prev_mouse.y = rm->m_prev_mouse.y;
      rm->m_cameraDrag = true;
    } else if (GLFW_RELEASE == action) {
      rm->m_cameraDrag = false;
    }
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    rm->m_selection->startCapture(xpos, ypos);
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    rm->m_selection->m_captureInProgress = false;
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