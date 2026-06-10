# syntax=docker/dockerfile:1
#
# Nextor development image  --  first increment: linux/amd64 only.
#
# Build with the wrapper (fills in NEXTOR_VERSION for you):
#   docker/build.sh
# or explicitly from the repository root:
#   docker build -f docker/Dockerfile \
#     --build-arg NEXTOR_VERSION="$(cat sdk/nextor-kernel-version.txt)" \
#     -t nextor-dev .
#
# Contains the modern Nextor toolchain (N80/LK80/LB80, SDCC pruned to the Z80
# world, mknexrom, make, binutils), the baked SDK, the six kernel base-file
# variants built from this repository's source, and the `nextor-init` project
# scaffolding command.
#
# Nestor80 is taken as the *framework-dependent* build, so the tools use the
# .NET runtime from the dotnet/runtime base (small binaries, no bundled
# runtime). They target .NET 6, so DOTNET_ROLL_FORWARD lets them run on this
# image's newer, still-supported .NET runtime.

ARG DOTNET_TAG=8.0-bookworm-slim
ARG N80_VERSION=1.3.5
ARG LK80_TAG=n80-v1.3.3-lk80-v1.1
ARG LK80_VERSION=1.1.0
ARG LB80_VERSION=1.0
ARG SDCC_VERSION=4.2.0
ARG MKNEXROM_VERSION=1.1
ARG NEXTOR_VERSION=unknown
# Build revision: bumped when the toolchain changes but the kernel doesn't.
# CI passes the real "rN"; local builds get "dev".
ARG NEXTOR_IMAGE_REVISION=dev

# ---------------------------------------------------------------------------
# Stage 1: assemble the host-side toolchain
#   /opt/nextor/bin    -> N80, LK80, LB80, mknexrom
#   /opt/nextor/sdcc   -> SDCC install (bin/, lib/, ...)
# ---------------------------------------------------------------------------
FROM mcr.microsoft.com/dotnet/runtime:${DOTNET_TAG} AS toolchain
ARG N80_VERSION LK80_TAG LK80_VERSION LB80_VERSION SDCC_VERSION

