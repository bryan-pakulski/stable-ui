// Description of commands and the parameters we can use when sending to server
#pragma once
#include "yaml-cpp/node/node.h"
#include "Config/config.h"
#include <vector>
#include <string>
#include <sstream>

// Base command class
class command {
public:
  std::string m_name;
  std::vector<std::pair<std::string, std::string>> m_parameters;

public:
  // Return a formatted and delimited command string i.e. "command:k1=v1:k2=v2"
  std::string getCommandString() {
    std::stringstream cmdStream;

    cmdStream << m_name;

    for (auto &kvp : m_parameters) {
      cmdStream << ":" << kvp.first << "=" << kvp.second;
    }
    return cmdStream.str();
  }

  // Make a key value pair
  std::pair<std::string, std::string> makePair(std::string a, std::string b) {
    return std::pair<std::string, std::string>{a, b};
  }
  std::pair<std::string, std::string> makePair(std::string a, double b) {
    return std::pair<std::string, std::string>{a, std::to_string(b)};
  }
  std::pair<std::string, std::string> makePair(std::string a, int b) {
    return std::pair<std::string, std::string>{a, std::to_string(b)};
  }
  std::pair<std::string, std::string> makePair(std::string a, bool b) {
    return std::pair<std::string, std::string>{a, std::to_string(b)};
  }
};

