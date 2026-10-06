# COMMAND3.COM help files

This directory contains the help files of `COMMAND3.COM`, the Nextor 3 command interpreter, converted to Markdown. Start with [the help index](INDEX.md) (also available [in Japanese](JINDEX.md)).

These files are a mirror of the help files intended to be read on the MSX, which are the plain text `.HLP` files in [the source/commandcom/helpfiles directory of the repository](https://github.com/Konamiman/Nextor/tree/HEAD/source/commandcom/helpfiles). On the MSX they are displayed with the `HELP` command (for example `HELP COPY`, or just `HELP` for the index); they are supplied in the `HELP` directory of the Nextor tools disk, and the `HELP` environment item tells `COMMAND3.COM` where to find them (e.g. `SET HELP=C:\HELP`). See [the HELP help file](HELP.md) for the details.

The text of each file is the same as in the original `.HLP` file; only the layout has been adapted to Markdown: the paragraphs are no longer split in lines of fixed width, the command syntax and the examples are formatted as code, and the references to other help subjects (as in "see `HELP ENV`") are links to the corresponding file. The `.HLP` files are the reference: when one of them is changed, the corresponding file here must be updated accordingly.

The line under the title of each command, _Versions: ..._, lists the versions in which the command (or tool) was introduced or modified: 2.20 is MSX-DOS 2.20 (its `COMMAND2.COM` and its transient tools), 2.40 to 2.44 are versions of the enhanced COMMAND 2.4x interpreter on which `COMMAND3.COM` is based, and 3.0 is Nextor 3.0 (`COMMAND3.COM` and the Nextor tools).

Besides the subjects listed in the index, there's [a help file for COMMAND3 itself](COMMAND3.md).
