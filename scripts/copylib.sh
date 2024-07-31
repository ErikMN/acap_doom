#!/usr/bin/env bash
#
# Copy multiple files or directories from Docker to the host
#
set -e

# Print colors:
FMT_RED=$(printf '\033[31m')
FMT_GREEN=$(printf '\033[32m')
FMT_YELLOW=$(printf '\033[33m')
FMT_BOLD=$(printf '\033[1m')
FMT_RESET=$(printf '\033[0m')

# Check if at least IMAGE_NAME and one LIB_NAME are provided as arguments:
if [ $# -lt 2 ]; then
  echo "${FMT_RED}Error: Not enough arguments provided.${FMT_RESET}"
  echo "Usage: $0 <IMAGE_NAME> <LIB_NAME1> [LIB_NAME2 ... LIB_NAMEn]"
  exit 1
fi

IMAGE_NAME=$1
shift
LIB_NAMES=("$@") # Remaining arguments are file or directory names

# Get the absolute path of the script's directory:
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

# Navigate to the parent directory (one level up):
cd "$SCRIPT_DIR/.." || exit

# Create a temporary container from the specified image:
CONTAINER_ID=$(docker create --name temp_container "$IMAGE_NAME") || {
  echo "${FMT_RED}Error: Failed to create temporary container${FMT_RESET}"
  exit 1
}

# Function to clean up container
cleanup() {
  echo "${FMT_YELLOW}Cleaning up temporary container...${FMT_RESET}"
  docker rm "$CONTAINER_ID" || {
    echo "${FMT_RED}Warning: Failed to remove temporary container${FMT_RESET}"
  }
}

# Ensure the container is removed on script exit:
trap cleanup EXIT

# Copy each specified file or directory from the temporary container to the parent directory:
for LIB_NAME in "${LIB_NAMES[@]}"; do
  # Set the default path if LIB_NAME does not contain a path:
  DEFAULT_PATH="/opt/app"
  if [[ "$LIB_NAME" != */* ]]; then
    LIB_NAME="$DEFAULT_PATH/$LIB_NAME"
  fi
  # Extract the file or directory name from the LIB_NAME path:
  LIB_NAME_BASENAME=$(basename "$LIB_NAME")
  # Check if the specified file or directory already exists in the parent directory:
  if [ ! -e "$LIB_NAME_BASENAME" ]; then
    # Attempt to copy the specified file or directory from the temporary container to the parent directory:
    if docker cp "$CONTAINER_ID:$LIB_NAME" .; then
      echo "${FMT_BOLD}${FMT_GREEN}Successfully copied '$LIB_NAME' to $(pwd) from $IMAGE_NAME${FMT_RESET}"
    else
      echo "${FMT_RED}Error: Failed to copy $LIB_NAME${FMT_RESET}"
      # Continue with the next file or directory
    fi
  else
    echo "${FMT_BOLD}${FMT_YELLOW}The '$LIB_NAME' file or directory already exists in the parent directory.${FMT_RESET}"
  fi
done
