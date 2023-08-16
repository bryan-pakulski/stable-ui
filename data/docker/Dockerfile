# Local Repository
#FROM localhost:5000/nvidia-cuda-12.2.0-runtime-ubuntu20.04

# External repository
FROM nvidia/cuda:12.2.0-runtime-ubuntu20.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /

SHELL ["/bin/bash", "-c"]

# Software useage
# supervisor: manager for starting/restarting different sd scripts
# libjemalloc: more efficient memory manager
# numactl: more efficient core pinning for python
# exempi: libraries to manipulate xmp data

RUN apt-get update && \
  apt-get install -y libglib2.0-0 wget git supervisor libjemalloc2 numactl exempi vim && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/*

# jemalloc is a general purpose malloc(3) implementation that emphasizes fragmentation avoidance and scalable concurrency support
# env configuration is found in .env_docker file
ENV LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2

# Install miniconda
ENV CONDA_DIR /opt/conda
RUN wget -O ~/miniconda.sh -q --show-progress --progress=bar:force https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
  /bin/bash ~/miniconda.sh -b -p $CONDA_DIR && \
  rm ~/miniconda.sh
ENV PATH=$CONDA_DIR/bin:$PATH

COPY ./entrypoint.sh /home/
COPY ./environment.yaml /home/
COPY ./setup.py /home/
COPY ./sd-server.sh /home/
COPY ./sd-supervisor.sh /home/
COPY ./supervisord.conf /etc/supervisor/conf.d/supervisord.conf

RUN ["chmod", "+x", "/home/sd-supervisor.sh"]
RUN ["chmod", "+x", "/home/sd-server.sh"]
RUN ["chmod", "+x", "/home/entrypoint.sh"]
CMD sleep infinity