## Summary

<!--
A short explanation of the changes introduced by this pull request. Ideally one or at most two short paragraphs.
-->

<!--
If the pull request fixes an existing issue from https://github.com/Konamiman/Nextor/issues/, add the issue number at the end of the line below. Otherwise delete the line.
-->

Closes #

## Details

<!--
A detailed explanation of what this pull request does. Unless the pull request only touches documentation or the build pipeline, please include:

- What was broken, missing or suboptimal in the code and, if applicable, in the user experience.
- What changes are introduced in the code and, if applicable, in the user experience. If applicable and possible, include "before" and "after" screenshots, for example:

Before                 |  After
:---------------------:|:---------------------:
![](before-image-url)  |  ![](after-image-url)

- Any relevant technical details, e.g. more details on the code changes.
- What's still broken or suboptimal and why a fix is not suitable for this pull request, if applicable.
-->

## Testing instructions

<!--
Not needed if the pull request only touches documentation or the build pipeline.

For bug fixes and new/enhanced features: how to test that the broken user flow is fixed, or how to test the feature.

For code optimizations and other similar changes that don't modify the user experience: how to test that the changes didn't break anything (how to exercise the user flows that depend on the code that has been changed).
-->

## Checklist

<!--
Check the items that apply (put an "x" between the brackets) and fill in the ones that ask for details. Delete the items that don't apply, or the whole section if the pull request only touches documentation or the build pipeline.
-->

- [ ] Tested in MSX-DOS 2 mode (normal boot).
- [ ] Tested in MSX-DOS 1 mode (boot while pressing the "1" key).
- [ ] Tested in an emulator (emulator and emulated machine):
- [ ] Tested in real hardware (computer and storage device):
- [ ] The documentation in `docs/` has been updated to reflect the changes.
- [ ] The help texts (`CALL` commands help in the kernel, help texts embedded in the `.COM` tools, help files in `source/commandcom/helpfiles`) have been updated to reflect the changes.
- [ ] New kernel code doesn't use undocumented Z80 instructions directly: the macros in `sdk/asm/macros/undoc.inc` are used instead, so that the kernel still works when built with `NO_UNDOC_CPU_INSTRUCTIONS`.
