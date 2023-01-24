from sd import sd_base_process as sdb

def generate(canvas_name, outpath_samples, subfolder_name, prompt, negative_prompt, seed, sampler_name, batch_size, n_iter, steps, cfg_scale, width, height):
    gen = sdb.StableDiffusionTxt2Img(canvas_name, outpath_samples, subfolder_name, prompt, negative_prompt, seed, sampler_name, batch_size, n_iter, steps, cfg_scale, width, height)
    gen.sample()

    return f"Saved generated images to: {gen.outpath_samples}"