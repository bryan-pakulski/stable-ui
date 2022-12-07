#include "Image.h"
#include "../../../QLogger.h"
#include <cstddef>

Image::Image(int width, int height, std::string name) : m_width(width), m_height(height), m_name(name) {
  float color[] = {0.0f, 0.0f, 0.0f, 1.0f};

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, color);
  glBindTexture(GL_TEXTURE_2D, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Successfully created blank texture", m_name);
}

void Image::loadFromImage(std::string path) {
  if (!GLHELPER::LoadTextureFromFile(path.c_str(), &m_texture, &m_width, &m_height, false)) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "Failed to load image from file: ", path.c_str());
  }
  m_image_source = path;
}

void Image::drawMaskToTexture(int xPos, int yPos, float size) {

  // Set up framebuffer for additional rendering
  glGenFramebuffers(1, &m_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
  // Attach texture1 to the framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
  // Set up the viewport
  glViewport(0, 0, m_width, m_height);
  // Clear the framebuffer
  glClear(GL_COLOR_BUFFER_BIT);

  // TODO: Draw whatever you want directly onto texture1 here
  // ...

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}