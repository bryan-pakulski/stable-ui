from sd import sd_base_process as sdb
from common import devices

def generate(outpath_samples: str = "", subfolder_name: str = "", prompt: str = "", negative_prompt: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, model: object = None):
    gen = sdb.StableDiffusionTxt2Img(outpath_samples, subfolder_name, prompt, negative_prompt,
                                     seed, sampler_name, batch_size, n_iter, steps, cfg_scale, width, height, model)
    
    # SD / SDXL pipelines
    if (model.model_type == "sdxl"):
        gen.sdxl_sample()
    else:
        # CPU specific inference & optimisations
        if (devices.get_cuda_device_string() == "cpu"):
            gen.cpu_sample()
        else:
            gen.sample()

    return f"Saved generated images to: {gen.outpath_samples}"
