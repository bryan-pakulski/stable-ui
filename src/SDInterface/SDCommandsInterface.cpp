#include "SDCommandsInterface.h"

#include "../Helpers/States.h"

SDCommandsInterface::SDCommandsInterface() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Initialising SDCommandsInterface");

  std::string path = "sys.path.append(\"" + CONFIG::PYTHON_CONFIG_PATH.get() + "\")";

  Py_Initialize();
  PyRun_SimpleString("import sys");
  PyRun_SimpleString(path.c_str());

  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Attached PYTHONPATH: ", path);

  // sd_commands.py is the script that handles all incoming calls from the stable-ui application
  m_py_handle = std::unique_ptr<SnakeHandler>(new SnakeHandler("sd_commands"));
}

SDCommandsInterface::~SDCommandsInterface() { delete arguments; }

// Starts up SD Model Server
void SDCommandsInterface::launchSDModelServer() {
  std::string functionName = "launchSDModelServer";
  arguments->emplace_back(
      std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", CONFIG::SD_MODEL_SERVER.get(), 0)));
  m_dockerState = EXECUTION_STATE::LOADING;

  // Offload thread execution, image generation can take some time
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Starting up SD Model Server...");
  m_Thread = std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, std::ref(arguments),
                         std::ref(m_dockerState));
  m_Thread.detach();
}

// Terminates SD Model Server
void SDCommandsInterface::terminateSDModelServer() {
  std::string functionName = "terminateSDModelServer";
  arguments->emplace_back(
      std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", CONFIG::SD_SERVER_CLIENT.get(), 0)));
  m_dockerState = EXECUTION_STATE::LOADING;

  // Wait for thread execution to finish
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Shutting down SD Model Server...");
  m_Thread = std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, std::ref(arguments),
                         std::ref(m_dockerState));
  m_Thread.join();
}

// Connect a new model to SD Server
void SDCommandsInterface::attachModelToServer(std::string ckpt_path, std::string config_path, std::string vae_path,
                                              std::string precision, int &state) {
  std::string functionName = "attachSDModelToServer";
  std::string execPath = CONFIG::SD_SERVER_CLIENT.get();
  state = EXECUTION_STATE::LOADING;

  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", execPath, 0)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "ckpt_path", ckpt_path, 1)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "config_path", config_path, 2)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "vae_path", vae_path, 3)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "precision", precision, 4)));

  // Offload thread execution, image generation can take some time
  m_Thread =
      std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, std::ref(arguments), std::ref(state));
  m_Thread.detach();
}

// Calls text to image command from client -> sd model server
void SDCommandsInterface::textToImage(std::string prompt, std::string negative_prompt, int samples, int steps,
                                      double cfg, int seed, int width, int height, int &renderState) {
  /*
  std::string functionName = "txt2image";
  std::string exec_path = CONFIG::TXT_TO_IMG_PATH.get();
  std::string out_dir = CONFIG::OUTPUT_DIRECTORY.get();
  std::string model_path = "/models/" + model_name;
  std::string precision = half_precision ? "autocast" : "full";

  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", exec_path, 0)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "prompt", prompt, 1)));
  arguments->emplace_back(
      std::unique_ptr<base_type>(new d_type<std::string>('s', "negative_prompt", negative_prompt, 2)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "samples", samples, 3)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "steps", steps, 4)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<double>('f', "scale", cfg, 5)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "seed", seed, 6)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "width", width, 7)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "height", height, 8)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "out_dir", out_dir, 9)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "ckpt_name", model_path, 10)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "precision", precision, 11)));

  // Offload thread execution, image generation can take some time
  m_Thread = std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, std::ref(arguments),
                         std::ref(finishedFlag));
  m_Thread.detach();
*/
}

void SDCommandsInterface::imageToImage(std::string path, std::string prompt, std::string negative_prompt, int samples,
                                       int steps, double strength, int seed, int &renderState) {
  /*
  std::string functionName = "img2image";
  std::string exec_path = CONFIG::IMG_TO_IMG_PATH.get();
  std::string out_dir = CONFIG::OUTPUT_DIRECTORY.get();
  std::string model_path = "/models/" + model_name;
  std::string precision = half_precision ? "autocast" : "full";

  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", exec_path, 0)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "img_path", path, 1)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "prompt", prompt, 2)));
  arguments->emplace_back(
      std::unique_ptr<base_type>(new d_type<std::string>('s', "negative_prompt", negative_prompt, 3)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "samples", samples, 4)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "steps", steps, 5)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<double>('f', "strength", strength, 6)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "seed", seed, 7)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "out_dir", out_dir, 8)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "ckpt_name", model_path, 9)));
  arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "precision", precision, 10)));

  // Offload thread execution, image generation can take some time
  m_Thread = std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, std::ref(arguments),
                         std::ref(finishedFlag));
  m_Thread.detach();
*/
}