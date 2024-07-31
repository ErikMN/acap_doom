#!/usr/bin/env bash
set -e
# SOURCE ME!

# Print colors:
FMT_RED=$(printf '\033[31m')
FMT_GREEN=$(printf '\033[32m')
# FMT_BLUE=$(printf '\033[34m')
# FMT_YELLOW=$(printf '\033[33m')
FMT_WHITE=$(printf '\033[37m')
FMT_BOLD=$(printf '\033[1m')
FMT_RESET=$(printf '\033[0m')

# BASH: Print warning if script is not sourced.
[[ "${BASH_SOURCE[0]}" != "${0}" ]] || echo "${FMT_RED}This script needs to be sourced, not run directly.${FMT_RESET}"

# NOTE: need to source this script from the project dir.
# Determine the script directory:
SCRIPT_DIR="$(pwd)"

# HACK: trim web from end of path if sourcing from that dir:
SCRIPT_DIR=$(echo "$SCRIPT_DIR" | sed 's|/web$||')

# HACK: Don't polute git status with modified .vscode stuff (disable: --no-skip-worktree)
# https://stackoverflow.com/questions/1274057/how-do-i-make-git-forget-about-a-file-that-was-tracked-but-is-now-in-gitignore
git update-index --skip-worktree "$SCRIPT_DIR"/.vscode/*

rm -f "${SCRIPT_DIR}/.eap-install.cfg"

if ! command -v jq >/dev/null 2>&1; then
  echo "${FMT_RED}Error: 'jq' is not installed. Please install jq before running this script.${FMT_RESET}"
  exit 1
fi

# Set the device IP and credentials from the following file:
CREDENTIALS_FILE="$SCRIPT_DIR/credentials.json"
DEFAULT_IP="192.168.0.90"
DEFAULT_USR="root"
DEFAULT_PWD="pass"

# Check if the credentials file exists:
if [ -e "$CREDENTIALS_FILE" ]; then
  TARGET_IP=$(jq -r '.TARGET_IP' "$CREDENTIALS_FILE")
  TARGET_USR=$(jq -r '.TARGET_USR' "$CREDENTIALS_FILE")
  TARGET_PWD=$(jq -r '.TARGET_PWD' "$CREDENTIALS_FILE")
else
  # Set default values if the file doesn't exist:
  TARGET_IP="$DEFAULT_IP"
  TARGET_USR="$DEFAULT_USR"
  TARGET_PWD="$DEFAULT_PWD"
  # Create the credentials file with default values:
  echo '{"TARGET_IP": "'"$TARGET_IP"'", "TARGET_USR": "'"$TARGET_USR"'", "TARGET_PWD": "'"$TARGET_PWD"'"}' >"$CREDENTIALS_FILE"
fi

# Export credentials:
export TARGET_IP
export TARGET_USR
export TARGET_PWD

package_conf_file="$SCRIPT_DIR/package.conf"
packagename=$(grep '^PACKAGENAME=' "$package_conf_file" | awk -F'=' '{print $2}' | tr -d '"')

echo "${FMT_BOLD}${FMT_GREEN}*** ACAP project $packagename for ""${FMT_WHITE}""$TARGET_IP${FMT_GREEN}" initialized"${FMT_RESET}"
echo "*** Run 'make help' to get started"
