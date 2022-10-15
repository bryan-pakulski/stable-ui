#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>

class Canvas {
  GLuint m_canvas = 0;
  const int c_width;
  const int c_height;

public:
  Canvas(int width, int height);
  ~Canvas() = default;
};