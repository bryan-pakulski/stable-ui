#pragma once

#include "Third Party/yaml-cpp/yaml.h"

namespace CONFIG {

static YAML::Node config_file = YAML::LoadFile("data/config.yaml");

// StableDiffusion configuration
static const int PROMPT_LENGTH_LIMIT = config_file["PROMPT_LENGTH_LIMIT"].as<int>();
static const std::string STABLE_DIFFUSION_DOCKER_PATH = config_file["STABLE_DIFFUSION_DOCKER_PATH"].as<std::string>();
static const std::string MODELS_DIRECTORY = config_file["MODELS_DIRECTORY"].as<std::string>();
static const std::string TXT_TO_IMG_PATH = config_file["TXT_TO_IMG_PATH"].as<std::string>();
static const std::string IMG_TO_IMG_PATH = config_file["TXT_TO_IMG_PATH"].as<std::string>();
static const int CANVAS_SIZE_X_LIMIT = config_file["CANVAS_SIZE_X_LIMIT"].as<int>();
static const int CANVAS_SIZE_Y_LIMIT = config_file["CANVAS_SIZE_Y_LIMIT"].as<int>();

// ImGui configuration
static const char *PROGRAM_NAME = "stable-ui";
static const float HIGH_DPI_SCALE_FACTOR = config_file["HIGH_DPI_SCALE_FACTOR"].as<float>();
static const int WINDOW_WIDTH = config_file["WINDOW_WIDTH"].as<int>();
static const int WINDOW_HEIGHT = config_file["WINDOW_HEIGHT"].as<int>();

// Program configuration
static const std::string PYTHON_CONFIG_PATH = config_file["PYTHON_CONFIG_PATH"].as<std::string>();
static const u_int ENABLE_GL_DEBUG = config_file["ENABLE_OPENGL_DEBUG_OUTPUT"].as<u_int>();
static const std::string OUTPUT_DIRECTORY = config_file["OUTPUT_DIRECTORY"].as<std::string>();
} // namespace CONFIG