//
// Created by BryanP on 1/8/2022.
//

#pragma once

#include "../Config/config.h"
#include "../QLogger.h"
#include "../SDInterface/SDCommandsInterface.h"
#include "../Helpers/States.h"

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

struct model {
  std::string name;
  std::string hash;
  std::string path;
};

class StableManager {
public:
  explicit StableManager(GLFWwindow &w);
  ~StableManager();

  // Actively rendered canvas
  int m_activeId = 0;
  std::vector<std::shared_ptr<Canvas>> m_canvas;
  // Camera details
  bool m_cameraDrag = false;
  std::shared_ptr<Camera> m_camera;
  // Selection
  std::shared_ptr<Selection> m_selection;

  // Main update loop
  void update();

  // CANVAS FUNCTIONS

  // Create new canvas object for rendering
  std::shared_ptr<Canvas> createCanvas(int x, int y, const std::string &name);
  // Get current active canvas
  std::shared_ptr<Canvas> getActiveCanvas();
  // Make a canvas active
  void selectCanvas(int id);
  // Create a new canvas with a base image
  void sendImageToCanvas(Image &im);

  // MODEL SERVER INTERACTION

  // Attach model to server
  void attachModel(YAML::Node model, std::string &hash, std::string &precision);
  // Return model state
  int getModelState();

  // IMAGE GENERATION

  // Generate txt2img
  void textToImage(std::string prompt, std::string negative_prompt, std::string &sampler_name, int samples, int steps,
                   double cfg, int seed, int width, int height, int &renderState);

  // Generate img2img
  void imageToImage(std::string imgPath, std::string prompt, std::string negative_prompt, int samples, int steps,
                    double strength, int seed, int &renderState);

  // CALLBACKS
  static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
  static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
  static void mouse_btn_callback(GLFWwindow *window, int button, int action, int mods);
  static void GLFWErrorCallBack(int, const char *err_str);
  static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                         const GLchar *message, const void *userParam);

private:
  GLFWwindow &m_window;

  int m_modelLoaded = EXECUTION_STATE::PENDING;
  model m_model;

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};