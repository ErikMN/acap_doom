#!/bin/sh

BASEDIR=$(readlink -f "$0")
APPNAME=${BASEDIR##*/}
BASEDIR=${BASEDIR%/*}

echo "*** Postinstall script for $APPNAME"

SERVICE_FILE=dropin_service.service
TARGET=/etc/systemd/system/sdk"$APPNAME".service.d/60-doom-wad-path.conf

sed -i "s/DOOMWADDIR=/DOOMWADDIR=$BASEDIR/g" $SERVICE_FILE

cp $SERVICE_FILE "$TARGET"
systemctl daemon-reload

echo "*** Copied $SERVICE_FILE to $TARGET"

# HACK: Create a directory for game saves:
mkdir "$BASEDIR"/saves

echo "*** Created directory for game saves"
