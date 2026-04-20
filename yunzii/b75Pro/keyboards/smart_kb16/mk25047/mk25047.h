#pragma once
#include "quantum.h"

#define OSC_IN       D0
#define OSC_OUT      D1
#define RENUM_PIN    A8
#define PLUG_IN      B9
#define BT_MODE      C15
#define PRF_MODE     C14
#define EN_BACKLIT   B5
#define LED_CAP      C9
#define LED_WIN      D2
// #define MAC_WIN      C13
#define SCREEN_EN    B3
#define ENL_PIN      B10
#define ENR_PIN      B11
#define NRST         C13


enum custom_keycodes {
#ifdef VIA_ENABLE
//    KC_USB = QK_USER,
    KC_24G = QK_KB_0,
#else
    KC_24G = SAFE_RANGE,
#endif
    KC_BLE1,
    KC_BLE2,
    KC_BLE3,
    KC_Mctl,
    KC_Lpad,
    KC_Lopt,
    KC_Lcmd,
    KC_Rcmd,
    KC_MAC,
    KC_WIN,
    WIN_LOCK,
    KC_RESET,
    KC_BAT,
    KC_MODE,
    KC_VAI,
    KC_VAD,
    KC_WHITE,
    ROTARY_MOD,
    ROTARY_LEFT,
    ROTARY_RIGHT,
    ORIENT_MOD,
    MOD_DIS,
    Scr_Home,
    Scr_Pic,
    Scr_Gif,
    Scr_EN,
    Scr_Time
};

extern bool win_lock_flag;
extern bool flag_win_lock;

enum kb_mode_t get_kb_mode(void);
bool get_plug_mode(void);
void communicate_mode_toggle(void);
void avoid_hold_key_when_usb_wakeup(void);
void ble_send_batt(void);
void key_pressd_update_heartbeat_time(void);
void toggle_layer(void);
void key_nkro_toggle(void);
void mode_indicator_light_init(void);
void wireless_connected_or_disconnected_operate(void);
void mode_indicator_light_init(void);
void usb_suspend_power_down(void);
void First_Level_Sleep(void);
void Second_Level_Sleep(void);
void sleep_mode(void);
void mcu_reset_init(void);
void slow_switch_fast(void);
void set_row_and_col_when_sleep(void);
void key_debounce(void);
