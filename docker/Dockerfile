##################### Overridable arguments ####################################
ARG ARCH=aarch64
ARG REPO=docker.io/axisecp

# ACAP 4: https://hub.docker.com/r/axisecp/acap-native-sdk
ARG SDK_VERSION=12.11.0
ARG UBUNTU_VERSION=24.04
ARG SDK=acap-native-sdk
ARG NODE_MAJOR=20
ARG CGLM_VER=v0.9.4
ARG CGLM_SHA256=101376d9f5db7139a54db35ccc439e40b679bc2efb756d3469d39ee38e69c41b
ARG PW_VER=1.0.5
ARG PW_SHA256=c5a5de26d684a1a84060ad7b6131654fb2835e03fccad85059be92f8e3ffe993
ARG LWS_VER=v4.3-stable
ARG LWS_SHA256=73966103951e1b7be9ed8cc1fcc5a288b6b4f795b0cd39d863b0cc98fe7aa302

# Download resources (that can be overridden by build args)
ARG DOOM_WAD_URL=https://github.com/nneonneo/universal-doom/raw/refs/heads/main/DOOM1.WAD
ARG DOOM_WAD_SHA256=1d7d43be501e67d927e415e0b8f3e29c3bf33075e859721816f652a526cac771

##################### ACAP SDK aarch64 image ###################################
FROM $REPO/$SDK:$SDK_VERSION-aarch64-ubuntu$UBUNTU_VERSION AS builder-aarch64

##################### ACAP SDK armv7hf image ###################################
FROM $REPO/$SDK:$SDK_VERSION-armv7hf-ubuntu$UBUNTU_VERSION AS builder-armv7hf
ARG EXTRA_WS_ARGS='-D ENABLE_NEON=ON -D ENABLE_VFPV3=ON'

##################### ACAP SDK image ###########################################
FROM builder-$ARCH
ARG ARCH
ARG NODE_MAJOR
ARG DOOM_WAD_URL
ARG DOOM_WAD_SHA256
ARG CGLM_VER
ARG CGLM_SHA256
ARG PW_VER
ARG PW_SHA256
ARG LWS_VER
ARG LWS_SHA256
ARG EXTRA_WS_ARGS

##################### General setup ############################################

# Install some utils and source-setup prerequisites:
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update && apt-get install -y -f --no-install-recommends \
      cmake \
      curl \
      gnupg \
      meson \
      ninja-build \
    pkgconf \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install Node.js and Yarn:
RUN curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | \
    gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg && \
    echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" > /etc/apt/sources.list.d/nodesource.list && \
    apt-get update && \
    apt-get install -y -f --no-install-recommends nodejs && \
    npm install --global yarn && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# ACAP SDK install path:
ARG ACAP_SDK_INSTALL_DIR=/opt/axis/acapsdk/sysroots/$ARCH/usr

# Change permissions of the "KHR" folder and its contents:
RUN chmod -R 555 "$ACAP_SDK_INSTALL_DIR/include/KHR"

# Install libs here:
ARG INC_DIR="$ACAP_SDK_INSTALL_DIR/include"

# OpenGL Mathematics (glm) for C:
WORKDIR /opt/build/CGLM
RUN curl -fsSLo /tmp/cglm.tar.gz "https://github.com/recp/cglm/archive/refs/tags/$CGLM_VER.tar.gz" && \
    echo "$CGLM_SHA256  /tmp/cglm.tar.gz" | sha256sum -c - && \
    tar -xzf /tmp/cglm.tar.gz --strip-components=1 && \
    rm /tmp/cglm.tar.gz
RUN cp -r include/cglm "$INC_DIR"

# Download and verify shareware Doom WAD:
RUN curl -fsSLo /opt/app/doom1.wad "$DOOM_WAD_URL" && \
    expected_sha256="$DOOM_WAD_SHA256" && \
    echo "$expected_sha256  /opt/app/doom1.wad" | sha256sum -c - || \
    (echo 'Checksum verification failed. Removing file.' && rm -f /opt/app/doom1.wad && exit 1)

##################### Setup and build libwebsockets ############################

# libwebsockets paths and version:
ARG BUILD_DIR=/opt/build
ARG LWS_SRC_DIR="$BUILD_DIR/LWS"
ARG LWS_BUILD_DIR="$LWS_SRC_DIR/build"

