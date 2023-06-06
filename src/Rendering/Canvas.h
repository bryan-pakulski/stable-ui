#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include "Helpers/QLogger.h"
#include "Helpers/GLHelper.h"
#include "Rendering/OrthographicCamera.h"

#include "Rendering/objects/BaseObject.h"
#include "Rendering/objects/Image.h"
#include "Rendering/objects/GLImage/GLImage.h"

class Canvas : public BaseObject {
private:
  std::pair<int, int> m_screen{}; // Screen size

  std::shared_ptr<OrthographicCamera> m_camera;

  // Reference to texture for main window
  GLuint m_texture_id;
  float m_time = 0.0f;

public:
  bool m_active = false;
  std::vector<std::unique_ptr<Image>> m_editorGrid;
  std::string m_name;

  Canvas(glm::ivec2 position, const std::string &name, GLFWwindow *w, std::shared_ptr<OrthographicCamera> c);
  virtual ~Canvas();

  void updateLogic() override;
  void updateVisual() override;
  void renderStarField();
  void renderGrid();
  void renderImages();
  void setTexture(GLuint *id);

  // Update or create new chunk for a given Image
  void createImage(std::shared_ptr<GLImage>, glm::ivec2 position);

  // Delete chunk by index
  void deleteChunk(int index);

  // Hide chunk by index
  void hideChunk(int index);
  void showChunk(int index);
};