#!/usr/bin/env bash
set -e

FMT_BLUE=$(printf '\033[34m')
FMT_YELLOW=$(printf '\033[33m')
FMT_RESET=$(printf '\033[0m')

FINAL=$1

# Source the SDK:
. /opt/axis/acapsdk/environment-setup*

echo
echo -e "${FMT_BLUE}>>> BUILDING SOUND SERVER using ACAPSDK $OECORE_SDK_VERSION${FMT_RESET}"
if [ "$FINAL" = "y" ]; then
  echo -e "${FMT_YELLOW}*** RELEASE VERSION ***${FMT_RESET}"
else
  echo -e "${FMT_YELLOW}*** DEBUG VERSION ***${FMT_RESET}"
fi
echo

# Build sndserv:
make -C ./sndserv clean && FINAL=$FINAL make -j"$(nproc)" -C ./sndserv && cp ./sndserv/sndserver .
