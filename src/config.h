#pragma once

#define ENABLE_GL_DEBUG 1

namespace CONFIG {
// StableDiffusion configuration
static const int PROMPT_LENGTH_LIMIT = 2048;

// ImGui configuration
static const char *PROGRAM_NAME = "stable-ui";
static const float HIGH_DPI_SCALE_FACTOR = 1.0;
static const int WINDOW_WIDTH = 1200;
static const int WINDOW_HEIGHT = 800;
} // namespace CONFIG