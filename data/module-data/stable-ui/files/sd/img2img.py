from sd import sd_base_process as sdb

def generate(outpath_samples: str = "", subfolder_name: str = "", prompt: str = "", negative_prompt: str = "", init_img: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, strength: float = 0.5, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, model: object = None, precision: str = ""):
    gen = sdb.StableDiffusionImg2Img(outpath_samples, subfolder_name, prompt, negative_prompt, init_img,
                                     seed, sampler_name, batch_size, strength, n_iter, steps, cfg_scale, model, precision)
    gen.sample()

    return f"Saved generated images to: {gen.outpath_samples}"
