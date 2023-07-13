from diffusers import (DDIMScheduler, 
  DDPMScheduler, 
  PNDMScheduler, 
  LMSDiscreteScheduler, 
  EulerDiscreteScheduler, 
  HeunDiscreteScheduler, 
  EulerAncestralDiscreteScheduler, 
  DPMSolverMultistepScheduler, 
  DPMSolverSinglestepScheduler, 
  KDPM2DiscreteScheduler, 
  KDPM2AncestralDiscreteScheduler, 
  DEISMultistepScheduler, 
  UniPCMultistepScheduler,
  DDIMInverseScheduler
)


SCHEDULERS = {
    "ddim": DDIMScheduler,
    "ddiminverse": DDIMInverseScheduler,
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

def isCompatibleScheduler(current, scheduler):
    return SCHEDULERS[scheduler] in current.compatibles

# Return first compatible scheduler with current one
def getCompatibleSampler(current):
    for scheduler in SCHEDULERS.keys():
        if isCompatibleScheduler(current, scheduler):
            print(f"Using first found compatible scheduler: {scheduler}")
            return scheduler
    
    print(f"No compatible Scheduler found, no change made!")
    return ""

def getSheduler(sampler_name, current):
    if (sampler_name == "") or sampler_name not in SCHEDULERS.keys():
        print(f"Scheduler name invalid, got: '{sampler_name}'")
        sampler_name = getCompatibleSampler(current)
    elif not isCompatibleScheduler(current, sampler_name):
        print(f"Scheduler not compatible, {sampler_name}")
        sampler_name = getCompatibleSampler(current)

    if sampler_name == "":
        print("No modification made to scheduler! using existing...")
        return (current, "")

    return (SCHEDULERS[sampler_name].from_config(current.config, local_files_only=True), sampler_name)
