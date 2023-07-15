import gc
import os
import logging
import random
import torch
import base64

import PIL
from PIL import Image
import numpy as np

from common import schedulers
from common import metadata
from common import devices

from diffusers import DiffusionPipeline
from diffusers import StableDiffusionPipeline
from diffusers import StableDiffusionImg2ImgPipeline
from diffusers import StableDiffusionInpaintPipeline
from diffusers import StableDiffusionXLPipeline
from compel import Compel

#from optimum.intel.openvino import OVStableDiffusionPipeline

# TODO: implement weighted prompts see:
# https://huggingface.co/docs/diffusers/using-diffusers/weighted_prompts

class StableDiffusionBaseProcess():
    """
    The first set of paramaters: sd_models -> do_not_reload_embeddings represent the minimum required to create a StableDiffusionProcessing
    """

    def __init__(self, outpath_samples: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = None):
        self.outpath_samples = outpath_samples
        self.prompt = prompt
        self.prompt_embeds = None
        self.negative_prompt = negative_prompt
        self.seed = seed
        self.sd_model = model
        self.pipeline = self.sd_model.model
        self.sampler, self.sampler_name = schedulers.getSheduler(sampler_name, self.pipeline.scheduler)
        self.batch_size = batch_size
        self.n_iter = n_iter
        self.steps = steps
        self.cfg_scale = cfg_scale
        self.width = width
        self.height = height

        self.generator = torch.Generator(device=devices.get_cuda_device_string()).manual_seed(seed)

        # Initialise logging
        logging.basicConfig(
            filename="/logs/sd_server.log",
            level=logging.INFO,
            format="[SDSERVER] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "info")
        logging.addLevelName(logging.WARNING, "warn")
        logging.addLevelName(logging.ERROR, "err")
        logging.addLevelName(logging.DEBUG, "dbg")

        logging.info("Starting up Stable Diffusion Process")

    # Create subfolder for our images
    def create_sub_folder(self, name, default):
        if (self.outpath_samples != ""):
            logging.debug(f"Creating folder: {self.outpath_samples}")
            os.makedirs(self.outpath_samples, exist_ok=True)
        if name == "":
            name = default
        logging.debug(f"Creating subfolder: {name}")
        self.outpath_samples = os.path.join(self.outpath_samples, name)
        os.makedirs(self.outpath_samples, exist_ok=True)

    # Get prompt embeds
    def get_prompt_embeds(self, pipe):
      compel = Compel(tokenizer=pipe.tokenizer, text_encoder=pipe.text_encoder)
      self.prompt_embeds = compel.build_conditioning_tensor(self.prompt)

    # Cleanup
    def cleanup(self):
        self.prompt_embeds = None
        gc.collect()

class StableDiffusionTxt2Img(StableDiffusionBaseProcess):

    def __init__(self, outpath_samples: str = "", subfolder_name: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = None):
        super().__init__(outpath_samples, prompt, negative_prompt, seed,
                         sampler_name, batch_size, n_iter, steps, cfg_scale, width, height, model)
        
        # SDXL pipeline
        if self.sd_model.model_type == "sdxl":
            self.pipe = StableDiffusionXLPipeline(**self.pipeline.components)
        # Normal latent diffusion
        else:
            #TODO: openvino pipeline for cpu
            """
            if (devices.get_cuda_device_string() == "cpu"):
                self.pipe = OVStableDiffusionPipeline(**self.pipeline.components, export=False)
            else:
                self.pipe = StableDiffusionPipeline(**self.pipeline.components, requires_safety_checker=False)
            """

            self.pipe = StableDiffusionPipeline(**self.pipeline.components, requires_safety_checker=False)

        self.pipe.scheduler = self.sampler
        self.get_prompt_embeds(self.pipe)
        self.create_sub_folder(subfolder_name, "txt2img")

    def save_metadata(self, img):
        md = metadata.StableMetaData(img)

        data = {
            "prompt": self.prompt,
            "negative_prompt": self.negative_prompt,
            "model_hash": self.sd_model.model_hash,
            "seed": self.seed,
            "sampler": self.sampler_name,
            "steps": self.steps,
            "cfg": self.cfg_scale,
            "width": self.width,
            "height": self.height
        }

        md.addMetaData(data)
        md.save()

    def cpu_sample(self):
        #TODO: CPU specific changes here???
        self.sample()

    def sdxl_sample(self):
        self.pipeline.scheduler = self.sampler
        
        
        prompt_embeds, negative_prompt_embeds, pooled_prompt_embeds, negative_pooled_prompt_embeds = self.pipeline.encode_prompt(prompt=self.prompt, device=devices.get_cuda_device_string(), negative_prompt=self.negative_prompt)

        outputs = self.pipeline(prompt_embeds=prompt_embeds, pooled_prompt_embeds=pooled_prompt_embeds, negative_prompt_embeds=negative_prompt_embeds, negative_pooled_prompt_embeds=negative_pooled_prompt_embeds, width=self.width, height=self.height, generator=self.generator, num_inference_steps=self.steps, guidance_scale=self.cfg_scale, num_images_per_prompt=self.n_iter)

        for image in outputs.images:
            base_count = len(os.listdir(self.outpath_samples))
            img_name = os.path.join(self.outpath_samples, f"{base_count:05}.png")
            image.save(img_name)
            self.save_metadata(img_name)
        self.cleanup()

    
    def sample(self):
        self.pipeline.scheduler = self.sampler

        outputs = self.pipeline(prompt_embeds=self.prompt_embeds, negative_prompt=self.negative_prompt, width=self.width, height=self.height, generator=self.generator, num_inference_steps=self.steps, guidance_scale=self.cfg_scale, num_images_per_prompt=self.n_iter)
        for image in outputs.images:
            base_count = len(os.listdir(self.outpath_samples))
            img_name = os.path.join(self.outpath_samples, f"{base_count:05}.png")
            image.save(img_name)
            self.save_metadata(img_name)
        self.cleanup()


class StableDiffusionImg2Img(StableDiffusionBaseProcess):

    def __init__(self, outpath_samples, subfolder_name, prompt, negative_prompt, init_img, seed, sampler_name, batch_size, strength, n_iter, steps, cfg_scale, model):
        super().__init__(outpath_samples=outpath_samples, prompt=prompt, negative_prompt=negative_prompt, seed=seed,
                         sampler_name=sampler_name, batch_size=batch_size, n_iter=n_iter, steps=steps, cfg_scale=cfg_scale, model=model)

        self.pipe = StableDiffusionImg2ImgPipeline(**self.pipeline.components, requires_safety_checker=False)
        self.pipe.scheduler = self.sampler
        self.get_prompt_embeds(self.pipe)
        self.create_sub_folder(subfolder_name, "img2img")

        self.init_img = init_img
        self.strength = strength

    def save_metadata(self, img):
        md = metadata.StableMetaData(img)

        data = {
            "prompt": self.prompt,
            "negative_prompt": self.negative_prompt,
            "model_hash": self.sd_model.model_hash,
            "init_img": self.init_img,
            "seed": self.seed,
            "sampler": self.sampler_name,
            "steps": self.steps,
            "cfg": self.cfg_scale,
            "strength": self.strength,
            "width": self.width,
            "height": self.height
        }

        md.addMetaData(data)
        md.save()

    def load_img(self, path):
        return Image.open(path).convert("RGB")
    
    def legacy_load_img(self, path):
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
      
    def cpu_sample(self):
        pass

    def sample(self):
        outputs = self.pipe(prompt_embeds=self.prompt_embeds, negative_prompt=self.negative_prompt, image=self.legacy_load_img(self.init_img), generator=self.generator, num_inference_steps=self.steps, strength=self.strength, guidance_scale = self.cfg_scale, num_images_per_prompt=self.n_iter)
        for image in outputs.images:
            base_count = len(os.listdir(self.outpath_samples))
            img_name = os.path.join(self.outpath_samples, f"{base_count:05}.png")
            image.save(img_name)
            self.save_metadata(img_name)
        self.cleanup()

class StableDiffusionOutpainting(StableDiffusionBaseProcess):

    def __init__(self, outpath_samples, subfolder_name, prompt, negative_prompt, img_width, img_height, img_data, img_mask, seed, sampler_name, batch_size, strength, n_iter, steps, cfg_scale, model):
        super().__init__(outpath_samples=outpath_samples, prompt=prompt, negative_prompt=negative_prompt, seed=seed,
                         sampler_name=sampler_name, batch_size=batch_size, n_iter=n_iter, steps=steps, cfg_scale=cfg_scale, model=model)

        self.pipe = StableDiffusionInpaintPipeline(**self.pipeline.components, requires_safety_checker=False)
        self.pipe.scheduler = self.sampler
        self.get_prompt_embeds(self.pipe)
        self.create_sub_folder(subfolder_name, "outpaint")

        # Process base64 img data into PIL image for sd pipeline
        self.width = img_width
        self.height = img_height
        self.image = self.convert_img(img_data)
        self.mask = self.convert_img(img_mask)
        # Extract mask from alpha channel, where index 3 == 0
        #Image.fromarray(np.array(self.image)[:, :, 3] == 0)
        
        self.strength = strength

    def save_metadata(self, img):
        md = metadata.StableMetaData(img)

        data = {
            "prompt": self.prompt,
            "negative_prompt": self.negative_prompt,
            "model_hash": self.sd_model.model_hash,
            "seed": self.seed,
            "sampler": self.sampler_name,
            "steps": self.steps,
            "cfg": self.cfg_scale,
            "strength": self.strength,
            "width": self.width,
            "height": self.height
        }

        md.addMetaData(data)
        md.save()

    def convert_img(self, data):
        pixels = 4 # RGBA
        w = self.width
        h = self.height

        message_bytes = base64.b64decode(data)
        arr = np.uint8([c for c in message_bytes])
        arr = arr.reshape(h, w, pixels)

        return Image.fromarray(arr).convert('RGB')

    def cpu_sample(self):
        pass
      
    # Inpainting pipeline
    def sample(self):
        outputs = self.pipe(prompt_embeds=self.prompt_embeds, negative_prompt=self.negative_prompt, image=self.image, width=self.width, height=self.height, mask_image=self.mask, generator=self.generator, num_inference_steps=self.steps, strength=self.strength, guidance_scale = self.cfg_scale, num_images_per_prompt=self.n_iter)
        for image in outputs.images:
            base_count = len(os.listdir(self.outpath_samples))
            img_name = os.path.join(self.outpath_samples, f"{base_count:05}.png")
            image.save(img_name)
            self.save_metadata(img_name)
        self.cleanup()

    # Img2Img pipeline
    # TODO: https://huggingface.co/spaces/Rothfeld/stable-diffusion-mat-outpainting-primer/blob/main/outpainting_example2.py