#!/bin/sh
set -eu

cd "$(dirname "$0")/.."
binary=$(mktemp /tmp/acap-doom-controls.XXXXXX)

trap 'rm -f "$binary"' EXIT

${CC:-cc} -std=c11 -Wall -Wextra -Werror -DLINUX -pthread \
  -ffunction-sections -fdata-sections -Wl,--gc-sections -Isrc \
  tests/controls_test.c src/controls.c src/linuxdoom/i_video.c -o "$binary"

"$binary"
