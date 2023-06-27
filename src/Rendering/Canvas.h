#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include "Helpers/QLogger.h"
#include "Helpers/GLHelper.h"
#include "Rendering/OrthographicCamera.h"

#include "Rendering/objects/BaseObject.h"
#include "Rendering/objects/Image.h"
#include "Rendering/objects/GLImage/GLImage.h"

class Canvas : public BaseObject {
public:
  bool m_active = false;
  std::vector<std::unique_ptr<Image>> m_editorGrid;
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

  // Update or create new image wrapper from a basic GL image
  void createImage(std::shared_ptr<GLImage>, glm::ivec2 position);

  // Delete image by index
  void deleteImage(int index) { m_editorGrid.erase(m_editorGrid.begin() + index); }

  // image visibility control
  void hideImage(int index) { m_editorGrid[index]->m_renderFlag = false; }
  void showImage(int index) { m_editorGrid[index]->m_renderFlag = true; }

private:
  std::shared_ptr<OrthographicCamera> m_camera;
  float m_time = 0.0f;
};