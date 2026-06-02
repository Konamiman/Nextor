# Nextor SDK

This directory contains the public Software Development Kit for
Nextor: the constants, macros, data structures and helper sources that Nextor-aware external tools and drivers can use to integrate with the Nextor API.

## Layout

```
sdk/
├── nextor-kernel-version.txt
│                     The Nextor kernel version this SDK corresponds to, on a
│                     single line, e.g. "3.0.0" (or "3.0.0-beta1" for a
│                     pre-release). Machine-readable; read it to stamp the
│                     version into your ROM / output names without building
│                     anything.
├── asm/                  Assembly language SDK (Nestor80 syntax)
│   ├── chgbnk/           ROM bank change routines ASCII8 and ASCII16
│   │                     mappers (to be used when building a
│   │                     custom Nextor ROM with mknexrom).
│   ├── code/             Reusable assembly helper routines, one file per
│   │                     public routine.
│   ├── constants/        Public constant include files: DOS function and
│   │                     error codes, driver routine / query codes and
│   │                     work area addresses, ROM bank header entries,
│   │                     MSX BIOS / work area addresses, etc.
│   └── macros/           The `const` and `var` macros used by the .inc
│                         files in `constants/`.
└── C/                    C language SDK (SDCC)
    ├── code/             Reusable C and crt0 sources.
    └── includes/         C header files (DOS function/error codes, the
                          driver/device API, common data structures, etc.)
```

The SDK is included in the main Nextor repository, there is no separate
SDK-only repository. To bring it into your own project, pick one of the
two methods below.

## Using the SDK as a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)

This is the recommended approach for projects already managed with git.

```sh
# From your project's root, in your default branch:
git submodule add https://github.com/Konamiman/Nextor.git external/nextor
git commit -m "Add Nextor as submodule for the SDK"
```

The whole Nextor repository will be checked out under
`external/nextor/`; the SDK sits at `external/nextor/sdk/`. Reference
files from there in your build, e.g.:

```makefile
# In a makefile
NEXTOR_SDK := external/nextor/sdk

# (Optional) the kernel version this SDK targets, e.g. to stamp output names:
KERNEL_VERSION := $(shell cat $(NEXTOR_SDK)/nextor-kernel-version.txt)

mydriver.bin: mydriver.asm
	N80 $< $@ \
	    --include-directory $(NEXTOR_SDK)/asm/constants \
	    --include-directory $(NEXTOR_SDK)/asm/macros
```

From an assembly source assembled by N80:

```asm
  ; If you add --include-directory when assembling
	include dos_calls.inc      ; brings in _CONOUT, _STROUT, ...

  ; If you DON'T add --include-directory when assembling
  ; (assuming "external" and this source file are at the same directory level)
  include external/nextor/sdk/asm/constants/dos_calls.inc

	ld   c,_CONOUT
	ld   e,'A'
	call 5                      ; print 'A' via DOS
```


```c
// From a C source built with SDCC
// Compile with: -I$(NEXTOR_SDK)/C/includes -I$(NEXTOR_SDK)/C/code
#include "drivers.h"
#include "dos_functions.h"
```

To update the SDK to the latest Nextor revision later:

```sh
cd external/nextor
git fetch
git checkout <commit or tag>
cd ../..
git add external/nextor
git commit -m "Bump Nextor SDK to <commit or tag>"
```

When someone else clones your project, they will need
`git clone --recurse-submodules <your repo>` (or, after a plain clone,
`git submodule update --init --recursive`) to populate the submodule.

### Minimising the submodule footprint

The Nextor working tree is ~92 MB and the `.git/` directory is ~59 MB, but the SDK
itself is well under 1 MB. Two git features can be combined to bring
only the SDK down:

- **Sparse checkout** keeps the rest of the tree out of your working
  copy. Available in git 2.25+ and works against any clone:

  ```sh
  cd external/nextor
  git sparse-checkout init --cone
  git sparse-checkout set sdk
  ```

  Only `sdk/` materialises on disk; the other directories stay packed
  inside `.git/` and never appear in the working tree.

- **Partial clone** (`--filter=blob:none`) skips downloading file
  contents until they are actually needed. `git submodule add` accepts
  `--filter` from git 2.36+:

  ```sh
  git submodule add --filter=blob:none \
      https://github.com/Konamiman/Nextor.git external/nextor
  git -C external/nextor sparse-checkout init --cone
  git -C external/nextor sparse-checkout set sdk
  ```

  GitHub supports this filter, and the combined effect is that only
  blobs reachable from `sdk/` (plus tree objects to walk to them) are
  downloaded.

If your contributors might be on an older git (≤ 2.35), drop the
`--filter` flag: they will still benefit from the sparse-checkout step,
which is the bigger win in absolute disk terms.

## Using the SDK via a local symlink

For projects that don't use git directly, or for quick experimentation
in a repo where you don't want a submodule, you can clone Nextor
somewhere on your machine once and symlink to it from each project.

```sh
# One-time clone (anywhere outside your project):
git clone https://github.com/Konamiman/Nextor.git ~/src/Nextor

# Then, from your project's root, create a symlink to the SDK:
ln -s ~/src/Nextor/sdk nextor_sdk
```

Your build now references `nextor_sdk/asm/...` and `nextor_sdk/C/...` exactly
as it would the submodule version.

To update, just `git pull` in `~/src/Nextor`; every project that
symlinks to it picks up the new version automatically.

