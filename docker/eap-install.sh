#!/bin/bash -e

GREEN='\033[0;32m'
NC='\033[0m'

# Source the SDK:
. /opt/axis/acapsdk/environment-setup*

echo
echo -e "${GREEN}>>> Installing using ACAPSDK $OECORE_SDK_VERSION for $OECORE_TARGET_ARCH${NC}"
echo

export axis_device_ip=$TARGET_IP
export password=$TARGET_PWD

eap-install.sh
