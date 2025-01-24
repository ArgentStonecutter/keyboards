# Link to patch to clean up the lighting code.

## Branch kb83_simplify

[https://github.com/ArgentStonecutter/qmk_firmware/tree/kb83_simplify]

This is branched from [the last commit before they removed the via code](https://github.com/ArgentStonecutter/qmk_firmware/tree/last_known_good_commit).

## Commit 3b12067 - Remove lighting/keycode shenanigans.

KB83 has a bunch of code that seems to be how to change how lighting changes update, as well as implementing a delayed reset on holding Fn-ESC. This code interferes with changing the mapping for the default lighting code and for adding a new Fn key as a tap-and-hold operation, and doesn't seem to actually do anything useful.

Also, dip_switch_update has the values of the base Mac and Windows layers hardcoded.

This update fixes this and cleans up a lot of unnecessary code.

[https://github.com/ArgentStonecutter/qmk_firmware/commit/3b12067d7ffeaad6069ba986733f61d3e65f79cf]
