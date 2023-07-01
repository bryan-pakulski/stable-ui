#include "Rendering/objects/Layer.h"
#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "glm/fwd.hpp"

Layer::Layer(std::shared_ptr<OrthographicCamera> c, glm::ivec2 dimensions, std::string name)
    : BaseObject(glm::ivec2{0, 0}), m_camera(c), m_name(name), c_size(dimensions) {

  pixelData.resize(dimensions.x * dimensions.y, RGBAPixel{0x00, 0x00, 0x00, 0x00});

  m_rawImage = std::shared_ptr<GLImage>(new GLImage(c_size.x, c_size.y, name));
  m_layerImage = std::shared_ptr<Image>(new Image(m_rawImage, c, m_position));
  m_updateTexture = true;
}

void Layer::addImage(Image im) { m_images.push_back(im); }

void Layer::mergeImage(Image im) {

  // Bind image texture and read pixel data
  glBindTexture(GL_TEXTURE_2D, im.m_image->m_texture);
  std::vector<RGBAPixel> imgPixels(im.m_image->m_width * im.m_image->m_height);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgPixels.data());
  glBindTexture(GL_TEXTURE_2D, 0);

  LocalIntersect intersect = GLHELPER::GetLocalIntersectSourceDest(
      m_position, c_size, im.getPosition(), glm::ivec2{im.m_image->m_width, im.m_image->m_height});

  // If no intersection with our main layer and image there is nothing to do
  if (intersect.collision) {
    // Iterate over the pixels in the intersection rectangle and extract the relative pixels from the image into
    // its own vector, note that openGL textures are indexed from the bottom left opposed to our coordinates which
    // index from the top left
    std::vector<RGBAPixel> selectionPixels;
    for (int y = intersect.sourceCoordinates.y; y < intersect.sourceCoordinates.w; y++) {
      for (int x = intersect.sourceCoordinates.x; x < intersect.sourceCoordinates.z; x++) {
        int index = y * im.m_image->m_width + x;
        selectionPixels.push_back(imgPixels[index]);
      }
    }

    // Copy the selectionPixels to our main image
    int i = 0;
    for (int y = intersect.destinationCoordinates.y; y < intersect.destinationCoordinates.w; y++) {
      for (int x = intersect.destinationCoordinates.x; x < intersect.destinationCoordinates.z; x++) {
        int index = y * c_size.x + x;

        if (index > c_size.x * c_size.y || index < 0) {
          continue;
        } else {
          imgPixels[index] = selectionPixels.at(i);
        }

        i++;
      }
    }

    // Inverse Y
    imgPixels = GLHELPER::FlipMatrixY(imgPixels, c_size.x, c_size.y);
  }
}

void Layer::updateLogic() {

  if (m_updateTexture) {
    glBindTexture(GL_TEXTURE_2D, m_rawImage->m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_rawImage->m_width, m_rawImage->m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelData.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  for (auto &image : m_images) {
    image.updateLogic();
  }

  m_layerImage->updateLogic();
}

void Layer::updateVisual() {
  for (auto &image : m_images) {
    image.updateVisual();
  }

  m_layerImage->updateVisual();
  // TODO: render black rectangle around layer size limits
}
