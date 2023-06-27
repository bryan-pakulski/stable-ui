import gc
import os
import logging
import torch

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
from compel import Compel

# TODO: implement weighted prompts see:
# https://huggingface.co/docs/diffusers/using-diffusers/weighted_prompts

class StableDiffusionBaseProcess():
    """
    The first set of paramaters: sd_models -> do_not_reload_embeddings represent the minimum required to create a StableDiffusionProcessing
    """

    def __init__(self, outpath_samples: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = DiffusionPipeline):
        self.outpath_samples = outpath_samples
        self.prompt = prompt
        self.prompt_embeds = None
        self.negative_prompt = negative_prompt
        self.seed = seed
        self.model = model
        self.sampler, self.sampler_name = schedulers.getSheduler(sampler_name, self.model.scheduler)
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

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")
        logging.addLevelName(logging.DEBUG, "DBG")

        logging.info("Starting up sd_server")

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

        self.pipe = StableDiffusionPipeline(**self.model.components, requires_safety_checker=False)
        self.pipe.scheduler = self.sampler
        self.get_prompt_embeds(self.pipe)
        self.create_sub_folder(subfolder_name, "txt2img")

    def save_metadata(self, img):
        md = metadata.StableMetaData(img)

        data = {
            "prompt": self.prompt,
            "negative_prompt": self.negative_prompt,
            "model_hash": self.model.model_hash,
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
        pass
    
    def sample(self):
        self.model.scheduler = self.sampler
        outputs = self.model(prompt_embeds=self.prompt_embeds, negative_prompt=self.prompt, width=self.width, height=self.height, generator=self.generator, num_inference_steps=self.steps, guidance_scale=self.cfg_scale, num_images_per_prompt=self.n_iter)
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

        self.pipe = StableDiffusionImg2ImgPipeline(**self.model.components, requires_safety_checker=False)
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
            "model_hash": self.model.model_hash,
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
      outputs = self.pipe(prompt_embeds=self.prompt_embeds, negative_prompt=self.prompt, image=self.load_img(self.init_img), generator=self.generator, num_inference_steps=self.steps, strength=self.strength, guidance_scale = self.cfg_scale, num_images_per_prompt=self.n_iter)
      for image in outputs.images:
          base_count = len(os.listdir(self.outpath_samples))
          img_name = os.path.join(self.outpath_samples, f"{base_count:05}.png")
          image.save(img_name)
          self.save_metadata(img_name)
      self.cleanup()

class StableDiffusionOutpainting(StableDiffusionBaseProcess):

    def __init__(self, outpath_samples, subfolder_name, prompt, negative_prompt, img_data, img_mask, seed, sampler_name, batch_size, strength, n_iter, steps, cfg_scale, model):
        super().__init__(outpath_samples=outpath_samples, prompt=prompt, negative_prompt=negative_prompt, seed=seed,
                         sampler_name=sampler_name, batch_size=batch_size, n_iter=n_iter, steps=steps, cfg_scale=cfg_scale, model=model)

        self.pipe = StableDiffusionInpaintPipeline(**self.model.components, requires_safety_checker=False)
        self.pipe.scheduler = self.sampler
        self.get_prompt_embeds(self.pipe)
        self.create_sub_folder(subfolder_name, "outpaint")

        # Process base64 img data into PIL image for sd pipeline
        self.image = self.convert_img(img_data)
        self.mask = self.convert_img(img_mask)
        
        self.strength = strength

    def save_metadata(self, img):
        md = metadata.StableMetaData(img)

        data = {
            "prompt": self.prompt,
            "negative_prompt": self.negative_prompt,
            "model_hash": self.model.model_hash,
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
        image = Image.fromarray(np.uint8([ord(c) for c in data.decode('base64')])).convert('RGBA')
        w, h = image.size
        print(f"loaded input image of size ({w}, {h}) from base64 data")
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
      outputs = self.pipe(prompt_embeds=self.prompt_embeds, negative_prompt=self.prompt, image=self.image, mask_image=self.mask, generator=self.generator, num_inference_steps=self.steps, strength=self.strength, guidance_scale = self.cfg_scale, num_images_per_prompt=self.n_iter)
      for image in outputs.images:
          base_count = len(os.listdir(self.outpath_samples))
          img_name = os.path.join(self.outpath_samples, f"{base_count:05}.png")
          image.save(img_name)
          self.save_metadata(img_name)
      self.cleanup()