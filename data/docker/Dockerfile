FROM nvidia/cuda:11.3.1-runtime-ubuntu20.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /

SHELL ["/bin/bash", "-c"]

RUN apt-get update && \
    apt-get install -y libglib2.0-0 wget git supervisor && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

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
COPY ./supervisord.conf /etc/supervisor/conf.d/supervisord.conf

RUN ["chmod", "+x", "/home/entrypoint.sh"]
CMD sleep infinity