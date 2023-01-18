#!/bin/python3

import glob
import docker
import subprocess
import yaml


def getContainer():
    client = docker.from_env()
    return client.containers.get('sd')

# Runs the sha1sum function against a model file inside the docker container
# The yaml configuration file found under data/shared-config/model_config.yaml contains the hash mapping and custom configuration
def loadConfig(data, key, default):
    return data[key] if (key in data) else default

def getAdditionalConfig(ckpt_filepath):
    cmd = f"bash -c \"sha1sum {ckpt_filepath}" + " | awk '{print $1}'\""

    extra_conf = {
        "config": "",
        "working_dir": "/sd",
        "trigger_prompt": ""
    }

    print("Processing command: ", cmd)
    _e, response = getContainer().exec_run(cmd, workdir="/", demux=True)

    if (_e == 0):
        hash = response[0].decode("utf-8").strip()
        print("Checking model hash: ", hash)

        with open("data/shared-config/model_config.yaml", "r") as stream:
            # Load extra config
            try:
                yaml_dict = yaml.safe_load(stream)
                data = yaml_dict["models"][hash]
                print("Using additional configuration: ", data)

                # Get data from yaml file
                extra_conf["config"] = loadConfig(data, "config", "")
                extra_conf["vae"] = loadConfig(data, "vae", "")
                extra_conf["working_dir"] = loadConfig(
                    data, "working_dir", "/sd")
                extra_conf["trigger_prompt"] = loadConfig(
                    data, "trigger_prompt", "")

                if extra_conf["vae"] != "":
                    extra_conf["vae"] = f"--vae {extra_conf['vae']}"
            except yaml.YAMLError as exc:
                print(exc)
                print("Invalid YAML for hash key: ", hash)

            except KeyError:
                print("Model hash doesn't exist in model_config.yaml", hash)

                # Use default configuration
                data = yaml_dict["models"]["default"]
                extra_conf["working_dir"] = loadConfig(
                    data, "working_dir", "/sd")
    else:
        print("Command failed with error code: ", _e)
        print("==== STDOUT ====")
        print(response[0])
        print("==== STDERR ====")
        print(response[1])
        print("No additional configuration loaded for model: ", ckpt_filepath)

    return extra_conf


# Start SD_Model server
def launchSDModelServer(exec_path):
    command = f"conda run -n ldm python '{exec_path}'"
    print("Starting server with command: ", command)
    _e, response = getContainer().exec_run(
        command, workdir="/modules/stable-ui-scripts/scripts", demux=True)
    if (_e != 0):
        print("Command failed with error code: ", _e)
        print("==== STDOUT ====")
        print(response[0])
        print("==== STDERR ====")
        print(response[1])
    else:
        print("Command success!")

    # Error code response is used by cpp integration to check for success / failure status
    return _e


# Default txt2image command
def txt2image(exec_path, prompt, negative_prompt, samples, steps, scale, seed, width, height, out_dir, ckpt_model, precision):
    # Check if additional configuration is required for loaded ckpt
    extra_conf = getAdditionalConfig(ckpt_model)
    command = f"conda run -n ldm python '{exec_path}' --prompt '{prompt + ' ' + extra_conf['trigger_prompt']}' --negative_prompt '{negative_prompt}' --n_samples {samples} --n_iter 1 --ddim_steps {steps} --scale {scale} --seed {seed} --H {height} --W {width} --outdir {out_dir} --skip_grid --ckpt {ckpt_model} --precision {precision} {extra_conf['vae']} {extra_conf['config']}"

    print("Processing command: ", command)
    _e, response = getContainer().exec_run(
        command, workdir=extra_conf["working_dir"], demux=True)
    if (_e != 0):
        print("Command failed with error code: ", _e)
        print("==== STDOUT ====")
        print(response[0])
        print("==== STDERR ====")
        print(response[1])
    else:
        print("Command success!")

    # Error code response is used by cpp integration to check for success / failure status
    return _e


# Default img2img commnad
def img2image(exec_path, img_path, prompt, negative_prompt, samples, steps, strength, seed, out_dir, ckpt_model, precision):
    # Check if additional configuration is required for loaded ckpt
    extra_conf = getAdditionalConfig(ckpt_model)
    command = f"conda run -n ldm python '{exec_path}' --prompt '{prompt + extra_conf['trigger_prompt']}' --negative_prompt '{negative_prompt}' --init-img {img_path} --strength {strength} --n_samples {samples} --n_iter 1 --ddim_steps {steps} --seed {seed} --outdir {out_dir} --skip_grid --ckpt {ckpt_model} --precision {precision} {extra_conf['vae']} {extra_conf['config']}"

    print("Processing command: ", command)
    _e, response = getContainer().exec_run(
        command, workdir=extra_conf["working_dir"], demux=True)
    if (_e != 0):
        print("Command failed with error code: ", _e)
        print("==== STDOUT ====")
        print(response[0])
        print("==== STDERR ====")
        print(response[1])
    else:
        print("Command success!")

    # Error code response is used by cpp integration to check for success / failure status
    return _e


def textualInversion(exec_path, base_config, model, run_name, dataset_path):
    command = f"conda run -n ldm python '{exec_path}' --base '{base_config}' -t --actual_resume '{model}' -n {run_name} --gpus 0, --data_root {dataset_path}"

    print("Processign command: ", command)
    _e, response = getContainer().exec_run(
        command, workdir="/sd/textual-inversion", demux=True)
    if (_e != 0):
        print("Command failed with error code: ", _e)
        print("==== STDOUT ====")
        print(response[0])
        print("==== STDERR ====")
        print(response[1])
    else:
        print("Command success!")

    # Error code response is used by cpp integration to check for success / failure status
    return _e
