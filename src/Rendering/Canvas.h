#pragma once

#include <cmath>
#include <vector>

#include "../QLogger.h"
#include "Helper.h"

#include "objects/BaseObject.h"
#include "objects/grid/GridChunk.h"

class Canvas : public BaseObject {
private:
  std::pair<int, int> m_coords{};   // Pixel Coordinates (top left)
  std::pair<int, int> m_screen{};   // Screen size

  std::vector<std::unique_ptr<GridChunk>> m_editorGrid;

  // Reference to texture for main window
  GLuint m_texture_id;

  void setTexture(GLuint *id);

public:
  bool m_active = false;
  std::string m_name;

  Canvas(std::pair<int, int> coords, const std::string &name, GLFWwindow *w);
  virtual ~Canvas();

  void updateLogic() override;
  void updateVisual() override;

  // Checks which grids are visible and creates a texture to apply to the main window
  void updateMainWindowTexture();
};