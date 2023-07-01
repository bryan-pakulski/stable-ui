#pragma once

#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "Rendering/objects/Image.h"

// This class wraps an image class and contains some flags to check for visibility based on
// Camera and world coordinates
class Layer : public BaseObject {

public:
  bool m_renderFlag = true;

public:
  Layer(std::shared_ptr<OrthographicCamera> c, glm::ivec2 dimensions, std::string name);
  ~Layer() {}

  void updateLogic() override;
  void updateVisual() override;

  // Adds image to render stack, not merged into the raw pixel data
  void addImage(Image im);

  // Merge image into render stack
  void mergeImage(Image im);

  // Deletes selected pixels from raw pixel data
  void deleteSelection(glm::ivec2 p1, glm::ivec2 p2);

private:
  std::vector<Image> m_images;
  std::shared_ptr<OrthographicCamera> m_camera;
  std::string m_name = "Layer";

  bool m_updateTexture;
  std::shared_ptr<GLImage> m_rawImage;
  std::shared_ptr<Image> m_layerImage;
  const glm::ivec2 c_size;
  std::vector<RGBAPixel> pixelData;
};