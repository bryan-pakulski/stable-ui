#include "SDCommandsInterface.h"

SDCommandsInterface::SDCommandsInterface() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Initialising SDCommandsInterface");

  m_Thread = std::thread(&SDCommandsInterface::m_Thread, this);

  std::string path = "sys.path.append(\"" + CONFIG::PYTHON_CONFIG_PATH + "\")";

  Py_Initialize();
  PyRun_SimpleString("import sys");
  PyRun_SimpleString(path.c_str());

  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Attached PYTHONPATH: ", path);

  m_py_handle = std::unique_ptr<SnakeHandler>(new SnakeHandler("sd_commands"));
}

SDCommandsInterface::~SDCommandsInterface() { m_Thread.join(); }

void SDCommandsInterface::textToImage(std::string prompt, int samples, int steps, int seed, int width, int height) {
  // Build arguments
  std::vector<std::unique_ptr<base_type>> arguments;

  std::string functionName = "txt2image";
  std::string exec_path = CONFIG::STABLE_DIFFUSION_DOCKER_PATH + CONFIG::TXT_TO_IMG_PATH;

  arguments.emplace_back(std::unique_ptr<base_type>(new d_type<const char *>('s', "exec_path", exec_path.c_str(), 0)));
  arguments.emplace_back(std::unique_ptr<base_type>(new d_type<const char *>('s', "prompt", prompt.c_str(), 1)));
  arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "samples", samples, 2)));
  arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "steps", steps, 3)));
  arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "seed", seed, 4)));
  arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "width", width, 5)));
  arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "height", height, 6)));

  m_py_handle->callFunction(functionName, arguments);
}

void SDCommandsInterface::imageToImage() {}