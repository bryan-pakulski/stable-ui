from diffusers import DDIMScheduler
from diffusers import EulerDiscreteScheduler
from diffusers import DPMSolverMultistepScheduler

from ldm.models.diffusion.plms import PLMSSampler
from ldm.models.diffusion.uni_pc import UniPCSampler

SAMPLERS = {
    "DDIM": DDIMScheduler,
    "DPMS": DPMSolverMultistepScheduler,
    "EULER": EulerDiscreteScheduler,
    "PLMS": PLMSSampler,
    "UNIPC": UniPCSampler
}


def getSampler(sampler, config):
    if (sampler == ""):
        print("Sampler name not provided, defaulting to DPMS")
        sampler = "DPMS"

    return SAMPLERS[sampler].from_pretrained(config,local_files_only=True)
