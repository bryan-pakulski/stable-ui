import os
import sys
import torch
import numpy as np
from tqdm import tqdm, trange
import PIL
from PIL import Image
from pytorch_lightning import seed_everything
from contextlib import nullcontext
from einops import rearrange, repeat

from ldm.util import instantiate_from_config

import logging

from common import samplers
from common import devices
from common import lowvram

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

        # ddim eta (eta=0.0 corresponds to deterministic sampling",
        self.ddim_eta = 0.0
        self.latent_channels = 4
        self.downsampling_factor = 8

        self.iteration = 0

        # Initialise logging
        logging.basicConfig(
            filename="/logs/sd_server.log",
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

    def __init__(self, outpath_samples: str = "", subfolder_name: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = None, precision : str = ""):
        super().__init__(outpath_samples, prompt, negative_prompt, seed,
                         sampler_name, batch_size, n_iter, steps, cfg_scale, width, height, model)

        self.create_sub_folder(subfolder_name, "txt2img")
        self.data = [self.batch_size * [self.prompt]]
        self.precision = precision

    def sample(self):
        with torch.no_grad(), self.model.ema_scope():
            with devices.autocast(precision=self.precision):
                seed_everything(self.seed)

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
                        samples_ddim, _ = self.sampler.sample(S=self.steps,
                                                                conditioning=conditioning,
                                                                batch_size=self.batch_size,
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

                        del samples_ddim
                        
                        if (self.precision == "low") or (self.precision == "med"):
                            lowvram.send_everything_to_cpu()

                        for x_sample in x_samples_ddim:
                            x_sample = 255. * \
                                rearrange(x_sample.cpu().numpy(),
                                            'c h w -> h w c')
                            base_count = len(
                                os.listdir(self.outpath_samples))
                            Image.fromarray(x_sample.astype(np.uint8)).save(
                                os.path.join(self.outpath_samples, f"{base_count:05}.png"))

                    


class StableDiffusionImg2Img(StableDiffusionBaseProcess):

    def __init__(self, outpath_samples, subfolder_name, prompt, negative_prompt, init_img, seed, sampler_name, batch_size, strength, n_iter, steps, cfg_scale, model, precision):
        super().__init__(outpath_samples=outpath_samples, prompt=prompt, negative_prompt=negative_prompt, seed=seed,
                         sampler_name=sampler_name, batch_size=batch_size, n_iter=n_iter, steps=steps, cfg_scale=cfg_scale, model=model)

        self.create_sub_folder(subfolder_name, "img2img")
        self.data = [self.batch_size * [self.prompt]]
        self.precision = precision

        self.init_img = init_img
        self.strength = strength

    def load_img(self, path):
        image = Image.open(path).convert("RGB")
        w, h = image.size
        print(f"loaded input image of size ({w}, {h}) from {path}")
        # resize to integer multiple of 32
        w, h = map(lambda x: x - x % 32, (w, h))
        image = image.resize((w, h), resample=PIL.Image.LANCZOS)
        image = np.array(image).astype(np.float32) / 255.0
        image = image[None].transpose(0, 3, 1, 2)
        image = torch.from_numpy(image)
        return 2.*image - 1.

    def sample(self):
        with torch.no_grad(), self.model.ema_scope():
            with devices.autocast(precision=self.precision):
                init_image = self.load_img(self.init_img).to(devices.get_optimal_device())
                init_image = repeat(init_image, '1 ... -> b ...', b=self.batch_size)
                init_latent = self.model.get_first_stage_encoding(
                    self.model.encode_first_stage(init_image))  # move to latent space

                self.sampler.make_schedule(ddim_num_steps=self.steps,
                                    ddim_eta=self.ddim_eta, verbose=False)

                t_enc = int(self.strength * self.steps)
                seed_everything(self.seed)

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

                        conditioning = self.model.get_learned_conditioning(prompts)

                        # encode (scaled latent)
                        z_enc = self.sampler.stochastic_encode(
                            init_latent, torch.tensor([t_enc]*self.batch_size).to(devices.get_optimal_device()))
                        # decode it
                        samples = self.sampler.decode(z_enc, 
                                                 conditioning, 
                                                 t_enc, 
                                                 unconditional_guidance_scale=self.cfg_scale,
                                                 unconditional_conditioning=unconditional_conditioning)

                        x_samples = self.model.decode_first_stage(samples)
                        x_samples = torch.clamp(
                            (x_samples + 1.0) / 2.0, min=0.0, max=1.0)

                        del samples
                        
                        if (self.precision == "low") or (self.precision == "med"):
                            lowvram.send_everything_to_cpu()

                        for x_sample in x_samples:
                            x_sample = 255. * \
                                rearrange(x_sample.cpu().numpy(),
                                            'c h w -> h w c')
                            base_count = len(os.listdir(self.outpath_samples))
                            Image.fromarray(x_sample.astype(np.uint8)).save(
                                os.path.join(self.outpath_samples, f"{base_count:05}.png"))