# Download and verify libwebsockets to LWS_SRC_DIR:
WORKDIR "$LWS_SRC_DIR"
RUN curl -fsSLo /tmp/libwebsockets.tar.gz "https://github.com/warmcat/libwebsockets/archive/refs/heads/$LWS_VER.tar.gz" && \
    echo "$LWS_SHA256  /tmp/libwebsockets.tar.gz" | sha256sum -c - && \
    tar -xzf /tmp/libwebsockets.tar.gz --strip-components=1 && \
    rm /tmp/libwebsockets.tar.gz

# Configure and build libwebsockets:
WORKDIR "$LWS_BUILD_DIR"
RUN . /opt/axis/acapsdk/environment-setup* && \
    cmake \
    -D CMAKE_INSTALL_PREFIX="$ACAP_SDK_INSTALL_DIR" \
    -D CMAKE_CXX_COMPILER="${CXX%-g++*}-g++" \
    -D CMAKE_CXX_FLAGS="${CXX#*-g++}" \
    -D CMAKE_C_COMPILER="${CC%-gcc*}-gcc" \
    -D CMAKE_C_FLAGS="${CC#*-gcc}" \
    -D CMAKE_BUILD_TYPE=RELEASE \
    -D CPU_BASELINE=NEON,VFPV3 \
    -D LWS_WITH_SSL=OFF \
    -D LWS_WITH_CRYPTO=OFF \
    -D LWS_WITH_ZLIB=OFF \
    -D LWS_WITH_STATIC=OFF \
    -D LWS_WITH_SHARED=ON \
    -D LWS_WITHOUT_TESTAPPS=ON \
    -D LWS_HAVE_LIBCAP=OFF \
    $EXTRA_WS_ARGS \
    "$LWS_SRC_DIR" && \
    make -j$(nproc) install

# Copy libwebsockets libs to /opt/app in order to easily get them to host for packaging in ACAP:
WORKDIR /opt/app/libwebsockets
RUN cp -P "$ACAP_SDK_INSTALL_DIR"/lib/libwebsockets.so* .

##################### Setup and build PipeWire #################################

# PW paths and version:
ARG BUILD_DIR=/opt/build
ARG PW_BUILD_PATH="$BUILD_DIR/PW"
ARG PW_BUILD_DIR="$PW_BUILD_PATH/build"
ARG PW_INSTALL_DIR="$PW_BUILD_PATH/install"
ENV PKG_CONFIG_ALLOW_CROSS=1

# Download and verify PipeWire to PW_BUILD_PATH:
WORKDIR "$PW_BUILD_PATH"
RUN curl -fsSLo /tmp/pipewire.tar.gz "https://gitlab.freedesktop.org/pipewire/pipewire/-/archive/$PW_VER/pipewire-$PW_VER.tar.gz" && \
    echo "$PW_SHA256  /tmp/pipewire.tar.gz" | sha256sum -c - && \
    tar -xzf /tmp/pipewire.tar.gz --strip-components=1 && \
    rm /tmp/pipewire.tar.gz && \
    sed -i "/subdir('tools')/d" src/meson.build

# Meson crossfile source: https://github.com/mesonbuild/meson/blob/master/cross/
# Copy PW meson-crossfiles to PW_BUILD_PATH in image:
COPY ./pipewire/meson-crossfile-*.txt ./

# Build PW using meson-crossfile:
RUN . /opt/axis/acapsdk/environment-setup* && \
    meson setup --cross-file meson-crossfile-"$ARCH".txt "$PW_BUILD_DIR" \
    -Ddbus=disabled \
    -Dsession-managers=[] \
    -Dudev=disabled \
    -Dalsa=disabled \
    -Dv4l2=disabled \
    -Davb=disabled \
    -Dprefix=${ACAP_SDK_INSTALL_DIR} && \
    meson compile -C "$PW_BUILD_DIR" && meson install -C "$PW_BUILD_DIR"

# Copy PW libs to /opt/app:
WORKDIR /opt/app/libpipewire/pipewire-0.3
RUN cp -r "$ACAP_SDK_INSTALL_DIR"/lib/libpipewire-* ../
RUN cp -r "$ACAP_SDK_INSTALL_DIR"/lib/pipewire-0.3/* ./

################################################################################
