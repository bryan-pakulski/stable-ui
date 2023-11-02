from sd import sd_base_process as sdb

def generate(outpath_samples: str = "", subfolder_name: str = "", prompt: str = "", negative_prompt: str = "", img_width: int = 0, img_height: int = 0, img_data: str = "", img_mask: str = "", seed: int = -1, sampler_name: str = "", batch_size: int = 1, strength: float = 0.5, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, model: object = None):
    gen = sdb.StableDiffusionOutpainting(outpath_samples, subfolder_name, prompt, negative_prompt, img_width, img_height, img_data, img_mask,
                                     seed, sampler_name, batch_size, strength, n_iter, steps, cfg_scale, model)
    
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
