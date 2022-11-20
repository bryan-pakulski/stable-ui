#include "Selection.h"

Selection::Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c) : BaseObject(coords) {
  m_window = w;
  m_camera = std::shared_ptr<Camera>(c);
}