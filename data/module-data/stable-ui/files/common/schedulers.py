from diffusers import DDIMScheduler
from diffusers import DDPMScheduler
from diffusers import PNDMScheduler
from diffusers import LMSDiscreteScheduler
from diffusers import EulerDiscreteScheduler
from diffusers import HeunDiscreteScheduler
from diffusers import EulerAncestralDiscreteScheduler
from diffusers import DPMSolverMultistepScheduler
from diffusers import DPMSolverSinglestepScheduler
from diffusers import KDPM2DiscreteScheduler
from diffusers import KDPM2AncestralDiscreteScheduler
from diffusers import DEISMultistepScheduler
from diffusers import UniPCMultistepScheduler


SCHEDULERS = {
    "ddim": DDIMScheduler,
    "ddpm": DDPMScheduler,
    "deis": DEISMultistepScheduler,
    "dpmsmulti": DPMSolverMultistepScheduler,
    "dpmssingle": DPMSolverSinglestepScheduler,
    "eulerancestral": EulerAncestralDiscreteScheduler,
    "euler": EulerDiscreteScheduler,
    "heun": HeunDiscreteScheduler,
    "kdpm2": KDPM2DiscreteScheduler,
    "kdpm2ancestral": KDPM2AncestralDiscreteScheduler,
    "lms": LMSDiscreteScheduler,
    "pndm": PNDMScheduler,
    "unipc": UniPCMultistepScheduler
}

DDIMScheduler.from_config

def isCompatibleScheduler(current, scheduler):
    if SCHEDULERS[scheduler] in current.compatibles:
        return True
    else:
        return False

def getCompatibleSampler(current):
    for item in current.compatibles:
        for k,v in SCHEDULERS:
            if v == item:
                return k;
    
    print(f"No compatible Scheduler found!")
    return ""

def getSheduler(sampler_name, current):
    if (sampler_name == "") or sampler_name not in SCHEDULERS:
        print(f"Scheduler name not provided, {sampler_name}")
        sampler_name = getCompatibleSampler(current)

    if not isCompatibleScheduler(current, sampler_name):
        print(f"Scheduler not compatible, {sampler_name}")
        sampler_name = getCompatibleSampler(current)

    if sampler_name == "":
        print("No modification made to scheduler!")
        return current

    return SCHEDULERS[sampler_name].from_config(current.config, local_files_only=True)
