#pragma once

#include <cmath>

#include "../../QLogger.h"
#include "BaseObject.h"
#include "../Helper.h"

class WindowSelection : public BaseObject {
private:
  float m_scale = 0.3f;
  double m_mousePosX = 0.0f;
  double m_mousePosY = 0.0f;
  int m_width = 512;
  int m_height = 512;
  GLuint m_texture = 0;

  std::pair<int, int> m_pc{};   // Pixel Coordinates (top left)
  std::pair<int, int> screen{}; // Screen size

public:
  WindowSelection(std::pair<int, int> pc, GLFWwindow *w);

  void updateLogic() override;

  void updateVisual() override;

  virtual ~WindowSelection();
};