#pragma once
#include "../../Helper.h"
#include "../../../Helpers/States.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string>

class Image {

public:
  GLuint m_texture = 0;
  GLuint m_framebuffer = 0;
  int renderState = Q_EXECUTION_STATE::PENDING;
  bool textured = Q_EXECUTION_STATE::PENDING;
  std::string m_name;
  std::string m_image_source;
  int m_width;
  int m_height;

  Image(int width, int height, std::string name);
  ~Image() = default;

  // Draw a brush mask to our Texture, all data is in pixels
  void drawMaskToTexture(int xPos, int yPos, float size);

  void loadFromImage(std::string path);
};