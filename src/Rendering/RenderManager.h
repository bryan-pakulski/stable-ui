//
// Created by BryanP on 1/8/2022.
//

#pragma once

#include "../Config/config.h"
#include "../QLogger.h"
#include "canvas/Canvas.h"
#include "objects/BaseObject.h"
#include "objects/MainWindow.h"
#include "objects/WindowSelection.h"
#include "../SDInterface/SDCommandsInterface.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

namespace rm {
const int MAX_OBJECTS = 100;
const int MIN_OBJECTS = 5;
}; // namespace rm

class RenderManager {
public:
  explicit RenderManager(GLFWwindow &w);
  static inline std::vector<BaseObject *> objectList;
  ~RenderManager();

  int m_active = 0;
  std::vector<std::shared_ptr<Canvas>> m_canvas;

  // Main update loop
  void update();

  // Create new canvas object for rendering
  std::shared_ptr<Canvas> createCanvas(int x, int y);

  // Get current active canvas
  std::shared_ptr<Canvas> getActiveCanvas();

  // Make a canvas active
  void selectCanvas(int id);

  // Create a copy of a passed canvas to set for the main window
  void setCanvas(Canvas &c);

  // Generate txt2img
  void textToImage(Canvas &c, std::string prompt, std::string negative_prompt, int samples, int steps, double cfg, int seed, int width, int height,
                   bool &finishedFlag, std::string model_name);

  // Generate img2img
  void imageToImage();

  // Error callback for GLFW logging
  static void GLFWErrorCallBack(int, const char *err_str);
  static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                         const GLchar *message, const void *userParam);

private:
  GLFWwindow &m_window;
  MainWindow *m_mainWindow;
  WindowSelection *m_windowSelection;

  static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};