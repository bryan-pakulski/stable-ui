#!/opt/conda/envs/ldm/bin/python3.8

import sys
import zmq
import logging

def usage():
    print("Error: No parameter provided")
    print("Format of message should be \"<command_name>:<arg1>=<val1>:<arg2>=<val2>...\"")
    sys.exit()

if __name__ == "__main__":
    # Initialise logging
    logging.basicConfig(
        filename="/logs/sd_server.log",
        level=logging.INFO,
        format="[SDCLIENT] %(asctime)s - %(levelname)s - %(message)s"
    )

    logging.addLevelName(logging.INFO, "INFO")
    logging.addLevelName(logging.WARNING, "WARN")
    logging.addLevelName(logging.ERROR, "ERR")

    if len(sys.argv) < 2:
        logging.error("No parameter provided to client...")
        usage()

    # Socket to talk to server
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    heartbeatSocket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")
    heartbeatSocket.connect("tcp://localhost:5556")

    # Add a custom ping command
    msg = sys.argv[1]
    if msg == "ping":
        heartbeatSocket.setsockopt(zmq.RCVTIMEO, 5000)
        heartbeatSocket.send(msg.encode())
        try:
            message = heartbeatSocket.recv()
        except zmq.error.Again:
            pass
    else:
        logging.info(f"Sending message: {msg}")
        socket.send(msg.encode())

        # Get the reply.
        message = socket.recv()
        logging.info(f"Received reply {message.decode()}")