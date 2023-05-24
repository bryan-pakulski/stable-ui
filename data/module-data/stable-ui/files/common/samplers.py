from ldm.models.diffusion.ddim import DDIMSampler
from ldm.models.diffusion.plms import PLMSSampler
from ldm.models.diffusion.uni_pc import UniPCSampler

SAMPLERS = {
    "DDIM": DDIMSampler,
    "PLMS": PLMSSampler,
    "UNIPC": UniPCSampler
}


def getSampler(sampler, model):
    if (sampler == ""):
        print("Sampler name not provided, defaulting to DDIM")
        sampler = "DDIM"

    return SAMPLERS[sampler](model)
