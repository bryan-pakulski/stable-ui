#include "BaseObject.h"
#include "../../Helpers/GLHelper.h"
#include "../Camera.h"
#include "image/Image.h"
#include <memory>

class Selection : public BaseObject {

private:
  std::shared_ptr<Camera> m_camera; // Camera ptr

  // Mouse positions for dragging across screen
  glm::vec2 prev_mouse;
  glm::vec2 cur_mouse;
  glm::vec3 m_position;

  void setTexture(GLuint *id);

public:
  GLuint m_selection_texture_buffer = 0;
  std::pair<int, int> m_size{512, 512}; // Selection image size
  bool m_captureInProgress = false;     // Capture flag, when set we will update the m_position
  int m_pixelSnap = 512;                // Size of pixel snapping

  Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c);
  virtual ~Selection();

  std::pair<int, int> getCoordinates();

  void startCapture(float x, float y);
  void updateCapture(float x, float y);
  void captureBuffer();
  void saveBuffer();

  void updateLogic() override;
  void updateVisual() override;
};