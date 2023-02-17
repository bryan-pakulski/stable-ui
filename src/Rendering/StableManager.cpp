#include "StableManager.h"
#include "GLFW/glfw3.h"
#include "imgui_impl_glfw.h"

// Initialise render manager
StableManager::StableManager(GLFWwindow &w) : m_window{w} {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::StableManager StableManager initialized");

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
  SDCommandsInterface::GetInstance().launchSDModelServer();
}

// Destructor, destroy remaining instances
StableManager::~StableManager() {}

// Main update function, checks for object cap before calling instance logic /
// render loops
void StableManager::update() {
  logicLoop();
  renderLoop();
}

// Update logic on instanced objects
void StableManager::logicLoop() {
  m_camera->updateLogic();

  for (auto &canvas : m_canvas) {
    canvas->updateLogic();
  }

  m_selection->updateLogic();
}

// Update visuals on instanced objects
void StableManager::renderLoop() {
  m_camera->updateVisual();

  for (auto &canvas : m_canvas) {
    if (canvas->m_active) {
      canvas->updateVisual();
    }
  }

  m_selection->updateVisual();

  // Capture render buffer to texture if flag is set
  if (m_captureBuffer == true) {
    m_selection->captureBuffer();
    m_captureBuffer = false;
  }
}

// Set capture render buffer flag, check for out of bounds condition and raise error if required
void StableManager::captureBuffer() {
  // TODO: Check camera x / y offset in conjunction with selection offset and display size
  m_captureBuffer = true;
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::captureBuffer Setting capture buffer flag to ",
                             m_captureBuffer);
}

// Generate img2img from the selection buffer, check for out of bounds condition and raise error if required
void StableManager::genFromSelection() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "StableManager::genFromSelection Generating img2img from selection on canvas ");

  // TODO: call img2img with selection buffer
}

// Make a canvas active, pass texture to main window
void StableManager::selectCanvas(int id) {
  // Disable old canvas
  if (getActiveCanvas()) {
    m_canvas[m_activeId]->m_active = false;
  }

  m_activeId = id;
  // Set new canvas to active
  m_canvas[m_activeId]->m_active = true;
}

// Create new canvas object & return a reference
std::shared_ptr<Canvas> StableManager::createCanvas(int x, int y, const std::string &name) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "StableManager::createCanvas Creating new canvas");
  m_canvas.emplace_back(new Canvas(std::pair<int, int>{x, y}, name, &m_window, m_camera));
  selectCanvas(m_canvas.size() - 1);
  return m_canvas.back();
}

// Get active canvas
std::shared_ptr<Canvas> StableManager::getActiveCanvas() {
  if (m_canvas.size() > 0) {
    return m_canvas[m_activeId];
  } else {
    return nullptr;
  }
}

// Get image from canvas, based on selection coordinates
void StableManager::sendImageToCanvas(Image &im) {
  // Create copy of image to send to canvas
  m_canvas[m_activeId]->createChunk(std::shared_ptr<Image>(new Image(im)), m_selection->getCoordinates());
}

// STABLE DIFFUSION SERVER COMMANDS
void StableManager::attachModel(YAML::Node model, std::string &hash, std::string &precision) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "StableManager::attachModel Attaching model: ", model["name"].as<std::string>(),
                             " to Stable Diffusion Docker Server");
  // Optional parameters
  std::string vae = "";
  if (model["vae"]) {
    vae = model["vae"].as<std::string>();
  }

  // Build model struct
  m_model.name = model["name"].as<std::string>();
  m_model.hash = hash;
  m_model.path = model["path"].as<std::string>();

  SDCommandsInterface::GetInstance().attachModelToServer(
      model["path"].as<std::string>(), model["config"].as<std::string>(), vae, precision, m_modelLoaded);
}

int StableManager::getModelState() { return m_modelLoaded; }

