// Description of commands and the parameters we can use when sending to server
#pragma once
#include <vector>
#include <string>
#include <sstream>

// TODO: get additional configurations for certain commands

// Base command class
class command {
public:
  std::string m_name;
  std::vector<std::pair<std::string, std::string>> m_parameters;

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
  loadModelToMemory(std::string &ckpt_path, std::string &config_path, std::string &vae_path, std::string &precision) {
    m_name = "loadModel";

    m_parameters.push_back(makePair("checkpoint_path", ckpt_path));
    m_parameters.push_back(makePair("checkpoint_config_path", config_path));

    // Optionals:
    if (vae_path != "") {
      m_parameters.push_back(makePair("vae_path", vae_path));
    }
    if (precision != "") {
      m_parameters.push_back(makePair("precision", precision));
    }
  }
};

/*
Call command for txt2img inference

Parameters:
    - prompt                        (Prompt)
    - negative_prompt               (Negative prompt string)
    - subfolder_name                (Subfolder to store images in, named after canvas)
    - sampler_name                  (Sampler i.e. DPMS)
    - batch_size                    (Number of images per batch)
    - n_iter                        (Number of batches)
    - steps                         (Number of steps to generate)
    - cfg_scale                     (CFG scale value 0.0 -> 12.0)
    - seed                          (RNG seed, -1 for random)
    - height                        (image height in pixels)
    - width                         (image width in pixels)
    - outpath_samples               (output directory)
*/
class textToImage : public command {
  textToImage(std::string &prompt, int width, int height, std::string &negative_prompt, std::string &canvas_name,
              std::string &sampler_name, int batch_size, int n_iter, int steps, double cfg_scale, int seed,
              std::string &out_path) {
    m_name = "txt2img";

    m_parameters.push_back(makePair("prompt", prompt));
    m_parameters.push_back(makePair("negative_prompt", negative_prompt));
    m_parameters.push_back(makePair("subfolder_name", canvas_name));
    m_parameters.push_back(makePair("sampler_name", sampler_name));
    m_parameters.push_back(makePair("batch_size", batch_size));
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
    - batch_size                    (Number of images per batch)
    - n_iter                        (Number of batches)
    - steps                         (Number of steps to generate)
    - cfg_scale                     (CFG scale value 0.0 -> 12.0)
    - strength                      (Signal noise ratio of reference image 0 - 1.0)
    - seed                          (RNG seed, -1 for random)
    - outpath_samples               (output directory)
*/
class imageToImage : public command {
  imageToImage(std::string &prompt, std::string &negative_prompt, std::string &canvas_name, std::string &img_path,
               std::string &sampler_name, int batch_size, int n_iter, int steps, double cfg_scale, double strength,
               int seed, std::string &out_path) {
    m_name = "txt2img";

    m_parameters.push_back(makePair("prompt", prompt));
    m_parameters.push_back(makePair("negative_prompt", negative_prompt));
    m_parameters.push_back(makePair("subfolder_name", canvas_name));
    m_parameters.push_back(makePair("img_path", img_path));
    m_parameters.push_back(makePair("sampler_name", sampler_name));
    m_parameters.push_back(makePair("batch_size", batch_size));
    m_parameters.push_back(makePair("n_iter", n_iter));
    m_parameters.push_back(makePair("steps", steps));
    m_parameters.push_back(makePair("cfg_scale", cfg_scale));
    m_parameters.push_back(makePair("seed", seed));
    m_parameters.push_back(makePair("outpath_samples", out_path));
  }
};

} // namespace commands
