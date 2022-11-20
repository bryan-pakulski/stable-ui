#include "BaseObject.h"
#include "../Camera.h"
#include <memory>

class Selection : public BaseObject {

private:
  std::shared_ptr<Camera> m_camera;

public:
  Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c);
};