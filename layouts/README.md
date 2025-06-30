QMK/VIA layout files for various keyboards:
-----

* GK61 - Skyloong GK61 (Pro and Dual Spacebar versions)
* QK61 - CIDOO QK61
* RK61 - Royal Kludge RK61
* K717 - Redragon Alcor K717
* WK61 - Womier WK61 Pro

The Y&R 6096 is a different layout, see subdirectory.

These layouts all have the following setup:
-----

![RK61/GK61/QK61 layout](https://raw.githubusercontent.com/ArgentStonecutter/keyboards/refs/heads/main/layouts/XX61-layout.png)

Fn next to spacebar on the right.

Modifiers in the lower right corner, Righ-shift, Right-alt, Right-GUI, Right-control. The actual key changes are:

* Right-shift: `MT(MOD_LSFT | MOD_RSFT,KC_UP)`
* Function: `MO(N)` (where N is whatever is the function layer)
* Right-alt: `MT(MOD_LALT | MOD_RALT,KC_LEFT)`
* Right-GUI: `MT(MOD_LGUI | MOD_RGUI,KC_DOWN)`
* Right-control: `MT(MOD_LCTL | MOD_RCTL,KC_RGHT)`


Tapping on these generates arrow keys. Holding them they act as modifiers.

Tap and hold on Tab as a left-hand Fn key

Fn-arrows produce Left->Home / Up->PgUp / Down->PgDn / Right->End

Tap and Hold on Caps-lock as a secondary Fn key for media and backlight controls[^1].

Caps-up is volume up, Caps-down is volume-down[^2], other keys may vary, see VIA.

The RK61 comes with 16 layers so I have added a Mac layout with LGUI/LALT swapped, Fn-M for Mac, Fn-W for Windows. [Imgur - RK61 layers](https://imgur.com/a/custom-rk61-mods-nDKc9Kb)

[^1]: K717 layer 1 seems a bit dodgy, avoiding easy access. Use Fn-LALT for L1 access.
[^2]: K717 does not have this because the default keymap uses Fn-Shift to access layer 4 for things like factory reset.


