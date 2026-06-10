#!/bin/sh
# docker/build.sh - build the Nextor dev image with NEXTOR_VERSION filled in
# from sdk/nextor-kernel-version.txt (the authoritative kernel version), so you
# don't have to pass it by hand.
#
# Usage: docker/build.sh [docker-build-args...]
#
# With no -t/--tag the image is tagged 'nextor-dev'. Any extra arguments pass
# straight through to 'docker build', for example:
#   docker/build.sh
#   docker/build.sh -t ghcr.io/me/nextor-dev:3.0.0-beta1
#   docker/build.sh --no-cache --build-arg SDCC_VERSION=4.4.0
set -eu

usage() { sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'; }
case "${1:-}" in -h|--help) usage; exit 0 ;; esac

cd "$(dirname "$0")/.."
ver="$(cat sdk/nextor-kernel-version.txt)"

# Default the image tag unless the caller supplies their own -t/--tag.
case " $* " in
	*" -t "*|*" --tag "*) tag_args= ;;
	*)                    tag_args="-t nextor-dev" ;;
esac

echo "Building Nextor dev image (NEXTOR_VERSION=$ver)" >&2
# shellcheck disable=SC2086  # tag_args is intentionally word-split
exec docker build -f docker/Dockerfile \
	--build-arg NEXTOR_VERSION="$ver" \
	$tag_args "$@" .
