import os
import sys
import torch
import numpy as np
from tqdm import tqdm, trange
from PIL import Image
from pytorch_lightning import seed_everything
from contextlib import nullcontext
from einops import rearrange

from ldm.util import instantiate_from_config

import logging

from common import samplers

"""
 Supported Samplers:
 - PLMS
 - DDIM
"""


class StableDiffusionBaseProcess():
    """
    The first set of paramaters: sd_models -> do_not_reload_embeddings represent the minimum required to create a StableDiffusionProcessing
    """

    def __init__(self, outpath_samples: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = None):
        self.outpath_samples = outpath_samples
        self.prompt = prompt
        self.negative_prompt = negative_prompt
        self.seed = seed
        self.model = model
        self.sampler = samplers.getSampler(sampler_name, self.model)
        self.batch_size = batch_size
        self.n_iter = n_iter
        self.steps = steps
        self.cfg_scale = cfg_scale
        self.width = width
        self.height = height

        self.ddim_eta = 0.0
        self.latent_channels = 4
        self.downsampling_factor = 8

        self.iteration = 0

        # Initialise logging
        logging.basicConfig(
            filename="/shared-config/sd_server.log",
            level=logging.INFO,
            format="[SDSERVER] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")

        logging.info("Starting up sd_server")

    # Create subfolder for our images

    def create_sub_folder(self, name, default):
        if (self.outpath_samples != ""):
            logging.info(f"Creating folder: {self.outpath_samples}")
            os.makedirs(self.outpath_samples, exist_ok=True)
        if name == "":
            name = default
        logging.info(f"Creating subfolder: {name}")
        self.outpath_samples = os.path.join(self.outpath_samples, name)
        os.makedirs(self.outpath_samples, exist_ok=True)


class StableDiffusionTxt2Img(StableDiffusionBaseProcess):

    def __init__(self, outpath_samples: str = "", subfolder_name: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = None):
        super().__init__(outpath_samples, prompt, negative_prompt, seed,
                         sampler_name, batch_size, n_iter, steps, cfg_scale, width, height, model)

        self.create_sub_folder(subfolder_name, "txt2img")
        self.data = [self.batch_size * [self.prompt]]

    def sample(self):
        seed_everything(self.seed)
        # autocast if opt.precision == "autocast" else nullcontext
        precision_scope = nullcontext
        with torch.no_grad():
            with precision_scope("cuda"):
                with self.model.ema_scope():
                    for n in trange(self.n_iter, desc="Sampling"):
                        for prompts in tqdm(self.data, desc="data"):
                            unconditional_conditioning = None
                            if self.cfg_scale != 1.0 or self.negative_prompt == "":
                                unconditional_conditioning = self.model.get_learned_conditioning(
                                    self.batch_size * [""])
                            else:
                                unconditional_conditioning = self.model.get_learned_conditioning(
                                    len(prompts) * [self.negative_prompt])

                            if isinstance(prompts, tuple):
                                prompts = list(prompts)

                            conditioning = self.model.get_learned_conditioning(
                                prompts)
                            shape = [self.latent_channels, self.height //
                                     self.downsampling_factor, self.width // self.downsampling_factor]
                            samples_ddim, _ = self.sampler.sample(S=self.ddim_steps,
                                                                  conditioning=conditioning,
                                                                  batch_size=self.n_samples,
                                                                  shape=shape,
                                                                  verbose=False,
                                                                  unconditional_guidance_scale=self.cfg_scale,
                                                                  unconditional_conditioning=unconditional_conditioning,
                                                                  eta=self.ddim_eta,
                                                                  x_T=None)

                            x_samples_ddim = self.model.decode_first_stage(
                                samples_ddim)
                            x_samples_ddim = torch.clamp(
                                (x_samples_ddim + 1.0) / 2.0, min=0.0, max=1.0)

                            for x_sample in x_samples_ddim:
                                x_sample = 255. * \
                                    rearrange(x_sample.cpu().numpy(),
                                              'c h w -> h w c')
                                base_count = len(
                                    os.listdir(self.outpath_samples))
                                Image.fromarray(x_sample.astype(np.uint8)).save(
                                    os.path.join(self.outpath_samples, f"{base_count:05}.png"))


class StableDiffusionImg2Img(StableDiffusionBaseProcess):

    def __init__(self, outpath_samples, subfolder_name, prompt, negative_prompt, seed, sampler_name, batch_size, n_iter, steps, cfg_scale, width, height):
        super().__init__(outpath_samples, prompt, negative_prompt, seed,
                         sampler_name, batch_size, n_iter, steps, cfg_scale, width, height)

        self.create_sub_folder(subfolder_name, "img2img")
