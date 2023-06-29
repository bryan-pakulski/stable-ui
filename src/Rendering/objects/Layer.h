#pragma once

#include "Rendering/objects/Image.h"

// The layer class is responsible for aggregating all images that fall under it into a single texture on screen
class Layer : public BaseObject {

public:
  Layer(GLFWwindow *w, std::shared_ptr<OrthographicCamera> c);
  ~Layer();

  void updateLogic() override;
  void updateVisual() override;
  void renderGrid();
  void renderImages();
  void setTexture(GLuint *id);

  // Retrieve raw pixel data from all images that fall space between two world space coordinates
  std::vector<RGBAPixel> getPixelsAtSelection(glm::ivec2 position, glm::ivec2 size, bool mask = false);

  // Update or create new image wrapper from a basic GL image
  void createImage(std::shared_ptr<GLImage>, glm::ivec2 position);

  // Delete image by index
  void deleteImage(int index) { m_images.erase(m_images.begin() + index); }

  // image visibility control
  void hideImage(int index) { m_images[index]->m_renderFlag = false; }
  void showImage(int index) { m_images[index]->m_renderFlag = true; }

private:
  bool m_active = false;
  std::vector<std::unique_ptr<Image>> m_images;
  std::shared_ptr<OrthographicCamera> m_camera;
};