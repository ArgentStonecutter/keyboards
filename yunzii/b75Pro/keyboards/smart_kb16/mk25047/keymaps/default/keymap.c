/*
Copyright 2021 owlab
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT(
		KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_DEL,
		KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP,
		KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,  KC_PGDN,
		KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_UP,   KC_END,
		KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, MO(2),   KC_RCTL, KC_LEFT, KC_DOWN,  KC_RIGHT
	),
	[1] = LAYOUT(
		KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_BSPC, KC_DEL,
		KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP,
		KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,  KC_PGDN,
		KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_UP,   KC_END,
		KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, MO(3),   KC_RCTL, KC_LEFT, KC_DOWN,  KC_RIGHT
	),
	[2] = LAYOUT(
      	KC_GRV,    KC_BRID,   KC_BRIU,   LGUI(KC_TAB),  KC_MYCM,   KC_MAIL,   KC_WHOM,     KC_MPRV,     KC_MPLY,     KC_MNXT,     KC_MUTE,   KC_VOLD,   KC_VOLU,    RGB_TOG,   KC_MODE,
		_______,   KC_BLE1,   KC_BLE2,   KC_BLE3,   KC_24G,    _______,    _______,   _______,   _______,   _______,   _______,   _______,   _______,   RGB_MOD,    KC_VAI,
		_______,   KC_MAC,    _______,   _______,   _______,   _______,    _______,   _______,   _______,   _______,   _______,   _______,   RGB_HUI,   KC_VAD,
		_______,   KC_WHITE,  _______,   _______,   _______,   _______,    _______,   _______,   _______,   _______,   _______,   _______,   RGB_VAI,   _______,
		_______,   WIN_LOCK,  _______,                         KC_RESET,                         _______,   _______,   KC_BAT,    RGB_SPD,   RGB_VAD,   RGB_SPI
	),
	[3] = LAYOUT(
      	KC_GRV,    KC_1,   	  KC_2,   	 KC_3,  	KC_4,      KC_5,   	   KC_6,      KC_7,      KC_8,      KC_9,      KC_0,      _______,   _______,   RGB_TOG,   KC_MODE,
		_______,   KC_BLE1,   KC_BLE2,   KC_BLE3,   KC_24G,    _______,    _______,   _______,   _______,   _______,   _______,   _______,   _______,   RGB_MOD,   KC_VAI,
		_______,   KC_MAC,    _______,   _______,   _______,   _______,    _______,   _______,   _______,   _______,   _______,   _______,   RGB_HUI,   KC_VAD,
		_______,   KC_WHITE,  _______,   _______,   _______,   _______,    _______,   _______,   _______,   _______,   _______,   _______,   RGB_VAI,   _______,
		_______,   WIN_LOCK,  _______,                         KC_RESET,                         _______,   _______,   KC_BAT,    RGB_SPD,   RGB_VAD,   RGB_SPI
	),
	[4] = LAYOUT(
		KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_DEL,
		KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP,
		KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,  KC_PGDN,
		KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_UP,   KC_END,
		KC_LCTL, KC_Lopt, KC_Lcmd,                            KC_SPC,                             KC_Rcmd, MO(6),  KC_RCTL, KC_LEFT, KC_DOWN,  KC_RIGHT
	),
	[5] = LAYOUT(
		KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_BSPC, KC_DEL,
		KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP,
		KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,  KC_PGDN,
		KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_UP,   KC_END,
		KC_LCTL, KC_Lopt, KC_Lcmd,                            KC_SPC,                             KC_Rcmd, MO(7),   KC_RCTL, KC_LEFT, KC_DOWN,  KC_RIGHT
	),
	[6] = LAYOUT(
        KC_GRV,    KC_BRID,   KC_BRIU,    KC_Lpad,    KC_Mctl,    _______,    _______,    KC_MPRV,    KC_MPLY,    KC_MNXT,    KC_MUTE,   KC_VOLD,   KC_VOLU,  RGB_TOG,  KC_MODE,
		_______,   KC_BLE1,   KC_BLE2,    KC_BLE3,    KC_24G,	  _______,    _______,    _______,    _______,    _______,    _______,   _______,   _______,  RGB_MOD,  KC_VAI,
		_______,   _______,   KC_WIN,     _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,   _______,   RGB_HUI,  KC_VAD,
		_______,   KC_WHITE,  _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,   _______,   RGB_VAI,  _______,
		_______,   _______,   _______,                            KC_RESET,                           _______,    _______,    KC_BAT,    RGB_SPD,   RGB_VAD,  RGB_SPI
	),
	[7] = LAYOUT(
		KC_GRV,    KC_1,   	  KC_2,   	  KC_3,  	  KC_4,       KC_5,   	  KC_6,       KC_7,       KC_8,       KC_9,       KC_0,      _______,   _______,  RGB_TOG,  KC_MODE,
		_______,   KC_BLE1,   KC_BLE2,    KC_BLE3,    KC_24G,	  _______,    _______,    _______,    _______,    _______,    _______,   _______,   _______,  RGB_MOD,  KC_VAI,
		_______,   _______,   KC_WIN,     _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,   _______,   RGB_HUI,  KC_VAD,
		_______,   KC_WHITE,  _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,   _______,   RGB_VAI,  _______,
		_______,   _______,   _______,                            KC_RESET,                           _______,    _______,    KC_BAT,    RGB_SPD,   RGB_VAD,  RGB_SPI
	),
};
