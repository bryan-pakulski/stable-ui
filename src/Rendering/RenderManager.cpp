#include "RenderManager.h"

// Initialise render manager
RenderManager::RenderManager(GLFWwindow &w) : m_window{w} {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "RenderManager initialized");
  // glfwSetKeyCallback(&m_window, key_callback);

  m_mainWindow =
      new MainWindow(std::pair<int, int>{CONFIG::WINDOW_WIDTH.get(), CONFIG::WINDOW_HEIGHT.get()},
                     std::pair<int, int>{CONFIG::WINDOW_WIDTH.get(), CONFIG::WINDOW_HEIGHT.get()}, &m_window);

  m_windowSelection = new WindowSelection(std::pair<int, int>{0, 0}, &m_window);

  // Intialise python interface for calling commands
  SDCommandsInterface::GetInstance();

  // Create initial canvas
  createCanvas(CONFIG::WINDOW_WIDTH.get(), CONFIG::WINDOW_HEIGHT.get());
}

// Destructor, destroy remaining instances
RenderManager::~RenderManager() {
  for (auto &obj : objectList) {
    delete obj;
  }
  objectList.clear();

  delete m_mainWindow;
  delete m_windowSelection;
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

  m_mainWindow->updateLogic();
  m_windowSelection->updateLogic();
}

// Update visuals on instanced objects
void RenderManager::renderLoop() {
  for (auto &obj : objectList) {
    obj->updateVisual();
  }

  m_mainWindow->updateVisual();
  m_windowSelection->updateVisual();
}

// Text to Image, render result to canvas
void RenderManager::textToImage(Canvas &c, std::string prompt, std::string negative_prompt, int samples, int steps, double cfg, int seed, int width, int height,
                                bool &finishedFlag, std::string model_name) {

  // Generate & Retrieve newly generated image
  SDCommandsInterface::GetInstance().textToImage(prompt, negative_prompt, samples, steps, cfg, seed, width, height, finishedFlag, model_name);
}

// Key callback function to map keypresses / actions to object instantiation
void RenderManager::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  int width, height;
  glfwGetWindowSize(window, &width, &height);
}

// Make a canvas active, pass texture to main window
void RenderManager::selectCanvas(int id) {
  m_active = id;
  m_mainWindow->setMainWindowTexture(&m_canvas[m_active]->m_canvas);
}

// Create a new canvas from our generated one to set for the main window
void RenderManager::setCanvas(Canvas &c) {
  m_canvas[m_active]->m_image_source = c.m_image_source;
  m_canvas[m_active]->loadFromImage(m_canvas[m_active]->m_image_source);
}

// Create new canvas object & return a reference
std::shared_ptr<Canvas> RenderManager::createCanvas(int x, int y) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Creating canvas %d x %d", x, y);
  m_canvas.emplace_back(new Canvas(x, y, std::to_string(m_canvas.size())));
  selectCanvas(m_canvas.size() - 1);
  return m_canvas.back();
}

// Get active canvas
std::shared_ptr<Canvas> RenderManager::getActiveCanvas() {
  if (m_canvas.size() > 0) {
    return m_canvas[m_active];
  } else {
    return nullptr;
  }
}

// Callback to log GL errors
void RenderManager::GLFWErrorCallBack(int, const char *err_str) { QLogger::GetInstance().Log(LOGLEVEL::ERR, err_str); }

void GLAPIENTRY RenderManager::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                               const GLchar *message, const void *userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}