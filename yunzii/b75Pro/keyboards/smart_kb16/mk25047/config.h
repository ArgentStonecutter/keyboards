/* Copyright 2022 Jacky
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#define     AP_RGB_TEST            0x81             // RGB测试
/*Keyboard Mapping Layer Config*/
#define DYNAMIC_KEYMAP_LAYER_COUNT 4
/*UART Config*/
#define SERIAL_DRIVER SD1
#define SD1_TX_PAL_MODE PAL_MODE_ALTERNATE_PUSHPULL
/*EEPROM Config*/
#define WEAR_LEVELING_LOGICAL_SIZE 4096
#define WEAR_LEVELING_BACKING_SIZE (WEAR_LEVELING_LOGICAL_SIZE * 2)
/*module updata*/
#define Module_Updata
// #define Screen_AP
/*Number of RGB per DRIVER Config*/
#define DRIVER_1_LED_TOTAL 44
#define DRIVER_2_LED_TOTAL 41
#define RGB_MATRIX_LED_COUNT (DRIVER_1_LED_TOTAL + DRIVER_2_LED_TOTAL)
/*DRIVER Config*/
#define SPI_DRIVER SPID1
#define AW_SPI_MODE 3
#define SPI_SCK_PIN  A5
#define SPI_MISO_PIN A6
#define SPI_MOSI_PIN A7
#define DRIVER_1_CS C4
#define DRIVER_2_CS B8
#define DRIVER_1_EN B7  // 相当于B5
#define DRIVER_2_EN B7

#define DRIVER_COUNT 2

#define DEBOUNCE 2

#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_KEYPRESSES

#define ENCODER_RESOLUTION 4

#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 255
#define RGB_MATRIX_VAL_STEP 64
#define RGB_MATRIX_SPD_STEP 64

#define RGB_MATRIX_DEFAULT_VAL 192
/*RGBLIGHT的参数设置*/
#define RGBLIGHT_LIMIT_VAL 200
#define RGBLIGHT_DEFAULT_VAL  200
#define RGBLIGHT_VAL_STEP 50
#define RGBLIGHT_DEFAULT_MODE   RGBLIGHT_MODE_RAINBOW_SWIRL+5
#define RGBLIGHT_DISABLE_KEYCODES
/*定义有哪些中断线*/
#define INTERRUPT_TRIGGER_0
#define INTERRUPT_TRIGGER_1
#define INTERRUPT_TRIGGER_2
#define INTERRUPT_TRIGGER_3
#define INTERRUPT_TRIGGER_4
#define INTERRUPT_TRIGGER_5

#define EECONFIG_KB_DATA_SIZE  13
