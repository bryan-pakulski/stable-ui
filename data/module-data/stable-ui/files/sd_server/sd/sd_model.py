from ldm.util import instantiate_from_config
from omegaconf import OmegaConf
import torch
import safetensors.torch
import sys
import os
import logging

from common import devices
# Courtesy of Automatic111; https://github.com/AUTOMATIC1111/stable-diffusion-webui/blob/master/modules/sd_models.py


class StableDiffusionModel():

    def __init__(self, checkpoint_path: str = "", vae_path: str = "", checkpoint_config_path: str = "", precision: str = ""):
        # Initialise logging
        logging.basicConfig(
            filename="/shared-config/sd_server.log",
            level=logging.INFO,
            format="[SDSERVER] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")

        self.MODELS_PATH = "/models/"  # Path on docker
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
                self.vae_path, map_location=devices.weight_load_location)
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

    def read_state_dict(self, map_location=None):
        _, extension = os.path.splitext(self.checkpoint_path)
        if extension.lower() == ".safetensors":
            device = map_location
            if device is None:
                device = devices.get_cuda_device_string()
            pl_sd = safetensors.torch.load_file(
                self.checkpoint_path, device=device)
        else:
            pl_sd = torch.load(self.checkpoint_path, map_location=map_location)
        sd = self.get_state_dict_from_checkpoint(pl_sd)
        return sd

    def load_model_weights(self):
        logging.info(f"Loading weights from {self.checkpoint_path}")

        sd = self.read_state_dict()
        self.model.load_state_dict(sd, strict=False)
        del sd

        # TODO: additional logic here for different precision options
        if self.get_precision() != "full":
            vae = self.model.first_stage_model
            self.model.half()
            self.model.first_stage_model = vae

        devices.dtype = torch.float32 if self.get_precision() == "full" else torch.float16
        devices.dtype_vae = torch.float32 if self.get_precision() == "full" else torch.float16

        self.model.first_stage_model.to(devices.dtype_vae)

        # Load additional VAE model
        if self.vae_path != "":
            logging.info(f"Loading additional VAE: {self.vae_path}")
            self.load_vae()

    def load_model(self):
        from common import lowvram
        self.model = instantiate_from_config(self.checkpoint_config.model)

        if self.model is None:
            logging.error('Failed to create model', file=sys.stderr)
            return None

        self.load_model_weights()

        if self.get_precision() != "full":
            lowvram.setup_for_low_vram(self.model, self.get_precision())
        else:
            self.model.to(devices.get_cuda_device_string())

        self.model.eval()

    def clean(self):
        if self.model:
            devices.torch_gc()