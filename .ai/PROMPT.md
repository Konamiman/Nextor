# SDK extraction project

The NExtor code is scattered with constant definitions for MSX BIOS routines, work area addresses, driver entry points, driver query codes... that are often repeated across multiple files. Additionally, developers of tools or device drivers need to repeat these definitions themselves too. The only place where reusable constants are properly organized is the codes.mac file, which contains the Nextor function call and error codes.

The plan is to extract an SDK to consolidate all this information in one single place. Additionally to constants, reusable code snippets could be included too (e.g. the routine that drivers use to print a string given a maximum length, or the printf routine used by all the C code); codes.mac could be moved there too.

Actually I was thinking of two SDKs:

- Public SDK, with anything that's useful for tool and driver developers. The idea is to reference the Nextor repository from other repositories as a git submodule, and hydrate only this part of the repository.
- Private SDK that includes constants relevant only for Nextor internally, this allows to avoid repetition. Probably the standard MSX BIOS and work area used by the kernel would go here.

When some piece will be useful both publicly and privately, it goes in the public SDK.

So, your task is to:

- Scan the code and identify repeated constants (or even if not repeated, constants that would make sense to go in an include file) and code snippets that would be useful to external developers.
- Suggest a directory and files tree for these SDKs. Note that we at least need two levels of separation: public vs private, and assembler vs C.
- Ask me anything that's not clear.

Note that not all drivers have been converted to Nextor 3 yet and those that aren't converted are probably defining outdated constants. To be safe, ignore all the drivers that don't have the new "NEXTORv3_DRIVER" signature when scanning the code.

