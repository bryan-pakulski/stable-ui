#include "BaseObject.h"
#include "Helpers/GLHelper.h"
#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include <memory>

class Selection : public BaseObject {

private:
  std::shared_ptr<OrthographicCamera> m_camera; // Camera ptr

  // Mouse positions for dragging across screen
  glm::vec2 prev_mouse;
  glm::vec2 cur_mouse;

  void setTexture(GLuint *id);

public:
  GLuint m_selection_texture_buffer = 0;
  glm::ivec2 m_size{512, 512}; // Selection image size
  bool m_dragging = false;     // Dragging flag
  int m_pixelSnap = 256;       // Size of pixel snapping
  bool m_snap = true;

  Selection(glm::ivec2 position, GLFWwindow *w, std::shared_ptr<OrthographicCamera> c);
  virtual ~Selection();

  glm::ivec2 &getPosition() { return m_position; }

  void UpdateDrag(glm::vec2 position);
  void captureBuffer();
  void saveBuffer();

  void updateLogic() override;
  void updateVisual() override;
};