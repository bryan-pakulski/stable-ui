#pragma once

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

#include <Display/ErrorHandler.h>
#include <Helpers/QLogger.h>
#include "config.h"
#include "structs.h"

namespace MODEL_CONFIG {

static ModelConfig loadModelConfig(std::string hash) {
  ModelConfig config;

  try {
    YAML::Node data = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get())["models"][hash];

    config.config = data["config"].as<std::string>();
    config.name = data["name"].as<std::string>();
    config.hash = data["hash"].as<std::string>();
    config.path = data["path"].as<std::string>();
    config.trigger_prompt = data["trigger_prompt"].as<std::string>();
    config.vae = data["vae"].as<std::string>();
    config.vae_config = data["vae_config"].as<std::string>();
    config.convert_vae = data["convert_vae"].as<bool>();
    config.scheduler = data["scheduler"].as<std::string>();
    config.enable_tf32 = data["tf32"].as<bool>();
    config.enable_t16 = data["t16"].as<bool>();
    config.enable_vaeTiling = data["vae_tiling"].as<bool>();
    config.enable_vaeSlicing = data["vae_slicing"].as<bool>();
    config.enable_seqCPUOffload = data["seq_cpu_offloat"].as<bool>();
    config.enable_cpu_offload = data["enable_cpu_offload"].as<bool>();
  } catch (const YAML::Exception &err) {
    ErrorHandler::GetInstance().setError("Configuration error", "Failed to read MODELS_CONFIGURATION FILE");
  }

  return config;
}

static void saveModelConfig(const ModelConfig &config) {
  // Build yaml node to attach to model configuration file
  try {
    YAML::Node model_node;
    model_node["config"] = config.config;
    model_node["name"] = config.name;
    model_node["hash"] = config.hash;
    model_node["path"] = config.path;
    model_node["trigger_prompt"] = config.trigger_prompt;
    model_node["vae"] = config.vae;
    model_node["vae_config"] = config.vae_config;
    model_node["convert_vae"] = config.convert_vae;
    model_node["scheduler"] = config.scheduler;
    model_node["tf32"] = config.enable_tf32;
    model_node["t16"] = config.enable_t16;
    model_node["vae_tiling"] = config.enable_vaeTiling;
    model_node["vae_slicing"] = config.enable_vaeSlicing;
    model_node["seq_cpu_offloat"] = config.enable_seqCPUOffload;
    model_node["enable_cpu_offload"] = config.enable_cpu_offload;

    // Retrieve root node and dump back to file
    YAML::Node node, _baseNode = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
    _baseNode["models"][config.hash] = model_node;
    std::ofstream fout(CONFIG::MODELS_CONFIGURATION_FILE.get());
    fout << _baseNode;
  } catch (const YAML::Exception &err) {
    ErrorHandler::GetInstance().setError("Configuration error",
                                         "Failed to save configuration file" + CONFIG::MODELS_CONFIGURATION_FILE.get());
  }
}

} // namespace MODEL_CONFIG