# Nextor releases

[Nextor](https://github.com/Konamiman/Nextor) is an enhanced version of MSX-DOS 2 for MSX computers. Starting with Nextor 3, each component of the system is released separately, so this page groups the releases of [the Nextor repository](https://github.com/Konamiman/Nextor/releases) by component. Choose a component in the side menu to see its releases, or go to **Latest** to get the most recent version of everything in one place.

## What's what

- **Kernel base**: the Nextor kernel without any device driver, as a `.dat` file. Driver developers combine it with their driver using `mknexrom` to produce a complete kernel ROM. Available in several variants (`NO_UNDOC`, `SHIFT_INV`, `CTRL_INV`, `KANJI_INV`), explained in the category itself.

- **Drivers**: the device drivers supplied with the Nextor repository.
  - **Standalone ROMs**: complete kernel ROMs with a dummy device driver that handles no devices, for computers and emulators where the storage devices are handled by other ROMs, or for testing drivers loaded in RAM.
  - **Example RAM driver**: a minimal driver loadable in RAM, intended as a template and a test subject for driver developers.

- **NEXTOR.SYS**: the DOS system file, Nextor's version of `MSXDOS2.SYS`, loaded from the boot drive to bring up the DOS environment (English and Japanese variants are available).

- **COMMAND3.COM**: the Nextor 3 command interpreter, an enhanced replacement for `COMMAND2.COM`.

- **Tools**: the command line tools (`.COM` files to be run from the DOS prompt). The **Tools disk image** is a bootable disk with `NEXTOR.SYS`, `COMMAND3.COM`, all the tools and the help files; the **Tools ZIP file** contains just the tools. Each tool has also its own entry, listing the releases that contain that particular tool.

- **Build tools**: tools that run on a PC, not on the MSX. Currently just **mknexrom**, the tool that produces kernel ROMs from a kernel base file and a driver.

## About drivers

This repository doesn't contain drivers for specific storage devices: only the standalone ROMs (whose driver is a dummy one that handles no devices) and the example RAM driver. Drivers for actual devices (Sunrise IDE, MegaFlashROM SCC+ SD, and others) are developed and released in their own repositories; see the [Known Drivers](https://github.com/Konamiman/Nextor/blob/v3.0/docs/Nextor_3.0_Known_Drivers.md) document for the list and where to get each one.

## Looking for Nextor 2?

This page lists Nextor 3 releases only. The Nextor 2 releases (which include the kernel ROMs for all the drivers that used to live in this repository) are available directly in GitHub: [Nextor 2 releases](https://github.com/Konamiman/Nextor/releases?q=v2.&expanded=true). The latest Nextor 2 version is [v2.1.4](https://github.com/Konamiman/Nextor/releases/tag/v2.1.4).

## Documentation

The [Nextor 3.0 documentation](https://github.com/Konamiman/Nextor/tree/v3.0/docs) includes a [Getting Started Guide](https://github.com/Konamiman/Nextor/blob/v3.0/docs/Nextor_3.0_Getting_Started_Guide.md), the [User Manual](https://github.com/Konamiman/Nextor/blob/v3.0/docs/Nextor_3.0_User_Manual.md), the [Programmers Reference](https://github.com/Konamiman/Nextor/blob/v3.0/docs/Nextor_3.0_Programmers_Reference.md) and the [Driver Development Guide](https://github.com/Konamiman/Nextor/blob/v3.0/docs/Nextor_3.0_Driver_Development_Guide.md).
