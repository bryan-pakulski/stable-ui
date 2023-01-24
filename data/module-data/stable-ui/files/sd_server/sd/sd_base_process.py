import os
import sys
import torch
import numpy as np
from tqdm import tqdm, trange
from pytorch_lightning import seed_everything

from ldm.util import instantiate_from_config
from ldm.models.diffusion.ddim import DDIMSampler
from ldm.models.diffusion.plms import PLMSSampler

"""
 Supported Samplers:
 - PLMS
 - DDIM
"""
class StableDiffusionBaseProcess():
    """
    The first set of paramaters: sd_models -> do_not_reload_embeddings represent the minimum required to create a StableDiffusionProcessing
    """
    def __init__(self, outpath_samples=None, prompt: str = "", negative_prompt: str = None, seed: int = -1, sampler_name: str = None, batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512):
        self.outpath_samples: str = outpath_samples
        self.prompt: str = prompt
        self.negative_prompt: str = (negative_prompt or "")
        self.seed: int = seed
        self.sampler_name: str = sampler_name
        self.batch_size: int = batch_size
        self.n_iter: int = n_iter
        self.steps: int = steps
        self.cfg_scale: float = cfg_scale
        self.width: int = width
        self.height: int = height

        self.ddim_eta = 0.0
        self.latent_channels = 4
        self.downsampling_factor = 8

        self.iteration = 0

        os.makedirs(self.outpath_samples, exist_ok=True)

    @property
    def model(self):
        return shared.sd_model

    # Create subfolder for our images
    def create_sub_folder(self, name, default):
        if name == "":
            name = default
        self.outpath_samples = os.path.join(self.outpath_samples, name)
        os.makedirs(self.outpath_samples, exist_ok=True)

class StableDiffusionTxt2Img(StableDiffusionBaseProcess):
    sampler = None

    def __init__(self, canvas_name, outpath_samples, subfolder_name, prompt, negative_prompt, seed, sampler_name, batch_size, n_iter, steps, cfg_scale, width, height):
        super().__init__(**kwargs)
        
        self.create_sub_folder(canvas_name, "txt2img")
        self.data = [self.batch_size * [self.prompt]]

    def sample():
        seed_everything(self.seed)
        with torch.no_grad():
            with precision_scope("cuda"):
                with self.model().ema_scope():
                    samples = list()

                    for n in trange(self.n_iter, desc="Sampling"):
                        for prompts in tqdm(self.data, desc="data"):
                            unconditional_conditioning = None
                            if self.cfg_scale != 1.0 or self.negative_prompt == "":
                                unconditional_conditioning = self.model().get_leared_conditioning(
                                    self.batch_size * [""])
                            else:
                                unconditional_conditioning = self.model().get_learned_conditioning(
                                    len(self.prompt) * [self.negative_prompt])

                            if isinstance(self.prompt, tuple):
                                prompt = list(self.prompt)

                            conditioning = self.model.get_learned_conditioning(prompt)
                            shape = [self.latent_channels, self.height // self.downsampling_factor, self.width // self.downsampling_factor]
                            samples_ddim, _ = sampler.sample(S=self.ddim_steps,
                                    conditioning=conditioning,
                                    batch_size=self.n_samples,
                                    shape=shape,
                                    verbose=False,
                                    unconditional_guidance_scale=self.cfg_scale,
                                    unconditional_conditioning=unconditional_conditioning,
                                    eta=self.ddim_eta,
                                    x_T=None)

                            x_samples_ddim = self.model().decode_first_stage(samples_ddim)
                            x_samples_ddim = torch.clamp(
                            (x_samples_ddim + 1.0) / 2.0, min=0.0, max=1.0)

                        
                            for x_sample in x_samples_ddim:
                                x_sample = 255. * \
                                     rearrange(x_sample.cpu().numpy(),
                                            'c h w -> h w c')
                                Image.fromarray(x_sample.astype(np.uint8)).save(
                                   os.path.join(sample_path, f"{base_count:05}.png"))
                                base_count += 1


        
class StableDiffusionImg2Img(StableDiffusionBaseProcess):
    sampler = None

    def __init__(self):
        super().__init__(**kwargs)

        self.create_sub_folder(canvas_name, "img2img")