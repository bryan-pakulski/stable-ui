#include "RenderManager.h"
#include "Client/StableClient.h"
#include "Config/config.h"
#include "GLFW/glfw3.h"
#include "Helpers/QLogger.h"
#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "StableManager.h"
#include <cstdint>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "Indexer/MetaData.h"

// Initialise render manager
RenderManager::RenderManager(GLFWwindow &w) : m_window{w} {
  QLogger::GetInstance().Log(LOGLEVEL::TRACE, "RenderManager::RenderManager");

  // Allow access to camera variable through static callback function
  glfwSetWindowUserPointer(&m_window, (void *)this);

  // Create objects
  m_camera = std::shared_ptr<OrthographicCamera>(new OrthographicCamera(&m_window));
  m_selection = std::shared_ptr<Selection>(new Selection(glm::ivec2{0, 0}, &m_window, m_camera));

  // Initialise selection buffer texture
  m_selectionBuffer = std::shared_ptr<GLImage>(new GLImage(m_selection->m_size.x, m_selection->m_size.y, "buffer"));
  m_selectionMask = std::shared_ptr<GLImage>(new GLImage(m_selection->m_size.x, m_selection->m_size.y, "buffer_mask"));

  m_txtPipeline = std::shared_ptr<pipelineConfig>(new pipelineConfig());
  m_imgPipeline = std::shared_ptr<pipelineConfig>(new pipelineConfig());
  m_paintPipeline = std::shared_ptr<pipelineConfig>(new pipelineConfig());

  createCanvas(0, 0, "default");
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
void RenderManager::logicLoop() {
  m_camera->updateLogic();

  for (auto &canvas : m_canvas) {
    canvas->updateLogic();
  }

  m_selection->updateLogic();
}

// Update visuals on instanced objects
void RenderManager::renderLoop() {
  m_camera->updateVisual();

  getActiveCanvas()->updateVisual();
  getActiveCanvas()->renderImages();

  m_selection->updateVisual();
}

// Set capture render buffer flag, check for out of bounds condition and raise error if required
void RenderManager::captureBuffer() {
  QLogger::GetInstance().Log(LOGLEVEL::DBG4, "RenderManager::captureBuffer capturing to texture, selection at: ",
                             m_selection->getPosition().x, m_selection->getPosition().y);

  m_selectionBuffer->resize(m_selection->m_size.x, m_selection->m_size.y);
  m_selectionMask->resize(m_selection->m_size.x, m_selection->m_size.y);

  std::vector<RGBAPixel> pixels =
      getActiveCanvas()->getPixelsAtSelection(m_selection->getPosition(), m_selection->m_size);
  std::vector<RGBAPixel> mask =
      getActiveCanvas()->getPixelsAtSelection(m_selection->getPosition(), m_selection->m_size, true);

  // Create the texture buffer with correct format, internal format and properties
  glBindTexture(GL_TEXTURE_2D, m_selectionBuffer->m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_selectionBuffer->m_width, m_selectionBuffer->m_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);

  glBindTexture(GL_TEXTURE_2D, m_selectionMask->m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_selectionMask->m_width, m_selectionMask->m_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, mask.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderManager::saveBuffer() {
  QLogger::GetInstance().Log(LOGLEVEL::DBG4, "RenderManager::saveBuffer saving to data/output/buffer.png");
  GLHELPER::SaveTextureToFile("data/output/buffer.png", &m_selectionBuffer->m_texture, m_selectionBuffer->m_width,
                              m_selectionBuffer->m_height);
}

void RenderManager::eraseSelection() {
  QLogger::GetInstance().Log(LOGLEVEL::DBG4, "RenderManager::eraseSelection deleting selection");

  getActiveCanvas()->eraseSelection(m_selection->getPosition(), m_selection->m_size);
}

void RenderManager::paintSelection(bool sendToCanvas) {
  captureBuffer();

  m_paintPipeline->width = m_selectionBuffer->m_width;
  m_paintPipeline->height = m_selectionBuffer->m_height;
  glm::ivec2 coords = m_selection->getPosition();

  std::string b64Image = GLHELPER::textureToBase64String(&m_selectionBuffer->m_texture, m_selectionBuffer->m_width,
                                                         m_selectionBuffer->m_height);

  std::string b64Mask = GLHELPER::textureToBase64String(&m_selectionMask->m_texture, m_selectionBuffer->m_width,
                                                        m_selectionBuffer->m_height);

  StableManager::GetInstance().outpaint(b64Image, b64Mask, *m_paintPipeline);

  if (sendToCanvas) {
    // TODO: Save current selection coordinates, Once paintGenStatus returns as rendered then draw the new image to the
    // canvas
    GLImage im(m_paintPipeline->width, m_paintPipeline->height, "inpaint");

    // sendImageToCanvasAtPos(im, coords);
  }
}

void RenderManager::saveCanvas(const std::string &filename) {
  datafile df;
  if (datafile().write(getActiveCanvas()->serialise(), filename)) {
    QLogger::GetInstance().Log(LOGLEVEL::INFO, "Saved file: ", filename);
  } else {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Failed to save file: ", filename);
  }
}

void RenderManager::loadCanvas(const std::string &filename) {
  datafile df;

  if (datafile().read(df, filename)) {

    // check for duplicate filenames, attempt to recover if they exist
    for (auto &canvas : m_canvas) {
      if (df["canvas"].getString() == canvas->m_name) {
        QLogger::GetInstance().Log(LOGLEVEL::ERR, "Duplicate canvas! renaming..", filename);
        df["canvas"].setString(canvas->m_name + "_copy");
      }
    }

    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Loading canvas..", df["canvas"].getString());
    createCanvas(df["x"].getInt(), df["y"].getInt(), df["canvas"].getString());

    // Create layers
    for (int i = 0; i < df["layer_count"].getInt(); i++) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "Creating Layer ", std::to_string(i));
      datafile &layer = df["layer"][std::to_string(i)];
      getActiveCanvas()->createLayer(glm::ivec2{layer["width"].getInt(), layer["height"].getInt()},
                                     layer["name"].getString());

      // Copy Raw Pixel Data
      int idx = 0;
      for (RGBAPixel &pixel : getActiveCanvas()->m_editorGrid.back()->pixelData) {
        uint32_t px = df["layer"][std::to_string(i)]["pixelData"].getUInt(idx);
        pixel.red = (px >> 24) & 0xFF;
        pixel.green = (px >> 16) & 0xFF;
        pixel.blue = (px >> 8) & 0xFF;
        pixel.alpha = px & 0xFF;
        idx++;
      }

      // Copy image information
    }

    QLogger::GetInstance().Log(LOGLEVEL::INFO, "Loaded file: ", filename);
  } else {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Failed to load file: ", filename);
  }
}

// Make a canvas active
void RenderManager::selectCanvas(int id) {
  if (getActiveCanvas()) {
    m_canvas[m_activeId]->m_active = false;
  }

  m_activeId = id;
  m_canvas[m_activeId]->m_active = true;
}

// Create new canvas object & return a reference
std::shared_ptr<Canvas> RenderManager::createCanvas(int x, int y, const std::string &name) {
  QLogger::GetInstance().Log(LOGLEVEL::DBG4, "RenderManager::createCanvas Creating new canvas");
  m_canvas.emplace_back(new Canvas(glm::ivec2{x, y}, name, &m_window, m_camera));
  selectCanvas(m_canvas.size() - 1);

  // TODO: configurable layer size when creating canvas?
  getActiveCanvas()->createLayer(glm::ivec2{2560, 1440}, "Output", true);
  return m_canvas.back();
}

// Get active canvas
std::shared_ptr<Canvas> RenderManager::getActiveCanvas() { return m_canvas[m_activeId]; }

// Select an image to use as a base for generation
void RenderManager::useImage(std::string path) { m_baseImage = path; }
const std::string RenderManager::getImage() { return m_baseImage; }

// Send an image to the canvas, ignoring layers
void RenderManager::sendImageToCanvas(GLImage &im) {
  m_canvas[m_activeId]->createImage(m_canvas[m_activeId]->getActiveLayer(), std::shared_ptr<GLImage>(new GLImage(im)),
                                    m_selection->getPosition());
}

// Send an image to the canvas, ignoring layers
void RenderManager::sendImageToCanvasAtPos(GLImage &im, glm::ivec2 position) {
  // TODO: allow control of active layer
  m_canvas[m_activeId]->createImage(m_canvas[m_activeId]->getActiveLayer(), std::shared_ptr<GLImage>(new GLImage(im)),
                                    position);
}

// Mouse movement callback
void RenderManager::mouse_cursor_callback(GLFWwindow *window, double xposIn, double yposIn) {
  RenderManager *rm;
  rm = (RenderManager *)glfwGetWindowUserPointer(window);

  // Move camera view
  if (rm->m_cameraDrag) {
    rm->m_camera->moveCamera(glm::vec2{xposIn, yposIn});
  }

  // Catch selection coordinates
  if (rm->m_selection->m_dragging) {
    rm->m_selection->UpdateDrag(glm::vec2{xposIn, yposIn});
  }
}

// Mouse button callback function for dragging camera and interacting with canvas
void RenderManager::mouse_btn_callback(GLFWwindow *window, int button, int action, int mods) {
  // Check if the click was inside an ImGui window or popup
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }

  RenderManager *rm;
  rm = (RenderManager *)glfwGetWindowUserPointer(window);

  if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    if (GLFW_PRESS == action) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);

      rm->m_cameraDrag = true;
      rm->m_camera->onMousePressed(glm::vec2{xpos, ypos});
    } else if (GLFW_RELEASE == action) {
      rm->m_cameraDrag = false;
    }
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    rm->m_selection->m_dragging = true;
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    rm->m_selection->m_dragging = false;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    rm->m_contextWindowVisible = true;
  }
}

void RenderManager::mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  // Check if the click was inside an ImGui window or popup
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }

  RenderManager *rm = (RenderManager *)glfwGetWindowUserPointer(window);
  rm->m_camera->m_zoom -= yoffset * rm->m_camera->m_zoomSpeed;
}

// Callback to log GL errors
void RenderManager::GLFWErrorCallBack(int, const char *err_str) { QLogger::GetInstance().Log(LOGLEVEL::ERR, err_str); }

void GLAPIENTRY RenderManager::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                               const GLchar *message, const void *userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}