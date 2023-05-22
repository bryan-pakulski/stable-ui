#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

class GLHELPER {
public:
  static bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height,
                                  bool tiled);

  static void SaveTextureToFile(const char *filename, GLuint *texture, int width, int height);
};