#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include "Helpers/QLogger.h"
#include "Helpers/GLHelper.h"
#include "Camera.h"

#include "Rendering/objects/BaseObject.h"
#include "Rendering/objects/chunk/Chunk.h"
#include "Rendering/objects/image/Image.h"

class Canvas : public BaseObject {
private:
  std::pair<int, int> m_coords{}; // Pixel Coordinates (top left)
  std::pair<int, int> m_screen{}; // Screen size

  std::shared_ptr<Camera> m_camera;

  // Reference to texture for main window
  GLuint m_texture_id;
  float m_time = 0.0f;

public:
  bool m_active = false;
  std::vector<std::unique_ptr<Chunk>> m_editorGrid;
  std::string m_name;

  Canvas(std::pair<int, int> coords, const std::string &name, GLFWwindow *w, std::shared_ptr<Camera> c);
  virtual ~Canvas();

  void updateLogic() override;
  void updateVisual() override;
  void renderBackground();
  void renderImages();
  void setTexture(GLuint *id);

  // Update or create new chunk for a given Image
  void createImage(std::shared_ptr<Image>, std::pair<int, int> chunk_coordinates);

  // Delete chunk by index
  void deleteChunk(int index);

  // Hide chunk by index
  void hideChunk(int index);
  void showChunk(int index);
};