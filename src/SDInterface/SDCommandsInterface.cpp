#include "SDCommandsInterface.h"

SDCommandsInterface::SDCommandsInterface() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Initialising SDCommandsInterface");

  std::string path = "sys.path.append(\"" + CONFIG::PYTHON_CONFIG_PATH.get() + "\")";

  Py_Initialize();
  PyRun_SimpleString("import sys");
  PyRun_SimpleString(path.c_str());

  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Attached PYTHONPATH: ", path);

  m_py_handle = std::unique_ptr<SnakeHandler>(new SnakeHandler("sd_commands"));
}

SDCommandsInterface::~SDCommandsInterface() { delete arguments; }

void SDCommandsInterface::textToImage(std::string prompt, std::string negative_prompt, int samples, int steps, double cfg, int seed, int width, int height,
                                      bool &finishedFlag, std::string model_name) {
  std::string functionName = "txt2image";
  std::string exec_path = CONFIG::STABLE_DIFFUSION_DOCKER_PATH.get() + CONFIG::TXT_TO_IMG_PATH.get();
  std::string out_dir = CONFIG::OUTPUT_DIRECTORY.get();
  std::string model_path = "/models/" + model_name;

  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", exec_path, 0)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "prompt", prompt, 1)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "negative_prompt", negative_prompt, 2)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "samples", samples, 3)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "steps", steps, 4)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<double>('f', "scale", cfg, 5)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "seed", seed, 6)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "width", width, 7)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "height", height, 8)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "out_dir", out_dir, 9)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "ckpt_name", model_path, 10)));

  // Offload thread execution, image generation can take some time
  m_Thread = std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, std::ref(arguments),
                         std::ref(finishedFlag));
  m_Thread.detach();
}

void SDCommandsInterface::imageToImage() {}