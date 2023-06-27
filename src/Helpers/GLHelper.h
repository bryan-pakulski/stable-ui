#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <vector>

#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"
#include <bits/stdc++.h>

#define HASH_SIZE 128

class GLHELPER {
public:
  static bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height,
                                  bool tiled, bool flipImage);

  static void SaveTextureToFile(const char *filename, GLuint *texture, int width, int height);
  static std::string textureToBase64String(GLuint *texture, int width, int height);

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