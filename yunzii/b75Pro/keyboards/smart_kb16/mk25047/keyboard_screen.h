#pragma once
// #ifndef KEYBOARD_SCREEN_H
// #define KEYBOARD_SCREEN_H

// #include <zephyr.h>
// #include <soc.h>
// #include <console/console.h>
// #include "uart_mod.h"
// #include "common.h"


#include "uart_mod.h"


#define     AP_W_SCREEN_INFO        0x40    // 屏信息包
#define     AP_W_SCREEN_DATA        0x41    // 屏数据包
#define     AP_GIVE_SCREEN_SEM        0x42    // 释放信号量

#define KEYBOARD_SCREEN_ENABLE      1

#define R_FLAG(flag,type)				( flag[type/32] &   (1<<(type%32)) )
#define CLEAR_FLAG(flag,type) 			( flag[type/32] |=  (1<<(type%32)) )
#define SET_FLAG(flag,type) 			( flag[type/32] &= ~(1<<(type%32)) )
#define NEED_UPDATE						0


extern uint16_t screen_boot_time;
// extern uint8_t screen_boot_step;
extern uint8_t f_os_mode;
extern uint8_t Keyboard_mode;
extern uint8_t temp_u8;
extern uint8_t pre_batt_status;
extern uint8_t Batt_Baifen;
extern bool flag_SCREEN_switch;
// extern uint8_t flag_press_letter;

enum{
    KB_ID,
};

enum{
    SCREEN_BATT_UNCHARGING,
    SCREEN_BATT_WARNGING,
    SCREEN_BATT_CHARGING,
    SCREEN_BATT_LOW_POWER,
};

enum{
    batt_default,
    batt_is_charging1,
    batt_charge_full1,
    batt_no_charge1,
    batt_warning,
};


typedef struct{
    uint32_t flag[1];
	uint8_t *buff;
    uint8_t screen_boot_step;
    uint16_t screen_boot_time;
    uint32_t ap_countdown;
}KEYBOARD_SCREEN_T;


extern KEYBOARD_SCREEN_T kb_screen;
void keyboard_screen_init(void);
void keyboard_screen_process(void);

// #endif
