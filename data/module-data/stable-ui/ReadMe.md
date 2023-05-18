# Stable-UI
This module contains the backbone of server functionality, it has a public interface using a zero MQ server for interacting with the front end. The back end contains all the frameworks responsible for image generation.

# Networking & Connectivity

This python server runs inside the docker container and is responsible for the generative side of the stable-ui application. Being a server it's not restricted to just using the C++ front end and can be used with any zero-mq
client over ports 5555 / 5556.

By default the docker networking is set up with a bridge over to the server static address: `10.5.0.6`. This can be customised in the docker compose configuration as well as the gui `config.yaml`. In theory this could be set up for remote access with the right client.

Alternative clients can be used as long as they respect the keepalive requirement of the docker server. It is expected that a heartbeat is established by a client at least once within a `10000ms` period. If not the server will auto shut down.

This can be done by sending the command `ping` to the following interface: `tcp://10.5.0.6:5556` the server will reply with `pong`

# Available Commands:

## `help`
Prints a message with all available commands and arguments<br>
#### `Arguments`
None

## `quit`
Shuts down SD Model Server<br>
### `Arguments`
None

## `restart`
Reload SD Model Server and free cpu/gpu memory<br>
### `Arguments`
None

## `loadModel`
Loads a stable diffusion model into memory<br>
### `Arguments`
```
"checkpoint_path": {
    "help": "Filepath of the model",
    "required": True,
    "type": str
},
"checkpoint_config_path": {
    "help": "Filepath to checkpoint configuration yaml",
    "required": True,
    "type": str
},
"vae_path": {
    "help": "Optional filepath to a vae model",
    "required": False,
    "type": str
},
"precision": {
    "help": "Model precision, [full, med, low, autocast]",
    "required": False,
    "type": str
}
```

## `txt2img`
Generate Text to Image<br>
### `Arguments`
```
"outpath_samples": {
    "help": "Base output folder",
    "required": True,
    "type": str
},
"subfolder_name": {
    "help": "Subfolder name to save images into",
    "required": False,
    "type": str
},
"prompt": {
    "help": "txt2img prompt",
    "required": True,
    "type": str
},
"negative_prompt": {
    "help": "txt2img negative prompt",
    "required": False,
    "type": str
},
"seed": {
    "help": "seed to generate against",
    "required": True,
    "type": int
},
"sampler_name": {
    "help": "Sampler to use for image generation, defaults to PLMS",
    "required": True,
    "type": str
},
"batch_size": {
    "help": "Number of images to generate concurrently",
    "required": False,
    "type": int
},
"n_iter": {
    "help": "Number of images to create",
    "required": False,
    "type": int
},
"steps": {
    "help": "Number of steps to run image generation over",
    "required": True,
    "type": int
},
"cfg_scale": {
    "help": "The degree of freedom sd gets when following a prompt",
    "required": True,
    "type": float
},
"width": {
    "help": "Generated image width",
    "required": True,
    "type": int
},
"height": {
    "help": "Generated image height",
    "required": True,
    "type": int
}
```

## `img2img`
Generate Text to Image<br>
### `Arguments`
```
"outpath_samples": {
    "help": "Base output folder",
    "required": True,
    "type": str
},
"subfolder_name": {
    "help": "Subfolder name to save images into",
    "required": False,
    "type": str
},
"prompt": {
    "help": "txt2img prompt",
    "required": True,
    "type": str
},
"negative_prompt": {
    "help": "txt2img negative prompt",
    "required": False,
    "type": str
},
"init_img": {
    "help": "Init image path",
    "required": True,
    "type": str
},
"seed": {
    "help": "seed to generate against",
    "required": True,
    "type": int
},
"sampler_name": {
    "help": "Sampler to use for image generation, defaults to PLMS",
    "required": True,
    "type": str
},
"batch_size": {
    "help": "Number of images to generate concurrently",
    "required": False,
    "type": int
},
"strength": {
    "help": "Retain original image, range from 0.0 - 1.0",
    "required": True,
    "type": float
},
"n_iter": {
    "help": "Number of images to create",
    "required": False,
    "type": int
},
"steps": {
    "help": "Number of steps to run image generation over",
    "required": True,
    "type": int
},
"cfg_scale": {
    "help": "The degree of freedom sd gets when following a prompt",
    "required": True,
    "type": float
}
```