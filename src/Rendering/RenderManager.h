#pragma once

#include "Config/config.h"
#include "Indexer/Indexer.h"
#include "Helpers/QLogger.h"
#include "Client/SDCommandsInterface.h"
#include "Helpers/States.h"

#include "Rendering/objects/BaseObject.h"
#include "Rendering/objects/image/Image.h"
#include "Rendering/objects/Selection.h"
#include "Rendering/Camera.h"
#include "Rendering/Canvas.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

class RenderManager {
public:
  explicit RenderManager(GLFWwindow &w);
  ~RenderManager();

  // Actively rendered canvas
  int m_activeId = 0;
  std::vector<std::shared_ptr<Canvas>> m_canvas;
  // Camera details
  bool m_cameraDrag = false;
  std::shared_ptr<Camera> m_camera;
  // Selection
  std::shared_ptr<Selection> m_selection;
  // Right click context window
  bool m_contextWindowVisible = false;
  // Main update loop
  void update();

  /*
    CANVAS FUNCTIONS
  */

  // Recalculate frame buffer on window resize
  static void calculateFramebuffer(int width, int height);
  // Create new canvas object for rendering
  std::shared_ptr<Canvas> createCanvas(int x, int y, const std::string &name);
  // Get current active canvas
  std::shared_ptr<Canvas> getActiveCanvas();
  // Make a canvas active
  void selectCanvas(int id);
  // Send an image to current active canvas
  void sendImageToCanvas(Image &im);
  // Set capture buffer flag (Move pixels inside selection to a texture)
  void captureBuffer();

  /*
    IMAGE GENERATION
  */

  // Get / Set image to use for base rendering
  void useImage(std::string path);
  const std::string getImage();

  // Generate txt2img
  void textToImage(std::string prompt, std::string negative_prompt, std::string &sampler_name, int samples, int steps,
                   double cfg, int seed, int width, int height, int &renderState);

  // Generate img2img
  void imageToImage(std::string &imgPath, std::string prompt, std::string negative_prompt, std::string &samplerName,
                    int samples, int steps, double cfg, double strength, int seed, int &renderState);

  void genFromSelection();

  /*
    CALLBACKS
  */
  static void mouse_cursor_callback(GLFWwindow *window, double xposIn, double yposIn);
  static void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
  static void mouse_btn_callback(GLFWwindow *window, int button, int action, int mods);
  static void GLFWErrorCallBack(int, const char *err_str);
  static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                         const GLchar *message, const void *userParam);

private:
  GLFWwindow &m_window;
  static GLuint fbo;
  static GLuint m_colorBufferTexture;

  std::string m_useImage;
  bool m_captureBuffer = false;

  // Process inputs
  void logicLoop();
  // Render loop
  void renderLoop();
};