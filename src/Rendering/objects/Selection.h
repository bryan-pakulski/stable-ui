#include "BaseObject.h"
#include "../Helper.h"
#include "../Camera.h"
#include "image/Image.h"
#include <memory>

class Selection : public BaseObject {

private:
  std::shared_ptr<Camera> m_camera; // Camera ptr
  std::pair<int, int> m_screen{};   // Screen siz
  GLuint m_texture_id;

  void setTexture(GLuint *id);

public:
  GLuint m_selection_texture_buffer = 0;
  std::pair<int, int> m_size{}; // Selection image size

  // Mouse positions for dragging across screen
  glm::vec2 prev_mouse;
  glm::vec2 cur_mouse;
  glm::vec3 m_position;

  Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c);
  virtual ~Selection();

  std::pair<int, int> getCoordinates();
  void moveSelectionPosition(float x, float y);
  void captureBuffer();
  void saveBuffer();

  void updateLogic() override;
  void updateVisual() override;
};