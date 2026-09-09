FROM debian:trixie-slim AS doxygen-builder

ENV DEBIAN_FRONTEND=noninteractive

ARG DOXYGEN_VERSION=1.17.0
ARG DOXYGEN_RELEASE=Release_1_17_0
ARG DOXYGEN_SOURCE_SHA256=fa4c3dd78785abc11ccc992bc9c01e7a8c3120fe14b8a8dfd7cefa7014530814

RUN apt-get update && apt-get install -y --no-install-recommends \
  bison \
  build-essential \
  ca-certificates \
  cmake \
  curl \
  flex \
  python3 \
  && rm -rf /var/lib/apt/lists/*

RUN curl --fail --location --retry 3 "https://github.com/doxygen/doxygen/releases/download/${DOXYGEN_RELEASE}/doxygen-${DOXYGEN_VERSION}.src.tar.gz" --output /tmp/doxygen.tar.gz \
  && echo "${DOXYGEN_SOURCE_SHA256}  /tmp/doxygen.tar.gz" | sha256sum --check - \
  && tar --extract --gzip --file /tmp/doxygen.tar.gz --directory /tmp \
  && cmake -S "/tmp/doxygen-${DOXYGEN_VERSION}" -B /tmp/doxygen-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -Dbuild_wizard=NO \
  && cmake --build /tmp/doxygen-build --parallel 2 \
  && cmake --install /tmp/doxygen-build \
  && rm -rf /tmp/doxygen.tar.gz "/tmp/doxygen-${DOXYGEN_VERSION}" /tmp/doxygen-build \
  && doxygen --version | grep --quiet "^${DOXYGEN_VERSION}"

FROM debian:trixie-slim AS chaos

ENV DEBIAN_FRONTEND=noninteractive

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

COPY --from=doxygen-builder /usr/local/bin/doxygen /usr/local/bin/doxygen

WORKDIR /workspace

CMD ["/bin/bash"]
