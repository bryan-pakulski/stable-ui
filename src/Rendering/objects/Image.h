#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <utility>

#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/BaseObject.h"
#include "Rendering/objects/GLImage/GLImage.h"

// This class wraps an image class and contains some flags to check for visibility based on
// Camera and world coordinates
class Image : public BaseObject {

public:
  std::shared_ptr<GLImage> m_image;
  bool m_renderFlag = true;

public:
  Image(std::shared_ptr<GLImage> im, std::shared_ptr<OrthographicCamera> c, glm::ivec2 position);
  ~Image() {}

  // Check if image intersects a given selection space
  bool intersects(const glm::ivec2 &l1, const glm::ivec2 &r1, const glm::ivec2 &l2, const glm::ivec2 &r2);

  void updateLogic() override {}
  void updateVisual() override;

private:
  std::shared_ptr<OrthographicCamera> m_camera;
};