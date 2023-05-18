from ldm.models.diffusion.ddim import DDIMSampler
from ldm.models.diffusion.plms import PLMSSampler

SAMPLERS = {
    "DDIM": DDIMSampler,
    "PLMS": PLMSSampler
}


def getSampler(sampler, model):
    if (sampler == ""):
        print("Sampler name not provided, defaulting to DDIM")
        sampler = "DDIM"

    return SAMPLERS[sampler](model)
