# Starts and runs a model server, this maintains an active model in memory
# Should improve performance without having to reload constantly

from sd import sd_model
from common import devices

# Use a messaging queue
import time
import zmq
import json

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

class SD_Model_Server:

    def __init__(self):
        self.commandList = {
            "quit" : {
                "help": "Shuts down SD Model Server",
                "arguments": None
            },
            "help" : {
                "help": "Prints this message",
                "arguments": None
            },
            "loadModel" : {
                "help": "Loads a stable diffusion model into memory",
                "arguments": {
                    "checkpoint_path": "Filepath of the model",
                    "checkpoint_config_path": "Filepath to checkpoint configuration yaml",
                    "vae_path": "Optional filepath to a vae model",
                    "precision": "Model precision, [full, med, low, autocast]"
                }
            },
            "txt2img" : {
                "help": "Generate Text to Image",
                "arguments": None
            },
            "img2img" : {
                "help": "Generate Image to Image",
                "arguments": None
            }
        }

        self.model = None

    def __help(self):
        return json.dumps(self.commandList,sort_keys=True, indent=4)

    # Main loop
    def main(self):

        while True:
            #  Wait for next request from client
            message = socket.recv()
            print("SD Server Received request: %s" % message)

            if message.decode() == "quit":
                break

            #  Do some 'work'
            time.sleep(1)

            #  Send reply back to client
            socket.send(b"World")



sd_server = SD_Model_server()
sd_server.main()

print("SD Server shutting down...")

