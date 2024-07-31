#!/bin/sh
VERSION_FILE_NAME="version_info"
BRANCH_FILE_NAME="branch"

INSTALL_LOCATION="./src/assets/etc"
mkdir -p $INSTALL_LOCATION

# Version information:
COMMIT_HASH=$(git log -n 1 --pretty=format:"%h%x09%cd")
echo "$COMMIT_HASH" >"$INSTALL_LOCATION/$VERSION_FILE_NAME"
export VITE_COMMIT_HASH="$COMMIT_HASH"

# Branch information:
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "$CURRENT_BRANCH" >"$INSTALL_LOCATION/$BRANCH_FILE_NAME"

# License information:
cp ../LICENSE $INSTALL_LOCATION/
