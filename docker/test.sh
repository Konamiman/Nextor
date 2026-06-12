#!/bin/sh
# Acceptance test for the Nextor dev image. Runs a set of assertions against a
# built image WITHOUT needing a registry: build locally, then point this at the
# tag. The same script is the "test before publish" gate in CI.
#
#   docker/test.sh [image]
#
# Image resolution (same as docker/make.sh): the [image] argument, else
# $NEXTOR_IMAGE, else a local 'nextor-dev' build, else the latest official image.
set -eu

if [ -n "${1:-}" ]; then
	IMAGE=$1
elif [ -n "${NEXTOR_IMAGE:-}" ]; then
	IMAGE=$NEXTOR_IMAGE
elif docker image inspect nextor-dev >/dev/null 2>&1; then
	IMAGE=nextor-dev
else
	IMAGE=ghcr.io/konamiman/nextor-dev:latest
fi
echo "==> Testing image: $IMAGE"

# The assertions run inside the container as a login shell (bash -ls), which
# also verifies the on-PATH setup that login shells get via /etc/profile.d.
docker run --rm -i "$IMAGE" bash -ls <<'INNER'
set -u
fails=0
t() { # t "description" 'shell command'
	desc=$1
	if eval "$2" >/tmp/_t.out 2>&1; then
		printf 'PASS  %s\n' "$desc"
	else
		printf 'FAIL  %s\n' "$desc"
		sed 's/^/        /' /tmp/_t.out
		fails=$((fails + 1))
	fi
}

# --- tools present and runnable -------------------------------------------
t "N80 reports a version"      'N80 --version'
t "N80 -> LK80 round-trip"     'printf "\tdb 1,2,3\n\tend\n" > /tmp/s.asm && N80 /tmp/s.asm /tmp/s.rel --build-type rel --no-show-banner && LK80 --output-file /tmp/s.bin --output-format bin /tmp/s.rel --no-show-banner && test -s /tmp/s.bin'
t "LB80 on PATH"               'command -v LB80'
t "sdcc compiles for z80"      'printf "unsigned char m(unsigned char x){return x+1;}\n" > /tmp/t.c && sdcc -mz80 -c -o /tmp/t.rel /tmp/t.c'
t "mknexrom on PATH"           'command -v mknexrom'
t "make on PATH"               'command -v make'
t "objcopy on PATH"            'command -v objcopy'

# --- baked content --------------------------------------------------------
t "NEXTOR_BASE file exists"    'test -f "$NEXTOR_BASE"'
t "SDK present"                'test -d "$NEXTOR_SDK/asm" && test -d "$NEXTOR_SDK/C"'
t "6 kernel base variants"     'test "$(ls /opt/nextor/kernel_base/kernel_base*.dat | wc -l)" -eq 6'
t "env version = built kernel" '[ -n "$NEXTOR_VERSION" ] && [ "$NEXTOR_VERSION" = "$(cat "$NEXTOR_SDK/nextor-kernel-version.txt")" ]'
t "manifest matches version"   '[ "$NEXTOR_VERSION" = "$(grep -o "\"kernel_version\": \"[^\"]*\"" /opt/nextor/manifest.json | cut -d\" -f4)" ]'

# --- project templates end-to-end (copied verbatim, then make) ------------
t "driver template -> ROM" 'cd /tmp && rm -rf _d && cp -R "$NEXTOR_SDK/templates/driver" _d && cd _d && make >/dev/null && test -s mydriver.ROM'
t "tool template -> COM"   'cd /tmp && rm -rf _t && cp -R "$NEXTOR_SDK/templates/tool" _t && cd _t && make >/dev/null && test -s mytool.COM'

echo "------------------------------------------------------------"
if [ "$fails" -eq 0 ]; then
	echo "All checks passed."
else
	echo "$fails check(s) FAILED."
fi
exit "$fails"
INNER
