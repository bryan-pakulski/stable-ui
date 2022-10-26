#include "RenderManager.h"

// Initialise render manager
RenderManager::RenderManager(GLFWwindow &w) : m_window{w} {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager initialized");
  
  // TODO: Key callback breaks imgui input
  // glfwSetKeyCallback(&m_window, key_callback);

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
  for (auto &obj : objectList) {
    obj->updateLogic();
  }

  for (auto &canvas : m_canvas) {
    canvas->updateLogic();
  }
}

// Update visuals on instanced objects
void RenderManager::renderLoop() {
  for (auto &obj : objectList) {
    obj->updateVisual();
  }

  for (auto &canvas : m_canvas) {
    if (canvas->m_active) {
      canvas->updateVisual();
    }
  } 
}

// Text to Image, render result to canvas
void RenderManager::textToImage(std::string prompt, std::string negative_prompt, int samples, int steps, double cfg, int seed, int width, int height,
                                bool &finishedFlag, std::string model_name, bool half_precision) {

  // Generate & Retrieve newly generated image
  SDCommandsInterface::GetInstance().textToImage(prompt, negative_prompt, samples, steps, cfg, seed, width, height, finishedFlag, model_name, half_precision);
}

// Key callback function to map keypresses / actions to object instantiation
void RenderManager::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  int width, height;
  glfwGetWindowSize(window, &width, &height);
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
  m_canvas.emplace_back(new Canvas(std::pair<int, int>{x, y}, name, &m_window));
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

// Create a new canvas with a base image
void RenderManager::createCanvasFromImage(Image &im) {
  // TODO: create new canvas with image
}

// Callback to log GL errors
void RenderManager::GLFWErrorCallBack(int, const char *err_str) { QLogger::GetInstance().Log(LOGLEVEL::ERR, err_str); }

void GLAPIENTRY RenderManager::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                               const GLchar *message, const void *userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}