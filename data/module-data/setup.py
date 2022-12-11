# This script controls the installation of modules folders into the docker image
import errno
import os
import yaml
import shutil

# os.walk is a generator and calling next will get the first result in the form of a 
# 3-tuple (dirpath, dirnames, filenames). Thus the [1] index returns only the dirnames 
# from that tuple.
for module_folder in next(os.walk("."))[1]:
	print("Initialising: ", module_folder)

	# Read Yaml file and intialise git repos etc...
	with open(f"{module_folder}/init.yaml", "r") as stream:
		try:
			yaml_dict = yaml.safe_load(stream)

			repo      = yaml_dict["git-repo"] if ("git-repo" in yaml_dict) else None
			wd        = yaml_dict["working-directory"]
			filepaths = yaml_dict["filepaths"]
			
			if (repo != None):
				git_cmd = f"git clone {repo} {wd}"
				print("Running git command: ", git_cmd)
				os.system(git_cmd)

			# Copy files as required
			for src,dst in filepaths.items():
				src = f"{module_folder}/{src}"
				print("Copying file: ", src, "to: ", dst)
				try:
					shutil.copyfile(src, dst)
				except IOError as e:
					# ENOENT(2): file does not exist, raised also on missing dest parent dir
					if e.errno != errno.ENOENT:
						raise
					# try creating parent directories
					os.makedirs(os.path.dirname(dst))
					shutil.copy(src, dst)
				

			# TODO: fix file permissions
			os.system(f"./{module_folder}/init.sh")

		except yaml.YAMLError as exc:
			print(exc)
			print("Invalid YAML for module: ", module_folder)