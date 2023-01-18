from ldm.util import instantiate_from_config
import torch
import safetensors.torch
import sys
import os

from . import devices

# Courtesy of Automatic111; https://github.com/AUTOMATIC1111/stable-diffusion-webui/blob/master/modules/sd_models.py

MODELS_PATH = "/models/"

vae_ignore_keys = {"model_ema.decay", "model_ema.num_updates"}

def load_vae(model, vae_file=None, vae_source="from unknown source"):

    if vae_file:
        assert os.path.isfile(vae_file), f"VAE {vae_source} doesn't exist: {vae_file}"
        print(f"Loading VAE weights {vae_source}: {vae_file}")

        vae_ckpt = read_state_dict(vae_file, map_location=devices.weight_load_location)
        vae_dict = {k: v for k, v in vae_ckpt.items() if k[0:4] != "loss" and k not in vae_ignore_keys}
        _load_vae_dict(model, vae_dict)


# don't call this from outside
def _load_vae_dict(model, vae_dict):
    model.first_stage_model.load_state_dict(vae_dict)
    model.first_stage_model.to(devices.dtype_vae)

def transform_checkpoint_dict_key(k):
    chckpoint_dict_replacements = {
        'cond_stage_model.transformer.embeddings.': 'cond_stage_model.transformer.text_model.embeddings.',
        'cond_stage_model.transformer.encoder.': 'cond_stage_model.transformer.text_model.encoder.',
        'cond_stage_model.transformer.final_layer_norm.': 'cond_stage_model.transformer.text_model.final_layer_norm.',
    }

    for text, replacement in chckpoint_dict_replacements.items():
        if k.startswith(text):
            k = replacement + k[len(text):]

    return k


def get_state_dict_from_checkpoint(pl_sd):
    pl_sd = pl_sd.pop("state_dict", pl_sd)
    pl_sd.pop("state_dict", None)

    sd = {}
    for k, v in pl_sd.items():
        new_key = transform_checkpoint_dict_key(k)

        if new_key is not None:
            sd[new_key] = v

    pl_sd.clear()
    pl_sd.update(sd)

    return pl_sd


def read_state_dict(checkpoint_file, print_global_state=False, map_location=None):
    _, extension = os.path.splitext(checkpoint_file)
    if extension.lower() == ".safetensors":
        device = map_location
        if device is None:
            device = devices.get_cuda_device_string() if torch.cuda.is_available() else "cpu"
        pl_sd = safetensors.torch.load_file(checkpoint_file, device=device)
    else:
        pl_sd = torch.load(checkpoint_file, map_location=map_location)

    if print_global_state and "global_step" in pl_sd:
        print(f"Global Step: {pl_sd['global_step']}")

    sd = get_state_dict_from_checkpoint(pl_sd)
    return sd

def load_model_weights(model, ckpt, precision, vae_file=None):
    # load from file
    print(f"Loading weights from {ckpt}")

    sd = read_state_dict(ckpt)
    model.load_state_dict(sd, strict=False)
    del sd
    
    if precision == "full":
        vae = model.first_stage_model

        model.half()
        model.first_stage_model = vae

    dtype = torch.float32 if precision == "full" else torch.float16
    dtype_vae = torch.float32 if precision == "full" else torch.float16

    model.first_stage_model.to(dtype_vae)

    if vae_file != None:
        print(f"Loading additional VAE: {vae_file}")
        load_vae(model, vae_file)


def load_model(checkpoint_config, ckpt, precision, vae_file=None):
    from common import lowvram
    model = instantiate_from_config(checkpoint_config.model)

    if model is None:
        print('Failed to create model', file=sys.stderr)
        exit(1)

    load_model_weights(model, ckpt, precision, vae_file)

    #if shared.cmd_opts.lowvram or shared.cmd_opts.medvram:
    #    lowvram.setup_for_low_vram(model, shared.cmd_opts.medvram)
    #else:
    #    model.to(shared.device)

    model.to(devices.get_cuda_device_string() if torch.cuda.is_available() else "cpu")
    model.eval()

    print(f"Model loaded..")

    return model