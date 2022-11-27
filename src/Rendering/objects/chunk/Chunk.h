#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <utility>


#include "../../Camera.h"
#include "../BaseObject.h"
#include "../image/Image.h"

// This class wraps an image class and contains some flags to check for visibility based on
// Camera and world coordinates
class Chunk : public BaseObject {
	const std::pair<int, int> c_coordinates;
  std::pair<int, int> m_screen{};   // Screen size
  std::shared_ptr<Camera> m_camera;

public:
  std::shared_ptr<Image> m_image;

  Chunk(std::shared_ptr<Image> im, std::shared_ptr<Camera> c, int x, int y, int id);
  ~Chunk();

	// Check if grid is currently visible based on world coordinates and window size
	bool visible(const std::pair<int,int> &windowCoords, const std::pair<int,int> &windowSize);

  void updateLogic() override;
  void updateVisual() override;
};