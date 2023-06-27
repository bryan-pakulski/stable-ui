#!/opt/conda/envs/ldm/bin/python3.8
# Starts and runs a model server, this maintains an active model in memory
# Should improve performance without having to reload constantly

import traceback
import threading, _thread
from sd.commands import txt2img as txt2img
from sd.commands import img2img as img2img
from sd.commands import outpaint as outpaint
from sd import sd_model
from common import devices

# Use a messaging queue
import time
import zmq
import json
import logging

context = zmq.Context()
socket = context.socket(zmq.REP)
heartbeatSocket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")
heartbeatSocket.bind("tcp://*:5556")

# Initialise logging
logging.basicConfig(
    filename="/logs/sd_server.log",
    level=logging.INFO,
    format="%(asctime)s - [%(levelname)s][dkr]: %(message)s",
    datefmt='%a %b %d %H:%M:%S %Y'
)

logging.addLevelName(logging.INFO, "info")
logging.addLevelName(logging.WARNING, "warn")
logging.addLevelName(logging.ERROR, "err")
logging.addLevelName(logging.DEBUG, "dbg")

class MQServer():
    def __init__(self):
        self.running = True
        self.debuglogging = False
        self.commandList = {}
        
    # Create a command from a message
    def parseMessage(self, message):
        if self.debuglogging:
            logging.info(f"Recieved message: {message.decode()}")
        return Command(message.decode())

    # Validate a command is correct against the command list
    def validateCommand(self, cmd):
        command_name = cmd.command
        command_args = cmd.arguments

        # Get the argument requirements from the commandList
        if not command_name in self.commandList:
            logging.error(f"Command not found: {command_name}")
            return False

        # Return immediately if arguments aren't required
        command_arg_requirements = self.commandList[command_name]["arguments"]
        if command_arg_requirements == None:
            return True

        # Check if the command has all the required arguments
        for arg_name, arg_details in command_arg_requirements.items():
            # If the argument is required, check if it is present in the command
            if arg_details["required"] and arg_name not in command_args:
                # Return False if the argument is missing
                logging.error(f"Missing required argument: {arg_name}")
                return False

            # Set type
            if arg_name in command_args:
                # As we encode our true/false values as a 0/1 string we want to cast to int
                # a cast to bool will always return true for any string value
                if arg_details["type"] == bool:
                    cmd.arguments[arg_name] = bool(int(cmd.arguments[arg_name]))
                else:
                    cmd.arguments[arg_name] = arg_details["type"](
                        cmd.arguments[arg_name])

        # Return True if the command has all the required arguments
        return True

    # Sets up the callback for command
    def hookCommand(self, cmd):
        return self.commandList[cmd.command]["function"](cmd)


class Command():

    def __init__(self, command):
        self.command = None
        self.arguments = {}
        self.parse(command)

    def parse(self, command):
        # Split the message into the command and arguments
        command_and_args = command.split(":")
        self.command = str(command_and_args[0])

        print("Creating command: ", self.command)

        print(command_and_args)
        if (len(command_and_args) > 1):
            for pair in command_and_args[1:]:
                print(pair)
                arg, val = pair.split("=", 1)
                self.arguments[arg] = val

class HeartBeatServer(MQServer):

    def __init__(self):
        MQServer.__init__(self)
        self.commandList = {
            "ping": {
                "help": "pong",
                "arguments": {},
                "function": self.__pong
            }
        }

    def __pong(self, cmd):
        return "pong"

    # Main loop
    def main(self):
        heartbeatSocket.setsockopt(zmq.RCVTIMEO, 10000)
        response = ""

        logging.info(f"Heartbeat listener started")

        while self.running:
            #  Wait for next request from client
            try: 
                message = heartbeatSocket.recv()
            except zmq.error.Again:
                print("Heartbeat timeout")
                logging.info(f"heartbeat timeout detected, 10000ms without a response")
                self.running = False
                break

            # We received a response
            try:
                cmd = self.parseMessage(message)
                if (self.validateCommand(cmd)):
                    response = self.hookCommand(cmd)
                else:
                    response = "Invalid command"
            except Exception as e:
                logging.error(traceback.format_exc())
                response = f"Exception: {traceback.format_exc()}"

            #  Send reply back to client
            heartbeatSocket.send(response.encode())

        heartbeatSocket.close()
        

