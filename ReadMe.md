# About

This application stack provides an evironment to create & manage a Stable-Diffusion inference server. 
It is composed of a native front end app & dockerised backend server to ensure ease of setup and maintenance.

# Requirements

These are requirements on both Windows (WSL) & Linux

- Docker see: https://github.com/NVIDIA/nvidia-docker#quickstart
- nvidia-container-toolkit

# Installation

- Install all requirements
- Initialise the docker container with `start_docker.sh / start_docker.bat`
- Run `stable-ui`

# Features

- text to image / image to image generation
- custom module extensions
- vae, pickle & safetensor model support
- Stable Diffusion v1 & v2
- LORA support (TODO)
- low VRAM Support (TODO)
- CPU inference (TODO)
- infinite canvas inpainting & outpainting (TODO)
- textual inversion training (TODO)

# Usage

## Adding models

To add models to stable-ui select `tools -> import model` and load your `.ckpt / .vae` files. Once imported a copy of the model is saved
to the `data/models` folder. The original file can be deleted.

## Configuring models

On model import you will be prompted to provide configuration for a given ckpt, if you'd like to reconfigure a model you can 
redefine configuration via the `tools -> configure models` menu.

## Loading models & generating images

The image generation options only become available once a model has been loaded into memory, you can do this by importing a model and then loading it with
`tools -> load model to memory`. Once loading is completed the txt2img and img2img toolbar will unlock and you can begin prompting the model server.

## Custom Modules

Functionality of the application can be extended by loading custom modules, see `data/modules/ReadMe.md` for additional information, default modules include:

- `stable-ui`: SD V1 & V2 support for image/text generation scripts, runs the sd generation ZMQ server within docker, critical component and should not be removed

## Controls

- Middle mouse button to move the main window canvas

# Building

There is a github pipeline available for both linux & windows that runs on the master branch.
Release artifacts are saved for each tagged version and published.

If you'd like to build locally you can perform the following:

## Linux:

Initialise submodules:
`git submodule update --recursive --remote`

Install supporting libraries:
- libzmq

- Building:
  - Run the `./scripts/local_build.sh` shell script
  - Build is stored in `build/stable-ui-bin` (statically linked)

## Windows:

- Compiler: VSCC
- Libraries:
  - libzmq


Libraries can be installed using vcpkg in the same manner as they are set up in the pipeline:
```
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install zeromq
```

- Building: Run `cmake --build` and then the `.\scripts\pipeline\package.bat` batch script
