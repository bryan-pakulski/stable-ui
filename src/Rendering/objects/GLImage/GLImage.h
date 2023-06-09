#pragma once
#include "Helpers/GLHelper.h"
#include "Helpers/States.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string>

struct RGBAPixel {
  unsigned char red = 0xFF;
  unsigned char green = 0xFF;
  unsigned char blue = 0xFF;
  unsigned char alpha = 0xFF;
};

class GLImage {

public:
  GLuint m_texture = 0;
  int renderState = Q_RENDER_STATE::UNRENDERED;
  bool textured = false;
  std::string m_name;
  std::string m_image_source;
  int m_width;
  int m_height;

public:
  GLImage(int width, int height, std::string name);
  ~GLImage() = default;

  void loadFromImage(std::string path, bool flipImage = false);
  void resize(int width, int height);
};