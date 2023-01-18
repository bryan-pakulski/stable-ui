class StableDiffusionBaseProcess():
    """
    The first set of paramaters: sd_models -> do_not_reload_embeddings represent the minimum required to create a StableDiffusionProcessing
    """
    def __init__(self, sd_model=None, outpath_samples=None, prompt: str = "", seed: int = -1, sampler_name: str = None, batch_size: int = 1, n_iter: int = 1, steps: int = 50, cfg_scale: float = 7.0, width: int = 512, height: int = 512, negative_prompt: str = None, script_args: list = None):

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

        self.scripts = None
        self.script_args = script_args
        self.all_prompts = None
        self.all_negative_prompts = None
        self.all_seeds = None
        self.iteration = 0

    @property
    def sd_model(self):
        return shared.sd_model