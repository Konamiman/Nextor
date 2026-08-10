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

  $prog kernel              build the default kernel base file
  $prog kernel everything   all six kernel variants
  $prog drivers             the standalone ROMs (ASCII8 + ASCII16)
  $prog drivers ram-example the opt-in example RAM disk driver (.drv)
  $prog nextor_sys          build NEXTOR.SYS
  $prog tools               all command-line tools (assembler + C)
  $prog tools/C             just the C tools
  $prog all                 one variant of every part (umbrella Makefile)
  $prog all everything      the FULL matrix: every kernel + standalone-ROM
                            variant, plus NEXTOR.SYS and all tools
  $prog all tools-disk      NEXTOR.SYS + all tools, packed into the
                            bin/tools/nextor.dsk disk image
  $prog all clean           remove source-tree intermediates
  $prog all distclean       clear everything, including bin/ outputs

Parts (dirs under source/): kernel, drivers, nextor_sys, tools, tools/C.
'tools' builds source/tools AND source/tools/C; 'tools/C' builds only the latter.
'all' is the umbrella; pass it a target ('everything', 'tools-disk', 'clean',
'distclean').
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
# Map the part to one or more make directories under source/. 'tools' covers
# both the assembler tools (source/tools) and the C tools (source/tools/C),
# which have separate Makefiles; pass 'tools/C' to build just the C ones.
case "$part" in
	all|.|source) dirs="source" ;;
	tools)        dirs="source/tools source/tools/C" ;;
	*)            dirs="source/$part" ;;
esac

cd "$(dirname "$0")/.."
for d in $dirs; do
	if [ ! -d "$d" ]; then
		echo "$prog: no such part '$part' (looked for '$d/')" >&2
		echo "  parts: kernel, drivers, nextor_sys, tools, tools/C   (or 'all' for everything)" >&2
		exit 2
	fi
done

# Pick the image: explicit override, else a local build, else official latest.
if [ -n "${NEXTOR_IMAGE:-}" ]; then
	image=$NEXTOR_IMAGE
elif docker image inspect nextor-dev >/dev/null 2>&1; then
	image=nextor-dev
else
	image=ghcr.io/konamiman/nextor-dev:latest
fi

echo "$prog: using image '$image'" >&2
# The image bakes NEXTOR_BASE (its own kernel base) for driver/tool users.
# UNSET it for from-source builds so the drivers build the kernel base from the
# mounted sources. It must be *unset*, not merely emptied: an empty environment
# NEXTOR_BASE still has make origin "environment", so the drivers Makefile's
# internal `NEXTOR_BASE := <variant base>` gets re-exported to its per-variant
# sub-makes and breaks the matrix. Unsetting makes it origin "undefined", which
# make does not export. (Docker has no --unset-env, hence the sh wrapper.)
for d in $dirs; do
	docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/work "$image" \
		sh -c 'unset NEXTOR_BASE; exec make -C "$0" "$@"' "$d" "$@"
done
