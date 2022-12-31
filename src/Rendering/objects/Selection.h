#include "BaseObject.h"
#include "../Helper.h"
#include "../Camera.h"
#include "image/Image.h"
#include <memory>

class Selection : public BaseObject {

private:
  std::shared_ptr<Camera> m_camera;                                 // Camera ptr
  std::pair<float, float> m_coords{};                               // Translation Coordinates
  std::pair<int, int> m_screen{};                                   // Screen size
  std::pair<std::pair<int, int>, std::pair<int, int>> m_dragCoords; // Drag start / stop coordinates
  GLuint m_texture_id;

  const int c_drag_trigger = 10; // Only count mouse events over 10x10 pixels as a drag event

  void setTexture(GLuint *id);

public:
  GLuint m_selection_texture_buffer = 0;
  bool m_listening = false;     // Trigger to enable listening for mouse release
  std::pair<int, int> m_size{}; // Selection size

  Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c);
  virtual ~Selection();

  std::pair<int, int> getCoordinates();

  bool isDragged();
  void dragStart(int x, int y);
  void dragStop(int x, int y);
  void makeSelection();

  void updateLogic() override;
  void updateVisual() override;
};