namespace commands {

class heartbeat : public command {
public:
  heartbeat() { m_name = "ping"; }
};

class restartServer : public command {
public:
  restartServer() { m_name = "restart"; }
};

/*
Call command to load stable diffusion model to memory

Parameters:
    - checkpoint_path               (Path to model file inside docker image)
    - checkpoint_config_path        (Path to model configuration inside docker image)
    - vae_path                      (Path to VAE checkpoint inside docker image)
    - precision                     (Precision model to use i.e. auto, full, med, half, low)
*/
class loadModelToMemory : public command {
public:
  loadModelToMemory(const ModelConfig &model) {
    m_name = "loadModel";

    m_parameters.push_back(makePair("checkpoint_path", model.path));
    m_parameters.push_back(makePair("checkpoint_config_path", model.config));
    m_parameters.push_back(makePair("vae_path", model.vae));
    m_parameters.push_back(makePair("vae_config", model.vae_config));
    m_parameters.push_back(makePair("scheduler", model.scheduler));
    m_parameters.push_back(makePair("hash", model.hash));

    // Optimisations
    m_parameters.push_back(makePair("enable_xformers", model.enable_xformers));
    m_parameters.push_back(makePair("enable_tf32", model.enable_tf32));
    m_parameters.push_back(makePair("enable_t16", model.enable_t16));
    m_parameters.push_back(makePair("enable_vaeTiling", model.enable_vaeTiling));
    m_parameters.push_back(makePair("enable_vaeSlicing", model.enable_vaeSlicing));
    m_parameters.push_back(makePair("enable_seqCPUOffload", model.enable_seqCPUOffload));
  }
};

/*
Call command for txt2img inference

Parameters:
    - prompt                        (Prompt)
    - negative_prompt               (Negative prompt string)
    - subfolder_name                (Subfolder to store images in, named after canvas)
    - sampler_name                  (Sampler i.e. DPMS)
    - n_iter                        (Number of images)
    - steps                         (Number of steps to generate)
    - cfg_scale                     (CFG scale value 0.0 -> 12.0)
    - seed                          (RNG seed, -1 for random)
    - height                        (image height in pixels)
    - width                         (image width in pixels)
    - outpath_samples               (output directory)
*/
class textToImage : public command {
public:
  textToImage(ModelConfig model, std::string &prompt, int width, int height, std::string &negative_prompt,
              std::string canvas_name, std::string &sampler_name, int n_iter, int steps, double cfg_scale, int seed,
              std::string out_path) {
    m_name = "txt2img";

    prompt.append(" " + model.trigger_prompt);

    m_parameters.push_back(makePair("prompt", prompt));
    m_parameters.push_back(makePair("negative_prompt", negative_prompt));
    m_parameters.push_back(makePair("subfolder_name", canvas_name));
    m_parameters.push_back(makePair("sampler_name", sampler_name));
    m_parameters.push_back(makePair("n_iter", n_iter));
    m_parameters.push_back(makePair("steps", steps));
    m_parameters.push_back(makePair("cfg_scale", cfg_scale));
    m_parameters.push_back(makePair("seed", seed));
    m_parameters.push_back(makePair("height", height));
    m_parameters.push_back(makePair("width", width));
    m_parameters.push_back(makePair("outpath_samples", out_path));
  }
};

/*
Call command for img2img inference

Parameters:
    - prompt                        (Prompt)
    - negative_prompt               (Negative prompt string)
    - subfolder_name                (Subfolder to store images in, named after canvas)
    - img_path                      (Path to reference image)
    - sampler_name                  (Sampler i.e. DPMS)
    - n_iter                        (Number of images)
    - steps                         (Number of steps to generate)
    - cfg_scale                     (CFG scale value 0.0 -> 12.0)
    - strength                      (Signal noise ratio of reference image 0 - 1.0)
    - seed                          (RNG seed, -1 for random)
    - outpath_samples               (output directory)
*/
class imageToImage : public command {
public:
  imageToImage(ModelConfig &model, std::string &prompt, std::string &negative_prompt, std::string canvas_name,
               std::string &img_path, std::string &sampler_name, int n_iter, int steps, double cfg_scale,
               double strength, int seed, std::string out_path) {
    m_name = "img2img";

    prompt.append(" " + model.trigger_prompt);

    m_parameters.push_back(makePair("prompt", prompt));
    m_parameters.push_back(makePair("negative_prompt", negative_prompt));
    m_parameters.push_back(makePair("subfolder_name", canvas_name));
    m_parameters.push_back(makePair("init_img", img_path));
    m_parameters.push_back(makePair("sampler_name", sampler_name));
    m_parameters.push_back(makePair("n_iter", n_iter));
    m_parameters.push_back(makePair("steps", steps));
    m_parameters.push_back(makePair("cfg_scale", cfg_scale));
    m_parameters.push_back(makePair("strength", strength));
    m_parameters.push_back(makePair("seed", seed));
    m_parameters.push_back(makePair("outpath_samples", out_path));
  }
};

/*
Call command for outpainting

Parameters:
    - prompt                        (Prompt)
    - negative_prompt               (Negative prompt string)
    - subfolder_name                (Subfolder to store images in, named after canvas)
    - img_data                      (Base64 encoded image)
    - sampler_name                  (Sampler i.e. DPMS)
    - n_iter                        (Number of images)
    - steps                         (Number of steps to generate)
    - cfg_scale                     (CFG scale value 0.0 -> 12.0)
    - strength                      (Signal noise ratio of reference image 0 - 1.0)
    - seed                          (RNG seed, -1 for random)
    - outpath_samples               (output directory)
*/
class outpainting : public command {
public:
  outpainting(ModelConfig &model, std::string &prompt, std::string &negative_prompt, std::string canvas_name,
              std::string &img_data, std::string &img_mask, std::string &sampler_name, int n_iter, int steps,
              double cfg_scale, double strength, int seed, std::string out_path) {
    m_name = "outpaint";

    prompt.append(" " + model.trigger_prompt);

    m_parameters.push_back(makePair("prompt", prompt));
    m_parameters.push_back(makePair("negative_prompt", negative_prompt));
    m_parameters.push_back(makePair("subfolder_name", canvas_name));
    m_parameters.push_back(makePair("img_data", img_data));
    m_parameters.push_back(makePair("img_mask", img_mask));
    m_parameters.push_back(makePair("sampler_name", sampler_name));
    m_parameters.push_back(makePair("n_iter", n_iter));
    m_parameters.push_back(makePair("steps", steps));
    m_parameters.push_back(makePair("cfg_scale", cfg_scale));
    m_parameters.push_back(makePair("strength", strength));
    m_parameters.push_back(makePair("seed", seed));
    m_parameters.push_back(makePair("outpath_samples", out_path));
  }
};

} // namespace commands