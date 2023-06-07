#pragma once

#include "Config/config.h"
#include "Config/types.h"
#include "Indexer/Indexer.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"

#include "Rendering/objects/BaseObject.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "Rendering/objects/Selection.h"
#include "Rendering/OrthographicCamera.h"
#include "Rendering/Canvas.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <vector>

/*
  This class is responsible for managing the camera and rendering of our main canvas / providing canvas interaction
*/

class RenderManager {
public:
  // Actively rendered canvas
  int m_activeId = 0;
  std::vector<std::shared_ptr<Canvas>> m_canvas;
  // Camera details
  bool m_cameraDrag = false;
  std::shared_ptr<OrthographicCamera> m_camera;
  // Selection
  std::shared_ptr<Selection> m_selection;
  // Right click context window
  bool m_contextWindowVisible = false;

public:
  explicit RenderManager(GLFWwindow &w);
  ~RenderManager();

  // Main update loop
  void update();

  /*
    CANVAS FUNCTIONS
  */

  // Create new canvas object for rendering
  std::shared_ptr<Canvas> createCanvas(int x, int y, const std::string &name);
  // Get current active canvas
  std::shared_ptr<Canvas> getActiveCanvas();
  // Make a canvas active
  void selectCanvas(int id);
  // Send an image to current active canvas
  void sendImageToCanvas(GLImage &im);
  // Set capture buffer flag (Move pixels inside selection to a texture)
  void captureBuffer();
  void genFromSelection();
  void useImage(std::string path);
  const std::string getImage();

  /*
    CALLBACKS
  */
  // Recalculate frame buffer on window resize
  static void recalculateFramebuffer(int width, int height);
  static void mouse_cursor_callback(GLFWwindow *window, double xposIn, double yposIn);
  static void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
  static void mouse_btn_callback(GLFWwindow *window, int button, int action, int mods);
  static void GLFWErrorCallBack(int, const char *err_str);
  static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                         const GLchar *message, const void *userParam);

private:
  GLFWwindow &m_window;

  std::string m_baseImage;
  bool m_captureBuffer = false;

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};