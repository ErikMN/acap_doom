#!/usr/bin/bash
# SOURCE ME!
#
# Setup target device credentials and git hooks
# NOTE: Don't set -e to avoid exiting the shell when sourced
#
# Format with:
# shfmt -i 2 -w setuptarget.sh
#
# Lint with: shellcheck setuptarget.sh
#

# Print colors:
FMT_RED=$(printf '\033[31m')
FMT_GREEN=$(printf '\033[32m')
FMT_BLUE=$(printf '\033[34m')
FMT_YELLOW=$(printf '\033[33m')
FMT_WHITE=$(printf '\033[37m')
FMT_BOLD=$(printf '\033[1m')
FMT_RESET=$(printf '\033[0m')

# BASH: Print warning if script is not sourced then exit.
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  echo "${FMT_RED}ERROR: This Bash script needs to be sourced, not run directly.${FMT_RESET}"
  exit 1
fi

# NOTE: need to source this script from the project dir.
# BASH: Determine the script directory:
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# BASH: HACK: trim web from end of path if sourcing from that dir:
SCRIPT_DIR=${SCRIPT_DIR%/web}

# HACK: Don't pollute git status with modified .vscode stuff (disable: --no-skip-worktree)
# https://stackoverflow.com/questions/1274057/how-do-i-make-git-forget-about-a-file-that-was-tracked-but-is-now-in-gitignore
# Safely mark .vscode files as skip-worktree (only if they are tracked)
if [ -d "$SCRIPT_DIR/.vscode" ]; then
  for file in "$SCRIPT_DIR"/.vscode/*; do
    if git ls-files --error-unmatch "$file" >/dev/null 2>&1; then
      git update-index --skip-worktree "$file" || echo "${FMT_YELLOW}WARNING: Failed to mark $file as skip-worktree${FMT_RESET}"
    else
      echo "${FMT_WHITE}INFO: Skipping untracked file $file${FMT_RESET}"
    fi
  done
fi

# Set the git hooks path:
git config core.hooksPath "$SCRIPT_DIR/hooks"

# Remove old .eap-install.cfg file if it exists:
rm -f "${SCRIPT_DIR}/.eap-install.cfg"

if ! command -v jq >/dev/null 2>&1; then
  echo "${FMT_RED}ERROR: 'jq' is not installed. Please install jq before running this Bash script.${FMT_RESET}"
  exit 1
fi

# Set the device IP and credentials from the following file:
if [ -n "$1" ]; then
  if [ -f "$1" ]; then
    CREDENTIALS_FILE="$1"
  elif [ -f "$SCRIPT_DIR/$1" ]; then
    CREDENTIALS_FILE="$SCRIPT_DIR/$1"
  else
    echo "${FMT_YELLOW}WARNING: Credentials file '$1' not found. Falling back to default.${FMT_RESET}"
    CREDENTIALS_FILE="$SCRIPT_DIR/credentials.json"
  fi
else
  CREDENTIALS_FILE="$SCRIPT_DIR/credentials.json"
fi

# Default credentials (use HTTPS):
DEFAULT_IP="192.168.0.90"
DEFAULT_USR="root"
DEFAULT_PWD="pass"
DEFAULT_PORT="443"
DEFAULT_SSH_PORT="22"

# Check if the credentials file exists:
if [ -e "$CREDENTIALS_FILE" ]; then
  TARGET_IP=$(jq -r '.TARGET_IP // empty' "$CREDENTIALS_FILE")
  TARGET_USR=$(jq -r '.TARGET_USR // empty' "$CREDENTIALS_FILE")
  TARGET_PWD=$(jq -r '.TARGET_PWD // empty' "$CREDENTIALS_FILE")
  TARGET_PORT=$(jq -r '.TARGET_PORT // empty' "$CREDENTIALS_FILE")
  TARGET_SSH_PORT=$(jq -r '.TARGET_SSH_PORT // empty' "$CREDENTIALS_FILE")
else
  echo "${FMT_YELLOW}WARNING: No credentials.json file found: setting default values${FMT_RESET}"
  # Set default values if the file doesn't exist:
  TARGET_IP="$DEFAULT_IP"
  TARGET_USR="$DEFAULT_USR"
  TARGET_PWD="$DEFAULT_PWD"
  TARGET_PORT="$DEFAULT_PORT"
  TARGET_SSH_PORT="$DEFAULT_SSH_PORT"
  # Create the credentials file with default values:
  cat >"$CREDENTIALS_FILE" <<EOF
{
  "TARGET_IP": "$TARGET_IP",
  "TARGET_USR": "$TARGET_USR",
  "TARGET_PWD": "$TARGET_PWD",
  "TARGET_PORT": "$TARGET_PORT",
  "TARGET_SSH_PORT": "$TARGET_SSH_PORT"
}
EOF
fi

# BASH: Default values if not provided in credentials file:
if [ -z "$TARGET_IP" ] || [ "$TARGET_IP" = "null" ]; then
  TARGET_IP="$DEFAULT_IP"
fi
if [ -z "$TARGET_USR" ] || [ "$TARGET_USR" = "null" ]; then
  TARGET_USR="$DEFAULT_USR"
fi
if [ -z "$TARGET_PWD" ] || [ "$TARGET_PWD" = "null" ]; then
  TARGET_PWD="$DEFAULT_PWD"
fi
if [ -z "$TARGET_PORT" ] || [ "$TARGET_PORT" = "null" ]; then
  TARGET_PORT="$DEFAULT_PORT"
fi
if [ -z "$TARGET_SSH_PORT" ] || [ "$TARGET_SSH_PORT" = "null" ]; then
  TARGET_SSH_PORT="$DEFAULT_SSH_PORT"
fi

# Export credentials:
export TARGET_IP
export TARGET_USR
export TARGET_PWD
export TARGET_PORT
export TARGET_SSH_PORT

# Read packagename from manifest.json:
manifest_file="$SCRIPT_DIR/manifest.json"
if [ -f "$manifest_file" ]; then
  packagename=$(jq -r '.acapPackageConf.setup.friendlyName // "(unknown)"' "$manifest_file")
else
  packagename="(unknown)"
fi

# Success: Print info and exit:
echo "${FMT_BOLD}${FMT_GREEN}*** ACAP project $packagename for ""${FMT_WHITE}""$TARGET_IP${FMT_GREEN}" initialized"${FMT_RESET}"
echo "${FMT_BLUE}*** Credentials exported${FMT_RESET}"
echo "*** Run 'make help' to get started"
