#
# ACAP DOOM Makefile
#

# Print colors:
RED=\033[0;31m
LIGHT_RED=\033[1;31m
GREEN=\033[0;32m
BLUE=\033[0;34m
PURPLE=\033[0;35m
CYAN=\033[0;36m
YELLOW=\033[1;33m
NC=\033[0m # No color

SHELL := $(shell which sh)

PROGS = acap_doom
ACAP_NAME = "ACAP DOOM"
LDLIBS = -lm

DOCKER_TAG := acap_doom
ARCHS = aarch64 armv7hf
APPTYPE ?= aarch64
DOCKER := $(shell command -v docker 2> /dev/null)
NODE := $(shell command -v node 2> /dev/null)
YARN := $(shell command -v yarn 2> /dev/null)
ECHO := echo -e
BUILD_WEB = 1

# Make sure APPTYPE is not set to something we don't support here
ifneq ($(strip $(filter $(APPTYPE),$(ARCHS))),$(APPTYPE))
$(error Unsupported APPTYPE '$(APPTYPE)'. Valid values: $(ARCHS))
endif

# Helper targers:
include helpers.mak

# Older AXIS OS have /usr/local/packages, where AXIS OS 13.1 switched to
# /opt/apps
TARGET_DIR = /usr/local/packages/$(PROGS)/
#TARGET_DIR = /opt/apps/$(PROGS)/

d := $(CURDIR)
$(shell touch $(d)/.yarnrc)
$(shell chmod 644 $(d)/.yarnrc)

# Run Docker cmd with provided image:
DOCKER_CMD := docker run --rm -i -t \
              -e TARGET_IP=$(TARGET_IP) \
              -e TARGET_USR=$(TARGET_USR) \
              -e TARGET_PWD=$(TARGET_PWD) \
              -e HOME=$(d) \
              -w $(d) \
              -u $(shell id -u):$(shell id -g) \
              -v $(d):$(d) \
              -v /etc/passwd:/etc/passwd:ro \
              -v /etc/group:/etc/group:ro \
              -v $(d)/.yarnrc:$(d)/.yarnrc

# Static linking of libwebsockets:
LDLIBS += /opt/app/libwebsockets/libwebsockets.a

PKGS += glib-2.0 gio-2.0 axoverlay glesv2 jansson
ifdef PKGS
	LDLIBS += $(shell pkg-config --libs $(PKGS))
	CFLAGS += $(shell pkg-config --cflags $(PKGS))
endif

# C source files:
SRCS_C = $(wildcard src/*.c) $(wildcard src/linuxdoom/*.c)

# Object files:
OBJS = $(SRCS_C:.c=.o)

CFLAGS += -DAPP_NAME="\"$(PROGS)\""
# CFLAGS += -std=c99

# Enable all warnings:
CFLAGS += -Wall

# TODO: Extra warnings:
# CFLAGS += -Werror
CFLAGS += -W
CFLAGS += -Wextra
# CFLAGS += -Wpedantic
# CFLAGS += -Wmissing-prototypes
# CFLAGS += -Wstrict-prototypes
CFLAGS += -Wvla
CFLAGS += -Wformat=2
CFLAGS += -Wmaybe-uninitialized
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-but-set-parameter
CFLAGS += -Wpointer-arith
CFLAGS += -Wbad-function-cast
CFLAGS += -Wfloat-equal
CFLAGS += -Winline
CFLAGS += -Wdisabled-optimization

# NOTE: Disabled warnings:
CFLAGS += -Wno-implicit-fallthrough
CFLAGS += -Wno-absolute-value

# Doom flags:
CFLAGS += -DNORMALUNIX -DLINUX -D$(APPTYPE)

# Enable the sound server:
CFLAGS += -DSNDSERV

# Set default value for FINAL to 'y' if not already defined:
FINAL ?= y
# ASAN=y
ifeq ($(FINAL), y)
  LDFLAGS += -s
  CFLAGS += -DNDEBUG -g0 -O2
else
  CFLAGS += -g3 -DDEBUG
  ifeq ($(ASAN), y)
    CFLAGS += -fsanitize=address -O1 -fno-omit-frame-pointer
    LDLIBS += -fsanitize=address
  endif
endif

# Default target:
.DEFAULT_GOAL := all
.PHONY: all $(PROGS)
all: $(PROGS)

# Print help:
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  dockersetup    : Create the Docker images ($(addprefix $(DOCKER_TAG)_,$(ARCHS)))"
	@echo "  dockerlist     : List all Docker images"
	@echo "  dockerrun      : Log in to the Docker image for current arch"
	@echo "  armv7hf        : Build for 32-bit ARM in Docker"
	@echo "  aarch64        : Build for 64-bit ARM in Docker"
	@echo "  build          : Fast build ACAP binary for current arch"
	@echo "  install        : Install the ACAP to target device"
	@echo "  deploy         : Deploy the ACAP binary to target device (requires ACAP already installed)"
	@echo "  deploysnd      : Deploy the sound server to target device"
	@echo "  deployprofile  : Deploy shell profile to target device"
	@echo "  deploygdb      : Deploy gdbserver to target device"
	@echo "  checksdk       : Check SDK information for target device"
	@echo "  logon          : Logon to ACAP dir"
	@echo "  log            : Trace logs on target"
	@echo "  kill           : Kill ACAP running on target device"
	@echo "  openweb        : Open ACAP web on target device"
	@echo "  web            : Build the web using Node.js and Yarn"
	@echo "  deployweb      : Deploy the web to target device"
	@echo "  release        : Build ACAP release for all arch and put in a release dir"
	@echo "  clean          : Clean the build"
	@echo "  distclean      : Clean everything, web and *.old *.orig"

# Print flags:
.PHONY: debug
debug:
	$(info *** Debug info)
	$(info Compiler: $(CC))
	$(info C Source-files: $(SRCS_C))
	$(info Object-files: $(OBJS))
	$(info Compiler-flags: $(CFLAGS))
	$(info Linker-flags: $(LDFLAGS))
	$(info Linker-libs: $(LDLIBS))
	$(info User ID: $(shell id -u))
	$(info Group ID: $(shell id -g))
	$(info Target IP: $(TARGET_IP))
	$(info APPTYPE: $(APPTYPE))

# Build the app (if SDK is sourced):
ifdef OECORE_SDK_VERSION
# Build rule for C source files:
%.o: %.c
	@$(ECHO) "${GREEN}*** Compile $<${NC}"
	$(CC) $(CFLAGS) -c $< -o $@
# Build the target executable:
$(PROGS): $(OBJS)
	@$(ECHO) "${GREEN}*** Build $(PROGS)${NC}"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
else
$(PROGS):
	$(error Please build "$@" from Docker, run 'make help')
endif

# Build web:
.PHONY: web
web:
ifndef NODE
	$(error "Node.js is not installed")
endif
ifndef YARN
	$(error "Yarn is not installed")
endif
	@cd web && yarn && yarn build
	@$(RM) -r html
	@cp -R web/build html

# Check that Docker is installed:
.PHONY: checkdocker
checkdocker:
ifndef DOCKER
	$(error Please install Docker first!)
endif

# Create Docker image(s) to build in:
.PHONY: %.dockersetup
%.dockersetup: checkdocker
	@docker build --build-arg ARCH=$(*F) -t $(DOCKER_TAG)_$(*F) ./docker
# @docker build --progress=plain --no-cache --build-arg ARCH=$(*F) -t $(DOCKER_TAG)_$(*F) ./docker

.PHONY: dockersetup
dockersetup: $(addsuffix .dockersetup,$(ARCHS))

# Build ACAP for selected target architecture using Docker:
.PHONY: $(ARCHS)
$(ARCHS): checkdocker
	@./scripts/dockercopy.sh -i $(DOCKER_TAG)_$@ -f /opt/app/doom1.wad
	@$(DOCKER_CMD) $(DOCKER_TAG)_$@ ./docker/build_snd.sh $(FINAL)
	@$(DOCKER_CMD) $(DOCKER_TAG)_$@ ./docker/build_eap.sh $(BUILD_WEB) $(PROGS) $(ACAP_NAME) $@ $(FINAL)

# Fast build (only binary file) using Docker:
.PHONY: build
build: checkdocker
	@$(DOCKER_CMD) -e APPTYPE=$(APPTYPE) $(DOCKER_TAG)_$(APPTYPE) ./docker/build.sh $(FINAL)

# Build the sound server:
.PHONY: sndserv
sndserv: checkdocker
	@$(DOCKER_CMD) $(DOCKER_TAG)_$(APPTYPE) ./docker/build_snd.sh $(FINAL)

# Install ACAP using Docker:
.PHONY: install
install: checkdocker $(APPTYPE)
	@$(DOCKER_CMD) $(DOCKER_TAG)_$(APPTYPE) ./docker/eap-install.sh

# Clean up build artifacts:
.PHONY: clean
clean:
	@$(ECHO) "${RED}*** Clean build${NC}"
	$(RM) $(PROGS) $(OBJS) *.eap *LICENSE.txt
	$(MAKE) -C ./sndserv clean

# Clean up everything:
.PHONY: distclean
distclean: clean
	$(RM) -r html .*var_log_messages* *.old *.orig tmp* libwebsockets release* sndserver .yarn .yarnrc package.conf param.conf

# WARNING: Cleans up everything not tracked by git:
.PHONY: superclean
superclean: distclean
	@git clean -fdx
