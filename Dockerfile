FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

COPY . /init
COPY entrypoint.sh /entrypoint.sh
WORKDIR /workspace

VOLUME ["/workspace", "/submission"]

ENTRYPOINT ["/entrypoint.sh"]
