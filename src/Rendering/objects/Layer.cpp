#include "Rendering/objects/Layer.h"
#include "Rendering/OrthographicCamera.h"
#include "Rendering/objects/GLImage/GLImage.h"
#include "glm/fwd.hpp"

Layer::Layer(std::shared_ptr<OrthographicCamera> c, glm::ivec2 dimensions, std::string name, bool protect)
    : BaseObject(glm::ivec2{0, 0}), m_camera(c), m_name(name), c_size(dimensions), c_protected(protect) {

  pixelData.resize(dimensions.x * dimensions.y, RGBAPixel{0x00, 0x00, 0x00, 0x00});

  m_rawImage = std::shared_ptr<GLImage>(new GLImage(c_size.x, c_size.y, name));
  m_layerImage = std::shared_ptr<Image>(new Image(m_rawImage, c, m_position));
  m_updateTexture = true;
}

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
          pixelData[index] = selectionPixels.at(i);
        }

        i++;
      }
    }

    updateTexture();
  }
}

void Layer::eraseSelection(glm::ivec2 position, glm::ivec2 size) {
  LocalIntersect intersect = GLHELPER::GetLocalIntersectSourceDest(getPosition(), c_size, position, size);

  if (intersect.collision && m_renderFlag) {
    QLogger::GetInstance().Log(LOGLEVEL::DBG2, "Deleting selection from layer pixel data at",
                               intersect.destinationCoordinates.x, intersect.destinationCoordinates.w, "and",
                               intersect.destinationCoordinates.y, intersect.destinationCoordinates.z);

    for (int y = intersect.destinationCoordinates.y; y < intersect.destinationCoordinates.w; y++) {
      for (int x = intersect.destinationCoordinates.x; x < intersect.destinationCoordinates.z; x++) {
        int index = y * c_size.x + x;

        if (index > c_size.x * c_size.y || index < 0) {
          continue;
        } else {
          pixelData[index] = RGBAPixel{0x00, 0x00, 0x00, 0x00};
        }
      }
    }

    updateTexture();
  }
}

void Layer::updateLogic() {

  if (m_updateTexture) {
    glBindTexture(GL_TEXTURE_2D, m_rawImage->m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_rawImage->m_width, m_rawImage->m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelData.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_updateTexture = false;
  }

  for (auto &image : m_images) {
    image.updateLogic();
  }

  m_layerImage->updateLogic();
}

void Layer::updateVisual() {
  if (m_renderFlag) {
    m_layerImage->updateVisual();

    for (auto &image : m_images) {
      image.updateVisual();
    }

    // TODO: render black rectangle around layer size limits
  }
}

std::vector<RGBAPixel> Layer::getPixelsAtSelection(glm::ivec4 coordinates, glm::ivec2 position, glm::ivec2 size) {

  // Return values from images / layer at these coordinates
  std::vector<RGBAPixel> selectionPixels;

  for (int y = coordinates.y; y < coordinates.w; y++) {
    for (int x = coordinates.x; x < coordinates.z; x++) {
      int index = y * m_layerImage->m_image->m_width + x;
      if (index > c_size.x * c_size.y || index < 0) {
        continue;
      } else {
        selectionPixels.push_back(pixelData[index]);
      }
    }
  }

  // Get image data
  // TODO: Y offset is incorrect
  for (auto &image : m_images) {
    LocalIntersect intersect = GLHELPER::GetLocalIntersectSourceDest(
        position, size, image.getPosition(), glm::ivec2{image.m_image->m_width, image.m_image->m_width});

    if (intersect.collision && image.m_renderFlag) {

      // Bind image texture and read pixel data for our selected area
      glBindTexture(GL_TEXTURE_2D, image.m_image->m_texture);
      std::vector<RGBAPixel> imgPixels(image.m_image->m_width * image.m_image->m_height);
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgPixels.data());
      glBindTexture(GL_TEXTURE_2D, 0);

      for (int y = intersect.sourceCoordinates.y; y < intersect.sourceCoordinates.w; y++) {
        for (int x = intersect.sourceCoordinates.x; x < intersect.sourceCoordinates.z; x++) {
          int index = y * image.m_image->m_width + x;
          if (index > selectionPixels.size() || index < 0) {
            continue;
          } else {
            selectionPixels[index] = imgPixels[index];
          }
        }
      }
    }
  }

  return selectionPixels;
}