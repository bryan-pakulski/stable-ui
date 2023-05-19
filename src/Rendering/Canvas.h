#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include "../QLogger.h"
#include "../Helpers/GLHelper.h"
#include "Camera.h"

#include "objects/BaseObject.h"
#include "objects/chunk/Chunk.h"
#include "objects/image/Image.h"

class Canvas : public BaseObject {
private:
  std::pair<int, int> m_coords{}; // Pixel Coordinates (top left)
  std::pair<int, int> m_screen{}; // Screen size

  std::shared_ptr<Camera> m_camera;

  // Reference to texture for main window
  GLuint m_texture_id;
  float m_time = 0.0f;

  void setTexture(GLuint *id);

public:
  bool m_active = false;
  std::vector<std::unique_ptr<Chunk>> m_editorGrid;
  std::string m_name;

  Canvas(std::pair<int, int> coords, const std::string &name, GLFWwindow *w, std::shared_ptr<Camera> c);
  virtual ~Canvas();

  void updateLogic() override;
  void updateVisual() override;
  void renderChunks();

  // Checks which grids are visible and creates a texture to apply to the main window
  void updateMainWindowTexture();

  // Update or create new chunk for a given Image
  void createChunk(std::shared_ptr<Image>, std::pair<int, int> chunk_coordinates);

  // Delete chunk by index
  void deleteChunk(int index);

  // Hide chunk by index
  void hideChunk(int index);
  void showChunk(int index);
};