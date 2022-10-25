# Requirements
- Python 3.8+ 
- pip (see rquirements.txt for libraries, can be installed with `pip install -r requirements.txt`)
- Docker see: https://github.com/NVIDIA/nvidia-docker#quickstart
- nvidia-container-toolkit

# Setup
- Install all requirements
- Initialise docker container `./start_docker.sh`
- Run `./stable-ui`

# Features
- TXT2IMG generation
- IMG2IMG generation
- Inpainting & Outpainting
- Textual Inversion training

# Details
This UI environment spins up and manages a docker image for Stable-Diffusion. It interfaces with this using python scripts to run SD commands to generate images.

# Adding models
To add models to stable-ui copy your .ckpt files to the `data/models` folder