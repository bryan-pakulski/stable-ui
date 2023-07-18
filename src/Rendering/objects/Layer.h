#pragma once

#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "Rendering/objects/Image.h"

// This class wraps an image class and contains some flags to check for visibility based on
// Camera and world coordinates
class Layer : public BaseObject {

public:
  bool m_renderFlag = true;
  const glm::ivec2 c_size;
  const bool c_protected;
  std::string m_name = "Layer";
  std::vector<RGBAPixel> pixelData;

public:
  Layer(std::shared_ptr<OrthographicCamera> c, glm::ivec2 dimensions, std::string name, bool protect);
  ~Layer() {}

  void updateLogic() override;
  void updateVisual() override;

  // Adds image to render stack, not merged into the raw pixel data
  void addImage(Image im) { m_images.push_back(im); }

  std::vector<Image> &getImages() { return m_images; }

  // Merge new image into raw pixel data
  void mergeImage(Image im);

  // Merge existing image into raw pixel data and remove it from the render stack
  void mergeImageFromStack(int index) {
    mergeImage(m_images[index]);
    m_images.erase(m_images.begin() + index);
  }

  void deleteImage(int index) { m_images.erase(m_images.begin() + index); }

  // Deletes selected pixels from raw pixel data
  void eraseSelection(glm::ivec2 p1, glm::ivec2 p2);

  void updateTexture() { m_updateTexture = true; }

  // Returns array of pixels at coordinate, selection coordinates are indexed from the bottom left
  // In typical cartesian format
  std::vector<RGBAPixel> getPixelsAtSelection(glm::ivec4 coordinates, glm::ivec2 position, glm::ivec2 size);

private:
  std::vector<Image> m_images;
  std::shared_ptr<OrthographicCamera> m_camera;

  bool m_updateTexture;
  std::shared_ptr<GLImage> m_rawImage;
  std::shared_ptr<Image> m_layerImage;
};