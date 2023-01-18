//
// Created by BryanP on 1/8/2022.
//

#pragma once

#include "../Config/config.h"
#include "../QLogger.h"
#include "../SDInterface/SDCommandsInterface.h"

#include "objects/BaseObject.h"
#include "objects/image/Image.h"
#include "objects/Selection.h"
#include "Camera.h"
#include "Canvas.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

namespace rm {
const int MAX_OBJECTS = 100;
const int MIN_OBJECTS = 5;
}; // namespace rm

class StableManager {
public:
  explicit StableManager(GLFWwindow &w);
  static inline std::vector<BaseObject *> objectList;
  ~StableManager();

  int m_activeId = 0;
  std::vector<std::shared_ptr<Canvas>> m_canvas;

  // Camera details
  std::shared_ptr<Camera> m_camera;
  bool m_cameraDrag = false;

  // Selection
  std::shared_ptr<Selection> m_selection;

  // Main update loop
  void update();

  // Create new canvas object for rendering
  std::shared_ptr<Canvas> createCanvas(int x, int y, const std::string &name);

  // Get current active canvas
  std::shared_ptr<Canvas> getActiveCanvas();

  // Make a canvas active
  void selectCanvas(int id);

  // Create a new canvas with a base image
  void sendImageToCanvas(Image &im);

  // Generate txt2img
  void textToImage(std::string prompt, std::string negative_prompt, int samples, int steps, double cfg, int seed,
                   int width, int height, bool &finishedFlag, std::string model_name, bool half_precision);

  // Generate img2img
  void imageToImage(std::string path, std::string prompt, std::string negative_prompt, int samples, int steps,
                    double strength, int seed, bool &finishedFlag, std::string model_name, bool half_precision);

  // Callbacks
  static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
  static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
  static void mouse_btn_callback(GLFWwindow *window, int button, int action, int mods);
  static void GLFWErrorCallBack(int, const char *err_str);
  static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                         const GLchar *message, const void *userParam);

private:
  GLFWwindow &m_window;

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};