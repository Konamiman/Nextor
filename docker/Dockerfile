# syntax=docker/dockerfile:1
#
# Nextor development image -- multi-arch (linux/amd64, linux/arm64).
#
# Build with the wrapper (fills in NEXTOR_VERSION, builds your native arch):
#   docker/build.sh
# Multi-arch images are produced by .github/workflows/publish.yaml (buildx +
# QEMU, `--platform linux/amd64,linux/arm64 --push`).
#
# Contains the modern Nextor toolchain (N80/LK80/LB80, SDCC, mknexrom, make,
# binutils, mtools, zip), the baked SDK (including the driver/tool project
# templates under sdk/templates), and the twelve kernel base-file variants built
# from this repository's source.
#
# Nestor80 is the *framework-dependent* build (downloaded per target arch), so
# the tools use the .NET runtime from the dotnet/runtime base. They target
# .NET 6, so DOTNET_ROLL_FORWARD lets them run on this image's newer runtime.
# SDCC is Debian's apt package (4.2.0; the +dfsg repackaging already omits the
# non-free PIC support, and it is multi-arch).
#
# Multi-arch strategy: the kernel base files are Z80 binaries -- architecture
# independent -- so they are built ONCE on the build host's native arch
# (`kernelbuild`, pinned to $BUILDPLATFORM, no emulation) and copied into every
# target image. Only the shipped *host* tools (N80/LK80/LB80/sdcc/mknexrom) are
# per-target-arch.

ARG DOTNET_TAG=8.0-bookworm-slim
# Nestor80 release tags follow <tool>-v<version> (n80-v1.3.6, lk80-v1.1.1,
# lb80-v1.0), so the download URLs are derived from the versions alone.
# These three lines are the single source of truth for the Nestor80 versions:
# .github/workflows/ci.yaml parses them (expects exactly "ARG NAME=value"), so
# keep that form. SDCC is the exception -- see the comment in ci.yaml.
ARG N80_VERSION=1.3.6
ARG LK80_VERSION=1.1.1
ARG LB80_VERSION=1.0
ARG SDCC_VERSION=4.2.0
ARG MKNEXROM_VERSION=1.2
ARG NEXTOR_VERSION=unknown
# Build revision: bumped when the toolchain changes but the kernel doesn't.
# CI passes the real "rN" ("rN-test" for a test publish); local builds get "dev".
ARG NEXTOR_IMAGE_REVISION=dev

# ---------------------------------------------------------------------------
# Stage: kernelbuild -- on the BUILD host's arch (no emulation). Produces the
# architecture-independent kernel base files, shared by every target image.
# ---------------------------------------------------------------------------
FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/runtime:${DOTNET_TAG} AS kernelbuild
ARG N80_VERSION LK80_VERSION LB80_VERSION BUILDARCH

