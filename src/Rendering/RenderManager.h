//
// Created by BryanP on 1/8/2022.
//

#pragma once

#include "../QLogger.h"
#include "objects/Canvas.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <vector>

namespace rm {
const int MAX_OBJECTS = 100;
const int MIN_OBJECTS = 5;
}; // namespace rm

class RenderManager {
public:
  explicit RenderManager(GLFWwindow &w);
  ~RenderManager();

  // Main update loop
  void update();

  // Create new canvas object for rendering
  void createCanvas(int x, int y);

private:
  GLFWwindow &window;
  std::vector<std::unique_ptr<Canvas>> m_canvas;

  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};