from typing import Optional
from ldm.util import instantiate_from_config
from omegaconf import OmegaConf
import torch
import safetensors.torch
import sys
import os
import logging
import inspect

from common import devices
# Courtesy of Automatic111; https://github.com/AUTOMATIC1111/stable-diffusion-webui/blob/master/modules/sd_models.py


class StableDiffusionModel():

    def __init__(self, checkpoint_path: str = "", vae_path: str = "", checkpoint_config_path: str = "", precision: str = ""):
        # Initialise logging
        logging.basicConfig(
            filename="/logs/sd_server.log",
            level=logging.INFO,
            format="[SDSERVER] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")

        self.MODELS_PATH = "/models/"  # Path on docker
        self.no_half_vae = True
        self.vae_ignore_keys = {"model_ema.decay", "model_ema.num_updates"}
        self.precision_list = ["full", "mid", "low", "autocast"]

        # Default precision to autocast on incorrectly passed value
        try:
            self.precision_index = self.precision_list.index(precision)
        except ValueError as ve:
            logging.info(
                f"{precision} precision not valid, defaulting to autocast")
            self.precision_index = 3

        self.model = None
        self.checkpoint_path = checkpoint_path
        self.checkpoint_config = OmegaConf.load(checkpoint_config_path)
        self.vae_path = vae_path

    def get_precision(self):
        return self.precision_list[self.precision_index]

    def load_vae(self):
        if self.vae_path != "" and self.model != None:
            logging.info(f"Loading VAE weights from: {self.vae_path}")

            vae_ckpt = self.read_state_dict(
                self.vae_path, devices.weight_load_location)
            vae_dict = {k: v for k, v in vae_ckpt.items(
            ) if k[0:4] != "loss" and k not in self.vae_ignore_keys}
            self.model.first_stage_model.load_state_dict(vae_dict)
            self.model.first_stage_model.to(devices.dtype_vae)

    def transform_checkpoint_dict_key(self, k):
        chckpoint_dict_replacements = {
            'cond_stage_model.transformer.embeddings.': 'cond_stage_model.transformer.text_model.embeddings.',
            'cond_stage_model.transformer.encoder.': 'cond_stage_model.transformer.text_model.encoder.',
            'cond_stage_model.transformer.final_layer_norm.': 'cond_stage_model.transformer.text_model.final_layer_norm.',
        }

        for text, replacement in chckpoint_dict_replacements.items():
            if k.startswith(text):
                k = replacement + k[len(text):]

        return k

    def get_state_dict_from_checkpoint(self, pl_sd):
        pl_sd = pl_sd.pop("state_dict", pl_sd)
        pl_sd.pop("state_dict", None)

        sd = {}
        for k, v in pl_sd.items():
            new_key = self.transform_checkpoint_dict_key(k)

            if new_key is not None:
                sd[new_key] = v

        pl_sd.clear()
        pl_sd.update(sd)

        return pl_sd

    def read_state_dict(self, checkpoint_file, map_location=None):
        _, extension = os.path.splitext(checkpoint_file)
        device = map_location
        if device is None:
            device = devices.get_cuda_device_string()
        if extension.lower() == ".safetensors":
            pl_sd = safetensors.torch.load_file(
                checkpoint_file, device=device)
        else:
            pl_sd = torch.load(checkpoint_file, map_location=device)
        sd = self.get_state_dict_from_checkpoint(pl_sd)
        return sd

    def load_model_weights(self):
        logging.info(f"Loading weights from {self.checkpoint_path}")

        sd = self.read_state_dict(self.checkpoint_path)
        self.model.load_state_dict(sd, strict=False)
        del sd

        # TODO: additional logic here for different precision options
        if devices.get_cuda_device_string() == "cpu":
            devices.dtype = torch.float32
            devices.dtype_vae = torch.float32
        else:
            if self.get_precision() == "full":
                devices.dtype = torch.float32
                devices.dtype_vae = torch.float32

            if self.get_precision() == "autocast":
                # with --no-half-vae, remove VAE from model when doing half() to prevent its weights from being converted to float16
                vae = self.model.first_stage_model
                if self.no_half_vae:
                    self.model.first_stage_model = None
                self.model.half()
                self.model.first_stage_model = vae
                devices.dtype = torch.float16
                devices.dtype_vae = torch.float16

            if self.get_precision() == "med":
                # with --no-half-vae, remove VAE from model when doing half() to prevent its weights from being converted to float16
                vae = self.model.first_stage_model
                if self.no_half_vae:
                    self.model.first_stage_model = None
                self.model.half()
                self.model.first_stage_model = vae
                devices.dtype = torch.float16
                devices.dtype_vae = torch.float16

            if self.get_precision() == "low":
                # with --no-half-vae, remove VAE from model when doing half() to prevent its weights from being converted to float16
                vae = self.model.first_stage_model
                if self.no_half_vae:
                    self.model.first_stage_model = None
                self.model.half()
                self.model.first_stage_model = vae
                devices.dtype = torch.float16
                devices.dtype_vae = torch.float16

        # TODO: look into 8 bit quantization
        self.model.first_stage_model.to(devices.dtype_vae)

        # Load additional VAE model
        if self.vae_path != "":
            logging.info(f"Loading additional VAE: {self.vae_path}")
            self.load_vae()

    def load_model(self):
        try:
            from common import lowvram
            self.model = instantiate_from_config(self.checkpoint_config.model)

            if self.model is None:
                logging.error('Failed to create model', file=sys.stderr)
                return None

            self.load_model_weights()
            
            # TODO: adjust precision 
            # On CPU we send straight to device
            if (devices.get_cuda_device_string() == "cpu"):
                self.model.to(devices.get_cuda_device_string())
            else:
                if self.get_precision() == "low":
                    lowvram.setup_for_low_vram(self.model, False)
                    
                if self.get_precision() == "med":
                    lowvram.setup_for_low_vram(self.model, True)

                if self.get_precision() == "autocast":
                    self.model.to(devices.get_cuda_device_string())    

                if self.get_precision() == "full":
                    self.model.to(devices.get_cuda_device_string())                

            self.model.eval()
        except:
            self.clean()
            logging.error('Failed to create model', file=sys.stderr)
            return None

    def clean(self):
        devices.torch_gc()

        if (self.model is not None):
            del self.model
