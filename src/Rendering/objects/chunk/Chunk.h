#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <utility>

#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/BaseObject.h"
#include "Rendering/objects/image/Image.h"

// This class wraps an image class and contains some flags to check for visibility based on
// Camera and world coordinates
class Chunk : public BaseObject {
  std::pair<int, int> m_screen{}; // Screen size
  std::shared_ptr<OrthographicCamera> m_camera;

public:
  std::shared_ptr<Image> m_image;
  std::pair<int, int> m_coordinates;
  bool m_renderFlag = true;

  Chunk(std::shared_ptr<Image> im, std::shared_ptr<OrthographicCamera> c, int x, int y, int id);
  ~Chunk();

  // Check if grid is currently visible based on world coordinates and window size
  bool visible(const std::pair<int, int> &windowCoords, const std::pair<int, int> &windowSize);

  void updateLogic() override;
  void updateVisual() override;
};