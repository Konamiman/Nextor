# Nextor development image

A Docker image that bundles everything needed to build Nextor, develop Nextor drivers, and develop Nextor-aware tools, all without installing a Z80 toolchain on your host. It contains:

- **[Nestor80](https://github.com/Konamiman/Nestor80)** - `N80` (assembler), `LK80` (linker), `LB80` (library manager).
- **[SDCC](https://sdcc.sourceforge.net/)** - the Z80 C compiler (Debian's `sdcc` 4.2.0).
- **mknexrom** - combines a kernel base file with a driver into a ROM.
- **make**, **binutils** (`objcopy`), **bash**, **mtools** (`mformat`/`mcopy`, used to build the tools disk image).
- The **Nextor SDK** (asm + C, including ready to copy **driver/tool project templates**) and the **six kernel base-file variants** built from this repository's source.

**Platform:** the image is **multi-arch** - `linux/amd64` and `linux/arm64` (native on Apple Silicon and arm64 servers). Everything below assumes you have Docker installed.

> **How this document is organised.** The first three sections cover what most readers come here for: a quick intro to Docker, and how to use the official image to develop drivers, tools, and Nextor itself. The remaining sections are reference material for image internals, building the image yourself, and publishing it - useful for maintainers and advanced users.

---

## What's Docker?

[Docker](https://www.docker.com/) lets you run pre-packaged software in isolated environments called **containers**, so you don't have to install the software (or its dependencies) on your host machine. The package itself is an **image**: a read-only template with everything the software needs (files, libraries, environment variables) baked in. Running an image produces a container; each run is independent, and you can throw it away when you're done.

_If you are familiar with the concept of [virtual machine](https://en.wikipedia.org/wiki/Virtual_machine), you can think of Docker containers as "lightweight virtual machines", where instead of virtualizing the entire hardware you only virtualize the operating system._

For Nextor, this means you don't have to install Nestor80, SDCC, `mknexrom`, or any of the other tools on your host: install Docker, pull this image, and run commands "inside" it. Your project files stay on your host machine; you make them visible to the container by **mounting** a host directory into it - that's what the `-v "$PWD":/work` flag you'll see in every command below does (`$PWD` on the host becomes `/work` inside the container).

_Note: the companion `--rm` flag in these commands is for cleanup: each `docker run` always creates a brand new container regardless, and `--rm` only decides whether the stopped container gets deleted automatically afterwards or sits around in `docker ps -a` until you remove it manually._

If you've never used Docker, the [official "Get started" guide](https://docs.docker.com/get-started/) is a short introduction that covers everything this README assumes. The single command you'll use most is [`docker run`](https://docs.docker.com/reference/cli/docker/container/run/), which creates and starts a container from an image.

### Installing Docker

- **Linux:** follow your distro's instructions, or run `curl -fsSL https://get.docker.com | sh`.
- **macOS / Windows:** install [Docker Desktop](https://www.docker.com/products/docker-desktop/). On Windows, Docker Desktop integrates with the [Windows Subsystem for Linux (WSL)](https://learn.microsoft.com/en-us/windows/wsl/): open a WSL distro (Ubuntu, Debian, ...) and run the `docker` commands in this README from inside it. The *Appendix* at the end walks through a fresh WSL setup if you want to start from scratch.


## Developing drivers and tools

This section assumes you'll use the **official** Nextor development image, published as `ghcr.io/konamiman/nextor-dev`. You don't need to clone the Nextor repository or build the image yourself: Docker pulls it on first use.

### Quick start

Start a new driver project from the template baked into the image and build it into a bootable ROM:

```sh
mkdir my-ide && cd my-ide
docker run --rm -v "$PWD":/work ghcr.io/konamiman/nextor-dev sh -c 'cp -R "$NEXTOR_SDK"/templates/driver/. .'
docker run --rm -v "$PWD":/work ghcr.io/konamiman/nextor-dev make
# -> mydriver.ROM   (rename it via NAME in the Makefile)
```

The first `docker run` pulls the image automatically; subsequent runs reuse the local copy.

A shell alias makes day-to-day use feel native:

```sh
alias nextor='docker run --rm -it -v "$PWD":/work ghcr.io/konamiman/nextor-dev'
nextor make
nextor sh -c 'cp -R "$NEXTOR_SDK"/templates/tool mytool'
```

The rest of this section uses `nextor` for brevity; the full `docker run ...` form does exactly the same thing.


#### Tip: repeated builds

Each `docker run` creates a fresh container, which adds roughly 100–500 ms of startup on top of whatever you're running. That's negligible for a one-off `make`, but it dominates a tight edit/build loop. For those, drop into an interactive shell *once* and run every command inside it; then you pay the startup cost only at the beginning:

```sh
nextor                        # opens bash at /work
# now inside the container:
make
make clean && make
N80 mycode.asm mycode.bin
exit                          # the container is cleaned up on exit
```

The `.devcontainer/` config included in the project templates uses the same idea under the hood: VS Code keeps one long-lived container running and `docker exec`s into it for each build.

A related question: does dropping `--rm` keep the container "live" for reuse? No: without `--rm` the container still stops as soon as its command finishes; it just isn't deleted afterwards, so it sits around in `docker ps -a` taking up space. New `docker run` invocations create new containers either way. The image layers (toolchain, SDK, kernel base files) are what gets reused across runs, and those are cached automatically.

### Working on an existing driver or tool

If the project already exists (cloned from a Git repository, for example), all you need is a checkout and `make`:

```sh
git clone <driver-repo> && cd <driver-repo>
nextor make            # -> <name>.ROM (or <name>.COM for tools)
```

No host toolchain required; the image supplies everything.

### Starting from a project template

The SDK ships two ready-to-copy project templates, baked into the image at `$NEXTOR_SDK/templates` (and living at `sdk/templates/` in the Nextor repository, so they work without Docker too):

| Template | Produces |
|---|---|
| `driver` | a Nextor disk driver, built into a bootable kernel ROM (`driver.asm`, `chgbnk.asm`, `Makefile`, `README.md`, `.devcontainer/`) |
| `tool` | a Nextor-aware `.COM` program in Z80 assembler (`tool.asm`, `Makefile`, `README.md`, `.devcontainer/`) |

You just copy the template directory, follow the `TODO` comments (project name, driver strings, handler bodies, ...), and run `make`. Each template builds as-is before you touch anything, and includes a [VS Code **Dev Container**](https://containers.dev/) config (a `.devcontainer/` folder that tells VS Code to open the project inside a running container with the toolchain pre-installed) so "Reopen in Container" just works. The two usual patterns:

```sh
# A) named sub-directory: creates ./my-ide/ from the template
nextor sh -c 'cp -R "$NEXTOR_SDK"/templates/driver my-ide'
nextor sh -c 'cd my-ide && make'

# B) in place: copy into the mounted directory itself (note the /. - it also
#    brings the hidden .devcontainer/ along)
mkdir my-ide && cd my-ide
nextor sh -c 'cp -R "$NEXTOR_SDK"/templates/driver/. .'
nextor make            # -> mydriver.ROM
```

Note that the output name is **not** derived from the directory: it's the `NAME` variable at the top of the template's `Makefile` (`mydriver` / `mytool` until you change it - see the first `TODO`).

### Running the tools from the CLI

The image has no [`ENTRYPOINT`](https://docs.docker.com/engine/reference/builder/#entrypoint) (the command Docker would otherwise run by default when the container starts), so anything after the image name is run directly - handy for one-offs:

```sh
# version / help
nextor N80 --version

# assemble a single file (mount the cwd, write output beside it)
nextor N80 mycode.asm mycode.bin --include-directory /opt/nextor/sdk --no-show-banner

# compile a Z80 C file
nextor sdcc -mz80 -c hello.c
```

When a command needs the environment variables expanded (e.g. `$NEXTOR_SDK`), run it through a shell so the *container's* values are used:

```sh
nextor bash -lc 'N80 driver.asm driver.bin --include-directory "$NEXTOR_SDK"'
```

### How a driver Makefile is structured

A driver ROM is built in two steps - **assemble**, then **combine with a kernel base file via `mknexrom`**. The `driver` template's Makefile is the canonical shape (trimmed):

```make
NAME := mydriver
ROM  := $(NAME).ROM

NEXTOR_BASE ?= /opt/nextor/kernel_base/kernel_base.dat   # from the image env
NEXTOR_SDK  ?= /opt/nextor/sdk
N80         ?= N80
MKNEXROM    ?= mknexrom

N80_FLAGS := --no-string-escapes --no-show-banner --verbosity 0 \
             --include-directory $(NEXTOR_SDK)

$(ROM): driver.bin chgbnk.bin
	$(MKNEXROM) $(NEXTOR_BASE) $@ /d:driver.bin /m:chgbnk.bin

driver.bin: driver.asm
	$(N80) driver.asm driver.bin $(N80_FLAGS)

chgbnk.bin: chgbnk.asm
	$(N80) chgbnk.asm chgbnk.bin $(N80_FLAGS)
```

Key points:

- **`mknexrom <base> <out> /d:<driver.bin> /m:<chgbnk.bin>`** is the combine step. `/d:` is your assembled driver (which must begin with 256 dummy bytes - the template's `driver.asm` does this with `org 4000h` + `ds 4100h-$,0`), `/m:` is the bank-switching module for your cartridge's mapper.
- **`--include-directory $(NEXTOR_SDK)`** lets sources write `INCLUDE asm/constants/...` instead of long relative paths.
- **Variants:** point `NEXTOR_BASE` at another base file to target a different kernel. For a `.NO_UNDOC.` base, also assemble the driver undoc-free (`make NO_UNDOC=1 NEXTOR_BASE=.../kernel_base.NO_UNDOC.dat`); the template wires `NO_UNDOC=1` to `--define-symbols NO_UNDOC_CPU_INSTRUCTIONS`.

A **tool** Makefile is simpler - `tool.asm` uses `org 0100h`, so `N80` emits a ready-to-run `.COM` directly; there is no `mknexrom` step.

### File ownership (mounted volumes)

The container runs as **root**, so files it writes into a mounted directory are root-owned on the host. To get your own ownership, run as your host user - the tools work fine without a writable home:

```sh
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/work \
  ghcr.io/konamiman/nextor-dev make
```

(If a tool ever complains about a missing home directory, add `-e HOME=/tmp`.)

---

## Developing Nextor itself

The image already contains a built kernel, but to *work on* the kernel you mount your checkout and rebuild - the build uses the **mounted repository's** sources (its own `sdk/`, `source/`, version constants); the image only supplies the toolchain.

**Important note:** Always mount the repository root, and select the part with `make -C source/<part>` (or `cd source/<part>` *inside* the container). Mounting a subdirectory such as `source/kernel` on its own **fails immediately** - each part's Makefile reaches up the tree (`../version.mk`, `../../sdk/...` for SDK includes, `../../bin/kernel-base/` for outputs), so those paths must exist in the container.

The easiest way is the **`docker/make.sh`** wrapper: it mounts the root for you, runs `make -C source/<part>`, and writes outputs back as *your* user (not root).

```sh
git clone https://github.com/Konamiman/Nextor && cd Nextor

docker/make.sh kernel              # build the default kernel base file
docker/make.sh kernel everything   # all six kernel variants
docker/make.sh kernel clean
docker/make.sh nextor_sys          # build NEXTOR.SYS
docker/make.sh all                 # one variant of every part
docker/make.sh all everything      # the FULL matrix (see below)
docker/make.sh all tools-disk      # NEXTOR.SYS + all tools, packed in a disk image
docker/make.sh all distclean       # remove every build artifact, incl. bin/
```

`<part>` is any directory under `source/`; extra arguments (`everything`, `clean`, a target, `-j`, ...) pass straight through to `make`:

| `docker/make.sh ...` | Builds |
|---|---|
| `kernel` | the kernel base file(s) → `bin/kernel-base/Nextor-<version>.base[<suffix>].dat`; `everything` = all six variants |
| `nextor_sys` | `NEXTOR.SYS` (+ `.japanese`) |
| `tools` | all command-line `.COM` utilities - both the assembler tools (`source/tools`) and the C tools (`source/tools/C`) |
| `tools/C` | just the C tools |
| `drivers` | the standalone ROMs (ASCII8 + ASCII16); `everything` = all six variants of each; `ram-example` = the opt-in example RAM disk driver (`.drv` → `bin/ram-drivers/`) |
| `all` | the umbrella Makefile: bare = one variant of every part; `everything` = all kernel + standalone-ROM variants + NEXTOR.SYS + all tools; `tools-disk` = the tools disk image; `clean` / `distclean` |

`make.sh all everything` builds the complete release matrix: all six kernel base variants, both standalone ROMs (ASCII8/ASCII16) for each of those six variants, NEXTOR.SYS, and every command-line tool. `make.sh all distclean` removes all of that plus the source-tree intermediates.

`make.sh all tools-disk` builds NEXTOR.SYS and all the command line tools, then packs them into `bin/tools/nextor.dsk`, a 360K FAT12 disk image created with `mformat`/`mcopy`, carrying the same MSX-DOS 2 style boot sector that the built-in FORMAT command creates (so it boots in MSX-DOS 1 mode too). `COMMAND2.COM` is included too if available: drop it in the repository root, or point the `COMMAND2_PATH` variable at it (see the `tools-disk` target in `source/tools/Makefile` for the details). Note that the disk recipe lives in `source/tools`, but `make.sh tools tools-disk` won't work: the `tools` part maps to *both* tool Makefiles, and only `source/tools` has that target. Go through `all`, which builds everything the disk needs first.

**Note:** the legacy `source/command/` suite (`COMMAND2.COM`, `MSXDOS2.SYS`, and the classic DOS utilities) is for now out of scope. It still builds with the CP/M-era Microsoft toolchain (`m80`/`l80`/`c80`/`xl80`), which this image does **not** include - and the top-level `source/Makefile` doesn't build it either. Use the original vintage tools for that part of the repository.

The explicit equivalent, if you'd rather not use the wrapper, is just `make -C source/<part>` with the **repository root** mounted:

```sh
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/work \
  ghcr.io/konamiman/nextor-dev make -C source/kernel
```

`make.sh` picks the image automatically: `$NEXTOR_IMAGE` if you set it, else a local `nextor-dev` image if you built one with `docker/build.sh`, else the latest official image (`ghcr.io/konamiman/nextor-dev:latest`, which Docker pulls on first use). So a contributor who never builds the image just gets the published one.

So to answer the obvious question - "is `cd source/kernel` then `make` enough?" - yes, but the `cd` has to happen **inside** the container, after mounting the root (not on the host, which would mount only that subdirectory). An interactive session makes that natural:

```sh
nextor
# now inside the container, sitting at /work (the repo root):
#   cd source/kernel && make      # build the kernel
#   make -C ../tools              # or another part, by path
#   N80 --version
```

---

**Note: if you only want to develop Nextor, a Nextor driver, or a Nextor-aware tool with the image, you can stop reading here.** The remaining sections are reference material: how the image is laid out, how to build it yourself, how to test it, and how to publish it.

---

## Image internals

### What's inside

Everything lives under `/opt/nextor`:

```
/opt/nextor/
├── bin/            N80, LK80, LB80, mknexrom   (all on PATH)
│                   (sdcc and the mtools utilities are Debian apt
│                   packages, at /usr/bin)
├── kernel_base/
│   ├── kernel_base.dat                       default variant
│   ├── kernel_base.NO_UNDOC.dat
│   ├── kernel_base.SHIFT_INV.dat
│   ├── kernel_base.CTRL_INV.dat
│   ├── kernel_base.NO_UNDOC.SHIFT_INV.dat
│   └── kernel_base.NO_UNDOC.CTRL_INV.dat
├── sdk/            asm/ (constants, macros, code, chgbnk), C/ (includes, code)
│                   and templates/ (the driver/ and tool/ project templates)
└── manifest.json   every baked-in component version (JSON)
```

The working directory is `/work` and the default command is `bash`, so `docker run --rm -it -v "$PWD":/work ghcr.io/konamiman/nextor-dev` drops you into an interactive shell with your project mounted.

### Environment variables

Recipes (and you) can rely on these being set inside the container:

| Variable | Value | Use |
|---|---|---|
| `NEXTOR_VERSION` | e.g. `3.0.0-beta1` | the kernel version baked in |
| `NEXTOR_BASE` | `.../kernel_base/kernel_base.dat` | default base file for `mknexrom` |
| `NEXTOR_KERNEL_BASE_DIR` | `.../kernel_base` | directory of the six base files |
| `NEXTOR_SDK` | `.../sdk` | SDK root (pass to N80 as `--include-directory`) |
| `NEXTOR_SDK_ASM` / `NEXTOR_SDK_C` | `.../sdk/asm`, `.../sdk/C` | asm / C subtrees |
| `N80` `LK80` `LB80` `MKNEXROM` `SDCC` | absolute tool paths | for Makefiles that prefer explicit paths |

### Variants

The six base file **variants** differ by two independent axes:

| Suffix | Meaning |
|---|---|
| *(none)* | undocumented Z80 opcodes allowed; default boot keys |
| `.NO_UNDOC` | no undocumented opcodes - safe on Z180-based MSX (e.g. Victor HC-95) |
| `.SHIFT_INV` | SHIFT-at-boot behaviour inverted |
| `.CTRL_INV` | CTRL-at-boot behaviour inverted |

(`NO_UNDOC` combines with either key inversion, giving the two `.NO_UNDOC.*_INV` files.)

**Note:** The `.SHIFT_INV` / `.CTRL_INV` variants are a convenience: `mknexrom` can flip the boot keys on the *default* (or `.NO_UNDOC`) base at ROM-assembly time with `/k:<hex>` (LSB = byte 0, MSB = byte 1; e.g. `/k:1002` inverts SHIFT and "1"). The `NO_UNDOC` axis is different: it changes the assembled code, so it genuinely requires its own base file.

### Naming conventions

- **Image repository:** `ghcr.io/konamiman/nextor-dev` (official) / `ghcr.io/<account>/nextor-dev` (self-published). See *Publishing → Tag scheme* below.
- **Kernel base files** in the image are named `kernel_base[<variant>].dat` (the default variant is plain `kernel_base.dat`); the repository build emits them as `Nextor-<version>.base[<variant>].dat` and the image rename drops the version prefix. The variant suffix is the meaningful part either way.
- **Version of record:** inside the image the kernel version is the `NEXTOR_VERSION` env var and the one-line `$NEXTOR_SDK/nextor-kernel-version.txt` (handy for shell one-liners, and what driver Makefiles read); `manifest.json` is the structured superset (kernel version + every tool version + variant list). That same `nextor-kernel-version.txt` is what non-Docker SDK consumers read straight from the repository.
- **Driver ROMs:** the template produces `<name>.ROM`. Official drivers follow `Nextor-<version>.<DriverName>[.<variant>].ROM`, deriving `<version>` from `$NEXTOR_SDK/nextor-kernel-version.txt` (or `$NEXTOR_VERSION`).
- **SDK includes:** under `$NEXTOR_SDK`, reference as `asm/constants/*.inc`, `asm/macros/*.inc`, `asm/code/*.asm` (assembly) or `C/includes/*.h` (C), via N80's `--include-directory` or the C compiler's include path.

---

## Building the image locally

You only need to do this if you're modifying the image itself (Dockerfile, build scripts, baked-in tool versions) or experimenting with build-arg overrides; regular driver/tool development uses the published image and never needs this step.

Use the wrapper (runnable from anywhere - it locates the repo itself):

```sh
docker/build.sh                       # tags 'nextor-dev'
docker/build.sh -t my-nextor:test     # pass your own tag and/or build flags
docker/build.sh --no-cache --build-arg N80_VERSION=1.3.6
```

**Note:** `docker/build.sh` builds a **single-arch** image for your host (handy for local work). The multi-arch (amd64 + arm64) build lives in the publish workflow - see *Publishing*.

It fills in `--build-arg NEXTOR_VERSION` from `sdk/nextor-kernel-version.txt` (the authoritative kernel version) and forwards everything else to `docker build`. The explicit equivalent, invoking `docker build` directly against the [Dockerfile](https://docs.docker.com/build/concepts/dockerfile/) (the script in `docker/` that describes how the image is assembled, step by step), is:

```sh
docker build -f docker/Dockerfile \
  --build-arg NEXTOR_VERSION="$(cat sdk/nextor-kernel-version.txt)" \
  -t nextor-dev .
```

Why pass `NEXTOR_VERSION` at all? It stamps the version into the **[OCI labels](https://github.com/opencontainers/image-spec/blob/main/annotations.md)** (standardised key-value metadata baked into the image and readable later with `docker inspect`) and the baked **`NEXTOR_VERSION`** env var. Those are `LABEL`/`ENV` instructions, which accept only literals or build args - they can't `cat` the file the way a `RUN` can (`manifest.json` *is* written by a `RUN`, and the SDK's `nextor-kernel-version.txt` is produced by the kernel build). That's the whole reason the value is injected from outside, and why the wrapper exists. It defaults to `unknown` if omitted.

The build is fully reproducible from source: it downloads pinned Nestor80 releases (per target arch), installs SDCC 4.2.0 from Debian's apt, compiles `mknexrom` from `buildtools/sources`, and builds all six kernel base files via `make -C source/kernel everything`.

### Build arguments

Override with `--build-arg NAME=value`:

| Arg | Default | Purpose |
|---|---|---|
| `NEXTOR_VERSION` | `unknown` | kernel version stamped into the image |
| `NEXTOR_IMAGE_REVISION` | `dev` | image build revision (`rN`); CI sets the real value |
| `N80_VERSION` | `1.3.5` | Nestor80 assembler release |
| `LK80_VERSION` / `LK80_TAG` | `1.1.0` / `n80-v1.3.3-lk80-v1.1` | linker release + its GitHub tag |
| `LB80_VERSION` | `1.0` | librarian release |
| `SDCC_VERSION` | `4.2.0` | recorded in the manifest/labels; `sdcc` itself comes from Debian apt (also 4.2.0), so this only relabels |
| `MKNEXROM_VERSION` | `1.1` | recorded in the manifest; `mknexrom` is compiled from source |
| `DOTNET_TAG` | `8.0-bookworm-slim` | `dotnet/runtime` base image tag |

Bumping a tool is a one-line change, e.g. `--build-arg N80_VERSION=1.3.6`; the new version automatically flows into `manifest.json` and the labels. (`sdcc` is the apt package, so `SDCC_VERSION` only changes the recorded label, not the installed compiler.)

---

## Testing the image locally (no registry needed)

A **registry** is a server that hosts and distributes Docker images (Docker Hub, GitHub Container Registry, your own private one). You never need one to validate this image: it's the same artifact whether it lives only on your machine or on a registry. Three approaches, by goal:

**1. Run it directly** - the image exists as soon as you `docker build`. Use the acceptance test to assert everything works:

```sh
docker/test.sh                 # local 'nextor-dev' build, else the official latest
docker/test.sh my-other-tag    # or any explicit tag
```

`test.sh` runs inside the container as a login shell and checks: every tool runs (incl. a real N80→LK80 link and an `sdcc -mz80` compile), all six base variants are present, the baked `NEXTOR_VERSION` matches the built kernel and `manifest.json`, and both project templates (`driver`, `tool`) build end-to-end when copied verbatim. It exits non-zero on any failure, so it doubles as a CI gate - and indeed the **`Image CI`** workflow (`.github/workflows/image-ci.yaml`) runs exactly this (`build.sh` → `test.sh`, no push) on every pull request that touches an image input, so the build and the 14 checks are verified before merge.

**2. Move it to another machine** without a registry, via a tarball:

```sh
docker save nextor-dev | gzip > nextor-dev.tar.gz
# copy the file to the other host, then:
docker load < nextor-dev.tar.gz
```

**3. Exercise the full push/pull round-trip** with a throwaway local registry - useful to rehearse publishing. It is also the only way to test a **multi-arch** build locally, because [`docker buildx`](https://docs.docker.com/build/concepts/overview/) (Docker's modern, multi-platform-capable build tool) can `--push` a multi-arch [manifest list](https://github.com/opencontainers/image-spec/blob/main/manifest.md#manifest-list) ( an index that binds several platform-specific images under one tag) to a registry, but cannot `--load` it into the local image store:

```sh
docker run -d -p 5000:5000 --name reg registry:2
docker tag nextor-dev localhost:5000/nextor-dev:test
docker push localhost:5000/nextor-dev:test
docker pull localhost:5000/nextor-dev:test
docker rm -f reg          # tear down when done
```

---

## Publishing the image

The **official** image lives in Konamiman's account; anyone else can publish their own (for testing, forks, or private use) by pushing to their own account - the steps are identical, only the repository name changes.

### The automated way (GitHub Actions)

The official publish is the **`Publish image`** workflow (`.github/workflows/publish.yaml`): in the repo's *Actions* tab, run it and give it the build revision (`r1`, `r2`, ...). It builds the image, runs `docker/test.sh` as a gate, and pushes to `ghcr.io/<owner>/nextor-dev` only if the test passes, using the built-in `GITHUB_TOKEN` (no secrets to configure). Tags are derived from `sdk/nextor-kernel-version.txt` per the *Tag scheme* below: a prerelease publishes only its exact tags; a stable version also moves `major.minor` / `major` / `latest`. It's `workflow_dispatch`-only (no accidental publishes) and pushes a multi-arch (amd64 + arm64) manifest. Forks get `ghcr.io/<their-owner>/nextor-dev` automatically.

To avoid corrupting a build, the run **fails if the pinned `<version>-<rev>` tag already exists** unless you tick the **overwrite** checkbox, so re-running with the same revision is a deliberate choice, while bumping the revision (or moving the always-advancing `latest` / `major.minor` tags) is unaffected.

**Note:** The first publish creates a **private** package; flip it to public once in the package's settings on GitHub if others should be able to pull it.

### The manual way

For a one-off or a self-publish without Actions, push by hand. GitHub Container Registry (GHCR) is the primary home:

```sh
# 1. Authenticate. GHCR uses a GitHub Personal Access Token with
#    'write:packages' scope (classic token), or a fine-grained equivalent.
echo "$GITHUB_TOKEN" | docker login ghcr.io -u YOUR_GITHUB_USER --password-stdin

# 2. Build and tag (official: account = konamiman; self-publish: your account).
ver="$(cat sdk/nextor-kernel-version.txt)"
docker/build.sh -t ghcr.io/YOUR_ACCOUNT/nextor-dev:"$ver" \
  --build-arg NEXTOR_IMAGE_REVISION=r1

# 3. Push.
docker push ghcr.io/YOUR_ACCOUNT/nextor-dev:"$ver"
```

After the first push the GHCR package is **private** by default - make it public (and link it to the repo) from the package's settings on GitHub if you want others to pull it. Docker Hub works the same way with `docker login` (no registry prefix) and `docker.io/YOUR_ACCOUNT/nextor-dev`.

### Tag scheme

The advertised tag is the **kernel version**; image-only changes (a tool bump) ride a build revision rather than a new kernel tag:

| Tag | Meaning |
|---|---|
| `3.0.0` | an exact kernel version (moves to the newest revision of that kernel) |
| `3.0` | newest `3.0.x` |
| `latest` | newest stable |
| `edge` | latest build from the development branch, no promises |
| `3.0.0-r2` | a specific image revision (the kernel base files are byte-identical across `-rN` of the same kernel; only tools differ) |

Every component version is recorded in `manifest.json` and in OCI labels, so a moving tag is still fully traceable:

```sh
docker inspect --format '{{json .Config.Labels}}' nextor-dev | tr ',' '\n'
docker run --rm nextor-dev cat /opt/nextor/manifest.json
```

### Multi-arch

The published image is a **manifest list** covering `linux/amd64` and `linux/arm64`; `docker pull` picks the right one automatically. The publish workflow builds and tests each arch (arm64 via QEMU) before pushing the manifest. The kernel base files are Z80 binaries (architecture-independent) so they're built once on the runner's native arch (the `kernelbuild` stage is pinned to `$BUILDPLATFORM`); only the host tools (N80/LK80/LB80/sdcc/mknexrom) are per-arch. To build a multi-arch image by hand:

```sh
docker buildx build --platform linux/amd64,linux/arm64 \
  -f docker/Dockerfile --build-arg NEXTOR_VERSION="$ver" \
  -t ghcr.io/YOUR_ACCOUNT/nextor-dev:"$ver" --push .
```

---

## Appendix: trying it in a clean WSL distro

A good way to confirm the "nothing but git and Docker" promise: spin up a fresh WSL distro and exercise the real workflows in it. In priority order:

1. **Develop a driver or tool** - *pull* the image, copy a project template, build it. Needs only Docker (plus git for your own project); the Nextor sources are **not** required.
2. **Develop Nextor itself** - clone the repo and rebuild a part. Needs git + the image.
3. **Build the image** - rare, so just do it in your regular distro (`docker/build.sh`); no need to reproduce it on the clean box.

The host distro doesn't affect the image - Docker runs it with its own userland - so pick whichever is easiest to get a Docker daemon running in.

### 1. A minimal Docker-capable distro

Debian is the smallest of the one-command official WSL distros:

```powershell
wsl --install -d Debian            # in PowerShell
```

Whichever way you set up Docker below, git is needed too and the fresh distro doesn't include it, so install it first (from inside the distro):

```sh
sudo apt-get update && sudo apt-get install -y git
```

Then you have two ways to make `docker` available inside the distro; either is fine, pick based on what you already have on the host.

**Option A - use [Docker Desktop](https://docs.docker.com/desktop/setup/install/windows-install/)'s WSL integration (easiest if you already have Docker Desktop installed on Windows).** Docker Desktop exposes its daemon and the `docker` CLI to selected WSL distros automatically. Open *Docker Desktop → Settings → Resources → WSL integration*, enable the new Debian distro, and apply. Then verify from inside Debian:

```sh
docker run --rm hello-world        # daemon sanity check (via Docker Desktop)
```

That's it: no systemd, no `usermod`, no in-distro Docker install. This is also the simplest path for everyday use on Windows.

**Option B - install Docker Engine directly inside the distro (no Docker Desktop needed).** Useful if you can't (or don't want to) run Docker Desktop (because of licensing, minimal install, faster startup, etc). Enable systemd (so Docker runs as a service) and install Docker:

```sh
sudo tee /etc/wsl.conf >/dev/null <<'EOF'
[boot]
systemd=true
EOF
```
```powershell
wsl --shutdown                     # back in PowerShell, then reopen Debian
```
```sh
curl -fsSL https://get.docker.com | sudo sh
sudo usermod -aG docker "$USER"    # then reopen the shell
docker run --rm hello-world        # daemon sanity check
```

(Ubuntu works identically and is more documented; Alpine is smaller but needs manual `dockerd`/OpenRC setup.)

### 2. Get the image (you *pull* it, you don't build it)

Most readers want the **official** image - same flow as section *Developing drivers and tools*. The "publish it yourself first" path below it is only relevant to image maintainers rehearsing the publishing workflow.

**Pull the official image (what you probably want):**

```sh
docker pull ghcr.io/konamiman/nextor-dev:latest
docker tag  ghcr.io/konamiman/nextor-dev:latest nextor-dev  # short local name for what follows
```

Done: skip ahead to *3. Primary flow*.

---

The remainder of this sub-section is for **image maintainers** who want to rehearse the publishing workflow on a clean distro before pushing the official image. Two ways, both end with a `nextor-dev` tag locally in the clean distro:

**A - self-publish to your own account** (most faithful: it's exactly the consumer flow, over the internet):

```sh
# regular distro, logged in to GHCR (or Docker Hub):
docker/build.sh -t ghcr.io/<you>/nextor-dev:test
docker push ghcr.io/<you>/nextor-dev:test
# clean distro:
docker login ghcr.io                                  # only if the package is private
docker pull ghcr.io/<you>/nextor-dev:test
docker tag  ghcr.io/<you>/nextor-dev:test nextor-dev  # short local name for what follows
```

**B - local registry** (all-local; uses the shared WSL2 network):

```sh
# regular distro:
docker run -d -p 5000:5000 --name reg registry:2
docker tag  nextor-dev localhost:5000/nextor-dev:test
docker push localhost:5000/nextor-dev:test
# clean distro (WSL2 distros share the VM network):
docker pull localhost:5000/nextor-dev:test
docker tag  localhost:5000/nextor-dev:test nextor-dev
```

If `localhost:5000` isn't reachable from the clean distro, use the regular distro's WSL IP (`hostname -I`) instead and add `<ip>:5000` to the clean distro's `/etc/docker/daemon.json` under `insecure-registries`, then `sudo systemctl restart docker`.

### 3. Primary flow - develop a driver or tool

No Nextor checkout, no toolchain - just the pulled image and your own project:

```sh
mkdir my-ide && cd my-ide
docker run --rm -v "$PWD":/work nextor-dev sh -c 'cp -R "$NEXTOR_SDK"/templates/driver/. .'
docker run --rm -v "$PWD":/work nextor-dev make                      # -> mydriver.ROM

# or a tool:
docker run --rm -v "$PWD":/work nextor-dev sh -c 'cp -R "$NEXTOR_SDK"/templates/tool nxinfo'
docker run --rm -v "$PWD":/work nextor-dev sh -c 'cd nxinfo && make' # -> mytool.COM
```

A `.ROM` (or `.COM`) out the other end means the driver-developer story holds on a machine with nothing else installed.

### 4. Secondary flow - develop Nextor itself

This one *does* need the repository (the build uses its sources):

```sh
git clone <your-Nextor-repo> && cd Nextor
docker run --rm -v "$PWD":/work nextor-dev make -C source/kernel     # -> bin/kernel-base/*.dat
```

See *Developing Nextor itself* above for the full list of buildable parts.

A base `.dat` and a `.ROM` out the other end means the clean-machine story holds.
