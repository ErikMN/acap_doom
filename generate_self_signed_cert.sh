#!/usr/bin/env bash
#
# generate_ca_and_server.sh
#
# Usage:
#   ./generate_ca_and_server.sh
#
# This script creates:
#   1) rootCA.key   (PRIVATE key for the Root CA - do NOT import into Chrome)
#   2) rootCA.pem   (Root CA certificate in PEM format)
#   3) rootCA.crt   (Root CA certificate in DER format, for Chrome import)
#   4) server.key   (PRIVATE key for the server)
#   5) server.crt   (Server certificate, signed by rootCA.pem)
#
# Steps to use:
#   1) Import rootCA.crt into Chrome/OS trust store as an Authority.
#   2) Place server.key & server.crt on your device for HTTPS/WSS.
#   3) Access the device via the exact DNS/IP from the Subject Alternative Name.
set -e

CERT_DIR="$(pwd)"

# Root CA files:
ROOT_CA_KEY="$CERT_DIR/rootCA.key"
ROOT_CA_PEM="$CERT_DIR/rootCA.pem"
ROOT_CA_DER="$CERT_DIR/rootCA.crt" # DER format for Chrome import chrome://settings/certificates

# Server files:
SERVER_KEY="$CERT_DIR/server.key"
SERVER_CSR="$CERT_DIR/server.csr"
SERVER_CRT="$CERT_DIR/server.crt"

########################
# CONFIGURABLE SETTINGS
########################
DAYS_VALID=3650 # 10 years

# Change these subject fields as needed:
ROOT_CA_SUBJ="/C=XX/ST=StateName/L=CityName/O=CompanyName/OU=IT/CN=MyAxisDevice"
SERVER_SUBJ="/C=XX/ST=StateName/L=CityName/O=CompanyName/OU=IT/CN=my-embedded-device"

# Use TARGET_IP from environment if set, otherwise default to 192.168.0.90
TARGET_IP="${TARGET_IP:-192.168.0.90}"

# Subject Alternative Name (SAN):
SAN="IP:$TARGET_IP"

####################
# 1) Create Root CA
####################
echo "*** Generating Root CA ***"
openssl genrsa -out "$ROOT_CA_KEY" 4096

# Generate a self-signed Root CA certificate in PEM format:
openssl req -x509 -new -nodes \
  -key "$ROOT_CA_KEY" \
  -sha256 \
  -days "$DAYS_VALID" \
  -subj "$ROOT_CA_SUBJ" \
  -out "$ROOT_CA_PEM" \
  -addext "basicConstraints=critical,CA:true" \
  -addext "keyUsage=critical, keyCertSign, cRLSign" \
  -addext "subjectKeyIdentifier=hash" \
  -addext "authorityKeyIdentifier=keyid,issuer"

# Also produce a DER-encoded version for Chrome import:
openssl x509 -in "$ROOT_CA_PEM" -out "$ROOT_CA_DER" -outform DER

##############################
# 2) Generate Server Cert/key
##############################
echo "*** Generating Server certificate + key (signed by Root CA) ***"

# Server private key:
openssl genrsa -out "$SERVER_KEY" 2048

# Server CSR:
openssl req -new \
  -key "$SERVER_KEY" \
  -out "$SERVER_CSR" \
  -subj "$SERVER_SUBJ"

# Create a temporary extension file for the server certificate's v3 extensions:
TEMP_EXTFILE="$(mktemp)"
cat <<EOF >"$TEMP_EXTFILE"
basicConstraints=CA:false
keyUsage=digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
subjectAltName=$SAN
EOF

# Sign the server CSR with the Root CA
openssl x509 -req \
  -in "$SERVER_CSR" \
  -CA "$ROOT_CA_PEM" \
  -CAkey "$ROOT_CA_KEY" \
  -CAcreateserial \
  -out "$SERVER_CRT" \
  -days "$DAYS_VALID" \
  -sha256 \
  -extfile "$TEMP_EXTFILE"

# Cleanup:
rm -f "$TEMP_EXTFILE" "$SERVER_CSR"
chmod 600 "$SERVER_KEY"

# Optional: display the server certificate summary:
echo "*** Server certificate details ***"
openssl x509 -noout -text -in "$SERVER_CRT"

###################
# Final Information
###################
echo
echo "=============================================================="
echo "All done! Generated files in $CERT_DIR:"
echo " - rootCA.key  (PRIVATE key for your CA - do NOT distribute)"
echo " - rootCA.pem  (PEM-encoded Root CA cert, can also be used by some tools)"
echo " - rootCA.crt  (DER-encoded Root CA cert for Chrome import)"
echo " - server.key  (Server PRIVATE key)"
echo " - server.crt  (Server cert, signed by Root CA)"
echo
echo "Next steps:"
echo " 1) Import '$ROOT_CA_DER' into Chrome chrome://settings/certificates (under 'Authorities'),"
echo "    selecting 'Trust this certificate for identifying websites'."
echo " 2) On your device, place '$SERVER_KEY' and '$SERVER_CRT' for HTTPS/WSS."
echo " 3) Access device using SAN: 'my-embedded-device' or '$TARGET_IP'."
echo "    (Chrome must match IP/hostname exactly with the SAN.)"
echo "=============================================================="
