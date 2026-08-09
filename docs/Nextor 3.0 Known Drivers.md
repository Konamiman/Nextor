# Nextor 3.0 Known Drivers

As of Nextor 3.0 the device drivers for specific hardware no longer live in the Nextor repository: drivers are developed and distributed independently. A ready-to-use Nextor kernel ROM is produced by combining the Nextor kernel base file with the driver code; some drivers additionally provide a RAM-loadable driver file, which is a direct assembly of the driver code alone and doesn't involve the kernel base file. Each developer chooses how to distribute their work: a git repository, a dedicated web site, plain downloadable binaries, etc.

The drivers that were part of Nextor 2 now live in their own dedicated GitHub repositories, which consume [the Nextor SDK](../sdk/README.md) and offer the downloadable ROM files in their releases sections; see the README file of each repository for the details and the build instructions.

These are the known device drivers available for Nextor 3:

| Driver | Hardware | Where to get it |
| --- | --- | --- |
| Sunrise IDE | Sunrise IDE cartridges and compatible storage controllers | [SunriseIDE-Nextor-driver](https://github.com/Konamiman/SunriseIDE-Nextor-driver) |
| MegaFlashROM SCC+ SD | [MegaFlashROM SCC+ SD](https://www.msxcartridgeshop.com/) cartridges | [MegaFlashROM-SD-Nextor-driver](https://github.com/Konamiman/MegaFlashROM-SCC-SD-Nextor-driver) |
| FlashJacks | FlashJacks IDE interface | [Flashjacks-Nextor-driver](https://github.com/Konamiman/Flashjacks-Nextor-driver) |
| MSX Turbo-R FDD | The floppy disk controller built into the MSX Turbo-R computers (Panasonic FS-A1GT and FS-A1ST); can be built as a ROM kernel or as a RAM-loadable driver | [TurboR-FDD-Nextor-driver](https://github.com/Konamiman/Turbo-R-FDD-Nextor-driver) |

Additionally, the Nextor repository itself contains [the standalone ROM driver](../source/drivers/standalone-rom-driver.asm) (a dummy driver that doesn't handle any real hardware, used to build the standalone Nextor ROMs) and [an example RAM-loadable driver](../source/drivers/ram-driver-example.asm); both are useful as reference code when developing a new driver.

Notes:

* The Nextor 2 versions of these drivers (except the Turbo-R FDD driver, which is new in Nextor 3) remain available in [the v2.1 branch](https://github.com/Konamiman/Nextor/tree/v2.1/source/kernel/drivers) of the Nextor repository. Remember that Nextor 2 drivers don't work with Nextor 3 and vice versa; see [the Nextor 3.0 Driver Migration Guide](Nextor%203.0%20Driver%20Migration%20Guide.md) for how to adapt a Nextor 2 driver to Nextor 3.

* The OCM (One Chip MSX) driver that was part of Nextor 2 has been discontinued, since its source code is not available.

If you have developed a driver for Nextor 3 and want it listed here (no matter how you distribute it: a git repository, a dedicated web site, plain downloadable binaries...), please open an issue or a pull request in [the Nextor repository](https://github.com/Konamiman/Nextor).
