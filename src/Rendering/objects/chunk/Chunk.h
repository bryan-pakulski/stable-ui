#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <utility>

#include "../image/Image.h"

// This class wraps an image class and contains some flags to check for visibility based on
// Camera and world coordinates
class Chunk {
  std::shared_ptr<Image> m_image;
	const std::pair<int, int> c_coordinates;

public:
  Chunk(int x, int y, int id);
  ~Chunk();

	// Check if grid is currently visible based on world coordinates and window size
	bool visible(const std::pair<int,int> &windowCoords, const std::pair<int,int> &windowSize);

	// Render onto screen, offset based on world coordinates, window size & camera
  void render(const std::pair<int,int> windowCoords);
};