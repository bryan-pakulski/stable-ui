#include "Image.h"
#include "../../../QLogger.h"
#include <cstddef>

Image::Image(int width, int height, std::string name) : m_width(width), m_height(height), m_name(name) {

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Successfully created blank image with dimensions: ", m_width, m_height);
}

void Image::loadFromImage(std::string path) {
  if (!GLHELPER::LoadTextureFromFile(path.c_str(), &m_texture, &m_width, &m_height, false)) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Failed to load image from file: ", path.c_str());
  }
  m_image_source = path;
}