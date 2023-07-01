#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"
#include <bits/stdc++.h>

#define HASH_SIZE 128

struct LocalIntersect {
  glm::ivec4 sourceCoordinates{0};
  glm::ivec4 destinationCoordinates{0};
  bool collision = false;
};

class GLHELPER {
public:
  static bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height,
                                  bool tiled, bool flipImage);

  static void SaveTextureToFile(const char *filename, GLuint *texture, int width, int height, int flip = 0);
  static std::string textureToBase64String(GLuint *texture, int width, int height);

  static bool intersects(const glm::ivec2 &l1, const glm::ivec2 &r1, const glm::ivec2 &l2, const glm::ivec2 &r2);

  // Get the local intersect of two objects, assumes that the origin starts at the center of each obj
  static LocalIntersect GetLocalIntersectSourceDest(glm::ivec2 o1, glm::ivec2 s1, glm::ivec2 o2, glm::ivec2 s2) {
    LocalIntersect intersect;

    // Convert the origin as center to bottom left for simpler calculation
    glm::ivec2 l_p1 = {(o1.x - (s1.x / 2)), (o1.y - (s1.y / 2))};
    glm::ivec2 l_p2 = {(o1.x + (s1.x / 2)), (o1.y + (s1.y / 2))};

    glm::ivec2 i_p1 = {(o2.x - s2.x / 2), (o2.y - s2.y / 2)};
    glm::ivec2 i_p2 = {(o2.x + s2.x / 2), (o2.y + s2.y / 2)};

    if (intersects(l_p1, l_p2, i_p1, i_p2)) {
      // Calculate intersection rectangle
      // As these fall on the cartesian plane, these can be negative!
      int leftX = std::max(l_p1.x, i_p1.x);
      int rightX = std::min(l_p2.x, i_p2.x);
      int topY = std::min(l_p2.y, i_p2.y);
      int bottomY = std::max(l_p1.y, i_p1.y);

      // Treat image as our origin (bottom left cartesion)
      intersect.sourceCoordinates = {leftX - i_p1.x, bottomY - i_p1.y, rightX - i_p1.x, topY - i_p1.y};

      // Get offset selection coordinates so we know where to copy the raw data
      intersect.destinationCoordinates = {leftX - l_p1.x, bottomY - l_p1.y, rightX - l_p1.x, topY - l_p1.y};
      intersect.collision = true;
    }

    return intersect;
  }

  template <class T> static std::vector<T> FlipMatrixY(std::vector<T> matrix, int width, int height) {
    std::vector<T> pixels(matrix.size());

    for (int i = 0; i < pixels.size(); i++) {

      pixels[(i / width) * width + (i % width)] = matrix[(height - (i / width) - 1) * width + (i % width)];
    }
    return pixels;
  }

  template <class T> static std::vector<T> Rotate1DSquareMatrixClockwise(std::vector<T> matrix, int rotations = 1) {
    int size = (int)sqrt(matrix.size());
    std::vector<T> result(matrix.size());

    do {
      for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
          result[i * size + j] = matrix[(size - j - 1) * size + i];
        }
      }
      rotations--;
      matrix = result;
    } while (rotations > 0);

    return result;
  }

  // Non-square matrix transpose of
  // matrix of size r x c and base address A
  template <class T> static void MatrixInplaceTranspose(T &A, int r, int c) {
    int size = r * c - 1;
    int t;                    // holds element to be replaced,
                              // eventually becomes next element to move
    int next;                 // location of 't' to be moved
    int cycleBegin;           // holds start of cycle
    int i;                    // iterator
    std::bitset<HASH_SIZE> b; // hash to mark moved elements

    b.reset();
    b[0] = b[size] = 1;
    i = 1; // Note that A[0] and A[size-1] won't move
    while (i < size) {
      cycleBegin = i;
      t = A[i];
      do {
        // Input matrix [r x c]
        // Output matrix
        // i_new = (i*r)%(N-1)
        next = (i * r) % size;
        swap(A[next], t);
        b[i] = 1;
        i = next;
      } while (i != cycleBegin);

      // Get Next Move (what about querying random location?)
      for (i = 1; i < size && b[i]; i++)
        ;
    }
  }
};