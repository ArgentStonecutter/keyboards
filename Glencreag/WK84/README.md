The two QMK source trees are basically identical, except:

* The RGB tree has keymaps/via populated with the same keymap.c and a rules.mk containing `VIA_ENABLE = yes`
* The QMK tree has `"device_version": "0.1.6"` in keyboard.json instead of `0.1.5`.

*Jazz hands*