// Text to Image, render result to canvas
void StableManager::textToImage(std::string prompt, std::string negative_prompt, std::string &samplerName, int samples,
                                int steps, double cfg, int seed, int width, int height, int &renderState) {

  // Generate & Retrieve newly generated image
  SDCommandsInterface::GetInstance().textToImage(m_model.path, getActiveCanvas()->m_name, prompt, negative_prompt,
                                                 samplerName, samples, steps, cfg, seed, width, height, renderState);
}

// Image to Image, render result to canvas
void StableManager::imageToImage(std::string &imgPath, std::string prompt, std::string negative_prompt,
                                 std::string &samplerName, int samples, int steps, double cfg, double strength,
                                 int seed, int &renderState) {

  // Generate & Retrieve newly generated image
  SDCommandsInterface::GetInstance().imageToImage(m_model.path, getActiveCanvas()->m_name, imgPath, prompt,
                                                  negative_prompt, samplerName, samples, steps, cfg, strength, seed,
                                                  renderState);
}

// Key callback function to map keypresses / actions to object instantiation
void StableManager::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  int width, height;
  glfwGetWindowSize(window, &width, &height);
}

// Mouse movement callback
void StableManager::mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  StableManager *rm;
  rm = (StableManager *)glfwGetWindowUserPointer(window);

  // Move camera view
  if (rm->m_cameraDrag && (rm->m_camera->cur_mouse.x > CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get())) {
    rm->m_camera->moveCameraPosition(static_cast<float>(xposIn) - rm->m_camera->prev_mouse.x,
                                     static_cast<float>(yposIn) - rm->m_camera->prev_mouse.y);
    rm->m_camera->prev_mouse.x = xposIn;
    rm->m_camera->prev_mouse.y = yposIn;
  }

  // Move selection window around
  if (rm->m_selectionDrag && (rm->m_selection->cur_mouse.x > CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get())) {
    rm->m_selection->moveSelectionPosition(static_cast<float>(xposIn) - rm->m_selection->prev_mouse.x,
                                           static_cast<float>(yposIn) - rm->m_selection->prev_mouse.y);
    rm->m_selection->prev_mouse.x = xposIn;
    rm->m_selection->prev_mouse.y = yposIn;
  }

  rm->m_camera->cur_mouse.x = xposIn;
  rm->m_camera->cur_mouse.y = yposIn;
  rm->m_selection->cur_mouse.x = xposIn;
  rm->m_selection->cur_mouse.y = yposIn;
}

// Mouse button callback function for dragging camera and interacting with canvas
void StableManager::mouse_btn_callback(GLFWwindow *window, int button, int action, int mods) {
  StableManager *rm;
  rm = (StableManager *)glfwGetWindowUserPointer(window);

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && (rm->m_camera->cur_mouse.x > CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get())) {
    if (GLFW_PRESS == action) {
      rm->m_camera->prev_mouse.x = rm->m_camera->cur_mouse.x;
      rm->m_camera->prev_mouse.y = rm->m_camera->cur_mouse.y;
      rm->m_cameraDrag = true;
    } else if (GLFW_RELEASE == action) {
      rm->m_cameraDrag = false;
    }
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && (rm->m_selection->cur_mouse.x > CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get())) {
    if (GLFW_PRESS == action) {
      rm->m_selection->prev_mouse.x = rm->m_selection->cur_mouse.x;
      rm->m_selection->prev_mouse.y = rm->m_selection->cur_mouse.y;
      rm->m_selectionDrag = true;
    } else if (GLFW_RELEASE == action) {
      rm->m_selectionDrag = false;
    }
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && (rm->m_camera->cur_mouse.x > CONFIG::IMGUI_TOOLS_WINDOW_WIDTH.get())) {
    rm->m_contextWindowVisible = true;
  }
}

// Close window callback
void StableManager::close_callback(GLFWwindow *window) {
  // SDCommandsInterface::GetInstance().terminateSDModelServer();
}

// Build image from canvas, based on selection coordinates

// Callback to log GL errors
void StableManager::GLFWErrorCallBack(int, const char *err_str) { QLogger::GetInstance().Log(LOGLEVEL::ERR, err_str); }

void GLAPIENTRY StableManager::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                               const GLchar *message, const void *userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}