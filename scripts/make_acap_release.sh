#!/usr/bin/env bash
#
# Build final ACAP releases for arch: aarch64 and armv7hf
#
set -e

# Print colors:
FMT_RED=$(printf '\033[31m')
FMT_GREEN=$(printf '\033[32m')
FMT_BLUE=$(printf '\033[34m')
FMT_YELLOW=$(printf '\033[33m')
# FMT_WHITE=$(printf '\033[37m')
FMT_BOLD=$(printf '\033[1m')
FMT_RESET=$(printf '\033[0m')

################################################################################
# Helper functions:

cleanup() {
  # NOTE: distclean would remove the build dir
  make clean
  # Remove arch specific artifacts:
  echo "${FMT_BOLD}*** Remove libwebsockets${FMT_RESET}"
  rm -rf libwebsockets || {
    echo "${FMT_RED}*** Failed to remove libwebsockets${FMT_RESET}"
    return 1
  }
  echo "${FMT_BOLD}*** Remove sndserver${FMT_RESET}"
  rm -rf sndserver || {
    echo "${FMT_RED}*** Failed to remove sndserver${FMT_RESET}"
    return 1
  }
}

checkfiles() {
  echo
  echo "${FMT_BOLD}${FMT_GREEN}$(file acap_doom)${FMT_RESET}"
  echo "${FMT_BOLD}${FMT_BLUE}$(file sndserver)${FMT_RESET}"
  echo "${FMT_BOLD}${FMT_YELLOW}$(file libwebsockets/libwebsockets.so.*)${FMT_RESET}"
  echo
}

################################################################################

# Go to project root dir:
script_dir=$(dirname "$0")
parent_dir=$(dirname "$script_dir")
cd "$parent_dir" || {
  echo "${FMT_RED}*** Failed to enter project root dir${FMT_RESET}"
  exit 1
}

# Get current git tag and replace '.' with '_':
GIT_TAG=$(git describe --tags --abbrev=0 | tr '.' '_')
if [ -z "$GIT_TAG" ]; then
  echo "*** No git tags found. Set tag to 0.0.1"
  GIT_TAG="0.0.1"
fi

OUT_DIR="release_$GIT_TAG"

echo
echo "${FMT_BOLD}*** Building ACAPs for aarch64 and armv7hf${FMT_RESET}"
echo "${FMT_BLUE}*** Output directory: $OUT_DIR${FMT_RESET}"
echo

# Remove old ACAPs first:
rm -rf "$OUT_DIR"

mkdir -p "$OUT_DIR" || {
  echo "${FMT_RED}*** Failed to create $OUT_DIR${FMT_RESET}"
  exit 1
}

################################################################################

cleanup || exit 1

################################################################################
# aarch64

FINAL=y make aarch64 || {
  echo "${FMT_RED}*** Failed to build aarch64 ACAP${FMT_RESET}"
  exit 1
}

checkfiles

mv ./*_aarch64.eap "$OUT_DIR" || {
  echo "${FMT_RED}*** Failed to move aarch64 ACAP to $OUT_DIR${FMT_RESET}"
  exit 1
}

################################################################################

cleanup || exit 1

################################################################################
# armv7hf

FINAL=y make armv7hf || {
  echo "${FMT_RED}*** Failed to build armv7hf ACAP"
  exit 1
}

checkfiles

mv ./*_armv7hf.eap "$OUT_DIR" || {
  echo "${FMT_RED}*** Failed to move armv7hf ACAP to $OUT_DIR${FMT_RESET}"
  exit 1
}

################################################################################

cp ./*_LICENSE.txt "$OUT_DIR" || {
  echo "${FMT_RED}*** Failed to move LICENSE to $OUT_DIR${FMT_RESET}"
  exit 1
}

################################################################################

cleanup || exit 1

git checkout package.conf

echo
echo "${FMT_GREEN}*** Done.${FMT_RESET}"
SIZE=$(du -sh "$OUT_DIR" | cut -f1)
echo "${FMT_BOLD}${FMT_BLUE}*** Release size: $SIZE${FMT_RESET}"
echo
