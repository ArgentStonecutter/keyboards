The two QMK source trees are basically identical, except:

* The RGB tree has keymaps/via populated with the same keymap.c and a rules.mk containing `VIA_ENABLE = yes`
* The QMK tree has `"device_version": "0.1.6"` in keyboard.json instead of `0.1.5`.

And... neither of them build in current QMK, so don't even bother. The author's got a working current tree in github.

* https://github.com/gskygithub/qmk_firmware/tree/WK84/keyboards/glencreag/wk84

Also attached: my VIAL firmware based on this code.

* https://github.com/ArgentStonecutter/vial-qmk/tree/glencreag/keyboards/glencreag/wk84
