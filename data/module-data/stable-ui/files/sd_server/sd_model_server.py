# Starts and runs a model server, this maintains an active model in memory
# Should improve performance without having to reload constantly

import traceback
from sd import txt2img as txt2img
from sd import sd_model
from common import devices

# Use a messaging queue
import time
import zmq
import json
import logging

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")


class Command():

    def __init__(self, command):
        self.command = None
        self.arguments = {}
        self.parse(command)

    def parse(self, command):
        # Split the message into the command and arguments
        command_and_args = command.split(":")
        self.command = command_and_args[0]

        print(command_and_args)
        if (len(command_and_args) > 1):
            for pair in command_and_args[1:]:
                print(pair)
                arg, val = pair.split("=")
                self.arguments[arg] = val


class SDModelServer():

    def __init__(self):
        self.running = True
        self.commandList = {
            "quit": {
                "help": "Shuts down SD Model Server",
                "arguments": None,
                "function": self.__quit
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
                    "precision": {
                        "help": "Model precision, [full, med, low, autocast]",
                        "required": False,
                        "type": str
                    }
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
                "arguments": None
            }
        }

        self.model = None

        # Initialise logging
        logging.basicConfig(
            filename="/shared-config/sd_server.log",
            level=logging.INFO,
            format="[SDSERVER] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")

        logging.info("Starting up sd_server")

    # Create a command from a message
    def parseMessage(self, message):
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
                cmd.arguments[arg_name] = arg_details["type"](
                    cmd.arguments[arg_name])

        # Return True if the command has all the required arguments
        return True

    # Sets up the callback for command
    def hookCommand(self, cmd):
        return self.commandList[cmd.command]["function"](cmd)

    # Function commands
    def __quit(self, cmd):
        self.running = False
        logging.info("Recieved Quit command")

        return "Quitting..."

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
            **cmd.arguments, model=self.model.model, precision=self.model.get_precision())
        return f"{result}"

    # Main loop
    def main(self):
        response = ""

        while self.running:
            #  Wait for next request from client
            logging.info("SD Server waiting for request...")

            message = socket.recv()
            logging.info("SD Server Received request: %s" % message)

            # Break down message, message format is as follows: command:var1=val1,var2=val2 ...
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
            time.sleep(0.5)
            socket.send(response.encode())


def main():
    sd_server = SDModelServer()
    sd_server.main()


if __name__ == "__main__":
    main()

    logging.info("SD Server shutting down...")
