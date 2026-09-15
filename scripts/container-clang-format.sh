#!/bin/sh
#
# Run a fixed clang-format version in an OCI container
# Requires a .clang-format-container.conf configuration file with:
# SCAN_DIRS="" IGNORE_DIRS="" FILE_PATTERNS=""
#
# container image ls project-clang-format
#
set -eu

CONTAINER_RUNTIME=${CONTAINER_RUNTIME:-docker}
IMAGE_NAME="project-clang-format"
CLANG_VERSION="${CLANG_VERSION:-18}"
FORMAT_BIN="/usr/lib/llvm${CLANG_VERSION}/bin/clang-format"
IMAGE_TAG="${IMAGE_NAME}:${CLANG_VERSION}"

PROJECT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
CONFIG_FILE="${CONFIG_FILE:-$PROJECT_ROOT/.clang-format-container.conf}"

die() {
	printf "%s\n" "$1" >&2
	exit 1
}

check_container_runtime() {
	command -v "$CONTAINER_RUNTIME" >/dev/null 2>&1 ||
		die ""$CONTAINER_RUNTIME" is not installed. Please install "$CONTAINER_RUNTIME" first."
}

load_config() {
	if [ -f "$CONFIG_FILE" ]; then
		# shellcheck disable=SC1090
		. "$CONFIG_FILE"

		[ -n "${SCAN_DIRS:-}" ] || die "SCAN_DIRS is empty in $CONFIG_FILE"
		[ -n "${FILE_PATTERNS:-}" ] || die "FILE_PATTERNS is empty in $CONFIG_FILE"
		return
	fi

	die "Missing config file: $CONFIG_FILE"
}

ensure_image() {
	if "$CONTAINER_RUNTIME" image inspect "$IMAGE_TAG" >/dev/null 2>&1; then
		return
	fi

	printf "Building clang-format $CONTAINER_RUNTIME image version %s\n" "$CLANG_VERSION"

	"$CONTAINER_RUNTIME" build -t "$IMAGE_TAG" - <<EOF
FROM alpine:3.20
RUN apk add --no-cache clang${CLANG_VERSION}-extra-tools
WORKDIR /workspace
EOF
}

container_run() {
	"$CONTAINER_RUNTIME" run --rm \
		-u "$(id -u):$(id -g)" \
		-v "$PROJECT_ROOT:$PROJECT_ROOT" \
		-w "$PROJECT_ROOT" \
		"$IMAGE_TAG" \
		"$@"
}

container_format() {
	"$CONTAINER_RUNTIME"_run \
		"$FORMAT_BIN" \
		-style=file \
		-i \
		-fallback-style=none \
		"$@"
}

format_tree() {
	set -- find

	found_scan_dir=0
	for dir in $SCAN_DIRS; do
		if [ ! -d "$dir" ]; then
			printf "Skipping missing scan dir: %s\n" "$dir" >&2
			continue
		fi

		found_scan_dir=1
		set -- "$@" "$dir"
	done

	[ "$found_scan_dir" -eq 1 ] || return

	if [ -n "${IGNORE_DIRS:-}" ]; then
		set -- "$@" \(
		first=1
		for dir in $IGNORE_DIRS; do
			if [ "$first" -eq 1 ]; then
				first=0
			else
				set -- "$@" -o
			fi
			set -- "$@" \( -path "$dir" -o -path "$dir/*" \)
		done
		set -- "$@" \) -prune -o
	fi

	set -- "$@" -type f \(

	first=1
	for pattern in $FILE_PATTERNS; do
		if [ "$first" -eq 1 ]; then
			first=0
		else
			set -- "$@" -o
		fi
		set -- "$@" -name "$pattern"
	done

	set -- "$@" \) \
		-exec "$FORMAT_BIN" -style=file -i -fallback-style=none {} +

	container_run "$@"
}

main() {
	check_container_runtime
	load_config
	ensure_image

	if [ "$#" -eq 0 ]; then
		format_tree
		return
	fi

	container_format "$@"
}

main "$@"
