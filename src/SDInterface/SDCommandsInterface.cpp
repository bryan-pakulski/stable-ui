#include "SDCommandsInterface.h"

SDCommandsInterface::SDCommandsInterface() {
    QLogger::GetInstance().Log(LOGLEVEL::INFO, "Initialising SDCommandsInterface");

    std::string path = "sys.path.append(\"" + CONFIG::PYTHON_CONFIG_PATH + "\")";

    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(path.c_str());

    QLogger::GetInstance().Log(LOGLEVEL::INFO, "Attached PYTHONPATH: ", path);

    m_py_handle = std::unique_ptr<SnakeHandler>(new SnakeHandler("sd_commands"));

    // Test image generation
    TextToImage();
}

SDCommandsInterface::~SDCommandsInterface() {

}

void SDCommandsInterface::TextToImage() {
    // Build arguments
    std::vector<std::unique_ptr<base_type>> arguments; 

    std::string functionName = "txt2image";
    std::string exec_path = CONFIG::STABLE_DIFFUSION_DOCKER_PATH + CONFIG::TXT_TO_IMG_PATH;
    std::string prompt = "Cat with a tin hat, studio ghibli style";
    int samples = 1;
    int steps = 80;
    int seed = 54321;
    int width = 512;
    int height = 512;

    arguments.emplace_back(std::unique_ptr<base_type>(new d_type<const char*>('s', "exec_path", exec_path.c_str(), 0)));
    arguments.emplace_back(std::unique_ptr<base_type>(new d_type<const char*>('s', "prompt", prompt.c_str(), 1)));
    arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "samples", samples, 2)));
    arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "steps", steps, 3)));
    arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "seed", seed, 4)));
    arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "width", width, 5)));
    arguments.emplace_back(std::unique_ptr<base_type>(new d_type<int>('d', "height", height, 6)));
   

    m_py_handle->callFunction(functionName, arguments);
}

void SDCommandsInterface::ImageToImage() {

}