//
// Created by BryanP on 1/8/2022.
//

#pragma once

#include "../QLogger.h"

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

private:
  GLFWwindow &window;

  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};