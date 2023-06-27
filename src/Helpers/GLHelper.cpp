#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GLHelper.h"
#include "ThirdParty/base64/base64.h"

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
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

void GLHELPER::SaveTextureToFile(const char *filename, GLuint *texture, int width, int height) {
  // Allocate array of pixels
  uint8_t *pixels = new uint8_t[width * height * 4];

  // Get texture data
  glGetTexImage(*texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // Save texture as png file
  stbi_flip_vertically_on_write(1);
  stbi_write_png(filename, width, height, 4, pixels, width * 4);

  // Free array of pixels
  free(pixels);
}

// Converts OpenGL Texture into a Base64 string
// This assumes an RGBA texture
std::string GLHELPER::textureToBase64String(GLuint *texture, int width, int height) {
  uint8_t *pixels = new uint8_t[width * height * 4];
  glGetTexImage(*texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  int data_length = width * height * 4;
  int encoded_data_length = Base64encode_len(data_length);
  std::string base64_string(encoded_data_length, ' ');

  Base64encode(base64_string.data(), pixels, data_length);

  /* Decode can be done as follows
  char* data = NULL;
  int data_length = 0;
  int alloc_length = Base64decode_len(base64_string);
  some_random_data = malloc(alloc_length);
  data_length = Base64decode(data, base64_string);
  */

  free(pixels);
  return base64_string;
}
