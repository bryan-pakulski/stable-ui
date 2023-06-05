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
  std::string scheduler = "pndm";

  bool enable_xformers = true;
  bool enable_tf32 = true;
  bool enable_t16 = true;
  bool enable_vaeTiling = true;
  bool enable_vaeSlicing = false;
  bool enable_seqCPUOffload = false;
};