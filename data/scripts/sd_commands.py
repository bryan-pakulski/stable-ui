#!/bin/python3.8

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

                if extra_conf["config"] != "":
                    extra_conf["config"] = f"--config {extra_conf['config']}"
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
    command = f"conda run -n ldm python '{exec_path}' > /logs/sd_server.out 2>&1"
    print("Starting server with command: ", command)
    getContainer().exec_run(
        command, workdir="/modules/stable-ui/sd_server", demux=True, detach=True)
    return 0

# Terminate SD_Model server on shutdown
def terminateSDModelServer(exec_path):
    command = f"conda run -n ldm python '{exec_path}' quit"
    _e, response = getContainer().exec_run(
        command, workdir="/modules/stable-ui/sd_client", demux=True)

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

# Ping pong heartbeat
def heartbeat(exec_path):
    command = f"conda run -n ldm python '{exec_path}' ping"
    _e, response = getContainer().exec_run(
        command, workdir="/modules/stable-ui/sd_client", demux=True)

    # Error code response is used by cpp integration to check for success / failure status
    return _e

# Launch a client command
def attachSDModelToServer(exec_path, ckpt_path, config_path, vae_path: str = "", precision: str = ""):
    msg = f"loadModel:checkpoint_path={ckpt_path}:checkpoint_config_path={config_path}"

    # Optionals
    msg += f":vae_path={vae_path}" if vae_path != "" else ""
    msg += f":precision={precision}" if precision != "" else ""

    command = f"conda run -n ldm python '{exec_path}' {msg}"

    print("Starting client with command: ", command)
    _e, response = getContainer().exec_run(
        command, workdir="/modules/stable-ui/sd_client", demux=True)
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


def txt2img(exec_path, sd_model_path, canvas_name, prompt, negative_prompt, sampler_name, batch_size, steps, scale, seed, width, height, out_dir, n_iter):
    # Check if additional configuration is required for loaded ckpt
    extra_conf = getAdditionalConfig(sd_model_path)
    if (extra_conf['trigger_prompt'] != ""):
        prompt += f", {extra_conf['trigger_prompt']}"
    msg = f"txt2img:prompt={prompt}:negative_prompt={negative_prompt}:subfolder_name={canvas_name}:sampler_name={sampler_name}:batch_size={batch_size}:n_iter={n_iter}:steps={steps}:cfg_scale={scale}:seed={seed}:height={height}:width={width}:outpath_samples={out_dir}"
    command = f"conda run -n ldm python '{exec_path}' '{msg}'"

    print("Processing command: ", command)
    _e, response = getContainer().exec_run(
        command, workdir="/modules/stable-ui/sd_client", demux=True)
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


# Default img2img commAND
def img2img(exec_path, sd_model_path, canvas_name, img_path, prompt, negative_prompt, sampler_name, batch_size, steps, scale, strength, seed, out_dir, n_iter):
    extra_conf = getAdditionalConfig(sd_model_path)
    if (extra_conf['trigger_prompt'] != ""):
        prompt += f", {extra_conf['trigger_prompt']}"
    msg = f"img2img:prompt={prompt}:negative_prompt={negative_prompt}:init_img={img_path}:subfolder_name={canvas_name}:sampler_name={sampler_name}:batch_size={batch_size}:strength={strength}:cfg_scale={scale}:n_iter={n_iter}:steps={steps}:seed={seed}:outpath_samples={out_dir}"
    command = f"conda run -n ldm python '{exec_path}' '{msg}'"

    print("Processing command: ", command)
    _e, response = getContainer().exec_run(
        command, workdir="/modules/stable-ui/sd_client", demux=True)
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
