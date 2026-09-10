#!/usr/bin/env bash
set -e

FMT_BLUE=$(printf '\033[34m')
FMT_YELLOW=$(printf '\033[33m')
FMT_RESET=$(printf '\033[0m')

if [ "$#" -lt 4 ] || [ "$#" -gt 5 ]; then
	echo "Usage: $0 <BUILD_WEB> <PROGS> <ACAP_NAME> <APPTYPE> [FINAL]"
	exit 1
fi

BUILD_WEB=$1
PROGS=$2
ACAP_NAME=$3
APPTYPE=$4
FINAL=${5:-n}

# Source the SDK:
# shellcheck disable=SC1090
. /opt/axis/acapsdk/environment-setup*

echo
echo -e "${FMT_BLUE}>>> BUILDING FOR $APPTYPE using ACAPSDK $OECORE_SDK_VERSION${FMT_RESET}"
if [ "$FINAL" = 'y' ]; then
	echo -e "${FMT_YELLOW}*** RELEASE VERSION ***${FMT_RESET}"
else
	echo -e "${FMT_YELLOW}*** DEBUG VERSION ***${FMT_RESET}"
fi
echo

# Set PACKAGENAME/friendlyName:
sed -i "s/\"friendlyName\": \"[^\"]*\"/\"friendlyName\": \"$ACAP_NAME\"/" manifest.json
# Set APPNAME/appName:
sed -i "s/\"appName\": \"[^\"]*\"/\"appName\": \"$PROGS\"/" manifest.json

make clean
FINAL=$FINAL APPTYPE=$APPTYPE make -j"$(nproc)"
if [ "$BUILD_WEB" = '1' ] || [ "$BUILD_WEB" = 'y' ]; then
	# Build with web:
	make web
fi
acap-build -a doom1.wad -a dropin_service.service -a libwebsockets -a sndserver .
