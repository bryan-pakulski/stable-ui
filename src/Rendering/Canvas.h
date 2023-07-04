#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include "Helpers/QLogger.h"
#include "Helpers/GLHelper.h"
#include "Rendering/OrthographicCamera.h"

#include "Rendering/objects/Layer.h"

#include "Rendering/objects/BaseObject.h"
#include "Rendering/objects/Image.h"
#include "Rendering/objects/GLImage/GLImage.h"

class Canvas : public BaseObject {
public:
  bool m_active = false;
  std::vector<std::unique_ptr<Layer>> m_editorGrid;
  std::string m_name;

public:
  Canvas(glm::ivec2 position, const std::string &name, GLFWwindow *w, std::shared_ptr<OrthographicCamera> c);
  virtual ~Canvas();

  void updateLogic() override;
  void updateVisual() override;
  void renderStarField();
  void renderGrid();
  void renderImages();
  void setTexture(GLuint *id);

  // Retrieve raw pixel data from all images that fall space between two world space coordinates
  std::vector<RGBAPixel> getPixelsAtSelection(glm::ivec2 position, glm::ivec2 size, bool mask = false);

  void eraseSelection(glm::ivec2 position, glm::ivec2 size);

  // Update or create new image wrapper from a basic GL image
  void createImage(int layerId, std::shared_ptr<GLImage>, glm::ivec2 position);

  int getActiveLayer() { return m_activeLayer; }
  void setActiveLayer(int id) {
    if (id < 0 || id > m_editorGrid.size()) {
      m_activeLayer = 0;
    } else {
      m_activeLayer = id;
    }
  }

  // Create a new layer
  void createLayer(glm::ivec2 dimensions, std::string name, bool protect = false) {
    m_editorGrid.push_back(std::unique_ptr<Layer>(new Layer(m_camera, dimensions, name, protect)));
  }

  // Delete layer by index
  void deleteLayer(int index) {
    if (m_editorGrid[index]->c_protected) {
      QLogger::GetInstance().Log(LOGLEVEL::INFO, "Unable to delete protected layer!");
    } else {
      m_editorGrid.erase(m_editorGrid.begin() + index);
    }
  }

  // layer visibility control
  void hideLayer(int index) { m_editorGrid[index]->m_renderFlag = false; }
  void showLayer(int index) { m_editorGrid[index]->m_renderFlag = true; }

private:
  std::shared_ptr<OrthographicCamera> m_camera;
  float m_time = 0.0f;
  int m_activeLayer = 0;
};