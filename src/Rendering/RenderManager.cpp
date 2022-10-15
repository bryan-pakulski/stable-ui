#include "RenderManager.h"

// Initialise render manager
RenderManager::RenderManager(GLFWwindow &w) : window{w} {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager initialized");
  glfwSetKeyCallback(&window, key_callback);
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
void RenderManager::logicLoop() {}

// Update visuals on instanced objects
void RenderManager::renderLoop() {}

// Key callback function to map keypresses / actions to object instantiation
void RenderManager::key_callback(GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
  int width, height;
  glfwGetWindowSize(window, &width, &height);
}

// Create new canvas object
void RenderManager::createCanvas(int x, int y) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Creating canvas %d x %d", x, y);
  m_canvas.emplace_back(new Canvas(x, y));
}