#include "SDCommandsInterface.h"

#include "../Helpers/States.h"
#include <memory>

SDCommandsInterface::SDCommandsInterface() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SDCommandsInterface::SDCommandsInterface initialising");

  std::string path = "sys.path.append(\"" + CONFIG::PYTHON_CONFIG_PATH.get() + "\")";

  Py_Initialize();
  PyRun_SimpleString("import sys");
  PyRun_SimpleString(path.c_str());

  PyEval_SaveThread();

  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SDCommandsInterface::SDCommandsInterface attached PYTHONPATH: ", path);

  // sd_commands.py is the script that handles all incoming calls from the stable-ui application
  m_py_handle = std::unique_ptr<SnakeHandler>(new SnakeHandler("sd_commands"));
  m_arguments = std::shared_ptr<PyArgs>(new PyArgs);
}

SDCommandsInterface::~SDCommandsInterface() {}

// Starts up SD Model Server
void SDCommandsInterface::launchSDModelServer() {
  std::string functionName = "launchSDModelServer";
  m_arguments->emplace_back(
      std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", CONFIG::SD_MODEL_SERVER.get(), 0)));
  m_dockerState = EXECUTION_STATE::LOADING;

  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SDCommandsInterface::launchSDModelServer starting up SD Model Server...");
  m_Thread =
      std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, m_arguments, std::ref(m_dockerState));
  m_Thread.detach();
}

// Terminates SD Model Server
void SDCommandsInterface::terminateSDModelServer() {
  std::string functionName = "terminateSDModelServer";
  std::string execPath = CONFIG::SD_SERVER_CLIENT.get();

  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", execPath, 0)));
  m_dockerState = EXECUTION_STATE::LOADING;

  // Wait for thread execution to finish
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "SDCommandsInterface::terminateSDModelServer shutting down SD Model Server...");
  m_Thread =
      std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, m_arguments, std::ref(m_dockerState));
  m_Thread.detach();
}

// Connect a new model to SD Server
void SDCommandsInterface::attachModelToServer(std::string ckpt_path, std::string config_path, std::string vae_path,
                                              std::string precision, int &state) {
  std::string functionName = "attachSDModelToServer";
  std::string execPath = CONFIG::SD_SERVER_CLIENT.get();
  state = EXECUTION_STATE::LOADING;

  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", execPath, 0)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "ckpt_path", ckpt_path, 1)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "config_path", config_path, 2)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "vae_path", vae_path, 3)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "precision", precision, 4)));

  m_Thread = std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, m_arguments, std::ref(state));
  m_Thread.detach();
}

// Ping server to establish online status
void SDCommandsInterface::heartbeat(int &heartBeatState) {
  std::string functionName = "heartbeat";
  std::string execPath = CONFIG::SD_SERVER_CLIENT.get();

  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", execPath, 0)));

  m_Thread =
      std::thread(&SnakeHandler::asyncCall, m_py_handle.get(), functionName, m_arguments, std::ref(heartBeatState));

  m_Thread.detach();
}

// Calls text to image command from client -> sd model server
void SDCommandsInterface::textToImage(std::string sdModelPath, std::string &canvasName, std::string prompt,
                                      std::string negative_prompt, std::string &samplerName, int batch_size, int steps,
                                      double cfg, int seed, int width, int height, int &renderState) {
  std::string functionName = "txt2img";
  std::string execPath = CONFIG::SD_SERVER_CLIENT.get();
  std::string outDir = CONFIG::OUTPUT_DIRECTORY.get();

  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", execPath, 0)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "sd_model_path", sdModelPath, 1)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "canvas_name", canvasName, 2)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "prompt", prompt, 3)));
  m_arguments->emplace_back(
      std::unique_ptr<base_type>(new d_type<std::string>('s', "negative_prompt", negative_prompt, 4)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "sampler_name", samplerName, 5)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "batch_size", batch_size, 6)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "steps", steps, 7)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<double>('f', "scale", cfg, 8)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "seed", seed, 9)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "width", width, 10)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "height", height, 11)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "out_dir", outDir, 12)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "n_iter", 1, 13)));

  // Offload thread execution, image generation can take some time
  renderState = EXECUTION_STATE::LOADING;
  m_Thread =
      std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, m_arguments, std::ref(renderState));
  m_Thread.detach();
}

void SDCommandsInterface::imageToImage(std::string &sdModelPath, std::string &canvasName, std::string &imgPath,
                                       std::string &prompt, std::string &negative_prompt, std::string &samplerName,
                                       int batch_size, int steps, double cfg, double strength, int seed,
                                       int &renderState) {
  std::string functionName = "img2img";
  std::string execPath = CONFIG::SD_SERVER_CLIENT.get();
  std::string outDir = CONFIG::OUTPUT_DIRECTORY.get();

  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "exec_path", execPath, 0)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "sd_model_path", sdModelPath, 1)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "canvas_name", canvasName, 2)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "img_path", imgPath, 3)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "prompt", prompt, 4)));
  m_arguments->emplace_back(
      std::unique_ptr<base_type>(new d_type<std::string>('s', "negative_prompt", negative_prompt, 5)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "sampler_name", samplerName, 6)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "batch_size", batch_size, 7)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "steps", steps, 8)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<double>('f', "scale", cfg, 9)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<double>('f', "strength", strength, 10)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "seed", seed, 11)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<std::string>('s', "out_dir", outDir, 12)));
  m_arguments->emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "n_iter", 1, 13)));

  // Offload thread execution, image generation can take some time
  renderState = EXECUTION_STATE::LOADING;
  m_Thread =
      std::thread(&SnakeHandler::callFunction, m_py_handle.get(), functionName, m_arguments, std::ref(renderState));
  m_Thread.detach();
}