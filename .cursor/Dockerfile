# Cloud Agent base image for box3d-wasm.
# System packages for native reference-dump (clang++/libstdc++) and CMake live here;
# Bun, emsdk, and the WASM artifact are installed by install.sh after checkout.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		ca-certificates \
		curl \
		git \
		sudo \
		build-essential \
		cmake \
		ninja-build \
		clang \
		g++ \
		libstdc++-14-dev \
		pkg-config \
		python3 \
	&& rm -rf /var/lib/apt/lists/*

# Match Cursor's default cloud-agent user so install/terminals run as ubuntu.
RUN id -u ubuntu >/dev/null 2>&1 || useradd -m -s /bin/bash ubuntu \
	&& echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' >/etc/sudoers.d/ubuntu \
	&& chmod 0440 /etc/sudoers.d/ubuntu

USER ubuntu
WORKDIR /home/ubuntu
