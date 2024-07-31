#!/bin/bash

# Save the current directory to return later:
original_dir=$(pwd)

# Determine the project root directory:
project_root_dir="$(dirname "$0")"

# Source version.sh:
. "${project_root_dir}/version.sh"

# Source setuptarget.sh:
. "${project_root_dir}/../setuptarget.sh"

# Export environment variables required by React:
export VITE_TARGET_IP=$TARGET_IP

# Return to the original directory:
cd "$original_dir"

# Start the React application:
vite
