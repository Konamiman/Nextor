#!/bin/sh
# Builds a dev build locally: the same files that the dev build workflow
# (.github/workflows/dev-build.yaml) publishes, with the kernel showing
# "Dev build <date> (<commit>)" when booting. <date> (in UTC) and <commit>
# are those of the last commit in the current branch, so local changes that
# aren't committed yet aren't reflected in the banner.
#
# The files are generated in the usual places under bin/ and with the usual
# names (the workflow is what adds "dev-<date>" to the names when publishing):
#
#   bin/kernel-base   The kernel base files, both variants
#   bin/drivers       The standalone ROMs, both variants, and the example RAM driver
#   bin/tools         The tools disk image (nextor.dsk) and zip file (tools.zip)
#
# Requires git and the tools needed to build Nextor (see "How to build Nextor"
# in the README file at the root of the repository). A later regular build
# removes the banner line.

set -eu

repo_root="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$repo_root"

build_id="$(TZ=UTC git log -1 --abbrev=7 --date=format-local:%Y-%m-%d --format='%cd (%h)')"
echo "Building dev build $build_id"

make -C source everything DEV_BUILD="$build_id"
make -C source/drivers ram-example   # Not built by "everything"
make -C source tools-all             # The tools disk image and zip file

echo
echo "Dev build $build_id generated in bin/kernel-base, bin/drivers and bin/tools"
