#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "../Third Party/stb/stb_image.h"

class GLHELPER {
public:
  static bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height);
};