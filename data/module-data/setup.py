# This script controls the installation of modules folders into the docker image
import errno
import os
import yaml
import shutil

MODULE_CONFIG_PATH = "/shared-config/module_config.yaml"


# Set up a new module in the MODULE_CONFIG_PATH file, updates existing entries
def update_module_config(name, filepath, module_yaml):
    stream = open(filepath, 'r')
    module_config = yaml.safe_load(stream)
    if module_config["modules"] == None:
        module_config["modules"] = {f"{name}": module_yaml}
    else:
        module_config["modules"][name] = module_yaml

    with open(filepath, 'w') as yaml_file:
        yaml_file.write(yaml.dump(module_config, default_flow_style=False))


# Deletes inactive modules from yaml config
def purge_inactive_modules(filepath, active_modules):
    stream = open(filepath, 'r')
    module_config = yaml.safe_load(stream)
    if module_config["modules"] == None:
        pass
    else:
        # Check if the module_config file contains an active key, if not delete the entry
        for key in module_config["modules"].keys():
            if not (key in active_modules):
                print("Removing old module configuration: ", key)
                module_config["modules"].pop(key, None)

        with open(filepath, 'w') as yaml_file:
            yaml_file.write(yaml.dump(module_config, default_flow_style=False))


# os.walk is a generator and calling next will get the first result in the form of a
# 3-tuple (dirpath, dirnames, filenames). Thus the [1] index returns only the dirnames
# from that tuple.
active_modules = []
for module_folder in next(os.walk("."))[1]:
    print("Initialising: ", module_folder)

    # Read Yaml file
    with open(f"{module_folder}/init.yaml", "r") as stream:
        try:
            yaml_dict = yaml.safe_load(stream)
            data = yaml_dict["module"]

            name = data["name"]
            wd = data["working-directory"]
            filepaths = data["filepaths"]

            update_module_config(name, MODULE_CONFIG_PATH, data)
            active_modules.append(name)

            # Copy files as required
            for src, dst in filepaths.items():
                src = f"{module_folder}/{src}"
                print("Copying file: ", src, "to: ", dst)
                try:
                    if (os.path.isfile(src)):
                        shutil.copyfile(src, dst)
                    else:
                        shutil.copytree(src, dst, dirs_exist_ok=True)
                except IOError as e:
                    # ENOENT(2): file does not exist, raised also on missing dest parent dir
                    if e.errno != errno.ENOENT:
                        raise
                    # try creating parent directories
                    os.makedirs(os.path.dirname(dst))
                    shutil.copy(src, dst)
            
            os.system(f"chmod +x ./{module_folder}/init.sh")
            os.system(f"./{module_folder}/init.sh")

        except yaml.YAMLError as exc:
            print(exc)
            print("Invalid YAML for module: ", module_folder)

print("Loaded modules: ", active_modules)
