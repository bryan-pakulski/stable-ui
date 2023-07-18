#pragma once

#include <string>

struct model {
  std::string name;
  std::string hash;
  std::string path;
};

struct ModelConfig {
  std::string name = "";
  std::string path = "";
  std::string config = "";
  std::string hash = "";
  // TODO: make trigger words a vector and make a multi selectable dropdown during inference to chose which triggers
  // to use
  std::string trigger_prompt = "";
  std::string vae = "";
  std::string vae_config = "";
  bool convert_vae = false;
  std::string scheduler = "pndm";

  bool enable_xformers = false;
  bool enable_tf32 = true;
  bool enable_t16 = true;
  bool enable_vaeTiling = true;
  bool enable_vaeSlicing = false;
  bool enable_seqCPUOffload = false;
};

// Supported pipelines
struct PIPELINE {
  const static int TXT = 0;
  const static int IMG = 1;
  const static int PAINT = 2;
};

// Data structure to hold prompting configuration
struct pipelineConfig {
  std::string prompt = "";
  std::string negative_prompt = "";
  std::string sampler = "pndm";
  int iterations = 1;
  int steps = 35;
  double cfg = 7.5;
  float strength = 0.5;
  int width = 512;
  int height = 512;
  int seed = 0;
};