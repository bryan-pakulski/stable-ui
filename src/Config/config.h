#pragma once

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
#include "types.h"

namespace CONFIG {

static YAML::Node configFile = YAML::LoadFile("data/config.yaml");

// Load config with default value if item doesn't exist
template <class T> static T loadConfig(const std::string &configPath, const T &defaultValue) {
  try {
    return configFile[configPath].as<T>();
  } catch (YAML::Exception) {
    return defaultValue;
  }
}

// Prompt user for config, save to file
template <class T> static void setConfig(T &variable, const std::string &configPath) {
  // Call error logging function
}

// StableDiffusion configuration
static CInt PROMPT_LENGTH_LIMIT(loadConfig<int>("PROMPT_LENGTH_LIMIT", 200));
static CString STABLE_DIFFUSION_DOCKER_PATH(loadConfig<std::string>("STABLE_DIFFUSION_DOCKER_PATH", "/sd/"));
static CString MODELS_DIRECTORY(loadConfig<std::string>("MODELS_DIRECTORY", "/sd/models/"));
static CString TXT_TO_IMG_PATH(loadConfig<std::string>("TXT_TO_IMG_PATH", "scripts/txt2img.py"));
static CString IMG_TO_IMG_PATH(loadConfig<std::string>("IMG_TO_IMG_PATH", "scripts/img2img.py"));
static CInt IMAGE_SIZE_X_LIMIT(loadConfig<int>("IMAGE_SIZE_X_LIMIT", 512));
static CInt IMAGE_SIZE_Y_LIMIT(loadConfig<int>("IMAGE_SIZE_Y_LIMIT", 512));
static CString OUTPUT_DIRECTORY(loadConfig<std::string>("OUTPUT_DIRECTORY", "/output"));

// ImGui configuration
static CString PROGRAM_NAME("stable-ui");
static CFloat HIGH_DPI_SCALE_FACTOR(loadConfig<float>("HIGH_DPI_SCALE_FACTOR", 1.0f));
static CInt WINDOW_WIDTH(loadConfig("WINDOW_WIDTH", 1280));
static CInt WINDOW_HEIGHT(loadConfig("WINDOW_HEIGHT", 720));

static CFloat IMGUI_BOTTOM_WINDOW_HEIGHT(loadConfig("IMGUI_BOTTOM_WINDOW_HEIGHT", 320.0f));
static CFloat IMGUI_LEFT_WINDOW_WIDTH(loadConfig("IMGUI_LEFT_WINDOW_WIDTH", 320.0f));
static CFloat IMGUI_TOP_WINDOW_HEIGHT(loadConfig("IMGUI_TOP_WINDOW_HEIGHT", 18.0f));

static CFloat IMGUI_LOG_WINDOW_HEIGHT(loadConfig("IMGUI_LOG_WINDOW_HEIGHT", 820.0f));
static CFloat IMGUI_LOG_WINDOW_WIDTH(loadConfig("IMGUI_LOG_WINDOW_WIDTH", 620.0f));

// Program configuration
static CString PYTHON_CONFIG_PATH(loadConfig<std::string>("PYTHON_CONFIG_PATH", "data/scripts"));
static CInt ENABLE_GL_DEBUG(loadConfig<int>("ENABLE_OPENGL_DEBUG_OUTPUT", 0));
static CInt DEFAULT_BUFFER_LENGTH(loadConfig("DEFAULT_BUFFER_LENGTH", 200));
static CString MODEL_CONFIGURATIONS_DIRECTORY(loadConfig<std::string>("MODEL_CONFIGURATIONS_DIRECTORY",
                                                                      "data/models/configs"));
static CString MODELS_CONFIGURATION_FILE(loadConfig<std::string>("MODELS_CONFIGURATION_FILE",
                                                                 "data/models/configs/model_config.yaml"));
static CString MODULES_CONFIGURATION_FILE(loadConfig<std::string>("MODULES_CONFIGURATION_FILE",
                                                                  "data/models/configs/module_config.yaml"));

} // namespace CONFIG