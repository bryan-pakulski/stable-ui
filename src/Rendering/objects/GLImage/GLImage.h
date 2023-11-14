#pragma once
#include "Helpers/GLHelper.h"
#include "Helpers/States.h"

#include <cstdint>
#include <glad/glad.h>
#include <iterator>

#include <GLFW/glfw3.h>
#include <string>

struct RGBAPixel {
  unsigned char red = 0xFF;
  unsigned char green = 0xFF;
  unsigned char blue = 0xFF;
  unsigned char alpha = 0xFF;

  uint32_t asInt() {

    uint32_t packedColor = 0;
    packedColor |= ((uint32_t)red) << 24;
    packedColor |= ((uint32_t)green) << 16;
    packedColor |= ((uint32_t)blue) << 8;
    packedColor |= ((uint32_t)alpha);

    return packedColor;
  }
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