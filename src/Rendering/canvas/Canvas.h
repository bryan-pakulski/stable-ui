#pragma once
#include "../Helper.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string>

class Canvas {

public:
  GLuint m_canvas = 0;
  bool rendered = true;
  bool textured = false;
  std::string m_name;
  std::string m_image_source;
  int m_width;
  int m_height;

  Canvas(int width, int height, std::string name);
  ~Canvas() = default;

  void loadFromImage(std::string path);
};