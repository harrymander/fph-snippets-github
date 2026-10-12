FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get -y install --no-install-recommends \
    gcc-arm-none-eabi \
    git \
    libnewlib-arm-none-eabi \
    make \
    python3 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