RUN apt-get update && apt-get install -y --no-install-recommends \
        curl ca-certificates unzip bzip2 gcc libc6-dev \
    && rm -rf /var/lib/apt/lists/*

# Nestor80: framework-dependent linux-x64 builds (small; use the base's .NET).
RUN set -eux; \
    base=https://github.com/Konamiman/Nestor80/releases/download; \
    mkdir -p /opt/nextor/bin; \
    curl -fsSL --retry 3 -o /tmp/n80.zip  "$base/n80-v${N80_VERSION}/N80_${N80_VERSION}_FrameworkDependant_linux-x64.zip"; \
    curl -fsSL --retry 3 -o /tmp/lk80.zip "$base/${LK80_TAG}/LK80_${LK80_VERSION}_FrameworkDependant_linux-x64.zip"; \
    curl -fsSL --retry 3 -o /tmp/lb80.zip "$base/lb80-v${LB80_VERSION}/LB80_${LB80_VERSION}_FrameworkDependant_linux-x64.zip"; \
    unzip -o -j /tmp/n80.zip  N80  -d /opt/nextor/bin; \
    unzip -o -j /tmp/lk80.zip LK80 -d /opt/nextor/bin; \
    unzip -o -j /tmp/lb80.zip LB80 -d /opt/nextor/bin; \
    chmod +x /opt/nextor/bin/N80 /opt/nextor/bin/LK80 /opt/nextor/bin/LB80; \
    rm -f /tmp/*.zip

# The framework-dependent tools target .NET 6; roll them forward onto this
# image's newer (and still supported) .NET runtime.
ENV DOTNET_ROLL_FORWARD=LatestMajor

# Fail fast: assemble + link a trivial file, so a missing .NET runtime or a
# roll-forward misconfiguration surfaces here in seconds instead of minutes
# into the kernel build. N80 (assemble) + LK80 (link) are a representative
# round-trip; LB80 is exercised later by the kernel build itself.
RUN set -eux; \
    printf '\tdb 1,2,3\n\tend\n' > /tmp/smoke.asm; \
    /opt/nextor/bin/N80  /tmp/smoke.asm /tmp/smoke.rel --build-type rel --no-show-banner; \
    /opt/nextor/bin/LK80 --output-file /tmp/smoke.bin --output-format bin /tmp/smoke.rel --no-show-banner; \
    rm -f /tmp/smoke.*

# SDCC: official linux-amd64 binary tarball (the version the kernel build is
# currently validated against in CI). Installed OUTSIDE bin/ so that a bare
# `sdcc` on PATH does not resolve to a directory. Pruned to the Z80 world:
# the non-free PIC trees, PIC libs and manuals (~210 MB) are dropped, since
# neither Nextor nor any SDCC Z80-family target uses them.
RUN set -eux; \
    curl -fsSL --retry 3 -o /tmp/sdcc.tar.bz2 \
       "https://sourceforge.net/projects/sdcc/files/sdcc-linux-amd64/${SDCC_VERSION}/sdcc-${SDCC_VERSION}-amd64-unknown-linux2.5.tar.bz2/download"; \
    mkdir -p /opt/nextor/sdcc; \
    tar -xjf /tmp/sdcc.tar.bz2 -C /opt/nextor/sdcc --strip-components=1; \
    rm -rf /opt/nextor/sdcc/share/sdcc/non-free \
           /opt/nextor/sdcc/share/sdcc/lib/pic14 \
           /opt/nextor/sdcc/share/sdcc/lib/pic16 \
           /opt/nextor/sdcc/share/sdcc/doc; \
    rm -f /tmp/sdcc.tar.bz2

# mknexrom: compiled from this repository's source.
COPY buildtools/sources/mknexrom.c /tmp/mknexrom.c
RUN gcc -O2 -o /opt/nextor/bin/mknexrom /tmp/mknexrom.c && rm -f /tmp/mknexrom.c

# ---------------------------------------------------------------------------
# Stage 2: build the six kernel base-file variants from source
# ---------------------------------------------------------------------------
FROM toolchain AS kernel
RUN apt-get update && apt-get install -y --no-install-recommends \
        make binutils \
    && rm -rf /var/lib/apt/lists/*
ENV PATH="/opt/nextor/bin:/opt/nextor/sdcc/bin:${PATH}"

COPY source /src/source
COPY sdk    /src/sdk
RUN make -C /src/source/kernel everything

# Stage the image's kernel_base/ layout: rename Nextor-<ver>.base<suffix>.dat
# -> kernel_base<suffix>.dat. (The version isn't duplicated here: it's the
# NEXTOR_VERSION env var, sdk/nextor-kernel-version.txt, and manifest.json.)
RUN set -eux; \
    mkdir -p /staging/kernel_base; \
    for f in /src/bin/kernel-base/Nextor-*.base*.dat; do \
        s="$(basename "$f")"; s="${s#Nextor-*.base}"; \
        cp "$f" "/staging/kernel_base/kernel_base${s}"; \
    done; \
    ls -la /staging/kernel_base

# ---------------------------------------------------------------------------
# Stage 3: final runtime image
# ---------------------------------------------------------------------------
FROM mcr.microsoft.com/dotnet/runtime:${DOTNET_TAG} AS runtime
ARG NEXTOR_VERSION N80_VERSION LK80_VERSION LB80_VERSION SDCC_VERSION \
    MKNEXROM_VERSION NEXTOR_IMAGE_REVISION

# Nestor80 targets .NET 6; roll forward onto this image's .NET runtime.
ENV DOTNET_ROLL_FORWARD=LatestMajor

# make drives the build, binutils provides objcopy (ihex->bin in the kernel
# Makefile), bash is the shell. No host C compiler: mknexrom is shipped
# prebuilt and Z80 C goes through SDCC, so gcc/libc6-dev would be dead weight.
RUN apt-get update && apt-get install -y --no-install-recommends \
        make binutils bash \
    && rm -rf /var/lib/apt/lists/*

# Keep the toolchain on PATH for login shells too (Debian's /etc/profile resets
# PATH, which would otherwise drop the ENV PATH set below).
RUN printf 'export PATH="/opt/nextor/bin:/opt/nextor/sdcc/bin:$PATH"\n' \
        > /etc/profile.d/nextor.sh

COPY --from=toolchain /opt/nextor/bin       /opt/nextor/bin
COPY --from=toolchain /opt/nextor/sdcc      /opt/nextor/sdcc
COPY --from=kernel    /staging/kernel_base  /opt/nextor/kernel_base
COPY sdk                                     /opt/nextor/sdk

# Project scaffolding: the `nextor-init` command and its templates.
COPY docker/templates                        /opt/nextor/templates
COPY --chmod=0755 docker/tools/nextor-init   /opt/nextor/bin/nextor-init

# Structured record of everything baked in, for CI/tooling that wants every
# component version at once (the same data is also exposed as OCI labels below).
RUN cat <<EOF > /opt/nextor/manifest.json
{
  "kernel_version": "${NEXTOR_VERSION}",
  "image_revision": "${NEXTOR_IMAGE_REVISION}",
  "tools": {
    "N80": "${N80_VERSION}",
    "LK80": "${LK80_VERSION}",
    "LB80": "${LB80_VERSION}",
    "sdcc": "${SDCC_VERSION}",
    "mknexrom": "${MKNEXROM_VERSION}"
  },
  "base_variants": ["default", "NO_UNDOC", "SHIFT_INV", "CTRL_INV", "NO_UNDOC.SHIFT_INV", "NO_UNDOC.CTRL_INV"]
}
EOF

ENV NEXTOR_VERSION=${NEXTOR_VERSION} \
    NEXTOR_KERNEL_BASE_DIR=/opt/nextor/kernel_base \
    NEXTOR_BASE=/opt/nextor/kernel_base/kernel_base.dat \
    NEXTOR_SDK=/opt/nextor/sdk \
    NEXTOR_SDK_ASM=/opt/nextor/sdk/asm \
    NEXTOR_SDK_C=/opt/nextor/sdk/C \
    NEXTOR_TEMPLATES=/opt/nextor/templates \
    N80=/opt/nextor/bin/N80 \
    LK80=/opt/nextor/bin/LK80 \
    LB80=/opt/nextor/bin/LB80 \
    MKNEXROM=/opt/nextor/bin/mknexrom \
    SDCC=/opt/nextor/sdcc/bin/sdcc \
    PATH=/opt/nextor/bin:/opt/nextor/sdcc/bin:/usr/local/bin:/usr/bin:/bin

LABEL org.opencontainers.image.title="Nextor dev" \
      org.opencontainers.image.description="Nextor kernel/driver/tool development toolchain" \
      org.opencontainers.image.source="https://github.com/Konamiman/Nextor" \
      org.opencontainers.image.version="${NEXTOR_VERSION}" \
      org.nextor.kernel-version="${NEXTOR_VERSION}" \
      org.nextor.image-revision="${NEXTOR_IMAGE_REVISION}" \
      org.nextor.tool.n80="${N80_VERSION}" \
      org.nextor.tool.lk80="${LK80_VERSION}" \
      org.nextor.tool.lb80="${LB80_VERSION}" \
      org.nextor.tool.sdcc="${SDCC_VERSION}" \
      org.nextor.tool.mknexrom="${MKNEXROM_VERSION}" \
      org.nextor.base-variants="default,NO_UNDOC,SHIFT_INV,CTRL_INV,NO_UNDOC.SHIFT_INV,NO_UNDOC.CTRL_INV"

WORKDIR /work
CMD ["bash"]
