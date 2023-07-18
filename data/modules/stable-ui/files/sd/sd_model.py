from diffusers import StableDiffusionPipeline, EulerAncestralDiscreteScheduler
from diffusers.models import modeling_utils
from diffusers.pipelines.stable_diffusion.convert_from_ckpt import (
    download_from_original_stable_diffusion_ckpt,
    create_vae_diffusers_config
)

from sd import convert

import torch
import yaml
import safetensors.torch
from safetensors import safe_open
from omegaconf import OmegaConf
import logging
import sys
from diffusers.models import AutoencoderKL

from common import devices

class StableDiffusionModel():
    def __init__(self, checkpoint_path: str = "", vae_path: str = "", vae_config: str = "", convert_vae: bool = False, checkpoint_config_path: str = "", scheduler: str = "pndm", hash: str = "", enable_xformers: bool = False, enable_tf32: bool = False, enable_t16: bool = False, enable_vaeTiling: bool = False, enable_vaeSlicing: bool = False, enable_seqCPUOffload: bool = False):
        # Initialise logging
        logging.basicConfig(
            filename="/logs/sd_pipeline.log",
            level=logging.INFO,
            format="[SDPIPELINE] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")
        logging.addLevelName(logging.DEBUG, "DGB")

        self.model = None
        self.vae = None

        self.checkpoint_path = checkpoint_path
        self.checkpoint_config = checkpoint_config_path
        self.vae_path = vae_path
        self.vae_config = vae_config
        self.convert_vae = convert_vae
        self.scheduler = scheduler
        self.model_hash = hash
        self.model_type = ""
        
        if (self.checkpoint_path.endswith(".safetensors")):
            self.safe_tensors=True
        else:
            self.safe_tensors=False

        # For each LORA we are expecting the following object:
        # {
        #   path: "/path/to/lora/model"
        #   weight: 0.85
        # }
        # TODO: LORA support
        self.loras = []

        # TODO: if CPU is enabled we might want some of these optimisations disabled
        # Optimisations
        self.enable_xformers = enable_xformers
        self.enable_tf32 = enable_tf32
        self.enable_t16 = enable_t16
        self.enable_vaeTiling = enable_vaeTiling
        self.enable_vaeSlicing = enable_vaeSlicing
        self.enable_seqCPUOffload = enable_seqCPUOffload

    # Return type of model based on configuration used
    def get_model_type(self):
        with open(self.checkpoint_config) as f:
            config_dict = yaml.safe_load(f)
        
        target = config_dict['model']['target']

        target_map = {
            "sgm.models.diffusion.DiffusionEngine": "sdxl",
            "ldm.models.diffusion.ddpm.LatentInpaintDiffusion": "sd",
            "ldm.models.diffusion.ddpm.LatentDepth2ImageDiffusion": "sd",
            "ldm.models.diffusion.ddpm.LatentDiffusion": "sd"
        }

        if target in target_map:
            return target_map[target]
        else:
            return "sd"

    def clean(self):
        devices.torch_gc()

        if (self.model is not None):
            del self.model

    def load_vae(self, vae_config):
        if self.convert_vae:
            original_config = OmegaConf.load(vae_config)
            image_size = 512
            if self.vae_path.endswith(".safetensors"):
                checkpoint = {}
                with safe_open(self.vae_path, framework="pt", device="cpu") as f:
                    for key in f.keys():
                        checkpoint[key] = f.get_tensor(key)
            else:
                checkpoint = torch.load(self.vae_path, map_location=devices.get_cuda_device_string())["state_dict"]

            # Convert the VAE model.
            vae_config = create_vae_diffusers_config(original_config, image_size=image_size)
            converted_vae_checkpoint = convert.custom_convert_ldm_vae_checkpoint(checkpoint, vae_config)

            self.vae = AutoencoderKL(**vae_config)
            self.vae.load_state_dict(converted_vae_checkpoint)
        else:
            encoder = AutoencoderKL()
            modelConfig = encoder.from_config(vae_config, local_files_only=True)
            if (self.vae_path.endswith(".safetensors")):
                state_dict = safetensors.torch.load_file(self.vae_path, device="cpu")
            else:
                state_dict = torch.load(self.vae_path, map_location="cpu")

            load = encoder._load_pretrained_model(
                model=modelConfig,
                state_dict=state_dict,
                resolved_archive_file=None,
                pretrained_model_name_or_path = self.vae_path)
            self.vae = load[0]
            self.vae = self.vae.to(devices.dtype)

            self.vae.register_to_config(_name_or_path=self.vae_path)
        
        self.vae.eval()


    def loadLORA(self):
        for lora in self.loras:
            self.model.unet.load_attn_procs(lora.path)

    # Optimisations, see https://huggingface.co/docs/diffusers/optimization/fp16
    def enableOptimisations(self):
        if self.enable_xformers:
            logging.info("Enabling xformers memory efficient attention")
            self.model.enable_xformers_memory_efficient_attention()
        if self.enable_tf32:
            logging.info("Enabling TF32")
            torch.backends.cuda.matmul.allow_tf32 = True
        if self.enable_vaeTiling:
            logging.info("Enabling VAE Tiling")
            self.model.enable_vae_tiling()
        if self.enable_vaeSlicing:
            logging.info("Enabling VAE Slicing")
            self.model.enable_vae_slicing()
        if self.enable_seqCPUOffload:
            logging.info("Enabling CPU Offload")
            self.model.enable_sequential_cpu_offload()
        

    def load_model(self):
        try:
            logging.info(f"Using device: {devices.get_cuda_device_string()}")
            # Determine model type
            self.model_type = self.get_model_type()
            logging.info(f"Deducing model type from config: {self.model_type}")
            
            # Determine type size
            if devices.get_cuda_device_string() == "cpu" or not self.enable_t16:
                devices.dtype, devices.dtype_vae = torch.float32, torch.float32
                logging.info("Using torch.float32 weights")
            else:
                devices.dtype, devices.dtype_vae = torch.float16, torch.float16
                logging.info("Using torch.float16 weights")
            
            # Load VAE
            if (self.vae_path != ""):
                self.load_vae(self.vae_config)

            # Load Model
            self.model = download_from_original_stable_diffusion_ckpt(self.checkpoint_path, self.checkpoint_config, scheduler_type=self.scheduler, device=devices.get_cuda_device_string(), from_safetensors=self.safe_tensors, load_safety_checker=False)
            if self.vae != None:
                self.model.vae = self.vae
            
            # Load LORA
            self.loadLORA()
            
            # Optimise and send to device
            self.enableOptimisations()
            self.model.to(devices.get_cuda_device_string(), torch_dtype=devices.dtype)

            logging.info(f"Loaded model with hash: {self.model_hash}")
        except:
            self.clean()
            logging.error('Failed to create model')

    # TODO: When generating an image use the cross_attention_kwargs={"scale":"weight"} for lora support