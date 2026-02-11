# Nextor boot menu specification

## The problem

There are several keys that a Nextor user can press while booting in order to modify the boot behavior: slot keys to prevent specific Nextor kernels from booting (a key per slot number); and numeric keys for behavior like e.g. booting in DOS 1 mode or directly to BASIC.

This is confusing as the user needs to remember several keys and do some "gymnastics" if he wants to press several keys at once. So this is a proposal for a more user-friendly equivalent mechanism.

## The Nextor boot menu

The idea is that right when a Nextor 3 kernel boot sequence starts, and as soon as it detects that it is the first kernel in the system (or it's taking over a Nextor 2 or a MSX-DOS kernel) and if the user is pressing the N key, it displays a boot menu similar to this:

```
       Nextor 3.0.0 boot menu
----------------------------------------

* R. MegaFlashROM SCC+ SD in slot 1-3
* S. Kernel with a vey lo... in slot 2-1
* Q. ...
* W. (no name) in slot 1-0
* E. ...
  N. Disable all and boot

  0. Disable permanent disk emulation
  1. Boot in MSX-DOS 1 mode
  2. Boot in MSX-DOS 1+R800 ROM mode
  3. Boot in MSX-BASIC
  4. Boot in R800 ROM mode
  5. Reduced drive allocation mode
* 6. Single FDD drive (CTRL)
  7. Disable MSX-DOS kernels (SHIFT)

---------------------------------------
 ESC = Cancel, ENTER = Apply and boot
```

where the first half consists of all the Nextor 3 kernels present in the system, enabled (prepended by "*") by default; and the second half is the state of the numeric boot keys, by default those that have their bit set to 1 in KEYS_INV_0/1 in init.mac.

Thus the high level sequence of events would be:

1. The Nextor kernel detecting that it's the (new) master and that the N key is being pressed initializes the screen in text mode (40 columns in MSX1, 80 columns otherwise).
2. The Nextor kernels are detected by scanning all the existing slots (more on that later) and the name of each is requested via the device query routine.
3. All the kernel names and slots are displayed, together with their assigned slot disable key, and with a "*" indicating that in principle the kernel will boot normally.
4. All the numeric boot modification keys are shown (except 2 and 4 in non-Turbo-R computers). An asterisk is shown for the keys that are inverted according to KEYS_INV_0/1 in init.mac.
5. The kernel waits for the user to release the N key and waits for other key pressings.
6. If one of the keys shown is pressed (slot key or numeric key) the corresponded entry is toggled.
7. If the user presses ESC, the menu disappears and the boot proceeds normally, discarding any changes. It's as if the menu hadn't been shown.
8. If the user presses ENTER, the enabled entries are collected and the BOOTKEYS variable is set appropriately, including the NEXTOR_BOOT_KEYS signature. Then the menu disappears and the boot procedure continues.
9. If the user presses N, it's as when pressing ENTER, except that ALL the Nextor kernels are disabled. Thus an easy(ish) wait to disable all Nextor kernels is: press N, wait for the menu, release, press again.
10. The registered keys influence the boot procedure of all kernels as if the keys had been physically pressed or if the one-time boot keys mechanism had been used.

This is practical for one-off unusual boots, and also as a memory aid for users to remember which key does what without having to look for documentation.

## Technical details, nuances and challenges

- The menu needs to be shown before any Nextor driver has been initialized. Therefore all the existing slots need to be examined at that point in search for Nextor 3 kernels (there's no KERNEX table built yet). This implies that we need to put a short signature (like "NEXTOR3") at a reachable location in all the ROM pages of the Nextor kernel. The first 256 bytes page (dosehad.mac) seems like a good place, but that area is already quite cramped so some optimization work might be needed first.

- Theorically a MSX computer can have up to 16 slots (if all four main slots are extended), but the detection procedure can stop as soon as five kernels are detected: it's extremely unlikely that any real system will have more Nextor kernels, and only four are supported anyway given the current architecture of Nextor. Also this allows the full kernels list and the numeric keys list to fit in the screen.

- An interesting case is that the main kernel (the one showing the menu) can be itself disabled via the boot menu. But then there can be another Nextor kernel located in a higher slot, which hasn't been disabled. This one will become the effective master but it must NOT show the menu again, so some in-memory flag is probably needed for that.

- Drivers must support the "Get driver information string" driver query (and the "Get driver version number" query, for possible future enhancements of the boot menu) WITHOUT the "Initialize driver" query having been executed first. This needs to be very clearly documented.

- The boot menu layout needs to support 40 columns and 80 columns for MSX1 and MSX2/2+/Turbo-R. This implies adjustments on how the top and bottom lines are centered, the length of the hyphens lines, and the maximum string length provided to the driver when querying its name (and if the driver says that the name is truncated, "..." should be shown after it).

- Related to the above: in 40 columns mode the "in slot X[-Y]" text could be shortened to "in X[-Y]" so we can display five more characters of the driver name.

- Another interesting case: Nextor 3.0 boots and shows the menu, then later on a hypothetical Nextor 3.1 boots and becomes the new master. In this case too, care must be taken so the menu is shown only once. Also: Nextor 3.1 might have a newer version of the code that shows the menu, but it's the 3.0 kernel the one that needs to show it, because otherwise it wouldn't be possible for it to disable itself (not ideal but acceptable).

- For extra coolness we could temporarily redefine the asterisk character like a checkmark, but let's worry about that once a working implementation of the menu is in place.

- This needs to be compatible with the mechanism to disable Nextor 2 kernels that was implemented in https://github.com/Konamiman/Nextor/pull/193 .

## Additional information

- source/kernel/drivers/StandaloneASCII8/driver.mac for the structure of the Nextor 3 drivers.
- https://github.com/Konamiman/Nextor/blob/v2.1/docs/Nextor%202.1%20User%20Manual.md for the one-time keys configuration tool and the boot keys reference.
- https://github.com/Konamiman/Nextor/blob/v2.1/docs/Nextor%202.1%20Programmers%20Reference.md for the technical details on how the one-time boot keys works (section 7).
- https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Appendix1.md and https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Appendix4.md for the MSX BIOS and work area variables and buffers.

## Task for you

Validate the idea and the design. Is there something important I haven't taken in account? Can you suggest improvements or simplifications? Do you have questions?

