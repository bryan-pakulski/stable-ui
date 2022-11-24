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
- Inpainting & Outpainting (TODO)
- Textual Inversion training (TODO)
- Stable Diffusion v2 support (TODO)
- Custom module extension (TODO)

# Details
This UI environment spins up and manages a docker image for Stable-Diffusion. It interfaces with this using python scripts to run SD commands to generate images.

# Adding models
To add models to stable-ui copy your .ckpt files to the `data/models` folder, if a model requires additional configuration i.e. the stable diffusion v2 models
require using custom config i.e. `--config v2-inpainting-inference.yaml` 

You can specify this in the `data/models/model_config.yaml` folder. This contains a
`sha1sum` hash of the model files and a corresponding configuration, some popular ones are included by default.

# Custom Modules
Functionality of the application can be extended by loading custom modules, see `data/modules/ReadMe.md` for additional information, default modules include:
- `stable-diffusion-v2`: SD v2 support 

# Controls
- Middle mouse button to move the main window canvas