#include "BaseObject.h"
#include "Helpers/GLHelper.h"
#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include <memory>

class Selection : public BaseObject {

public:
  glm::ivec2 m_size{512, 512}; // Selection image size
  bool m_dragging = false;     // Dragging flag
  int m_pixelSnap = 64;        // Size of pixel snapping
  bool m_snap = true;

public:
  Selection(glm::ivec2 position, GLFWwindow *w, std::shared_ptr<OrthographicCamera> c);
  virtual ~Selection();

  glm::ivec2 getPosition() { return glm::ivec2{m_position.x, m_position.y}; }

  void UpdateDrag(glm::vec2 position);

  void updateLogic() override {}
  void updateVisual() override;

private:
  std::shared_ptr<OrthographicCamera> m_camera; // Camera ptr

  // Mouse positions for dragging across screen
  glm::vec2 prev_mouse;
  glm::vec2 cur_mouse;

private:
  void setTexture(GLuint *id);

  // Round to nearest multiple, works with negative numbers
  int roundUp(int x, int multiple) {
    int isPositive = (int)(x >= 0);
    return ((x + isPositive * (multiple - 1)) / multiple) * multiple;
  }
};