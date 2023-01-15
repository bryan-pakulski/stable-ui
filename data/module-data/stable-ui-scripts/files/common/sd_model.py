from ldm.util import instantiate_from_config
import torch
import safetensors.torch
import sys
import os

from . import devices

# Courtesy of Automatic111; https://github.com/AUTOMATIC1111/stable-diffusion-webui/blob/master/modules/sd_models.py

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

def load_model_weights(model, ckpt, no_half):
    # load from file
    print(f"Loading weights from {ckpt}")

    sd = read_state_dict(ckpt)
    model.load_state_dict(sd, strict=False)
    del sd
    
    if not no_half:
        vae = model.first_stage_model

        model.half()
        model.first_stage_model = vae

    dtype = torch.float32 if no_half else torch.float16
    dtype_vae = torch.float32 if no_half else torch.float16

    model.first_stage_model.to(dtype_vae)


def load_model(checkpoint_config, ckpt, precision):
    from common import lowvram
    sd_model = instantiate_from_config(checkpoint_config.model)

    if sd_model is None:
        print('Failed to create model', file=sys.stderr)
        exit(1)

    if precision == "autocast":
        load_model_weights(sd_model, ckpt, False)
    else:
        load_model_weights(sd_model, ckpt, True)

    #if shared.cmd_opts.lowvram or shared.cmd_opts.medvram:
    #    lowvram.setup_for_low_vram(sd_model, shared.cmd_opts.medvram)
    #else:
    #    sd_model.to(shared.device)

    sd_model.to(devices.get_cuda_device_string() if torch.cuda.is_available() else "cpu")
    sd_model.eval()

    print(f"Model loaded..")

    return sd_model