#include "Rendering/objects/GLImage/GLImage.h"
#include "Helpers/QLogger.h"
#include <cstddef>

GLImage::GLImage(int width, int height, std::string name) : m_name(name), m_width(width), m_height(height) {
  float color[] = {0.0f, 0.0f, 0.0f, 1.0f};

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, color);
  glBindTexture(GL_TEXTURE_2D, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Image::Image Successfully created blank texture", m_name);
}

// This is a destructive operation
void GLImage::resize(int width, int height) {
  float color[] = {0.0f, 0.0f, 0.0f, 1.0f};

  m_width = width;
  m_height = height;

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, color);
  glBindTexture(GL_TEXTURE_2D, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Image::Image Successfully resized", m_name, " to size: ", width, height);
}

void GLImage::loadFromImage(std::string path, bool flipImage) {
  if (!GLHELPER::LoadTextureFromFile(path.c_str(), &m_texture, &m_width, &m_height, false, flipImage)) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Image::loadFromImage Failed to load image from file: ", path.c_str());
  }
  m_image_source = path;
}