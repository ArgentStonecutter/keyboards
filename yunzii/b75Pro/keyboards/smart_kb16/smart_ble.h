/*
    Copyright (C) 2023 Jacky
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
        http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once
#include "quantum.h"

enum kb_mode_t {
  KB_MODE_USB=0,
  KB_MODE_BLE,
  KB_MODE_24G,
  KB_DEFULT_MODE
};

void sc_ble_battary(uint8_t batt_level);
void WIRELESS_START(uint32_t mode);
void WIRELESS_STOP(void);
void WIRELESS_PAIR(uint32_t mode);
void encode_boot(void);
void Module_UpdataHandle(void);

typedef struct  {
  bool      eeconfig_higher_blink_flag;
  bool      eeconfig_rgb_matrix_off_flag;
  bool      eeconfig_rgblight_off_flag;
  bool      eeconfig_win_lock_flag;
  uint8_t   eeconfig_nkro_flag;
  uint8_t   eeconfig_last_wireless_mode;
  bool      eeconfig_rotary_mode;
  bool      eeconfig_orient_mode;
  bool      eeconfig_SCREEN_switch_flag;                // Screen's Switch
  bool      eeconfig_SCREEN_off_flag_letter;            // Display word yes or on
  uint8_t   eeconfig_SCREEN_flag_state;                 // Screen's state
  uint8_t   eeconfig_SCREEN_flag_time;                  // Screen's time
  bool      eeconfig_RGB_flag_WHITE;
}MyVariables;

MyVariables  variable_data;






