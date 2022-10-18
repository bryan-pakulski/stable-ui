#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string>

class Canvas {

public:
  GLuint m_canvas = 0;
  std::string m_name;
  const int c_width;
  const int c_height;

  Canvas(int width, int height, std::string name);
  ~Canvas() = default;
};