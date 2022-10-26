#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <utility>

// X,Y size of a grid chunk
static int GRID_SIZE = 512;

class GridChunk {
  GLuint m_canvas = 0;
  const int c_canvasId;
	const std::pair<int, int> c_coordinates;

public:
  GridChunk(int x, int y, int id);
  ~GridChunk();

	// Check if grid is currently visible based on world coordinates and window size
	bool visible(const std::pair<int,int> &windowCoords, const std::pair<int,int> &windowSize);

	// Render onto screen, offset based on world coordinates & window size
  void render(const std::pair<int,int> windowCoords);
};