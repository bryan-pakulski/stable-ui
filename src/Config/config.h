#pragma once

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

#include "Config/structs.h"
#include "types.h"

namespace CONFIG {

static YAML::Node configFile = YAML::LoadFile("data/config.yaml");

// Load config with default value if item doesn't exist
template <class T> static T loadConfig(const std::string &configPath, const T &defaultValue) {
  try {
    return configFile[configPath].as<T>();
  } catch (const YAML::Exception &) {
    return defaultValue;
  }
}

// Prompt user for config, save to file
template <class T> static void setConfig(T &variable, const std::string &configPath) {
  // Call error logging function
}

// StableDiffusion configuration
static CInt IMAGE_SIZE_X_LIMIT(loadConfig<int>("IMAGE_SIZE_X_LIMIT", 512));
static CInt IMAGE_SIZE_Y_LIMIT(loadConfig<int>("IMAGE_SIZE_Y_LIMIT", 512));
static CString OUTPUT_DIRECTORY(loadConfig<std::string>("OUTPUT_DIRECTORY", "/data/output"));

// ImGui configuration
static CString PROGRAM_NAME("stable-ui");
static CFloat HIGH_DPI_SCALE_FACTOR(loadConfig<float>("HIGH_DPI_SCALE_FACTOR", 1.0f));
static CInt WINDOW_WIDTH(loadConfig("WINDOW_WIDTH", 1280));
static CInt WINDOW_HEIGHT(loadConfig("WINDOW_HEIGHT", 720));
static CBool STAR_FIELD(loadConfig("STAR_FIELD", 0));

static CFloat IMGUI_TOOLS_WINDOW_WIDTH(loadConfig("IMGUI_TOOLS_WINDOW_WIDTH", 380.0f));
static CFloat IMGUI_TOP_WINDOW_HEIGHT(loadConfig("IMGUI_TOP_WINDOW_HEIGHT", 18.0f));

static CFloat IMGUI_LOG_WINDOW_HEIGHT(loadConfig("IMGUI_LOG_WINDOW_HEIGHT", 900.0f));
static CFloat IMGUI_LOG_WINDOW_WIDTH(loadConfig("IMGUI_LOG_WINDOW_WIDTH", 820.0f));

// Program configuration
static CInt DEFAULT_BUFFER_LENGTH(loadConfig("DEFAULT_BUFFER_LENGTH", 200));
static CString DOCKER_IP_ADDRESS(loadConfig<std::string>("DOCKER_IP_ADDRESS", ""));
static CString MODEL_CONFIGURATIONS_DIRECTORY(loadConfig<std::string>("MODEL_CONFIGURATIONS_DIRECTORY",
                                                                      "data/models/configs/models"));
static CString MODELS_CONFIGURATION_FILE(loadConfig<std::string>("MODELS_CONFIGURATION_FILE",
                                                                 "data/config/model_config.yaml"));
static CString MODULES_CONFIGURATION_FILE(loadConfig<std::string>("MODULES_CONFIGURATION_FILE",
                                                                  "data/config/module_config.yaml"));
static CString VAE_FOLDER_PATH(loadConfig<std::string>("VAE_FOLDER_PATH", "data/models/vae"));
static CString VAE_CONFIG_FOLDER_PATH(loadConfig<std::string>("VAE_FOLDER_PATH", "data/models/configs/vae"));
static CString INDEX_CACHE(loadConfig<std::string>("INDEX_CACHE", "data/cache"));
static CString CRAWLER_PATH(loadConfig<std::string>("CRAWLER_PATH", "data/output"));

// Logging
static CInt ENABLE_GL_DEBUG(loadConfig<int>("ENABLE_OPENGL_DEBUG_OUTPUT", 0));
static CInt ENABLE_DEBUG_LOGGING(loadConfig<int>("ENABLE_DEBUG_LOGGING", 0));

} // namespace CONFIG