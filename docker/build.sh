#!/usr/bin/env bash
set -e

FMT_BLUE=$(printf '\033[34m')
FMT_YELLOW=$(printf '\033[33m')
FMT_RESET=$(printf '\033[0m')

FINAL=$1

# Source the SDK:
. /opt/axis/acapsdk/environment-setup*

echo
echo -e "${FMT_BLUE}>>> BUILDING FOR ARM64 using ACAPSDK $OECORE_SDK_VERSION${FMT_RESET}"
if [ "$FINAL" = "y" ]; then
  echo -e "${FMT_YELLOW}*** RELEASE VERSION ***${FMT_RESET}"
else
  echo -e "${FMT_YELLOW}*** DEBUG VERSION ***${FMT_RESET}"
fi
echo

# Build app only:
make clean && FINAL=$FINAL make -j"$(nproc)"