RUN apt-get update && apt-get install -y --no-install-recommends \
        curl ca-certificates unzip make binutils sdcc \
    && rm -rf /var/lib/apt/lists/*
ENV DOTNET_ROLL_FORWARD=LatestMajor

# Nestor80 for the BUILD arch (these run here to assemble/link the kernel).
RUN set -eux; \
    case "$BUILDARCH" in \
      amd64) rid=linux-x64 ;; arm64) rid=linux-arm64 ;; arm) rid=linux-arm ;; \
      *) echo "unsupported BUILDARCH=$BUILDARCH" >&2; exit 1 ;; \
    esac; \
    base=https://github.com/Konamiman/Nestor80/releases/download; \
    mkdir -p /opt/nextor/bin; \
    curl -fsSL --retry 3 -o /tmp/n80.zip  "$base/n80-v${N80_VERSION}/N80_${N80_VERSION}_FrameworkDependant_${rid}.zip"; \
    curl -fsSL --retry 3 -o /tmp/lk80.zip "$base/lk80-v${LK80_VERSION}/LK80_${LK80_VERSION}_FrameworkDependant_${rid}.zip"; \
    curl -fsSL --retry 3 -o /tmp/lb80.zip "$base/lb80-v${LB80_VERSION}/LB80_${LB80_VERSION}_FrameworkDependant_${rid}.zip"; \
    unzip -o -j /tmp/n80.zip  N80  -d /opt/nextor/bin; \
    unzip -o -j /tmp/lk80.zip LK80 -d /opt/nextor/bin; \
    unzip -o -j /tmp/lb80.zip LB80 -d /opt/nextor/bin; \
    chmod +x /opt/nextor/bin/N80 /opt/nextor/bin/LK80 /opt/nextor/bin/LB80; \
    rm -f /tmp/*.zip
ENV PATH="/opt/nextor/bin:${PATH}"

# Fail fast that the tools run before the multi-minute kernel build.
RUN set -eux; \
    printf '\tdb 1,2,3\n\tend\n' > /tmp/smoke.asm; \
    N80  /tmp/smoke.asm /tmp/smoke.rel --build-type rel --no-show-banner; \
    LK80 --output-file /tmp/smoke.bin --output-format bin /tmp/smoke.rel --no-show-banner; \
    rm -f /tmp/smoke.*

COPY source /src/source
COPY sdk    /src/sdk
RUN make -C /src/source/kernel everything

# Stage the kernel_base/ layout: Nextor-<ver>.base<suffix>.dat -> kernel_base<suffix>.dat.
RUN set -eux; \
    mkdir -p /staging/kernel_base; \
    for f in /src/bin/kernel-base/Nextor-*.base*.dat; do \
        s="$(basename "$f")"; s="${s#Nextor-*.base}"; \
        cp "$f" "/staging/kernel_base/kernel_base${s}"; \
    done; \
    ls -la /staging/kernel_base

# ---------------------------------------------------------------------------
# Stage: tools -- per TARGET arch. Builds the host tools SHIPPED in the image:
# the target-arch Nestor80 binaries and a target-arch mknexrom.
# ---------------------------------------------------------------------------
FROM mcr.microsoft.com/dotnet/runtime:${DOTNET_TAG} AS tools
ARG N80_VERSION LK80_VERSION LB80_VERSION TARGETARCH

RUN apt-get update && apt-get install -y --no-install-recommends \
        curl ca-certificates unzip gcc libc6-dev \
    && rm -rf /var/lib/apt/lists/*

# Nestor80 for the TARGET arch (shipped to users running that arch).
RUN set -eux; \
    case "$TARGETARCH" in \
      amd64) rid=linux-x64 ;; arm64) rid=linux-arm64 ;; arm) rid=linux-arm ;; \
      *) echo "unsupported TARGETARCH=$TARGETARCH" >&2; exit 1 ;; \
    esac; \
    base=https://github.com/Konamiman/Nestor80/releases/download; \
    mkdir -p /opt/nextor/bin; \
    curl -fsSL --retry 3 -o /tmp/n80.zip  "$base/n80-v${N80_VERSION}/N80_${N80_VERSION}_FrameworkDependant_${rid}.zip"; \
    curl -fsSL --retry 3 -o /tmp/lk80.zip "$base/lk80-v${LK80_VERSION}/LK80_${LK80_VERSION}_FrameworkDependant_${rid}.zip"; \
    curl -fsSL --retry 3 -o /tmp/lb80.zip "$base/lb80-v${LB80_VERSION}/LB80_${LB80_VERSION}_FrameworkDependant_${rid}.zip"; \
    unzip -o -j /tmp/n80.zip  N80  -d /opt/nextor/bin; \
    unzip -o -j /tmp/lk80.zip LK80 -d /opt/nextor/bin; \
    unzip -o -j /tmp/lb80.zip LB80 -d /opt/nextor/bin; \
    chmod +x /opt/nextor/bin/N80 /opt/nextor/bin/LK80 /opt/nextor/bin/LB80; \
    rm -f /tmp/*.zip

# mknexrom: compiled from this repository's source for the target arch.
COPY buildtools/sources/mknexrom.c /tmp/mknexrom.c
RUN gcc -O2 -o /opt/nextor/bin/mknexrom /tmp/mknexrom.c && rm -f /tmp/mknexrom.c

# ---------------------------------------------------------------------------
# Stage: runtime -- the final per-target-arch image.
# ---------------------------------------------------------------------------
FROM mcr.microsoft.com/dotnet/runtime:${DOTNET_TAG} AS runtime
ARG NEXTOR_VERSION N80_VERSION LK80_VERSION LB80_VERSION SDCC_VERSION \
    MKNEXROM_VERSION NEXTOR_IMAGE_REVISION

# Nestor80 targets .NET 6; roll forward onto this image's .NET runtime.
ENV DOTNET_ROLL_FORWARD=LatestMajor

# make drives the build, binutils provides objcopy (ihex->bin in the kernel
# Makefile), bash is the shell, sdcc is Debian's 4.2.0 (multi-arch; the +dfsg
# packaging already omits the non-free PIC support), mtools provides
# mformat/mcopy (the tools-disk image target), zip serves the tools-zip
# target. No host C compiler: mknexrom is shipped prebuilt and Z80 C goes
# through sdcc.
RUN apt-get update && apt-get install -y --no-install-recommends \
        make binutils bash sdcc mtools zip \
    && rm -rf /var/lib/apt/lists/*

# Keep the toolchain on PATH for login shells too (Debian's /etc/profile resets
# PATH, which would otherwise drop the ENV PATH set below). sdcc lives in /usr/bin.
RUN printf 'export PATH="/opt/nextor/bin:$PATH"\n' > /etc/profile.d/nextor.sh

COPY --from=tools       /opt/nextor/bin       /opt/nextor/bin
COPY --from=kernelbuild /staging/kernel_base  /opt/nextor/kernel_base
# The SDK includes the driver/tool project templates (sdk/templates/), so
# they are available in the image at $NEXTOR_SDK/templates.
COPY sdk                                       /opt/nextor/sdk

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
  "base_variants": ["default", "NO_UNDOC", "SHIFT_INV", "CTRL_INV", "NO_UNDOC.SHIFT_INV", "NO_UNDOC.CTRL_INV", "KANJI_INV", "NO_UNDOC.KANJI_INV", "SHIFT_INV.KANJI_INV", "CTRL_INV.KANJI_INV", "NO_UNDOC.SHIFT_INV.KANJI_INV", "NO_UNDOC.CTRL_INV.KANJI_INV"]
}
EOF

ENV NEXTOR_VERSION=${NEXTOR_VERSION} \
    NEXTOR_KERNEL_BASE_DIR=/opt/nextor/kernel_base \
    NEXTOR_BASE=/opt/nextor/kernel_base/kernel_base.dat \
    NEXTOR_SDK=/opt/nextor/sdk \
    NEXTOR_SDK_ASM=/opt/nextor/sdk/asm \
    NEXTOR_SDK_C=/opt/nextor/sdk/C \
    N80=/opt/nextor/bin/N80 \
    LK80=/opt/nextor/bin/LK80 \
    LB80=/opt/nextor/bin/LB80 \
    MKNEXROM=/opt/nextor/bin/mknexrom \
    SDCC=/usr/bin/sdcc \
    PATH=/opt/nextor/bin:/usr/local/bin:/usr/bin:/bin

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
      org.nextor.base-variants="default,NO_UNDOC,SHIFT_INV,CTRL_INV,NO_UNDOC.SHIFT_INV,NO_UNDOC.CTRL_INV,KANJI_INV,NO_UNDOC.KANJI_INV,SHIFT_INV.KANJI_INV,CTRL_INV.KANJI_INV,NO_UNDOC.SHIFT_INV.KANJI_INV,NO_UNDOC.CTRL_INV.KANJI_INV"

WORKDIR /work
CMD ["bash"]
