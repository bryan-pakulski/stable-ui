#include "Canvas.h"
#include "../../QLogger.h"
#include <cstddef>

Canvas::Canvas(int width, int height) : c_width(width), c_height(height) {

  glGenTextures(1, &m_canvas);
  glBindTexture(GL_TEXTURE_2D, m_canvas);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);

  QLogger::GetInstance().Log(
      LOGLEVEL::INFO,
      "Successfully created blank canvas with dimensions: ", c_width, c_height);
}