# Default Module

The default module for stable-ui is the `stable-ui` client/server package. This is what allows the stable-ui application to talk to the docker backend and run stable diffusion commands. It's not recommended to modify or change this module, plugin extension has been built around the idea of the `stable-ui` module sitting at the core and providing hooks for outside code to use.

# Module folder structure

Each module folder in this directory will be loaded by the `setup.py` file.

The modules should contain a `init.yaml` file with the following content:

```
module:
    name: MODULE_NAME
    working-directory: /modules/MODULE_NAME
    filepaths:
        "source": "destination"
```

If an `init.sh` script is included it will be run by the `entrypoint.sh` script when modules are initialised or updated on container launch.

The contents of the `modules` files are stored on the following docker path `/module-data/`

# Module usage

At this point in time modules are internalised by the SD_Server, they can access the SD_Server model in memory to save on processing power but their extandability is a little limited.

Modules can have a basic gui at this point in time, see `gui.yaml` at the root directory of the module files.

# Updating modules

In order to update the module files simply drag and drop the latest module into the `data/module-data` directory.

Modules inside docker will automatically be updated when the container is restarted and it detects that module files have changed. The `entrypoint.sh` script will call each module `update.sh` script.

This will run in the `data/module-data/<MODULE>` directory

# Module GUI

If a module contains a non empty `gui-path` vairbale in the `init.yaml` the GUI can be opened via the "modules" dropdown within `stable-ui`
