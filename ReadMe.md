# About

This application stack provides an evironment to create & manage a Stable-Diffusion inference server.
It is composed of a native front end app & dockerised backend server to ensure ease of setup and maintenance.

# Requirements

- Docker see: https://github.com/NVIDIA/nvidia-docker#quickstart

### Linux specific requirements:

- `libzmq` / `libzmq-dev` / `libzmq3-dev` (dependant on distribution)


# Installation

- Install all requirements
- Initialise the docker container with `start_docker.sh / start_docker.bat`
- Run `stable-ui`

# Features

- text to image / image to image / outpainting pipelines
- Stable Diffusion v1, v2 & sdxl support
- CPU support
- custom module extensions
- vae, pickle & safetensor model support
- low VRAM Support
- weighted prompts using compel (++ and -- to emphasize keywords)
- LORA support (TODO)
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

## Optimisations

In order to speed up inference time and use resources more effeciently the following optimisations are in place:

### Always enabled / CPU inference

- jemalloc memory allocation library is used inside the docker image for more efficient memory allocation (https://github.com/jemalloc/jemalloc)
- numactl is installed to ensure that inference is pinned across all available CPU cores (https://linux.die.net/man/8/numactl)

## Controls

- Middle mouse button to move the main window canvas

# Building

There is a github pipeline available for both linux & windows that runs on the master branch.
Release artifacts are saved for each tagged version and published.

If you'd like to build locally you can follow below steps after cloning the repo and initialising
submodules:

- Initialise submodules: `cd src/ThirdParty && git submodule update --init --recursive && git submodule update --remote`

It's assumed that you are using the CMake build system.

## Linux:

Install supporting libraries:

- `libzmq` / `libzmq-dev` / `libzmq3-dev` (dependant on distribution)
- `zlib`

### Building:

- Run the following build scripts:
  - `build.sh -r` compile thirdparty libraries followed by stable-ui gui and package docker container, `-r` flag is optional to enable release mode
  - `deploy.sh` Deploy docker changes and synchronise with `compose-up`
  - `run.sh` Run gui
- Build is stored in `build/stable-ui-bin` (statically linked)

## Windows:

- Compiler: VSCC Visual Studio 17 2022

The `build_release.bat` script assumes the you have vcpkg installed in the root `C:\` drive<br>
Libraries can be installed using vcpkg in the same manner as they are set up in the pipeline:

```
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install zeromq --triplet x64-windows
C:\vcpkg\vcpkg install zlib
```

### Building

- `./scripts/build.bat -r` to build & package application, stored in `build/stable-ui-bin`, `-r` is optional to enable release mode
- `./scripts/deploy.bat` to deploy / update existing docker container
- `./scripts/run.bat` to start application