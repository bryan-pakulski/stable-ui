#include "RenderManager.h"
#include "GLFW/glfw3.h"
#include "imgui_impl_glfw.h"

// Initialise render manager
RenderManager::RenderManager(GLFWwindow &w) : m_window{w} {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager initialized");

  // Allow access to camera variable through static callback function
  glfwSetWindowUserPointer(&m_window, (void *)this);

  // Create Camera
  m_camera = std::shared_ptr<Camera>(new Camera(&m_window));

  // Create additional objects
  m_selection = std::shared_ptr<Selection>(new Selection(std::pair<int, int>(0, 0), &m_window, m_camera));

  // Create initial canvas
  createCanvas(0, 0, "default");

  // Intialise python interface for calling commands
  SDCommandsInterface::GetInstance();
}

// Destructor, destroy remaining instances
RenderManager::~RenderManager() {
  for (auto &obj : objectList) {
    delete obj;
  }
  objectList.clear();
}

// Main update function, checks for object cap before calling instance logic /
// render loops
void RenderManager::update() {
  logicLoop();
  renderLoop();
}

// Update logic on instanced objects
void RenderManager::logicLoop() {
  m_camera->updateLogic();

  for (auto &obj : objectList) {
    obj->updateLogic();
  }

  for (auto &canvas : m_canvas) {
    canvas->updateLogic();
  }

  m_selection->updateLogic();
}

// Update visuals on instanced objects
void RenderManager::renderLoop() {
  m_camera->updateVisual();

  for (auto &obj : objectList) {
    obj->updateVisual();
  }

  for (auto &canvas : m_canvas) {
    if (canvas->m_active) {
      canvas->updateVisual();
    }
  }

  m_selection->updateVisual();
}

// Text to Image, render result to canvas
void RenderManager::textToImage(std::string prompt, std::string negative_prompt, int samples, int steps, double cfg,
                                int seed, int width, int height, bool &finishedFlag, std::string model_name,
                                bool half_precision) {

  // Generate & Retrieve newly generated image
  SDCommandsInterface::GetInstance().textToImage(prompt, negative_prompt, samples, steps, cfg, seed, width, height,
                                                 finishedFlag, model_name, half_precision);
}

// Image to Image, render result to canvas
void RenderManager::imageToImage(std::string path, std::string prompt, std::string negative_prompt, int samples,
                                 int steps, double strength, int seed, bool &finishedFlag, std::string model_name,
                                 bool half_precision) {

  // Generate & Retrieve newly generated image
  SDCommandsInterface::GetInstance().imageToImage(path, prompt, negative_prompt, samples, steps, strength, seed,
                                                  finishedFlag, model_name, half_precision);
}

// Key callback function to map keypresses / actions to object instantiation
void RenderManager::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  int width, height;
  glfwGetWindowSize(window, &width, &height);
}

// Mouse movement callback
void RenderManager::mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  RenderManager *rm;
  rm = (RenderManager *)glfwGetWindowUserPointer(window);

  if (rm->m_cameraDrag) {
    rm->m_camera->moveCameraPosition(static_cast<float>(xposIn) - rm->m_camera->prev_mouse.x,
                                     static_cast<float>(yposIn) - rm->m_camera->prev_mouse.y);
    rm->m_camera->prev_mouse.x = xposIn;
    rm->m_camera->prev_mouse.y = yposIn;
  }

  rm->m_camera->cur_mouse.x = xposIn;
  rm->m_camera->cur_mouse.y = yposIn;

  // Check if we are making a selection
  if (rm->m_selection->m_listening) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {

      // Store the final x,y coordinates
      rm->m_selection->dragStop(xposIn, yposIn);

      // Check if our action was an actual drag event
      if (rm->m_selection->isDragged()) {
        rm->m_selection->makeSelection();
      }
    }
  }
}

// Mouse button callback function for dragging camera and interacting with canvas
void RenderManager::mouse_btn_callback(GLFWwindow *window, int button, int action, int mods) {
  RenderManager *rm;
  rm = (RenderManager *)glfwGetWindowUserPointer(window);

  if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    if (GLFW_PRESS == action) {
      rm->m_camera->prev_mouse.x = rm->m_camera->cur_mouse.x;
      rm->m_camera->prev_mouse.y = rm->m_camera->cur_mouse.y;
      rm->m_cameraDrag = true;
    } else if (GLFW_RELEASE == action) {
      rm->m_cameraDrag = false;
    }
  }

  // Start dragging
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    rm->m_selection->dragStart(xpos, ypos);
  }
}

// Make a canvas active, pass texture to main window
void RenderManager::selectCanvas(int id) {
  // Disable old canvas
  if (getActiveCanvas()) {
    m_canvas[m_activeId]->m_active = false;
  }

  m_activeId = id;
  // Set new canvas to active
  m_canvas[m_activeId]->m_active = true;
}

// Create new canvas object & return a reference
std::shared_ptr<Canvas> RenderManager::createCanvas(int x, int y, const std::string &name) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Creating new canvas");
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

// Get image from canvas, based on selection coordinates
void RenderManager::sendImageToCanvas(Image &im) {
  // Create copy of image to send to canvas
  m_canvas[m_activeId]->createChunk(std::shared_ptr<Image>(new Image(im)), m_selection->getCoordinates());
}

// Build image from canvas, based on selection coordinates

// Callback to log GL errors
void RenderManager::GLFWErrorCallBack(int, const char *err_str) { QLogger::GetInstance().Log(LOGLEVEL::ERR, err_str); }

void GLAPIENTRY RenderManager::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                               const GLchar *message, const void *userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}