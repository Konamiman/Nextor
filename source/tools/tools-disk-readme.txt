This disk contains NEXTOR.SYS, COMMAND3.COM and the Nextor
command line tools. It can be used to boot straight to the DOS
prompt on a computer with a Nextor kernel ROM.

  NEXTOR.SYS    The Nextor system file.
  NEXTORJ.SYS   The variant of NEXTOR.SYS with Japanese
                messages; to use it, delete NEXTOR.SYS and
                rename NEXTORJ.SYS to NEXTOR.SYS.
  COMMAND3.COM  The command interpreter.
  AUTOEXEC.BAT  Sets the PATH and HELP environment items so
                that the tools and the help files are found
                from any directory.
  TOOLS         The command line tools.
  HELP          The help files, browsable with the HELP
                command.

When copying these files to another disk, keep this directory
structure: AUTOEXEC.BAT expects the TOOLS and HELP directories
to be in the root directory of the boot drive.

COMMAND3.COM displays its messages in Japanese when the kanji
mode is active (unless the ERRLANG environment item is set to
EN).

Run any tool without arguments to get usage information (except
XDIR, which lists the current directory; use HELP XDIR). See the
Nextor User Manual for the details.

Nextor documentation, source code and binaries:
https://github.com/Konamiman/Nextor
