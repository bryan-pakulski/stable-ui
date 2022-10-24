#!/bin/python3

import glob
import docker

def getContainer():
	client = docker.from_env()
	return client.containers.get('sd')

# Example command
# python optimizedSD/txt2img.py --prompt 'Cat on a roof' --n_samples 10 --n_iter 2 --ddim_steps 80 --seed 1234 --H 512 --W 512 --turbo"
def txt2image(exec_path, prompt, negative_prompt, samples, steps, scale, seed, width, height, out_dir, ckpt_model):
	if (ckpt_model != ""):
		command = f"conda run -n ldm python '{exec_path}' --prompt '{prompt}' --negative_prompt '{negative_prompt}' --n_samples {samples} --n_iter 1 --ddim_steps {steps} --scale {scale} --seed {seed} --H {height} --W {width} --outdir {out_dir} --skip_grid --ckpt {ckpt_model}"
	else:
		command = f"conda run -n ldm python '{exec_path}' --prompt '{prompt}' --negative_prompt '{negative_prompt}' --n_samples {samples} --n_iter 1 --ddim_steps {steps} --scale {scale} --seed {seed} --H {height} --W {width} --outdir {out_dir} --skip_grid"
	
	print("Processing command: ", command)
	_e, response = getContainer().exec_run(command, workdir="/sd", demux=True)
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

def img2img(exec_path, img_path, strength, prompt, samples, steps, seed, width, height, out_dir, ckpt_model):
	if (ckpt_model != ""):
		command = f"conda run -n ldm python '{exec_path}' --prompt '{prompt}' --init-img {img_path} --strength {strength} --n_samples {samples} --n_iter 1 --ddim_steps {steps} --seed {seed} --H {height} --W {width} -outdir {out_dir} --skip_grid --ckpt {ckpt_model}"
	else:
		command = f"conda run -n ldm python '{exec_path}' --prompt '{prompt}' --init-img {img_path} --strength {strength} --n_samples {samples} --n_iter 1 --ddim_steps {steps} --seed {seed} --H {height} --W {width} -outdir {out_dir} --skip_grid"

	print("Processing command: ", command)
	getContainer().exec_run(command, workdir="/sd")
	return True