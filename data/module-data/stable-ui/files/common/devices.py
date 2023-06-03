import torch

cpu = torch.device("cpu")
weight_load_location = "cpu"
dtype = torch.float16
dtype_vae = torch.float16

def get_cuda_device_string():
    if (torch.cuda.is_available()):
        print("Using GPU...")
        return "cuda"
    else:
        print("Using CPU...")
        return "cpu"


def get_optimal_device():
    if torch.cuda.is_available():
        return torch.device(get_cuda_device_string())

    return cpu


def torch_gc():
    if torch.cuda.is_available():
        with torch.cuda.device(get_cuda_device_string()):
            torch.cuda.empty_cache()
            torch.cuda.ipc_collect()

def enable_tf32():
    if torch.cuda.is_available():
        # enabling benchmark option seems to enable a range of cards to do fp16 when they otherwise can't
        # see https://github.com/AUTOMATIC1111/stable-diffusion-webui/pull/4407
        if any([torch.cuda.get_device_capability(devid) == (7, 5) for devid in range(0, torch.cuda.device_count())]):
            torch.backends.cudnn.benchmark = True

        torch.backends.cuda.matmul.allow_tf32 = True
        torch.backends.cudnn.allow_tf32 = True


enable_tf32()
