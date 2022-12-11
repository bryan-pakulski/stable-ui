# Module folder structure
Each module folder in this directory will be loaded by the `setup.py` file.

The modules should contain a `init.yaml` file with the following content:

```
git-repo: GIT_REPO_URL
working-directory: /modules/MODULE_NAME
```

This will install all git files to the `working-directory` path within the docker image.

If an `init.sh` script is included it will be run by the `entrypoint.sh` script when modules are initialised on container launch.

The contents of the `data/module-data` files are stored on the following docker path `/module-data/`

# Module usage
At this point in time modules are attached to `.ckpt` models, if a certain model hash is loaded and the `model_config.yaml` contains a `module` entry,
the docker working directory will be changed to the module and custom scripts can be run

# Updating modules
Modules will automatically be updated when the docker container is restarted. The `entrypoint.sh` script will call each module `update.sh` script.
This will run in the `data/module-data/<MODULE>` directory