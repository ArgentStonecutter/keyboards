# Improved VIAL layout with backlight color display

### Holding the Fn key illuminates the keys with a "Function" keycode.

![Function layer illuminated](hyper4.png)

### Holding the Hyper key illuminates the keys with a "Hyper" keycode.

![Hyper layer illuminated](hyper5.png)

## How it works:

Find the code here:

* https://github.com/ArgentStonecutter/vial-qmk/tree/inland-mk47-colors/keyboards/inland/mk47

keymap "vialstock" matches the stock MK47 VIA configurations, except that Fn-Space turns on the function hilights.

keymap "vial":

layer 0 - default layer<br>
![Layer 0](layer0.png)<br>
Bottom row has been changed to match the provided keycaps. `Hyper` is layer 5 (media and backlight), `Fn` is layer 4 (alternate keycodes), and Lower/Raise provide access to layers 1-3 for user use.

layer 1, 2, 3 - blank<br>
![Layer 1](layer1.png)<br>
Unused in the default layout, all identical.

layer 4 - Fn - alternate keycodes<br>
![layer 4](layer4.png)<br>
The existing `<>?` keys map to the second row bracket/backslash.
The remaining keys are mapped over `ZXCV`

layer 5 - Hyper - media and backlight<br>
![layer 5](layer5.png)<br>
Hyper exposes the displayed media controls, and `ZXCV` get the remaining backlight functions.
Hyper-Lower and -Raise are the volume contorls, and Hyper-Space turns on the function backlights.
