#!/usr/bin/env bash
set -e

FMT_GREEN=$(printf '\033[32m')
FMT_YELLOW=$(printf '\033[33m')
FMT_RESET=$(printf '\033[0m')

N_THREADS="$(nproc)"

if [ "$#" -lt 3 ] || [ "$#" -gt 4 ]; then
  echo "Usage: $0 <BUILD_WEB> <PROGS> <ACAP_NAME> [FINAL]"
  exit 1
fi

BUILD_WEB=$1
PROGS=$2
ACAP_NAME=$3
FINAL=${4:-n}

# Source the SDK:
. /opt/axis/acapsdk/environment-setup*

echo
echo -e "${FMT_GREEN}>>> BUILDING FOR ARMv7 using ACAPSDK $OECORE_SDK_VERSION${FMT_RESET}"
if [ "$FINAL" = "y" ]; then
  echo -e "${FMT_YELLOW}*** RELEASE VERSION ***${FMT_RESET}"
else
  echo -e "${FMT_YELLOW}*** DEBUG VERSION ***${FMT_RESET}"
fi
echo

# Set Arch to armv7hf:
sed -i 's/aarch64/armv7hf/g' ./package.conf
sed -i 's/aarch64/armv7hf/g' ./manifest.json
# Set PACKAGENAME/friendlyName:
sed -i "s/PACKAGENAME=\"[^\"]*\"/PACKAGENAME=\"${ACAP_NAME}\"/" package.conf
sed -i "s/\"friendlyName\": \"[^\"]*\"/\"friendlyName\": \"${ACAP_NAME}\"/" manifest.json
# Set APPNAME/appName:
sed -i "s/APPNAME=\"[^\"]*\"/APPNAME=\"${PROGS}\"/" package.conf
sed -i "s/\"appName\": \"[^\"]*\"/\"appName\": \"${PROGS}\"/" manifest.json

if [ "$BUILD_WEB" = "1" ] || [ "$BUILD_WEB" = "y" ]; then
  # Build with web:
  make clean && FINAL=$FINAL make -j"$N_THREADS" && make web && eap-create.sh
else
  # Build without web:
  make clean && FINAL=$FINAL make -j"$N_THREADS" && eap-create.sh
fi
