## Warning: there is apparrently a new version of the J2 that can be identified by the addition of a Win-Mac switch. The firmware in the Jamesdonkey repo does not seem to have been updated but I would recommend not trying to use the older version firmware on it.

# Jamesdonkey J2 experimental firmware

Added a Mac layer and a backlight layer, and the globe key mod from https://skip.house/blog/qmk-globe-key.

Use Fn + right-alt/right-command to switch to Mac layer. This swaps left-alt and left-command and converts right-alt into right-command and back again.

Use Fn-right-control to access the backlight layer.

Use Fn-right-control-page-down to toggle the layer-backlight mode, try it.

* https://github.com/ArgentStonecutter/jamesdonkey_qmk_firmware/tree/j2_sandbox/keyboards/jamesdonkey/j2

Note that the new names for the two new custom keys (CF_TOGGLE and AP_GLOBE) will not show up in Launcher, just in VIA. They are CUSTOM(16) and CUSTOM(17).
