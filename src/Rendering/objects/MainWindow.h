#pragma once

#include <cmath>

#include "../../QLogger.h"
#include "BaseObject.h"

class MainWindow : public BaseObject {
private:
  std::pair<int, int> m_pc{};   // Pixel Coordinates (top left)
  std::pair<int, int> m_size{}; // Size of object in pixels
  std::pair<int, int> screen{}; // Screen size

  // Reference to texture for main window
  GLuint *m_texture_id = 0;

public:
  MainWindow(std::pair<int, int> pc, std::pair<int, int>, GLFWwindow *w);

  void updateLogic() override;

  void updateVisual() override;

  void setMainWindowTexture(GLuint *id);

  virtual ~MainWindow();
};