FROM debian:trixie-slim

ENV DEBIAN_FRONTEND=noninteractive

ARG DOXYGEN_VERSION=1.17.0
ARG DOXYGEN_RELEASE=Release_1_17_0
ARG DOXYGEN_SHA256=75419ef4f446fc1c24ef12514b574e66e898ee6f527c6ae2ad84f91a905823c2

RUN apt-get update && apt-get install -y --no-install-recommends \
  gcc-aarch64-linux-gnu \
  binutils-aarch64-linux-gnu \
  build-essential \
  make \
  cpio \
  clang-format \
  clang-tidy \
  curl \
  python3 \
  python3-pip \
  graphviz \
  git \
  ca-certificates \
  && rm -rf /var/lib/apt/lists/*

RUN curl --fail --location --retry 3 \
  "https://github.com/doxygen/doxygen/releases/download/${DOXYGEN_RELEASE}/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz" \
  --output /tmp/doxygen.tar.gz \
  && echo "${DOXYGEN_SHA256}  /tmp/doxygen.tar.gz" | sha256sum --check - \
  && tar --extract --gzip --file /tmp/doxygen.tar.gz \
  --directory /usr/local/bin \
  --strip-components=2 \
  "doxygen-${DOXYGEN_VERSION}/bin/doxygen" \
  && rm /tmp/doxygen.tar.gz \
  && doxygen --version | grep --quiet "^${DOXYGEN_VERSION}"

WORKDIR /workspace

CMD ["/bin/bash"]
