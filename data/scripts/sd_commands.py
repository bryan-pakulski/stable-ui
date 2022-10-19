#!/bin/python3

import glob
import docker

def getContainer():
	client = docker.from_env()
	return client.containers.get('sd')

# Example command
# python optimizedSD/txt2img.py --prompt 'Cat on a roof' --n_samples 10 --n_iter 2 --ddim_steps 80 --seed 1234 --H 512 --W 512 --turbo"
def txt2image(exec_path, prompt, samples, steps, seed, width, height):
	command = f"conda run -n ldm python '{exec_path}' --prompt '{prompt}' --n_samples {samples} --n_iter 1 --ddim_steps {steps} --seed {seed} --H {height} --W {width} --turbo"
	print("Processing command: ", command)
	getContainer().exec_run(command)

def img2img(exec_path, img_path, strength, prompt, samples, steps, seed, width, height):
	command = f"conda run -n ldm python '{exec_path}' --prompt '{prompt}' --init-img {img_path} --strength {strength} --n_samples {samples} --n_iter 1 --ddim_steps {steps} --seed {seed} --H {height} --W {width} --turbo"
	print("Processing command: ", command)
	getContainer().exec_run(command)

# Returns the latest images created
def get_latest_image(count, path):
	files = glob.glob(path + "*.png")
	
	if count == -1:
		for i in range(len(files)):
			print(f"[{i}] {files[i]}")
		return files
	
	if count > len(files):
		count = len(files)
	
	for i in range(len(files[:count])):
		print(f"[{i}] {files[i]}")
		
	return files[:count]