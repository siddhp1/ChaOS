FROM debian:sid-slim

# Temporary, until permanent switch to containerized builds
ENV CROSS_COMPILE=aarch64-none-elf-
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
  gcc-aarch64-none-elf \
  binutils-aarch64-none-elf \
  build-essential \
  make \
  cpio \
  clang-format \
  clang-tidy \
  python3 \
  python3-pip \
  doxygen \
  graphviz \
  git \
  ca-certificates \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["/bin/bash"]
