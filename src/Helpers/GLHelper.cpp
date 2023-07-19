#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GLHelper.h"
#include "ThirdParty/base64/base64.h"
#include "QLogger.h"

bool GLHELPER::LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height,
                                   bool tiled, bool flipImage) {
  // Load from file
  int image_width = 0;
  int image_height = 0;
  stbi_set_flip_vertically_on_load(flipImage);
  unsigned char *image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
  if (image_data == NULL)
    return false;

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if (tiled) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}

void GLHELPER::SaveTextureToFile(const char *filename, GLuint *texture, int width, int height, int flip) {
  uint8_t *pixels = new uint8_t[width * height * 4];

  // Get texture data
  // Note if that texture is not aligned to 4 bytes as expected by open GL we may need to pack it
  // See:
  //      glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
  //      glPixelStorei(GL_PACK_ALIGNMENT, 1)

  glBindTexture(GL_TEXTURE_2D, *texture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Save texture as png file
  stbi_flip_vertically_on_write(flip);
  stbi_write_png(filename, width, height, 4, pixels, width * 4);

  // Free array of pixels
  delete[] pixels;
}

// Converts OpenGL Texture into a Base64 string
// This assumes an RGBA texture
std::string GLHELPER::textureToBase64String(GLuint *texture, int width, int height) {
  int data_length = width * height * 4;
  uint8_t *pixels = new uint8_t[data_length];

  glBindTexture(GL_TEXTURE_2D, *texture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  int encoded_data_length = Base64encode_len(data_length);
  std::vector<char> base64_string(encoded_data_length);

  Base64encode(base64_string.data(), reinterpret_cast<const unsigned char *>(pixels), data_length);

  /* Decode can be done as follows
  char* data = NULL;
  int data_length = 0;
  int alloc_length = Base64decode_len(base64_string);
  some_random_data = malloc(alloc_length);
  data_length = Base64decode(data, base64_string);
  */

  delete[] pixels;
  return base64_string.data();
}

bool GLHELPER::intersects(const glm::ivec2 &l1, const glm::ivec2 &r1, const glm::ivec2 &l2, const glm::ivec2 &r2) {

  QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "BaseObject::intersects, checking intersection of [", l1.x, l1.y, r1.x,
                             r1.y, "] and [", l2.x, l2.y, r2.x, r2.y, "]");

  bool wp = std::min(r1.x, r2.x) > std::max(l1.x, l2.x);
  bool hp = std::min(r1.y, r2.y) > std::max(l1.y, l2.y);

  if (wp && hp) {
    QLogger::GetInstance().Log(LOGLEVEL::DEBUG, "Image::intersects, found intersecting image!");
    return true;
  } else {
    return false;
  }
}