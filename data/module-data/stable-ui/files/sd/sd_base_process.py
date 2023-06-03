import os
import logging

from PIL import Image
from common import samplers
from common import metadata

from diffusers import StableDiffusionPipeline

class StableDiffusionBaseProcess():
    """
    The first set of paramaters: sd_models -> do_not_reload_embeddings represent the minimum required to create a StableDiffusionProcessing
    """

    def __init__(self, outpath_samples: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = StableDiffusionPipeline):
        self.outpath_samples = outpath_samples
        self.prompt = prompt
        self.negative_prompt = negative_prompt
        self.seed = seed
        self.model = model
        # TODO: fix sampler selection
        #self.sampler = samplers.getSampler(sampler_name, self.model)
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

    def __init__(self, outpath_samples: str = "", subfolder_name: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = None):
        super().__init__(outpath_samples, prompt, negative_prompt, seed,
                         sampler_name, batch_size, n_iter, steps, cfg_scale, width, height, model)

        self.create_sub_folder(subfolder_name, "txt2img")
        self.data = [self.batch_size * [self.prompt]]

    def save_metadata(self, img):
        md = metadata.StableMetaData(img)

        data = {
            "prompt": self.prompt,
            "negative_prompt": self.negative_prompt,
            "model_hash": "",
            "seed": self.seed,
            "sampler": "",
            "steps": self.steps,
            "cfg": self.cfg_scale,
            "width": self.width,
            "height": self.height
        }

        md.addMetaData(data)
        md.save()

    def cpu_sample(self):
        outputs = self.model(prompt=self.prompt, negative_prompt=self.prompt, width=self.width, height=self.height, num_inference_steps=self.steps, guidance_scale=self.cfg_scale, num_images_per_prompt=self.n_iter)
        for image in outputs.images:
            base_count = len(os.listdir(self.outpath_samples))
            img_name = os.path.join(self.outpath_samples, f"{base_count:05}.png")
            image.save(img_name)
            self.save_metadata(img_name)
    
    def sample(self):
        outputs = self.model(prompt=self.prompt, negative_prompt=self.prompt, width=self.width, height=self.height, num_inference_steps=self.steps, guidance_scale=self.cfg_scale, num_images_per_prompt=self.n_iter)
        for image in outputs.images:
            base_count = len(os.listdir(self.outpath_samples))
            img_name = os.path.join(self.outpath_samples, f"{base_count:05}.png")
            image.save(img_name)
            self.save_metadata(img_name)


class StableDiffusionImg2Img(StableDiffusionBaseProcess):

    def __init__(self, outpath_samples, subfolder_name, prompt, negative_prompt, init_img, seed, sampler_name, batch_size, strength, n_iter, steps, cfg_scale, model):
        super().__init__(outpath_samples=outpath_samples, prompt=prompt, negative_prompt=negative_prompt, seed=seed,
                         sampler_name=sampler_name, batch_size=batch_size, n_iter=n_iter, steps=steps, cfg_scale=cfg_scale, model=model)

        self.create_sub_folder(subfolder_name, "img2img")
        self.data = [self.batch_size * [self.prompt]]

        self.init_img = init_img
        self.strength = strength

    def save_metadata(self, img):
        md = metadata.StableMetaData(img)

        data = {
            "prompt": self.prompt,
            "negative_prompt": self.negative_prompt,
            "model_hash": "",
            "init_img": self.init_img,
            "seed": self.seed,
            "sampler": "",
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

    def sample(self):
        pass