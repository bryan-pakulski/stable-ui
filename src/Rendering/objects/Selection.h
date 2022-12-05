#include "BaseObject.h"
#include "../Helper.h"
#include "../Camera.h"
#include <memory>

class Selection : public BaseObject {

private:
  std::shared_ptr<Camera> m_camera;
  std::pair<float, float> m_coords{}; // Translation Coordinates
  std::pair<int, int> m_screen{};     // Screen size

  float m_scale;

  // Reference to texture for main window
  GLuint m_texture_id;

  void setTexture(GLuint *id);

public:
  Selection(std::pair<int, int> coords, GLFWwindow *w, std::shared_ptr<Camera> c);
  virtual ~Selection();

  std::pair<int, int> getCoordinates();

  void updateLogic() override;
  void updateVisual() override;
};