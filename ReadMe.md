# Requirements

- Python 3.8.0+
- pip (see requirements.txt for libraries, can be installed with `pip install -r requirements.txt`)
- Docker see: https://github.com/NVIDIA/nvidia-docker#quickstart
- nvidia-container-toolkit

# Setup

- Install all requirements
- Initialise docker container `./start_docker.sh`
- Run `./stable-ui`

# Features

- TXT2IMG generation
- IMG2IMG generation
- Custom module extension
- VAE Support
- Stable Diffusion v2 support
- Low VRAM Support (TODO)
- Inpainting & Outpainting (TODO)
- Textual Inversion training (TODO)

# Details

This UI environment spins up and manages a docker image for Stable-Diffusion. It interfaces with this using python scripts to run SD commands to generate images.

# Adding models

To add models to stable-ui select `tools -> import model` files will automatically be copied to the `data/models` folder, if a model requires additional configuration i.e. the stable diffusion v2 models you can define them while importing. The model configuration file contains a `sha1sum` hash of the model files and a corresponding configuration, some popular ones are included by default.

# Custom Modules

Functionality of the application can be extended by loading custom modules, see `data/modules/ReadMe.md` for additional information, default module:

- `stable-ui`: SD V2 support base image/text generation scripts, runs the sd generation server, critical component and should not be removed

# Controls

- Middle mouse button to move the main window canvas

# Building

## Linux:

Initialise submodules:
`git submodule update --recursive --remote`

Install supporting libraries:

- Python 3.8+ dev libraries
- Python 3.8+ debug symbols
- Building:
  - Run a cmake build and then the `./scripts/lin/package.sh` shell script
  - Build is stored in `build/stable-ui/bin`

## Windows:

- Compiler: VSCC
- Additional libraries: Python debug symbols & Debug binaries
- Building: Run a cmake build and then the `.\scripts\package.bat` batch script
