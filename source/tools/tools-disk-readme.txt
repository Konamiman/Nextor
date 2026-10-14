This disk contains NEXTOR.SYS, COMMAND3.COM, the MSX-DOS 1
system files and the Nextor command line tools. It can be used
to boot straight to the DOS prompt on a computer with a Nextor
kernel ROM, both in the normal mode and in MSX-DOS 1 mode.

  NEXTOR.SYS    The Nextor system file.
  NEXTORJ.SYS   The variant of NEXTOR.SYS with Japanese
                messages; to use it, delete NEXTOR.SYS and
                rename NEXTORJ.SYS to NEXTOR.SYS.
  COMMAND3.COM  The command interpreter.
  MSXDOS.SYS    The MSX-DOS 1 system file and command
  COMMAND.COM   interpreter, used when booting in MSX-DOS 1
                mode.
  MSR.COM       Installs the mapper support routines and an
                UNAPI RAM helper in MSX-DOS 1 (it's here and
                not in TOOLS because MSX-DOS 1 can't access
                directories).
  AUTOEXEC.BTM  Sets the PATH and HELP environment items so
                that the tools and the help files are found
                from any directory. It's a .BTM file so that
                MSX-DOS 1 doesn't execute it.
  TOOLS         The command line tools.
  HELP          The help files, browsable with the HELP
                command.

When copying these files to another disk, keep this directory
structure: AUTOEXEC.BTM expects the TOOLS and HELP directories
to be in the root directory of the boot drive.

COMMAND3.COM displays its messages in Japanese when the kanji
mode is active (unless the ERRLANG environment item is set to
EN).

Run any tool without arguments to get usage information (except
XDIR, which lists the current directory; use HELP XDIR). See the
Nextor User Manual for the details.

Nextor documentation, source code and binaries:
https://github.com/Konamiman/Nextor
