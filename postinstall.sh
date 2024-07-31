#!/bin/sh

APPNAME=$(basename "$(pwd)")

echo "*** Postinstall script for $APPNAME"

SERVICE_FILE=dropin_service.service
TARGET=/etc/systemd/system/sdk"$APPNAME".service.d/60-doom-wad-path.conf

sed -i "s/xxxxxx/$APPNAME/g" $SERVICE_FILE

cp $SERVICE_FILE "$TARGET"
systemctl daemon-reload

echo "*** Copied $SERVICE_FILE to $TARGET"

# HACK: Create a directory for game saves and make it writable for the app:
mkdir /usr/local/packages/"$APPNAME"/saves
chown -R sdk:sdk /usr/local/packages/"$APPNAME"/saves

echo "*** Created directory for game saves"
