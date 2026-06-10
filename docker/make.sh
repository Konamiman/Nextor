#!/bin/sh
# docker/make.sh - build a Nextor source part inside the dev image, with the
# repository root mounted correctly. The per-part Makefiles use repo-relative
# paths (../version.mk, ../../sdk, ../../bin), so mounting a subdirectory fails;
# this wrapper always mounts the root and runs `make -C source/<part>`.
set -eu

prog=$(basename "$0")
usage() {
	cat <<EOF
$prog - build a Nextor source part inside the dev image (repo root mounted).

Usage: $prog <part> [make-args...]

  $prog kernel              build the kernel base file
  $prog kernel everything   all six kernel variants
  $prog kernel clean
  $prog tools
  $prog all                 umbrella Makefile (builds every part)

Parts (dirs under source/): kernel, drivers, nextor_sys, tools, tools/C.
Outputs are written back as your user (via --user), not root.

Image resolution: \$NEXTOR_IMAGE if set, else a local 'nextor-dev' build if
present, else the latest official image (ghcr.io/konamiman/nextor-dev:latest),
which Docker pulls on first use.
EOF
}

if [ $# -eq 0 ]; then usage >&2; exit 2; fi
case "$1" in -h|--help) usage; exit 0 ;; esac

part=$1
shift
case "$part" in
	all|.|source) dir=source ;;
	*)            dir="source/$part" ;;
esac

cd "$(dirname "$0")/.."
if [ ! -d "$dir" ]; then
	echo "$prog: no such part '$part' (looked for '$dir/')" >&2
	echo "  parts: kernel, drivers, nextor_sys, tools, tools/C   (or 'all' for everything)" >&2
	exit 2
fi

# Pick the image: explicit override, else a local build, else official latest.
if [ -n "${NEXTOR_IMAGE:-}" ]; then
	image=$NEXTOR_IMAGE
elif docker image inspect nextor-dev >/dev/null 2>&1; then
	image=nextor-dev
else
	image=ghcr.io/konamiman/nextor-dev:latest
fi

echo "$prog: using image '$image'" >&2
exec docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/work "$image" \
	make -C "$dir" "$@"