class SDModelServer(MQServer):

    def __init__(self):
        MQServer.__init__(self)
        self.debuglogging = True
        self.commandList = {
            "quit": {
                "help": "Shuts down SD Model Server",
                "arguments": None,
                "function": self.quit
            },
            "restart": {
                "help": "Reload SD Model Server",
                "arguments": None,
                "function": self.restart
            },
            "help": {
                "help": "Prints this message",
                "arguments": None,
                "function": self.__help
            },
            "loadModel": {
                "help": "Loads a stable diffusion model into memory",
                "arguments": {
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
                    "vae_config": {
                        "help": "Configuration for vae, mandatory if vae_path is set",
                        "required": False,
                        "type": str
                    },
                    "scheduler": {
                        "help": "Scheduler to use, defaults to pndm, options available: [pndm, lms, heun, euler, ancestral, dpm, ddim]",
                        "required": False,
                        "type": str
                    },
                    "hash": {
                        "help": "Model hash, used for saving XMP metadata",
                        "required": False,
                        "type": str  
                    },
                    "enable_xformers": {
                        "help": "Memory efficient transformers",
                        "required": False,
                        "type": bool
                    },
                    "enable_tf32": {
                        "help": "Lower precision floating point for certain nvidia gpus",
                        "required": False,
                        "type": bool
                    },
                    "enable_t16": {
                        "help": "16 bit precision",
                        "required": False,
                        "type": bool
                    },
                    "enable_vaeTiling": {
                        "help": "Allow for lower vram consumption at high image resolutions",
                        "required": False,
                        "type": bool
                    },
                    "enable_vaeSlicing": {
                        "help": "Allow for lower vram consumption at high batch sizes",
                        "required": False,
                        "type": bool
                    },
                    "enable_seqCPUOffload": {
                        "help": "offload memory to cpu, low vram requirements but slower inference time",
                        "required": False,
                        "type": bool
                    },
                    
                },
                "function": self.__load_model
            },
            "txt2img": {
                "help": "Generate Text to Image",
                "arguments": {
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
                },
                "function": self.__text2image
            },
            "img2img": {
                "help": "Generate Image to Image",
                "arguments": {
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
                },
                "function": self.__image2image
            },
            "outpaint": {
                "help": "Outpaint using a self masked image",
                "arguments": {
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
                    "img_data": {
                        "help": "Base64 encoded string of raw RGBA pixel data (null terminated)",
                        "required": True,
                        "type": str
                    },
                    "img_mask": {
                        "help": "Base64 encoded string of raw RGBA mask data (null terminated)",
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
                },
                "function": self.__outpainting
            },
            "plugin": {
                "help": "Call custom plugin command",
                "arguments": {
                    "name": {
                        "help": "plugin to use",
                        "required": True,
                        "type": str
                    }
                },
                "function": None
            }            
        }

        self.model = None

    # Load and call python function from external module
    def SDModule(self, cmd):
        module = map(__import__, cmd.module_path)
        module.init(**cmd.arguments, model=self.model.model)
        module.run()

    # Function commands
    def quit(self, cmd):
        self.running = False
        return "Quitting..."

    def restart(self, cmd):
        logging.info("Recieved reset command")

        # Free the old model if it exists and recover resources
        if (self.model != None):
            logging.info(
                f"Freeing old model & recovering resource: {self.model.checkpoint_path}")
            self.model.clean()

        # TODO: any additional cleanup???

        return "Restarted!"

    def __help(self, cmd):
        dump = json.dumps(self.commandList, sort_keys=True,
                          indent=4, default=lambda o: f"<<non serializable: {type(o).__qualname__} >>")
        logging.info(f"Help dump: {dump}")
        return dump

    def __load_model(self, cmd):
        logging.info(f"Loading model: {cmd.arguments['checkpoint_path']}")

        # Free the old model if it exists and recover resources
        if (self.model != None):
            logging.info(
                f"Freeing old model & recovering resource: {self.model.checkpoint_path}")
            self.model.clean()

        self.model = sd_model.StableDiffusionModel(**cmd.arguments)
        self.model.load_model()

        return f"Model loaded... {cmd.arguments['checkpoint_path']}"

    def __text2image(self, cmd):
        result = txt2img.generate(
            **cmd.arguments, model=self.model.model)
        return f"{result}"

    def __image2image(self, cmd):
        result = img2img.generate(
            **cmd.arguments, model=self.model.model)
        return f"{result}"
    
    def __outpainting(self, cmd):
        result = outpaint.generate(
            **cmd.arguments, model=self.model.model)
        return f"{result}"

    # Main loop
    def main(self):
        response = ""

        #  Wait for next request from client
        logging.info("Server waiting for request...")

        while self.running:
            # Run in a non blocking state, polling every second for new messages
            try:
                message = socket.recv(flags=zmq.NOBLOCK)
            except zmq.Again as e:
                time.sleep(1.0)
                continue

            logging.info("Server Received request: %s" % message)

            # Break down message, message format is as follows: command:var1=val1,var2=val2 ...
            try:
                cmd = self.parseMessage(message)
                if (self.validateCommand(cmd)):
                    response = self.hookCommand(cmd)
                else:
                    response = "Invalid command"
            except Exception as e:
                logging.error(traceback.format_exc())
                response = f"FAILED"

            #  Send reply back to client
            socket.send(response.encode())

        socket.close()


def main():
    # Start sd model server on seperate thread running in background
    sd_server = SDModelServer()
    print("Starting SD Server")
    server_thread = threading.Thread(target =  sd_server.main)
    server_thread.start()

    print("Starting Heartbeat server")
    # Kill main process when heartbeat stops
    # This should only happen when the client is shut down and we don't receive a poll for over 5s
    heartbeat_server = HeartBeatServer()
    heartbeat_thread = threading.Thread(target = heartbeat_server.main)
    heartbeat_thread.start()
    heartbeat_thread.join()

    sd_server.quit({})
    server_thread.join()


if __name__ == "__main__":
    main()

    logging.info("SD Server shutting down...")
