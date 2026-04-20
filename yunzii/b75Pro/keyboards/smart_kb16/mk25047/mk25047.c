// /* Copyright 2022 Jacky
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 2 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
//  */
#include "quantum.h"
#include "uart.h"
#include "smart_ble.h"
#include "keyboard.h"
#include "analog.h"
#include "print.h"
#include "usb_main.h"
#include "usb_util.h"
#include "usb_driver.h"
#include "os_detection.h"
#include "adc.h"
#include "rtc.h"
#include "battery.h"
#include "mk25047.h"
#include "process_magic.h"
#include "keyboard_screen.h"
#include "common.h"
#include "hygui.h"
#include "raw_hid.h"

#ifdef Screen_AP
uint8_t	 time_50ms_count;
#endif

#define LONG_PRESS_ORIENT_SWITCH        2000  // 长按方向切换�?时间

uint8_t test_rgb_r = 0;
uint8_t test_rgb_g = 0;
uint8_t test_rgb_b = 0;

extern bool         peek_matrix(uint8_t row_index, uint8_t col_index, bool read_raw); //这个函数�?以�?�测按�?�?否一直�?�于�?按下
extern matrix_row_t raw_matrix[MATRIX_ROWS];                                          // raw values
/*loop_10Hz函数的定�?*/
#define LOOP_10HZ_PERIOD 100
deferred_token loop10hz_token = INVALID_DEFERRED_TOKEN;
uint32_t       loop_10Hz(uint32_t trigger_time, void *cb_arg);

/***********************切换通�??模式***************************/
enum kb_mode_t new_mode;
enum kb_mode_t new_kb_mode;
/***********************外部变量和一些枚举变�?***************************/
extern enum kb_mode_t kb_mode;
extern bool           wireless_connected;           /*连接标志�?*/
extern bool           suspend;                      /*2.4G休眠标志�?*/
extern bool           first_sleep_flag;             /*进入一级休眠标志位*/
extern bool           interrupt_source_flag;        /*按键�?�?来源标志�?*/
extern bool           rgb_matrix_off_flag;          /*关闭矩阵�?标志�?*/
extern bool           rgblight_off_flag;
extern bool           keyboard_no_idle_flag;        /*按键休眠不分连接以解决切模式异常*/
extern bool           rtc_interrupt_source_flag;    /*rtc�?�?�?*/
extern uint32_t       last_wireless_mode;           /*用来记忆之前的无线模�?*/
extern uint32_t       wireless_no_operation_time;   /*无线模式无操作�?�时*/
extern uint32_t       first_sleep_time;             /*发送心跳包计时*/
extern uint32_t       packet_send_time;             /*发送心跳包次数*/
extern uint32_t       keyboard_idle_time;           /*按键空闲计时*/
/*battery.h*/
extern enum batt_charge_status_t batt_charge_status;
extern bool                      battery_first_test_flag;   /*一旦重新上电就会重新�?�测一�?adc的标志位*/
extern bool                      ble_connect_flag;          /*蓝牙连接标志�?*/
extern bool                      battery_level_report_flag; /*电池待更新标�?*/
extern uint8_t                   battery_level;             /*电池电量百分�?*/
extern uint8_t                   temp;                      /*电量百分�?*/
extern uint8_t                   last_temp;                 /*记忆上�?�电量百分比*/
bool                             batt_is_charging_flag;
uint32_t                         batt_is_charging_time;
bool                             batt_charge_full_flag;
uint32_t                         batt_charge_full_time;
/*蓝牙连接标志�?*/
bool     ble_connect_flag  = false;
/****************变量定义************************/
uint8_t blink_count        = 0;
uint8_t blink_period       = 5;
uint8_t blink_rgb_count    = 0;
uint8_t blink_rgb_period   = 5;
uint8_t blink_reset_count  = 0;
uint8_t blink_reset_period = 5;
/***********************模式指示�?***************************/
#define  LONG_PRESS_TIME    3000        // 3000ms
bool     wireless_connect_flag = false; //无线连接标志�?
bool     long_press_ble_flag   = false; //长按蓝牙通道标志�?
bool     long_press_24G_flag   = false; //长按2.4G通道标志�?
uint32_t long_press_ble_time;           //蓝牙通道�?按下计时变量
uint32_t long_press_24G_time;           // 2.4G通道�?按下计时变量
uint32_t blink_wireless_time;           //正常�?烁�?�时
uint32_t higher_blink_wireless_time;    //�?�?计时
uint32_t ble_connect_time;
uint32_t prf_connect_time;
/***********************�?定义功能�?的变�?***************************/
/*复位*/
uint32_t long_press_reset_time; //按下复位�?开始�?�时变量
uint8_t  blink_reset_time;
uint8_t  mcu_reset_flag;
bool     reset_flag        = false; //恢�?�出厂标志位
bool     higher_blink_flag = false;
/*锁win*/
bool win_lock_flag        = false;
bool memory_win_lock_flag = false;
/*电量查�??*/
bool battery_consult_flag = false;
/*双击切层*/
uint8_t     set_layer_count;
uint32_t    set_layer_time;
/*白灯�?�?*/
bool        white_rgb_flag = false;
/***********************休眠***************************/
#define     RGB_MATRIX_WIRELESS_TIME    300000  // 原�?�版�?
#define     PACKET_SEND_TIME            RGB_MATRIX_WIRELESS_TIME/8000+1
/*USB跟随休眠*/
bool     usb_wakeup_send_code_flag;
uint8_t  release_count    = 0;  // usb休眠唤醒添加释放包，防�??hold�?
uint8_t  usb_suspend_flag = 2;  // usb检测到suspend标志�?;0：ACTIVE和在USB模式�?;1:SUSPEND和在USB模式�?;2:不在0,1的情况；
uint32_t usb_suspend_time;      // usb检测到suspend计时
uint32_t wakeup_count;
/*无线模式超时休眠*/
bool     wakeup_first_sleep_flag      = false;
uint32_t wakeup_first_sleep_time;
/*2.4G跟随休眠*/
bool     sleep_24G_flag = false; /*2.4G进入休眠标志�?*/
/*休眠关灯标志�?*/
bool     enable_rgb_pin_flag  = false;
uint8_t  rgb_matrix_off_state = 0;

/*旋转编码器变�?*/
bool switch_rotary_mod;
bool flag_rotary_mod;
bool mod_rotary = false;
uint32_t time_rotary_mod;
/*模式指示变量*/
bool flag_mod_dis = false;
/*WASD方向变量*/
bool switch_orient_mod = false;
bool mod_orient = false;
uint32_t time_common;
/*速度挡位溢出检测变�?*/
bool flag_speed_overflow = false;
uint8_t count_speed_flash = 0;
const uint8_t COUNT_SPEED_FLASH_NEED = 3;  // �?光闪烁�?�数
uint8_t blink_speed_count;
uint8_t blink_speed_period;

/*Screen's macro and const*/

const uint8_t SCREEN_DEFAULT    = 0;
const uint8_t SCREEN_HOME       = 1;
const uint8_t SCREEN_PICTURE    = 2;
const uint8_t SCREEN_GIF        = 3;

/*Screen's variables*/
bool     flag_SCREEN_switch                     = false;                // Screen's Switch
bool     flag_SCREEN_off_letter                 = false;                // Display word yes or on
uint8_t  flag_SCREEN_state                      = SCREEN_DEFAULT;       // Screen's state of print
bool     flag_SCREEN_init                       = true;                 // Screen's init
bool     flag_SCREEN_time                       = false;                // Screen's time

static const SerialConfig uart_config = {
    .speed = 921600,
};

/*Screen's Process*/
void screen_process(void) {
    /*Screen's Init*/
    if (flag_SCREEN_init) {  // after sleep or after off
        flag_SCREEN_init = false;
        /*delay 100ms*/
        wait_ms(3);
        SET_FLAG( kb_screen.flag , PK_CAPS_STATUS );
        if (eeconfig_read_default_layer() <= 2) {
            f_os_mode = 0;
        }
        else if (eeconfig_read_default_layer() >= 4) {
            f_os_mode = 1;
        }
        /*Screen's time*/
        if (flag_SCREEN_time != variable_data.eeconfig_SCREEN_flag_time) {
            // variable_data.eeconfig_SCREEN_flag_time = flag_SCREEN_time;
            flag_SCREEN_time = variable_data.eeconfig_SCREEN_flag_time;
            eeconfig_update_kb_datablock(&variable_data);
        }
        wait_ms(3);
        SET_FLAG( kb_screen.flag , PK_OS_TYPE );
        wait_ms(3);
        SET_FLAG( kb_screen.flag , PK_WINLOCK_STATUS );
        wait_ms(3);
        SET_FLAG(kb_screen.flag, PK_BATT_STATUS);
        wait_ms(3);
        SET_FLAG( kb_screen.flag , PK_BATT_QUANTITY);
        wait_ms(3);
        SET_FLAG( kb_screen.flag , PK_CONN_TYPE );
        wait_ms(3);
        SET_FLAG( kb_screen.flag , PK_BATT_QUANTITY);
        wait_ms(3);
        SET_FLAG( kb_screen.flag , PK_SWITCH_TIME);
    }
    /*Screen's Switch*/
    if (flag_SCREEN_switch && gpio_read_pin(SCREEN_EN)) {  // only enter once
        gpio_write_pin_low(SCREEN_EN);
        if(variable_data.eeconfig_SCREEN_switch_flag != flag_SCREEN_switch){  // flash
            variable_data.eeconfig_SCREEN_switch_flag = flag_SCREEN_switch;
            eeconfig_update_kb_datablock(&variable_data);
        }
    }
    else if (!flag_SCREEN_switch && !gpio_read_pin(SCREEN_EN)) {  // only enter once
        gpio_write_pin_high(SCREEN_EN);
        if(variable_data.eeconfig_SCREEN_switch_flag != flag_SCREEN_switch){  // flash
            variable_data.eeconfig_SCREEN_switch_flag = flag_SCREEN_switch;
            eeconfig_update_kb_datablock(&variable_data);
        }
        flag_SCREEN_init = true;
    }
    /*Screen's state of connection*/
    if (kb_mode == KB_MODE_USB && Keyboard_mode != 0) {
        wait_ms(2);
        Keyboard_mode = 0;
        SET_FLAG( kb_screen.flag , PK_CONN_TYPE );
    }
    else if (kb_mode == KB_MODE_BLE && Keyboard_mode != last_wireless_mode) {
        wait_ms(2);
        Keyboard_mode = last_wireless_mode;
        SET_FLAG( kb_screen.flag , PK_CONN_TYPE );
    }
    else if (kb_mode == KB_MODE_24G && Keyboard_mode != 4) {
        wait_ms(2);
        Keyboard_mode = 4;
        SET_FLAG( kb_screen.flag , PK_CONN_TYPE );
    }
    /*Screen's power*/
    if (Batt_Baifen != temp) {
        wait_ms(2);
        Batt_Baifen = temp;
        SET_FLAG( kb_screen.flag , PK_BATT_QUANTITY);
    }
    if (pre_batt_status != batt_charge_status) {
        wait_ms(2);
        pre_batt_status = batt_charge_status;
        SET_FLAG( kb_screen.flag , PK_BATT_STATUS );
    }
    /*Screen's State*/
    if (flag_SCREEN_state  == SCREEN_DEFAULT) {
        flag_SCREEN_state = variable_data.eeconfig_SCREEN_flag_state;
        if (flag_SCREEN_state == SCREEN_HOME) {
            wait_ms(2);
            SET_FLAG(kb_screen.flag, PK_GO_HOME);
        }
        else if (flag_SCREEN_state == SCREEN_PICTURE) {
            wait_ms(2);
            SET_FLAG(kb_screen.flag, PK_TOGGLE_PIC);
        }
        else if (flag_SCREEN_state == SCREEN_GIF) {
            wait_ms(2);
            SET_FLAG(kb_screen.flag, PK_GO_GIF);
        }
    }
    else {
        if (flag_SCREEN_state != variable_data.eeconfig_SCREEN_flag_state) {
            variable_data.eeconfig_SCREEN_flag_state = flag_SCREEN_state;
            eeconfig_update_kb_datablock(&variable_data);
        }
    }
}

/*�?光函�?*/
void light_function(void) {
    if (count_speed_flash && blink_speed_count > blink_period) {
        gpio_write_pin_low(LED_CAP);
        gpio_write_pin_low(LED_WIN);
    }
    else if (count_speed_flash){
        gpio_write_pin_high(LED_CAP);
        gpio_write_pin_high(LED_WIN);
    }
    else if (!count_speed_flash && !wireless_connected && kb_mode != KB_MODE_USB) {
        if (!gpio_read_pin(LED_CAP)) {
            gpio_write_pin_high(LED_CAP);
        }
        if (!gpio_read_pin(LED_WIN)) {
            gpio_write_pin_high(LED_WIN);
        }
    }
}
/*插入检�?  PLUG_IN为高电平USB插入;PLUG_IN为低电平USB没插1*/
bool get_plug_mode(void) {
    if (readPin(PLUG_IN))
        return true;
    else
        return false;
}

/*模式切换，带强转功能*/
enum kb_mode_t get_kb_mode(void) {
    if (!gpio_read_pin(BT_MODE)) {
        new_mode = KB_MODE_BLE;
    } else if (!gpio_read_pin(PRF_MODE)) {
        new_mode = KB_MODE_24G;
    } else {
        new_mode = KB_MODE_USB;
    }
    return new_mode;
}

/*模式切换*/
void communicate_mode_toggle(void) {
    new_kb_mode = get_kb_mode();
    /*具体进入那个模式*/
    if (kb_mode != new_kb_mode) // ONLY DO IT ONCE WHEN MODE SWITCHED
    {
        kb_mode = new_kb_mode;
        if (higher_blink_flag == false) {
            if (kb_mode == KB_MODE_BLE) {
                if (last_wireless_mode > 3) {
                    last_wireless_mode = last_wireless_mode & 0x03;
                }
                /*开�?蓝牙通道*/
                WIRELESS_START(last_wireless_mode);
                blink_wireless_time = timer_read32();
                /*发送心跳包变量*/
                first_sleep_time = timer_read32();
                packet_send_time       = 0;
            } else if (kb_mode == KB_MODE_24G) {
                WIRELESS_START(4);
                blink_wireless_time = timer_read32(); //慢闪超时计时
                /*发送心跳包变量*/
                first_sleep_time = timer_read32();
                packet_send_time  = 0;
            } else if (kb_mode == KB_MODE_USB) {
                WIRELESS_STOP();
            }
            wireless_no_operation_time  = timer_read32(); //无线模式无操作�?�时
            blink_count                 = 0;
            blink_period                = 5;
        } else {
            if (kb_mode != KB_MODE_USB) {
                if (kb_mode == KB_MODE_BLE) {
                    WIRELESS_PAIR(last_wireless_mode);
                } else {
                    WIRELESS_PAIR(4);
                }
                wireless_no_operation_time = timer_read32(); //无线模式无操作�?�时
                higher_blink_wireless_time = timer_read32();
                /*发送心跳包变量*/
                first_sleep_time = timer_read32();
                packet_send_time  = 0;
                blink_count       = 0;
                blink_period      = 2;
            } else {
                WIRELESS_STOP();
            }
            higher_blink_flag                        = false;
            variable_data.eeconfig_higher_blink_flag = higher_blink_flag;
            eeconfig_update_kb_datablock(&variable_data);
        }
    }
}

/*无线模式�?开或连上�?�改变的一些变�?*/
void wireless_connected_or_disconnected_operate(void) {
    /*无线�?�?*/
    if (!wireless_connected) {
        if (wireless_connect_flag == true) {
            blink_wireless_time   = timer_read32();
            wireless_connect_flag = false;
            ble_connect_flag      = false;
        }
    }
    /*无线连接上操�?*/
    if (wireless_connected) {
        if (wireless_connect_flag == false) {
            wireless_no_operation_time = timer_read32();
            blink_period                = 5;
            //发送心跳包变量
            first_sleep_time     = timer_read32();
            packet_send_time      = 0;
            wireless_connect_flag = true;
        }
    }
}

/*模式指示�?*/
void mode_indicator_light_init(void) {
    if (wireless_connected && flag_mod_dis) {
        switch(last_wireless_mode) {
            case 1:
                rgb_matrix_set_color(84, 0, 0, 144);
                rgb_matrix_set_color(83, 0, 0, 0);
                break;
            case 2:
                rgb_matrix_set_color(84, 144, 144, 0);
                rgb_matrix_set_color(83, 0, 0, 0);
                break;
            case 3:
                rgb_matrix_set_color(84, 144, 0, 0);
                rgb_matrix_set_color(83, 0, 0, 0);
                break;
            default:
                rgb_matrix_set_color(83, 0, 144, 0);
                rgb_matrix_set_color(84, 0, 0, 0);
                break;
        }
    }
    if (kb_mode == KB_MODE_BLE && !wireless_connected) {
        rgb_matrix_off_state |= 0x01 << 1;
        ble_connect_time = timer_read32();
        switch (last_wireless_mode) {
            case 1:
                if (blink_count > blink_period) {
                    rgb_matrix_set_color(84, 0, 0, 144);
                    rgb_matrix_set_color(83, 0, 0, 0);
                }
                else {
                    rgb_matrix_set_color(84, 0, 0, 0);
                    rgb_matrix_set_color(83, 0, 0, 0);
                }
                break;
            case 2:
                if (blink_count > blink_period) {
                    rgb_matrix_set_color(84, 144, 144, 0);
                    rgb_matrix_set_color(83, 0, 0, 0);
                }
                else {
                    rgb_matrix_set_color(84, 0, 0, 0);
                    rgb_matrix_set_color(83, 0, 0, 0);
                }
                break;
            case 3:
                if (blink_count > blink_period) {
                    rgb_matrix_set_color(84, 144, 0, 0);
                    rgb_matrix_set_color(83, 0, 0, 0);
                }
                else {
                    rgb_matrix_set_color(84, 0, 0, 0);
                    rgb_matrix_set_color(83, 0, 0, 0);
                }
                break;
            default:
                break;
        }
    } else if (kb_mode == KB_MODE_BLE && wireless_connected) {
        if(timer_elapsed32(ble_connect_time) < UINT32_MAX){
            rgb_matrix_off_state |= 0x01 << 1;
            switch (last_wireless_mode) {
                case 1:
                    rgb_matrix_set_color(84, 0, 0, 144);
                    rgb_matrix_set_color(83, 0, 0, 0);
                    break;
                case 2:
                    rgb_matrix_set_color(84, 144, 144, 0);
                    rgb_matrix_set_color(83, 0, 0, 0);
                    break;
                case 3:
                    rgb_matrix_set_color(84, 144, 0, 0);
                    rgb_matrix_set_color(83, 0, 0, 0);
                    break;
                default:
                    break;
            }
        }else {
            rgb_matrix_off_state &= ~(0x01 << 1);
        }
    }else if (kb_mode == KB_MODE_24G && !wireless_connected) {
        rgb_matrix_off_state |= 0x01 << 2;
        prf_connect_time = timer_read32();
        if (blink_count > blink_period) {
            rgb_matrix_set_color(83, 0, 144, 0);
            rgb_matrix_set_color(84, 0, 0, 0);
        }
        else {
            rgb_matrix_set_color(83, 0, 0, 0);
            rgb_matrix_set_color(84, 0, 0, 0);
        }
    } else if (kb_mode == KB_MODE_24G && wireless_connected) {
        if(timer_elapsed32(prf_connect_time) < UINT32_MAX){
            rgb_matrix_off_state |= 0x01 << 2;
            rgb_matrix_set_color(83, 0, 144, 0);
            rgb_matrix_set_color(84, 0, 0, 0);
        }else {
            rgb_matrix_off_state &= ~(0x01 << 2);
        }
        if (sleep_24G_flag == true) {
            tap_code16(KC_F24);
            sleep_24G_flag = false;
        }
    }else {
        rgb_matrix_off_state &= ~(0x03 << 1);
    }
}

/*慢闪变快�?*/
void slow_switch_fast(void) {
    if (timer_elapsed32(long_press_ble_time) > LONG_PRESS_TIME) {
        if (long_press_ble_flag == true) {
            WIRELESS_PAIR(last_wireless_mode);
            higher_blink_wireless_time = timer_read32();
            blink_period               = 2;
            blink_count                = 0;
            long_press_ble_flag        = false;
        }
    }
    if (timer_elapsed32(long_press_24G_time) > LONG_PRESS_TIME) {
        if (long_press_24G_flag == true) {
            WIRELESS_PAIR(4);
            higher_blink_wireless_time = timer_read32();
            blink_period               = 2;
            blink_count                = 0;
            long_press_24G_flag        = false;
        }
    }
}

/*蓝牙发送电�?*/
void ble_send_battery(void) {
    /*没有按键�?按下超过一秒钟，keyboard_no_idle_flag这个标志位才会是 fasle*/
    if (timer_elapsed32(keyboard_idle_time) >= 1000) {
        keyboard_no_idle_flag = false;
    }
    /*蓝牙模式下发送电池电�?*/
    if (kb_mode == KB_MODE_BLE && wireless_connected) // no battery info when 24G or connecting
    {
        if (ble_connect_flag == false) /*蓝牙�?连后，又重新连上*/
        {
            battery_level_report_flag = true; /*允�?�发送电池电量标志位*/
            ble_connect_flag          = true;
        }
        if ((battery_level_report_flag == true) && (keyboard_no_idle_flag == false) && (battery_first_test_flag == true)) {
            sc_ble_battary(temp);
            battery_level_report_flag = false;
        }
    }
}

/*休眠时需要�?�置�?盘的row和col�?*/
void set_row_and_col_when_sleep(void) {
#if (DIODE_DIRECTION == ROW2COL)

    pin_t col_pins[] = MATRIX_COL_PINS;

    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        pin_t pin;
        pin = col_pins[x];
        if (pin != NO_PIN) {
            setPinOutput(pin);
            gpio_write_pin_low(pin);
        }
    }
    const long unsigned int row_pins[] = MATRIX_ROW_PINS;
    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        pin_t pin;
        pin = row_pins[x];
        if (pin != NO_PIN) {
            gpio_set_pin_input_high(pin);
            palEnableLineEvent(pin, PAL_EVENT_MODE_BOTH_EDGES);
        }
    }
#else

    const long unsigned int row_pins[] = MATRIX_ROW_PINS;
    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        pin_t pin;
        pin = row_pins[x];
        if (pin != NO_PIN) {
            setPinOutput(pin);
            gpio_write_pin_low(pin);
        }
    }

    const long unsigned int col_pins[] = MATRIX_COL_PINS;
    for (uint8_t x = 0; x < MATRIX_COLS; x++)
    // for(uint8_t x=0; x<9; x++)
    {
        pin_t pin;
        pin = col_pins[x];
        gpio_set_pin_inputHigh(pin);
        palEnableLineEvent(pin, PAL_EVENT_MODE_BOTH_EDGES);
    }
#endif
}

/*按键休眠防抖*/
void key_debounce(void) {
    uint8_t                 isr_source[MATRIX_ROWS];
    uint8_t                 i;
    const long unsigned int row_pins[] = MATRIX_ROW_PINS;
    // 休眠防抖处理,前提�?�?�?源是按键唤醒
    if (interrupt_source_flag) {
        while (1) {
            wait_ms(3);
            //�?10�?row口的电平
            for (uint8_t j = 0; j < 10; j++) {
                for (i = 0; i < MATRIX_ROWS; i++) {
                    isr_source[i] = readPin(row_pins[i]);
                    if (!isr_source[i]) break;
                }
                if (i < MATRIX_ROWS) {
                    break;
                }
            }
            if (i >= MATRIX_ROWS) {
                set_row_and_col_when_sleep();
                PWR->CR |= (1 << 0) | (1 << 10) | (1 << 11) | (3 << 18);
                PWR->CR |= PWR_CR_CWUF | PWR_CR_CSBF;
                SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
                __WFI();
                SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
            } else {
                break;
            }
        }
    }
    /*关闭0~15号中�?�?*/
    EXTI->IMR   &= ~0xFFFF;
    EXTI->EMR   &= ~0xFFFF;
    EXTI->RTSR  &= ~0xFFFF;
    EXTI->FTSR  &= ~0xFFFF;
    EXTI->PR    = 0xFFFF;
    interrupt_source_flag = false;
}

void variable_init(void){
    blink_count          = 0;
    blink_rgb_count      = 0;
    blink_reset_count    = 0;
    mcu_reset_flag       = 0;
    blink_reset_time     = 0;
    set_layer_count      = 0;
    reset_flag           = false;
    // white_rgb_flag       = false;
    long_press_ble_flag  = false;
    long_press_24G_flag  = false;
    enable_rgb_pin_flag  = false;
    battery_consult_flag = false;
}

/*一级休�?*/
void First_Level_Sleep(void) {
    gpio_write_pin_low(DRIVER_1_EN);
    gpio_write_pin_low(SCREEN_EN);  // 增加关闭屏幕
    gpio_write_pin_low(NRST);
    gpio_write_pin_low(EN_BACKLIT);
    gpio_write_pin_high(LED_CAP);
    gpio_write_pin_high(LED_WIN);
    cancel_deferred_exec(loop10hz_token);
    usb_disconnect(); // Don't enter SLEEP when USB mode
    /*晶振*/
    palSetLineMode(OSC_IN,PAL_MODE_INPUT_ANALOG);
    palSetLineMode(OSC_OUT,PAL_MODE_INPUT_ANALOG);
    /*关闭usb的DP,DN*/
    gpio_write_pin_low(RENUM_PIN);
    gpio_set_pin_input(A11);
    gpio_set_pin_input(A12);
    /*设置按键唤醒休眠*/
    set_row_and_col_when_sleep();
    /*拔插USB唤醒*/
    gpio_set_pin_input(PLUG_IN);
    palEnableLineEvent(PLUG_IN, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(BT_MODE);
    palEnableLineEvent(BT_MODE, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(PRF_MODE);
    palEnableLineEvent(PRF_MODE, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(ENL_PIN);
    palEnableLineEvent(ENL_PIN, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(ENR_PIN);
    palEnableLineEvent(ENR_PIN, PAL_EVENT_MODE_BOTH_EDGES);
    ADC1->CR2 &= ~ADC_CR2_ADON;
    PWR->CR |= (1 << 0) | (1 << 10) | (1 << 11) | (3 << 18);
    /* Clear Wake-up flag */
    PWR->CR |= PWR_CR_CWUF | PWR_CR_CSBF;
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    /* Request Wait For Interrupt */
    __WFI();
    /* Reset SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    /*按键唤醒去抖*/
    key_debounce();
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    /*时钟初�?�化*/
    stm32_clock_init();
    /*矩阵初�?�化以及矩阵�?�?*/
    matrix_init();
    matrix_scan();
    adc_init();
    /*发送心跳包变量*/
    first_sleep_time = timer_read32();
    packet_send_time  = 0;
    /*电池电量检�?*/
    blink_wireless_time         = timer_read32();
    higher_blink_wireless_time  = timer_read32();
    wireless_no_operation_time  = timer_read32();
    /*usb初�?�化延时*/
    wakeup_first_sleep_flag = true;
    wakeup_first_sleep_time = timer_read32();
    /*一旦进入休眠，之前的恢复出厂将会失�?*/
    variable_init();
    flag_SCREEN_init = true;  // Screen's init
    gpio_write_pin_high(NRST);
    loop10hz_token      = defer_exec(LOOP_10HZ_PERIOD, loop_10Hz, NULL);
}

/*二级休眠*/
void Second_Level_Sleep(void) {
    gpio_write_pin_low(DRIVER_1_EN);
    gpio_write_pin_low(SCREEN_EN);  // 增加关闭屏幕
    gpio_write_pin_low(NRST);
    gpio_write_pin_low(EN_BACKLIT);
    gpio_write_pin_high(LED_CAP);
    gpio_write_pin_high(LED_WIN);
    cancel_deferred_exec(loop10hz_token);
    kb_mode        = KB_DEFULT_MODE;
    WIRELESS_STOP();
    wait_ms(10);
    usb_disconnect(); // Don't enter SLEEP when USB mode
    /*晶振*/
    palSetLineMode(OSC_IN,PAL_MODE_INPUT_ANALOG);
    palSetLineMode(OSC_OUT,PAL_MODE_INPUT_ANALOG);
    /*USB的DP、DN*/
    gpio_write_pin_low(RENUM_PIN);
    gpio_set_pin_input(A11);
    gpio_set_pin_input(A12);
    /*设置按键唤醒休眠*/
    set_row_and_col_when_sleep();
    /*拔插USB唤醒*/
    gpio_set_pin_input(PLUG_IN);
    palEnableLineEvent(PLUG_IN, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(BT_MODE);
    palEnableLineEvent(BT_MODE, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(PRF_MODE);
    palEnableLineEvent(PRF_MODE,PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(ENL_PIN);
    palEnableLineEvent(ENL_PIN, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(ENR_PIN);
    palEnableLineEvent(ENR_PIN, PAL_EVENT_MODE_BOTH_EDGES);
    ADC1->CR2 &= ~ADC_CR2_ADON;
    /* Clear Wake-up flag */
    PWR->CR |= (1 << 0) | (1 << 10) | (1 << 11) | (3 << 18);
    PWR->CR |= PWR_CR_CWUF | PWR_CR_CSBF;
    /* Set SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    /* Request Wait For Interrupt */
    __WFI();
    /* Reset SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    /*按键唤醒消抖*/
    key_debounce();
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    stm32_clock_init();
    init_usb_driver(&USB_DRIVER); // Should not enter SLEEP when USB mode
    matrix_init();
    adc_init();
    gpio_write_pin_high(RENUM_PIN);
    /*发送心跳包变量*/
    first_sleep_time = timer_read32();
    packet_send_time  = 0;
    /*超时会重新休眠的变量*/
    blink_wireless_time         = timer_read32();
    higher_blink_wireless_time  = timer_read32();
    wireless_no_operation_time  = timer_read32();
    /*一旦进入休眠，之前的恢复出厂将会失�?*/
    variable_init();
    flag_SCREEN_init = true;  // Screen's init
    gpio_write_pin_high(NRST);
    loop10hz_token   = defer_exec(LOOP_10HZ_PERIOD, loop_10Hz, NULL);
}

/*有线休眠*/
void usb_suspend_power_down(void) {
    gpio_write_pin_low(DRIVER_1_EN);
    gpio_write_pin_low(SCREEN_EN);  // 增加关闭屏幕
    gpio_write_pin_low(NRST);
    gpio_write_pin_low(EN_BACKLIT);
    gpio_write_pin_high(LED_CAP);
    gpio_write_pin_high(LED_WIN);
    cancel_deferred_exec(loop10hz_token);
    /*晶振*/
    palSetLineMode(OSC_IN,PAL_MODE_INPUT_ANALOG);
    palSetLineMode(OSC_OUT,PAL_MODE_INPUT_ANALOG);
    /*设置按键唤醒休眠*/
    set_row_and_col_when_sleep();
    /*拔插USB唤醒*/
    gpio_set_pin_input(PLUG_IN);
    palEnableLineEvent(PLUG_IN, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(BT_MODE);
    palEnableLineEvent(BT_MODE, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(PRF_MODE);
    palEnableLineEvent(PRF_MODE,PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(ENL_PIN);
    palEnableLineEvent(ENL_PIN, PAL_EVENT_MODE_BOTH_EDGES);
    gpio_set_pin_input(ENR_PIN);
    palEnableLineEvent(ENR_PIN, PAL_EVENT_MODE_BOTH_EDGES);
    _pal_lld_enablepadevent(0, 18, PAL_EVENT_MODE_BOTH_EDGES);
    ADC1->CR2 &= ~ADC_CR2_ADON;
    PWR->CR |= (1 << 0) | (1 << 10) | (1 << 11) | (3 << 18);
    PWR->CR |= PWR_CR_CWUF | PWR_CR_CSBF;
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    key_debounce();
    SCB->SCR   &= ~SCB_SCR_SLEEPDEEP_Msk;
    EXTI->IMR  &= ~(1 << 18);
    EXTI->EMR  &= ~(1 << 18);
    EXTI->RTSR &= ~(1 << 18);
    EXTI->FTSR &= ~(1 << 18);
    EXTI->PR   =   (1 << 18);
    stm32_clock_init();
    clear_keyboard();
    send_keyboard_report();
    usbWakeupHost(&USB_DRIVER); //解决休眠无法唤醒的问�?
    restart_usb_driver(&USB_DRIVER);
    init_usb_driver(&USB_DRIVER); // Should not enter SLEEP when USB mode
    matrix_init();
    adc_init();
    release_count    = 2;
    usb_suspend_flag = 2;
    wakeup_count     = 0;
    /*一旦进入休眠，之前的恢复出厂将会失�?*/
    variable_init();
    flag_SCREEN_init = true;  // Screen's init
    gpio_write_pin_high(NRST);
    loop10hz_token   = defer_exec(LOOP_10HZ_PERIOD, loop_10Hz, NULL);
}

void sleep_mode(void) {

    /*�?连接�?�?�?�?:�?�?60，慢�?20*/
    if (kb_mode != KB_MODE_USB) {
        if (timer_elapsed32(blink_wireless_time) > 20000) {
            if ((blink_period == 5) && (wireless_connect_flag == false)) {
                Second_Level_Sleep();
            }
        }
        if (timer_elapsed32(higher_blink_wireless_time) > 60000) {
            if ((blink_period == 2) && (wireless_connect_flag == false)) {
                Second_Level_Sleep();
            }
        }
    }
    /*有线跟随休眠*/
    if (USB_DRIVER.state == USB_SUSPENDED && kb_mode == KB_MODE_USB) {
        if (usb_suspend_flag == 0) {
            usb_suspend_time = timer_read32();
            usb_suspend_flag = 1; /*USB现在是SUSPENED的状�?*/
        }
    } else if (USB_DRIVER.state == USB_ACTIVE && kb_mode == KB_MODE_USB) {
        usb_suspend_flag = 0;   /*USB是ACTIVE的状�?*/
    } else {
        usb_suspend_flag = 2;   /*USB是ACTIVE和SUSPENED之�?�的状�?*/
    }
    if (timer_elapsed32(usb_suspend_time) > 500) {
        if (get_plug_mode() == true && kb_mode == KB_MODE_USB) {
            if (usb_suspend_flag == 1) {
                usb_suspend_power_down();
                usb_wakeup_send_code_flag = true;
            }
        }
    }
    /*2.4G跟随休眠*/
    if (suspend == true) {
        Second_Level_Sleep();
        sleep_24G_flag = true;
        suspend        = false;
    }
    /*无线连接上了无操作超�?*/
    if (wireless_connected) {
        if (timer_elapsed32(wireless_no_operation_time) > RGB_MATRIX_WIRELESS_TIME ) {
            config_time_alarm();
            First_Level_Sleep();
            exti_stop_config();
            /*如果唤醒源不�?按键唤醒，才进去下面*/
            if (rtc_interrupt_source_flag) {
                rtc_interrupt_source_flag = false;
                wakeup_first_sleep_flag   = false;
                Second_Level_Sleep();
            } else {
                first_sleep_flag = true;
                ble_connect_flag = false;
                gpio_write_pin_high(RENUM_PIN);
            }
        }
    }
    // /*一级休眠唤�?0.1s后再初�?�化usb_driver,初�?�化过程�?有延时，防�?�吞�?*/
    // if (timer_elapsed32(wakeup_first_sleep_time) > 100) {
    //     if (wakeup_first_sleep_flag == true) {
    //         init_usb_driver(&USB_DRIVER);
    //         wakeup_first_sleep_flag = false;
    //     }
    // }
    if (wakeup_first_sleep_flag)
    {
       if( (raw_matrix[0] == 0) &&  (raw_matrix[1] == 0)  &&  (raw_matrix[2] == 0)&&  (raw_matrix[3] == 0)&&  (raw_matrix[4] == 0) && (raw_matrix[5] == 0))
       {
           gpio_set_pin_output(RENUM_PIN);
           gpio_write_pin_high(RENUM_PIN);
           init_usb_driver(&USB_DRIVER);
           wakeup_first_sleep_flag = 0;
       }
       else if(wakeup_first_sleep_flag && timer_elapsed32(first_sleep_time) > 6000)
       {
           gpio_set_pin_output(RENUM_PIN);
           gpio_write_pin_high(RENUM_PIN);
           init_usb_driver(&USB_DRIVER);
           wakeup_first_sleep_flag = 0;
       }
    }
    /*低电量软关机*/
    if ((temp == 0) && (batt_charge_status == batt_no_charge)) {
        Second_Level_Sleep();
    }
}

/*恢�?�出厂�?�置*/
void mcu_reset_init(void) {
    if (timer_elapsed32(long_press_reset_time) > LONG_PRESS_TIME) {
        if (reset_flag == true) {

            if (white_rgb_flag == true) {  // 25047�?特�?�加，需要删�?
                white_rgb_flag = !white_rgb_flag;
                variable_data.eeconfig_RGB_flag_WHITE = white_rgb_flag;
                eeconfig_update_kb_datablock(&variable_data);
            }

            variable_data.eeconfig_orient_mode = false;
            eeconfig_update_kb_datablock(&variable_data);
            blink_reset_time  = 3;
            blink_reset_count = 0;
            reset_flag        = false;
            mcu_reset_flag    = 1;
        }
        if (blink_reset_time == 0) {
            if (mcu_reset_flag == 1){
                mcu_reset_flag = 2;
            }
            if (mcu_reset_flag == 2) {
                if (blink_period == 2) {
                    higher_blink_flag                        = true;
                    variable_data.eeconfig_higher_blink_flag = higher_blink_flag;
                    eeconfig_update_kb_datablock(&variable_data);
                }
                eeconfig_init();
                mcu_reset();
            }
        }
    }
}

/*按键按下更新心跳时间*/
void key_pressd_update_heartbeat_time(void) {
    if (wireless_connected) {
        /*有按�?�?按下*/
        if ((raw_matrix[0] != 0) || (raw_matrix[1] != 0) || (raw_matrix[2] != 0) || (raw_matrix[3] != 0) || (raw_matrix[4] != 0) || (raw_matrix[5] != 0)) {
            wireless_no_operation_time = timer_read32();
            packet_send_time           = 0;
        }
        /*给moduel模块发送心跳包*/
        if (timer_elapsed32(first_sleep_time) > 8000 && (packet_send_time < PACKET_SEND_TIME)) /*休眠�?20s停�?�发�?*/
        {
            first_sleep_time = timer_read32();
            packet_send_time++;
            for (int i = 0; i < 5; i++) {
                uart_write(0x00);
            }
        }
    }
}

void toggle_layer(void){
    if(timer_elapsed32(set_layer_time) > 200){
        if(set_layer_count == 2){
            if(eeconfig_read_default_layer() == 1){
                set_single_persistent_default_layer(1);
            }else if (eeconfig_read_default_layer() == 2) {
                set_single_persistent_default_layer(0);
            }else if (eeconfig_read_default_layer() == 16) {
                set_single_persistent_default_layer(5);
            }else if (eeconfig_read_default_layer() == 32){
                set_single_persistent_default_layer(4);
            }
        }
        set_layer_count = 0;
    }
    // if (!gpio_read_pin(MAC_WIN)) {
    //     set_single_persistent_default_layer(2);
    //     if (win_lock_flag) {
    //         win_lock_flag        = false;
    //         memory_win_lock_flag = true;
    //     }
    // }
    // else {
    //     set_single_persistent_default_layer(0);
    //     if (memory_win_lock_flag) {
    //         win_lock_flag = true;
    //         memory_win_lock_flag = false;
    //     }
    // }
}

/*全键无冲切换*/
void key_nkro_toggle(void) {
    //全键无冲带�?�忆
    if (variable_data.eeconfig_nkro_flag != keymap_config.nkro) {
        variable_data.eeconfig_nkro_flag = keymap_config.nkro;
        eeconfig_update_kb_datablock(&variable_data);
    }
}

/*防�??usb唤醒hold�?*/
void avoid_hold_key_when_usb_wakeup(void) {
    if(USB_DRIVER.state == USB_ACTIVE){
        if(usb_wakeup_send_code_flag == true){
            tap_code16(KC_F24);
            usb_wakeup_send_code_flag = false;
        }
        if(release_count){
            if(wakeup_count == 0){
                if (keymap_config.nkro == 1) {
                    report_nkro_t send_nkro_report;
                    memset(&send_nkro_report, 0, sizeof(report_nkro_t));
                    host_nkro_send(&send_nkro_report);
                } else {
                    report_keyboard_t send_6nkro_report;
                    memset(&send_6nkro_report, 0, sizeof(report_keyboard_t));
                    host_keyboard_send(&send_6nkro_report);
                }
            }
            wakeup_count++;
            if(wakeup_count == 10){
                wakeup_count = 0;
                release_count--;
            }
        }
    }
}

/*caps,num,scroll,win指示�?*/
void host_keyboard_indicator_light_update(void){
    if (kb_mode == KB_MODE_USB || wireless_connected) {
        if (host_keyboard_led_state().caps_lock && 0x02 == 0x02 && count_speed_flash == 0 && blink_reset_time == 0) {
            rgb_matrix_off_state |= 0x01 << 3;
            // rgb_matrix_set_color(44, 144, 144, 144);
            if (gpio_read_pin(LED_CAP)) {
                gpio_write_pin_low(LED_CAP);
                SET_FLAG( kb_screen.flag , PK_CAPS_STATUS );
            }
        }else if (count_speed_flash == 0 && blink_reset_time == 0) {
            rgb_matrix_off_state &= ~(0x01 << 3);
            if (!gpio_read_pin(LED_CAP)) {
                gpio_write_pin_high(LED_CAP);
                SET_FLAG( kb_screen.flag , PK_CAPS_STATUS );
            }

        }
        /*win_lock指示�?*/
        if (count_speed_flash == 0 && win_lock_flag == true  && blink_reset_time == 0) {
            rgb_matrix_off_state |= 0x01 << 4;
            if (gpio_read_pin(LED_WIN) && (kb_mode == KB_MODE_USB || wireless_connected)) {
                gpio_write_pin_low(LED_WIN);
                SET_FLAG( kb_screen.flag , PK_WINLOCK_STATUS );
            }
        }else if (count_speed_flash == 0 && blink_reset_time == 0) {
            rgb_matrix_off_state &= ~(0x01 << 4);
            if (!gpio_read_pin(LED_WIN)) {
                gpio_write_pin_high(LED_WIN);
                SET_FLAG( kb_screen.flag , PK_WINLOCK_STATUS );
            }
        }
    }
    else if (kb_mode != KB_MODE_USB && !wireless_connected) {
        if (win_lock_flag == true && !gpio_read_pin(LED_WIN)) {
            gpio_write_pin_high(LED_WIN);
            SET_FLAG( kb_screen.flag , PK_WINLOCK_STATUS );
        }
        if (!gpio_read_pin(LED_CAP)) {
            gpio_write_pin_low(LED_CAP);
            SET_FLAG( kb_screen.flag , PK_CAPS_STATUS );
        }
    }
}

/*电量查�??*/
void electricity_inquriy(void){
    if(batt_charge_status == batt_no_charge){
        if (battery_consult_flag) {
            rgb_matrix_off_state |= 0x01;
            rgb_matrix_set_color_all(0, 0, 0);
            if(temp <= 30){
                for (uint8_t i = 1; i <= (temp - 1) / 10 + 1; i++) {
                    rgb_matrix_set_color(13+i, 144, 0, 0);
                }
            }else if (temp > 30 && temp <= 70) {
                for (uint8_t i = 1; i <= (temp - 1) / 10 + 1; i++) {
                    rgb_matrix_set_color(13+i, 144, 144, 0);
                }
            }else {
                for (uint8_t i = 1; i <= (temp - 1) / 10 + 1; i++) {
                    rgb_matrix_set_color(13+i, 0, 144, 0);
                }
            }
        }else {
            rgb_matrix_off_state &= ~0x01;
        }
    }else {
        if(battery_consult_flag){
            rgb_matrix_off_state &= ~0x01;
            battery_consult_flag = !battery_consult_flag;
        }
    }
}

/*充电指示�?*/
void charging_indicator_light(void){
    /*充电指示�?*/
    if(batt_charge_status == batt_is_charging){
        batt_charge_full_flag = false;
        if(batt_is_charging_flag == false){
            batt_is_charging_time = timer_read32();
            batt_is_charging_flag = true;
        }
        if(timer_elapsed32(batt_is_charging_time) < 10000){
            rgb_matrix_off_state |= 0x01 << 5;
            rgb_matrix_set_color(76, 144, 0, 0);
        }else{
            rgb_matrix_off_state &= ~(0x01 << 5);
        }
    }else if (batt_charge_status == batt_no_charge) {
        batt_charge_full_flag = false;
        if(batt_is_charging_flag == true){
            batt_is_charging_flag = false;
        }
        if(temp <= 20){
            rgb_matrix_off_state |= 0x01 << 5;
            if (blink_rgb_count > blink_rgb_period) {
                // for (int i = 0; i < 10; ++i) {
                //     rgb_matrix_set_color(14 + i, 144, 0, 0);
                // }
                rgb_matrix_set_color(76, 144, 0, 0);
            } else {
                // for (int i = 0; i < 10; ++i) {
                //     rgb_matrix_set_color(14 + i, 0, 0, 0);
                // }
                rgb_matrix_set_color(76, 0, 0, 0);
            }
        }
        else {
            rgb_matrix_off_state &= ~(0x01 << 5);
        }
    }else {
        rgb_matrix_off_state &= ~(0x01 << 5);
    }
    if (batt_charge_status == batt_charge_full) {
        if (batt_charge_full_flag == false) {
            batt_charge_full_flag = true;
            batt_charge_full_time = timer_read32();
        }
        if(timer_elapsed32(batt_charge_full_time) < 10000) {
            rgb_matrix_set_color(76, 0, 144, 0);
        }
    }
}

/*复位指示�?*/
void mcu_reset_indicator_light(void){
    if (blink_reset_time != 0) {
        rgb_matrix_off_state |= 0x01 << 6;
        if (blink_reset_count > blink_reset_period) {
            rgb_matrix_set_color_all(255,255,255);
            // rgb_matrix_set_color_all(0,0,0);
            gpio_write_pin_low(LED_CAP);
            gpio_write_pin_low(LED_WIN);
        } else {
            rgb_matrix_set_color_all(0,0,0);
            gpio_write_pin_high(LED_CAP);
            gpio_write_pin_high(LED_WIN);
        }
    }else {
        rgb_matrix_off_state &= ~(0x01 << 6);
    }
}

/*休眠关灯*/
void sleep_off_rgb(void){
    if ((batt_charge_status == batt_no_charge && temp == 0) || enable_rgb_pin_flag == false ) {
        rgb_matrix_set_color_all(RGB_OFF);
    }
}

/*提前初�?�化*/
void keyboard_pre_init_kb(void) {
    gpio_set_pin_output(RENUM_PIN);
    gpio_write_pin_high(RENUM_PIN);
    gpio_set_pin_input(BT_MODE);
    gpio_set_pin_input(PRF_MODE);
    gpio_set_pin_input(PLUG_IN);
    gpio_write_pin_low(EN_BACKLIT);
    gpio_set_pin_output(EN_BACKLIT);
    gpio_write_pin_high(LED_CAP);
    gpio_set_pin_output(LED_CAP);
    gpio_write_pin_high(LED_WIN);
    gpio_set_pin_output(LED_WIN);
    gpio_write_pin_low(SCREEN_EN);
    gpio_set_pin_output(SCREEN_EN);
    gpio_set_pin_output(NRST);
}

/*初�?�化*/
void keyboard_post_init_kb(void) {
    gpio_write_pin_low(DRIVER_1_EN);
    AFIO->MAPR |= AFIO_MAPR_USART3_REMAP_PARTIALREMAP;
    /*bootloader校验*/
    encode_boot();
    /*复用引脚*/
    AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG_Msk);
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_DISABLE;
    adc_init();
    Rtc_Config_Api();
    uart_init(460800);

    palSetLineMode(C10, UART_TX_PAL_MODE);
    palSetLineMode(C11, UART_RX_PAL_MODE);
    sdStart(&SD3, &uart_config);

    wait_ms(700);
    eeconfig_read_kb_datablock(&variable_data);
    higher_blink_flag            = variable_data.eeconfig_higher_blink_flag;
    last_wireless_mode           = variable_data.eeconfig_last_wireless_mode;
    rgb_matrix_off_flag          = variable_data.eeconfig_rgb_matrix_off_flag;
    rgblight_off_flag            = variable_data.eeconfig_rgblight_off_flag;
    win_lock_flag                = variable_data.eeconfig_win_lock_flag;
    keymap_config.nkro           = variable_data.eeconfig_nkro_flag;
    mod_rotary                   = variable_data.eeconfig_rotary_mode;
    mod_orient                   = variable_data.eeconfig_orient_mode;
    flag_SCREEN_switch           = variable_data.eeconfig_SCREEN_switch_flag;
    white_rgb_flag               = variable_data.eeconfig_RGB_flag_WHITE;
    eeconfig_update_keymap(keymap_config.raw);
    /*mac层关�?锁win*/
    if(eeconfig_read_default_layer() == 4 || eeconfig_read_default_layer() == 8){
        if(win_lock_flag == true){
            win_lock_flag = false;
            memory_win_lock_flag = true;
        }
    }
    loop10hz_token = defer_exec(LOOP_10HZ_PERIOD, loop_10Hz, NULL);
}

uint32_t loop_10Hz(uint32_t trigger_time, void *cb_arg) {
    static uint8_t count_printf = 0;
    if (++count_printf == 20) {
        count_printf = 0;
        // uprintf("variable_data.eeconfig_orient_mode = %d\r\n", variable_data.eeconfig_orient_mode);
        // uprintf("mod_orient = %d\r\n", mod_orient);
        // uprintf("rgb_matrix_get_speed = %d\r\n", rgb_matrix_get_speed());
        // uprintf("temp = %d\r\n", temp);
        // uprintf("flag_SCREEN_time = %d\r\n", flag_SCREEN_time);
        // uprintf("variable_data.eeconfig_SCREEN_flag_time = %d\r\n", variable_data.eeconfig_SCREEN_flag_time);
    }
    /*变量计数*/
    blink_count       = (blink_count + 1) % (blink_period * 2);
    blink_rgb_count   = (blink_rgb_count + 1) % (blink_rgb_period *2);
    blink_reset_count = (blink_reset_count + 1) % (blink_reset_period * 2);

    /*10Hz�?询灯光速度溢出检�?*/
    blink_speed_count = (blink_speed_count + 1) % (blink_speed_period * 2);
    if (count_speed_flash) if (blink_speed_count == 0) --count_speed_flash;
    if (flag_speed_overflow) {
        flag_speed_overflow = false;
        blink_speed_count = 0;
        blink_speed_period = 5;
        count_speed_flash = COUNT_SPEED_FLASH_NEED;
    }
    if (blink_reset_count == 0) {
        if (blink_reset_time > 0)   blink_reset_time--;
        // else if (blink_reset_time == 0 && mcu_reset_flag > 0) gpio_write_pin_low(EN_BACKLIT);
    }
    /*caps指示*/
    host_keyboard_indicator_light_update();
    /*旋转编码器模式切�?*/
    if (switch_rotary_mod && timer_elapsed32(time_rotary_mod) > 2000) {
        mod_rotary = !mod_rotary;
        variable_data.eeconfig_rotary_mode = mod_rotary;
        eeconfig_update_kb_datablock(&variable_data);
    }
    /*方向模式切换*/
    if (switch_orient_mod && timer_elapsed32(time_common) > LONG_PRESS_ORIENT_SWITCH) {
        blink_reset_time = 3;
        switch_orient_mod = false;
        mod_orient = !mod_orient;
        variable_data.eeconfig_orient_mode = mod_orient;
        eeconfig_update_kb_datablock(&variable_data);
    }
    /*从usb休眠�?唤醒，防�?hold�?*/
    avoid_hold_key_when_usb_wakeup();
    /*充电标志�?*/
    battery_charge_status();
    /*电池电量检�?*/
    battery_level_test();
    /*按键切换模式，会在这里面进�?��?�理*/
    communicate_mode_toggle();
    /*蓝牙发送电池百分比电量*/
    ble_send_battery();
    /*按下按键更新心跳和休眠时�?*/
    key_pressd_update_heartbeat_time();
    /*无线连上或是�?开要执行的操作*/
    wireless_connected_or_disconnected_operate();
    /*休眠*/
    sleep_mode();
    /*Screen*/
    screen_process();

    // /*切层*/
    // toggle_layer();
    /*全键无冲切换*/
    key_nkro_toggle();
    /*复位*/
    mcu_reset_init();
    /*长按�?�?*/
    slow_switch_fast();

    return LOOP_10HZ_PERIOD;
}

void rgblight_indicators_advanced_kb(void) {
    light_function();

    if (rgblight_off_flag == true || (batt_charge_status == batt_no_charge && (temp <= 20))){
        for(uint8_t i = 83; i < RGBLIGHT_LED_COUNT; i++){
            rgb_matrix_set_color(i,0,0,0);
        }
    }
    if(white_rgb_flag){
        // for(uint8_t i = 83; i < RGBLIGHT_LED_COUNT; i++){
        //     rgb_matrix_set_color(i,255,255,255);
        // }
        rgb_matrix_set_color_all(200, 200, 200);
    }
    if (rgb_matrix_off_flag == true || (batt_charge_status == batt_no_charge && (temp <= 20))){
        rgb_matrix_set_color_all(RGB_OFF);
    }
    if (rgb_matrix_off_flag) {
        rgb_matrix_set_color_all(0, 0, 0);
    }
    /*充电指示�?*/
    charging_indicator_light();
    /*电量查�??*/
    electricity_inquriy();
    /*休眠关灯*/
    if (enable_rgb_pin_flag == false) {
        for(uint8_t i = 83; i < RGBLIGHT_LED_COUNT; i++){
            rgb_matrix_set_color(i,0,0,0);
        }
    }

    if((rgb_matrix_off_flag && rgblight_off_flag && rgb_matrix_off_state == 0) || (rgb_matrix_get_val() == 0 && rgblight_get_val() == 0 && rgb_matrix_off_state == 0)){
        if (enable_rgb_pin_flag) {
            // gpio_write_pin_low(EN_BACKLIT);
            enable_rgb_pin_flag = !enable_rgb_pin_flag;
        }
    }else {
        if(!enable_rgb_pin_flag){
            if(!wakeup_first_sleep_flag){
                gpio_write_pin_high(EN_BACKLIT);
                enable_rgb_pin_flag = !enable_rgb_pin_flag;
            }
        }
    }
    if (kb_mode == KB_MODE_USB || !wireless_connected) {  // 有线模式下，无线模式指示�?关闭 上电无线没有连接，灯也不�?
        for(uint8_t i = 83; i < RGB_MATRIX_LED_COUNT; i++){
            rgb_matrix_set_color(i,0,0,0);
        }
    }
    /*对应什么模式会�?什么灯*/
    mode_indicator_light_init();
    /*复位指示�?*/
    mcu_reset_indicator_light();

    if (test_rgb_r != 0 || test_rgb_g != 0 || test_rgb_b != 0) rgb_matrix_set_color_all(test_rgb_r, test_rgb_g, test_rgb_b);
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {


    if (rgb_matrix_off_flag) {
        for(uint8_t i = 0; i < 83; i++){
            rgb_matrix_set_color(i,0,0,0);
        }
    }

    // /*�?白灯�?*/
    // if(white_rgb_flag){
    //     rgb_matrix_off_state |= 0x01 << 6;
    //     rgb_matrix_set_color_all(255,255,255);
    // }else {
    //     rgb_matrix_off_state &= ~(0x01 << 6);
    // }
    /*休眠关灯*/
    sleep_off_rgb();


    return false;
}

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
    /*Screen*/
    if(keycode == Scr_Home)
    {
       if(record->event.pressed)
        {
            flag_SCREEN_state = SCREEN_HOME;
            SET_FLAG( kb_screen.flag , PK_GO_HOME );
        }
        return false;
    }

    if(keycode == Scr_Pic)
    {
       if(record->event.pressed)
        {
            flag_SCREEN_state = SCREEN_PICTURE;
            SET_FLAG( kb_screen.flag , PK_TOGGLE_PIC );
        }
        return false;
    }

    if(keycode == Scr_Gif)
    {
       if(record->event.pressed)
        {
            flag_SCREEN_state = SCREEN_GIF;
            SET_FLAG( kb_screen.flag , PK_GO_GIF);
        }
        return false;
    }

    if (keycode == Scr_EN) {
        if (record->event.pressed) {
            flag_SCREEN_switch = !flag_SCREEN_switch;
        }
        return false;
    }
    /*长按�?�?*/
    if(kb_mode == KB_MODE_BLE){
        if ((keycode >= KC_BLE1 && keycode <= KC_BLE3)) {
            if (last_wireless_mode != keycode - KC_24G){
                if (record->event.pressed) {
                    WIRELESS_START(keycode - KC_24G);
                    blink_wireless_time = timer_read32();
                    long_press_ble_time = timer_read32();
                    long_press_ble_flag = true;
                    blink_period        = 5;
                    blink_count         = 0;
                }
            }else{
                if (record->event.pressed) {
                    long_press_ble_time = timer_read32();
                    long_press_ble_flag = true;
                } else {
                    if (timer_elapsed32(long_press_ble_time) < LONG_PRESS_TIME) {
                        long_press_ble_flag = false;
                    }
                }
            }
            return false;
        }
    }
    /*2.4G切换*/
    if (kb_mode == KB_MODE_24G){
        if (keycode == KC_24G) {
            if (record->event.pressed) {
                long_press_24G_flag = true;
                long_press_24G_time = timer_read32();
            } else {
                if (timer_elapsed32(long_press_24G_time) < LONG_PRESS_TIME) {
                    long_press_24G_flag = false;
                }
            }
            return false;
        }
    }
    /*切mac�?*/
    if(keycode == KC_MAC){
        if(record->event.pressed){
            if(eeconfig_read_default_layer() == 1){
                set_single_persistent_default_layer(2);
                f_os_mode = 1;
                SET_FLAG( kb_screen.flag , PK_OS_TYPE );
            }
            if(win_lock_flag == true){
                win_lock_flag        = false;
                memory_win_lock_flag = true;
            }
            flag_speed_overflow = true;
        }
        return false;
    }
    /*切win�?*/
    if(keycode == KC_WIN){
        if(record->event.pressed){
            if(eeconfig_read_default_layer() == 4){
                set_single_persistent_default_layer(0);
                f_os_mode = 0;
                SET_FLAG( kb_screen.flag , PK_OS_TYPE );
            }
            if(memory_win_lock_flag == true){
                win_lock_flag        = true;
                SET_FLAG( kb_screen.flag , PK_WINLOCK_STATUS );
                memory_win_lock_flag = false;
            }
            flag_speed_overflow = true;
        }
        return false;
    }
    /*复位*/
    if (keycode == KC_RESET) {
        if (record->event.pressed) {
            long_press_reset_time = timer_read32();
            reset_flag            = true;
        } else {
            if (timer_elapsed32(long_press_reset_time) < 3000) {
                reset_flag = false;
            }
        }
		return false;
    }

    if(keycode == KC_WHITE){
        if(record->event.pressed) {
            white_rgb_flag = !white_rgb_flag;
            variable_data.eeconfig_RGB_flag_WHITE = white_rgb_flag;
            eeconfig_update_kb_datablock(&variable_data);
        }
    }
    if (keycode == RGB_MOD) {
        if (record->event.pressed) {
            if (white_rgb_flag == true) {
                white_rgb_flag = !white_rgb_flag;
                variable_data.eeconfig_RGB_flag_WHITE = white_rgb_flag;
                eeconfig_update_kb_datablock(&variable_data);
            }
        }
    }
    /*开关灯*/
    if((batt_charge_status != batt_no_charge || temp > 10)){
        if (keycode == RGB_TOG && record->event.pressed) {
            // if (white_rgb_flag == true) {
            //     white_rgb_flag = !white_rgb_flag;
            //     variable_data.eeconfig_RGB_flag_WHITE = white_rgb_flag;
            //     eeconfig_update_kb_datablock(&variable_data);
            // }
            if(rgb_matrix_off_flag == false || rgblight_off_flag == false){
                rgb_matrix_off_flag = true; /*关闭矩阵�?标志�?*/
                rgblight_off_flag   = true;
            }else {
                rgb_matrix_off_flag = false; /*关闭矩阵�?标志�?*/
                rgblight_off_flag   = false;
            }
            variable_data.eeconfig_rgb_matrix_off_flag = rgb_matrix_off_flag;
            variable_data.eeconfig_rgblight_off_flag   = rgblight_off_flag;
            eeconfig_update_kb_datablock(&variable_data);
            return false;
        }
    }
    /*关灯时候，所有rgb_martix�?光调试快捷键的全部失�?*/
    if (rgb_matrix_off_flag == true || (batt_charge_status == batt_no_charge && temp <= 10)) {
        if(keycode == RGB_VAI || keycode == RGB_VAD || keycode == RGB_SPI  || keycode == RGB_SPD || keycode == RGB_MOD
        ||keycode == RGB_HUI  || keycode == RGB_HUD || keycode == RGB_RMOD || keycode == RGB_SAI || keycode == RGB_SAD){
            return false;
        }
    }

    if(rgblight_off_flag == true || (batt_charge_status == batt_no_charge && temp <= 10)){
        if(keycode == KC_MODE || keycode == KC_VAD || keycode == KC_VAI){
            return false;
       }
    }

    /*MAC层的�?定义按键*/
    if (keycode == KC_Mctl) {
        if (record->event.pressed)
            register_code16(KC_MCTL);
        else
            unregister_code16(KC_MCTL);
    }
    if (keycode == KC_Lpad) {
        if (record->event.pressed)
            register_code16(KC_LPAD);
        else
            unregister_code16(KC_LPAD);
    }
    if (keycode == KC_Lopt) {
        if (record->event.pressed)
            register_code16(KC_LOPT);
        else
            unregister_code16(KC_LOPT);
    }
    if (keycode == KC_Lcmd) {
        if (record->event.pressed)
            register_code16(KC_LCMD);
        else
            unregister_code16(KC_LCMD);
    }
    if (keycode == KC_Rcmd) {
        if (record->event.pressed)
            register_code16(KC_RCMD);
        else
            unregister_code16(KC_RCMD);
    }
    /*锁win*/
    if(keycode == WIN_LOCK && eeconfig_read_default_layer() <= 2){
        if (record->event.pressed){
            win_lock_flag = !win_lock_flag;
            variable_data.eeconfig_win_lock_flag = win_lock_flag ;
            eeconfig_update_kb_datablock(&variable_data);
            return false;
        }
    }
    if(keycode == KC_LGUI || keycode == KC_RGUI){
        if (record->event.pressed){
            if(win_lock_flag){
                return false;
            }
        }
    }
    /*电池电量检�?*/
    if(batt_charge_status == batt_no_charge){
        if (keycode == KC_BAT) {
            // if (white_rgb_flag == true) {
            //     white_rgb_flag = !white_rgb_flag;
            //     variable_data.eeconfig_RGB_flag_WHITE = white_rgb_flag;
            //     eeconfig_update_kb_datablock(&variable_data);
            // }
            if (record->event.pressed)
                battery_consult_flag = true;
            else
                battery_consult_flag = false;
            return false;
        }
    }

    if(keycode == MO(2) || keycode == MO(3) || keycode == MO(6)||keycode == MO(7)){
        if(record->event.pressed){
            if(timer_elapsed32(set_layer_time) < 200){
                if(set_layer_count != 0){
                    set_layer_count++;
                }
            }
            if(set_layer_count == 0){
                set_layer_count = 1;
                set_layer_time = timer_read32();
            }
        }
        return true;
    }
    if (keycode == ROTARY_MOD) {  // 旋转编码器模式切�?
        uprintf("keycode == ROTARY_MOD");
        if (record->event.pressed) {
            switch_rotary_mod = true;
            time_rotary_mod = timer_read32();
        }
        else {
            if (time_rotary_mod < 2000) {
                if (mod_rotary) tap_code16(KC_MUTE);
                else tap_code16(KC_MODE);
            }
            switch_rotary_mod = false;
        }
        return false;
    }
    // /*旋转编码器模�?*/
    // if (keycode == ROTARY_LEFT) {
    //     uprintf("ROTARY_LEFT");
    //     if (record->event.pressed) {
    //         if (mod_rotary) tap_code16(KC_VOLD);
    //     }
    //     return false;
    // }
    // if (keycode == ROTARY_RIGHT) {
    //     uprintf("ROTARY_RIGHT");
    //     if (record->event.pressed) {
    //         if (mod_rotary) tap_code16(KC_VOLU);
    //     }
    //     return false;
    // }
    if (keycode == MOD_DIS) {  // 模式指示
        if (record->event.pressed) {
            flag_mod_dis = true;
        }
        else {
            flag_mod_dis = false;
        }
        return false;
    }
    if (keycode == ORIENT_MOD) {  // 方向�?模式切换
        if(record->event.pressed) {
            switch_orient_mod = true;
            time_common = timer_read32();
        }
        else {
            switch_orient_mod = false;
        }
        return false;
    }
    /*方向与WASD*/
    if (keycode == KC_W && mod_orient) {
        if (record->event.pressed) register_code16(KC_UP);
        else unregister_code16(KC_UP);
        return false;
    }
    if (keycode == KC_A && mod_orient) {
        if (record->event.pressed) register_code16(KC_LEFT);
        else unregister_code16(KC_LEFT);
        return false;
    }
    if (keycode == KC_S && mod_orient) {
        if (record->event.pressed) register_code16(KC_DOWN);
        else unregister_code16(KC_DOWN);
        return false;
    }
    if (keycode == KC_D && mod_orient) {
        if (record->event.pressed) register_code16(KC_RIGHT);
        else unregister_code16(KC_RIGHT);
        return false;
    }
    if (keycode == KC_UP && mod_orient) {
        if (record->event.pressed) register_code16(KC_W);
        else unregister_code16(KC_W);
        return false;
    }
    if (keycode == KC_LEFT && mod_orient) {
        if (record->event.pressed) register_code16(KC_A);
        else unregister_code16(KC_A);
        return false;
    }
    if (keycode == KC_DOWN && mod_orient) {
        if (record->event.pressed) register_code16(KC_S);
        else unregister_code16(KC_S);
        return false;
    }
    if (keycode == KC_RIGHT && mod_orient) {
        if (record->event.pressed) register_code16(KC_D);
        else unregister_code16(KC_D);
        return false;
    }
    /*速度挡位溢出检�?*/
    if (keycode == RGB_SPI) {
        if (record->event.pressed) {
            if (rgb_matrix_get_speed() == UINT8_MAX) {  // 速度已经�?最大值则向上溢出
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
                return false;
            }
            else if (UINT8_MAX - rgb_matrix_get_speed() < 70) {
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
            }
        }
    }
    if (keycode == RGB_SPD) {
        if (record->event.pressed) {
            if (rgb_matrix_get_speed() == 0) {  // 速度已经�?最小值则向下溢出
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
                return false;
            }
            else if (rgb_matrix_get_speed() < 70) {
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
            }
        }
    }
    if (keycode == Scr_Time) {
        if (record->event.pressed) {
            flag_SCREEN_time = !flag_SCREEN_time;
            variable_data.eeconfig_SCREEN_flag_time = flag_SCREEN_time;
            eeconfig_update_kb_datablock(&variable_data);
            SET_FLAG( kb_screen.flag, PK_SWITCH_TIME);
            return false;
        }
    }
    /*速度挡位溢出检�?*/
    if (keycode == RGB_SPI) {
        if (record->event.pressed) {
            if (rgb_matrix_get_speed() == UINT8_MAX) {  // 速度已经�?最大值则向上溢出
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
                return false;
            }
            else if (UINT8_MAX - rgb_matrix_get_speed() < 70) {
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
            }
        }
    }
    if (keycode == RGB_SPD) {
        if (record->event.pressed) {
            if (rgb_matrix_get_speed() == 0) {  // 速度已经�?最小值则向下溢出
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
                return false;
            }
            else if (rgb_matrix_get_speed() < 70) {
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
            }
        }
    }
    /*�?度挡位溢出�?��?*/
    if (keycode == RGB_VAI) {
        if (record->event.pressed) {
            if (rgb_matrix_get_val() == RGB_MATRIX_MAXIMUM_BRIGHTNESS) {  // �?度已经是最大值则向上溢出
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
                return false;
            }
            else if (RGB_MATRIX_MAXIMUM_BRIGHTNESS - rgb_matrix_get_val() <= RGB_MATRIX_VAL_STEP) {
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
            }
        }
    }
    if (keycode == RGB_VAD) {
        if (record->event.pressed) {
            if (rgb_matrix_get_val() == 0) {  // �?度已经是最小值则向下溢出
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
                return false;
            }
            else if (rgb_matrix_get_val() <= 50) {
                if (!count_speed_flash) {  //  持续按也�?刷新COUNT_SPEED_FLASH_NEED�?
                    flag_speed_overflow = true;
                }
            }
        }
    }
    return process_record_user(keycode, record);
}

#ifdef RGB_MATRIX_ENABLE
const aw_led g_aw_leds[RGB_MATRIX_LED_COUNT] = {
    /*轴灯*/
    {0, CS1_SW1,  CS2_SW1,  CS3_SW1},    // 1!
    {0, CS1_SW2,  CS2_SW2,  CS3_SW2},    // 2@
    {0, CS1_SW3,  CS2_SW3,  CS3_SW3},    // 3#
    {0, CS1_SW4,  CS2_SW4,  CS3_SW4},    // 4$
    {0, CS1_SW5,  CS2_SW5,  CS3_SW5},    // 5%
    {0, CS1_SW6,  CS2_SW6,  CS3_SW6},    // 6^
    {0, CS1_SW7,  CS2_SW7,  CS3_SW7},
    {0, CS1_SW8,  CS2_SW8,  CS3_SW8},
    {1, CS1_SW1,  CS2_SW1,  CS3_SW1},    // 7&
    {1, CS1_SW2,  CS2_SW2,  CS3_SW2},    // 8*
    {1, CS1_SW3,  CS2_SW3,  CS3_SW3},    // 9(
    {1, CS1_SW4,  CS2_SW4,  CS3_SW4},    // 0)
    {1, CS1_SW5,  CS2_SW5,  CS3_SW5},    // -_
    {0, CS4_SW1,  CS5_SW1,  CS6_SW1},    // Q
    {0, CS4_SW2,  CS5_SW2,  CS6_SW2},    // W
    {0, CS4_SW3,  CS5_SW3,  CS6_SW3},    // E
    {0, CS4_SW4,  CS5_SW4,  CS6_SW4},    // R
    {0, CS4_SW5,  CS5_SW5,  CS6_SW5},    // T
    {0, CS4_SW6,  CS5_SW6,  CS6_SW6},    // Y
    {0, CS4_SW7,  CS5_SW7,  CS6_SW7},
    {0, CS4_SW8,  CS5_SW8,  CS6_SW8},
    {1, CS4_SW1,  CS5_SW1,  CS6_SW1},    // U
    {1, CS4_SW2,  CS5_SW2,  CS6_SW2},    // I
    {1, CS4_SW3,  CS5_SW3,  CS6_SW3},    // O
    {1, CS4_SW4,  CS5_SW4,  CS6_SW4},    // P
    {1, CS4_SW5,  CS5_SW5,  CS6_SW5},    // [{
    {1, CS4_SW6,  CS5_SW6,  CS6_SW6},    // ]}
    {1, CS4_SW7,  CS5_SW7,  CS6_SW7},    // '\|'
    {0, CS7_SW1,  CS8_SW1,  CS9_SW1},    // A
    {0, CS7_SW2,  CS8_SW2,  CS9_SW2},    // S
    {0, CS7_SW3,  CS8_SW3,  CS9_SW3},    // D
    {0, CS7_SW4,  CS8_SW4,  CS9_SW4},    // F
    {0, CS7_SW5,  CS8_SW5,  CS9_SW5},    // G
    {0, CS7_SW6,  CS8_SW6,  CS9_SW6},    // H
    {0, CS7_SW7,  CS8_SW7,  CS9_SW7},
    {0, CS7_SW8,  CS8_SW8,  CS9_SW8},
    {1, CS7_SW1,  CS8_SW1,  CS9_SW1},    // J
    {1, CS7_SW2,  CS8_SW2,  CS9_SW2},    // K
    {1, CS7_SW3,  CS8_SW3,  CS9_SW3},    // L
    {1, CS7_SW4,  CS8_SW4,  CS9_SW4},    // ;:
    {1, CS7_SW5,  CS8_SW5,  CS9_SW5},    // '"
    {1, CS7_SW6,  CS8_SW6,  CS9_SW6},
    {1, CS7_SW7,  CS8_SW7,  CS9_SW7},    // Enter
    {0, CS10_SW1,  CS11_SW1,  CS12_SW1},
    {0, CS10_SW2,  CS11_SW2,  CS12_SW2}, // Z
    {0, CS10_SW3,  CS11_SW3,  CS12_SW3}, // X
    {0, CS10_SW4,  CS11_SW4,  CS12_SW4}, // C
    {0, CS10_SW5,  CS11_SW5,  CS12_SW5}, // V
    {0, CS10_SW6,  CS11_SW6,  CS12_SW6}, // B
    {0, CS10_SW7,  CS11_SW7,  CS12_SW7},
    {0, CS10_SW8,  CS11_SW8,  CS12_SW8},
    {1, CS10_SW1,  CS11_SW1,  CS12_SW1}, // N
    {1, CS10_SW2,  CS11_SW2,  CS12_SW2}, // M
    {1, CS10_SW3,  CS11_SW3,  CS12_SW3}, // <,
    {1, CS10_SW4,  CS11_SW4,  CS12_SW4}, // >.
    {1, CS10_SW5,  CS11_SW5,  CS12_SW5}, // /?
    {1, CS10_SW6,  CS11_SW6,  CS12_SW6}, // Right Shift
    {1, CS10_SW7,  CS11_SW7,  CS12_SW7}, // Up
    {0, CS13_SW1,  CS14_SW1,  CS15_SW1}, // Left Win
    {0, CS13_SW2,  CS14_SW2,  CS15_SW2}, // Left ALT
    {0, CS13_SW3,  CS14_SW3,  CS15_SW3},
    {0, CS13_SW4,  CS14_SW4,  CS15_SW4},
    {0, CS13_SW5,  CS14_SW5,  CS15_SW5},
    {0, CS13_SW6,  CS14_SW6,  CS15_SW6}, // Space
    {0, CS13_SW7,  CS14_SW7,  CS15_SW7},
    {0, CS13_SW8,  CS14_SW8,  CS15_SW8},
    {1, CS13_SW1,  CS14_SW1,  CS15_SW1},
    {1, CS13_SW2,  CS14_SW2,  CS15_SW2},
    {1, CS13_SW3,  CS14_SW3,  CS15_SW3}, // Right ALT
    {1, CS13_SW4,  CS14_SW4,  CS15_SW4}, // FN
    {1, CS13_SW5,  CS14_SW5,  CS15_SW5}, // Right Control
    {1, CS13_SW6,  CS14_SW6,  CS15_SW6}, // Left
    {1, CS13_SW7,  CS14_SW7,  CS15_SW7}, // Down
    {0, CS16_SW1,  CS17_SW1,  CS18_SW1},
    {0, CS16_SW2,  CS17_SW2,  CS18_SW2},
    {0, CS16_SW3,  CS17_SW3,  CS18_SW3},
    {0, CS16_SW7,  CS17_SW7,  CS18_SW7},
    {1, CS16_SW2,  CS17_SW2,  CS18_SW2},
    {1, CS16_SW3,  CS17_SW3,  CS18_SW3},
    {1, CS16_SW4,  CS17_SW4,  CS18_SW4},
    {1, CS16_SW5,  CS17_SW5,  CS18_SW5},
    {1, CS16_SW6,  CS17_SW6,  CS18_SW6},
    {1, CS16_SW7,  CS17_SW7,  CS18_SW7},
    {1, CS1_SW6,  CS2_SW6,  CS3_SW6},  // 2.4G
    {1, CS1_SW7,  CS2_SW7,  CS3_SW7}  // BT
};
#endif


uint8_t  ap_message_buff[64];
// #ifdef Module_Updata
bool via_command_kb(uint8_t *data, uint8_t length) {
    static uint16_t packcount = 0;
    uint8_t         modele_updata[66];

#ifdef Screen_AP
	// uint32_t	data_addr;
    uint8_t		data_len;
    memcpy(ap_message_buff, data, length);
   	data_len	= ap_message_buff[3];
	// data_addr 	= 0;
	// data_addr	= ap_message_buff[3];
	// data_addr	= (data_addr<<8)|ap_message_buff[2];
	ap_message_buff[6] = 0x55;
#endif


    //  uprintf("enter here4\r\n");

    if (data[0] == 0x0a) {
        switch (data[1]) {
            case 0x55: // connect
                for (uint8_t i = 0; i < 60; i++) {
                    uart_write(0x00);
                }
                wait_ms(5);
                modele_updata[0] = 0x55;
                modele_updata[1] = 0x40;
                for (uint8_t i = 2; i < 66; i++) {
                    modele_updata[i] = data[i - 2];
                }
                uart_transmit(modele_updata, sizeof(modele_updata));
                wait_ms(3);
                break;

            case 0x20: // disable EP1
                modele_updata[0] = 0x55;
                modele_updata[1] = 0x40;
                for (uint8_t i = 2; i < 66; i++) {
                    modele_updata[i] = data[i - 2];
                }
                uart_transmit(modele_updata, sizeof(modele_updata));
                wait_ms(3);
                break;

            case 0x00:
                for (uint8_t i = 0; i < 60; i++) {
                    uart_write(0x00);
                }
                wait_ms(5);
                modele_updata[0] = 0x55;
                modele_updata[1] = 0x40;
                for (uint8_t i = 2; i < 66; i++) {
                    modele_updata[i] = data[i - 2];
                }
                uart_transmit(modele_updata, sizeof(modele_updata));
                wait_ms(3);
                break;

            case 0x01: // check version
                modele_updata[0] = 0x55;
                modele_updata[1] = 0x40;
                for (uint8_t i = 2; i < 66; i++) {
                    modele_updata[i] = data[i - 2];
                }
                uart_transmit(modele_updata, sizeof(modele_updata));
                wait_ms(3);
                break;

            case 0x02: // DFU start
                packcount = 0;

                modele_updata[0] = 0x55;
                modele_updata[1] = 0x40;
                for (uint8_t i = 2; i < 30; i++) {
                    modele_updata[i] = data[i - 2];
                }
                uart_transmit(modele_updata, sizeof(modele_updata));
                wait_ms(3);
                break;

            case 0x03:
                packcount        = 0;
                modele_updata[0] = 0x55;
                modele_updata[1] = 0x40;
                for (uint8_t i = 2; i < 66; i++) {
                    modele_updata[i] = data[i - 2];
                }
                uart_transmit(modele_updata, sizeof(modele_updata));
                wait_ms(3);
                break;

            case 0x04:
                modele_updata[0] = 0x55;
                modele_updata[1] = 0x40;
                for (uint8_t i = 2; i < 66; i++) {
                    modele_updata[i] = data[i - 2];
                }
                uart_transmit(modele_updata, sizeof(modele_updata));
                wait_ms(3);
                break;

            case 0x10:
                //打印发下来的数据
                // if( ( (((packcount+1)*62)/256) >   ((packcount*62)/256)  )  || ((packcount+1)*62 >= bin_size)   )
                // {
                //     data[2]=((packcount*62)/256)%256;
                //     data[3]=((packcount*62)/256)/256;
                //     raw_hid_send(data, length);
                // }
                packcount++;
                //透传module数据，�?�加2�?字节
                //在串口接收�?�拆�?2�?字节，上传pc
                modele_updata[0] = 0x55;
                modele_updata[1] = 0x40;
                for (uint8_t i = 2; i < 66; i++) {
                    modele_updata[i] = data[i - 2];
                }
                uart_transmit(modele_updata, sizeof(modele_updata));
                wait_ms(3);
                break;

            default:
                if(data[1] <= 0x40)
                {
                    modele_updata[0] = 0x55;
                    modele_updata[1] = 0x40;
                    for (uint8_t i = 2; i < 66; i++) {
                        modele_updata[i] = data[i - 2];
                    }
                    uart_transmit(modele_updata, sizeof(modele_updata));
                    wait_ms(3);
                    break;
                }
                else
                {
                    return false;
                }
        }

        return true;
    }

#ifdef Screen_AP
    else if(ap_message_buff[0]  == 0x40  || (ap_message_buff[0]  == 0x41) || (ap_message_buff[0]  == 0x42  ) || (ap_message_buff[0] == AP_RGB_TEST))  //ap report id
    {
       // raw_hid_send(ap_message_buff, sizeof(ap_message_buff));
        // return true;
         switch(ap_message_buff[0])	//cmd
        {
                case 	0x40:
                    //    rgb_wireless_timer  = timer_read32();
                    if( R_FLAG( kb_screen.flag , PK_MOD_SEM )!=NEED_UPDATE ){
                         sdWrite(&SD3,  ap_message_buff+7,data_len);
                         SET_FLAG( kb_screen.flag , PK_MOD_SEM );
                         kb_screen.ap_countdown = 100;
                        ap_message_buff[6] = 0x55;
                    }
                    else{
                        ap_message_buff[6] = 0x0f;
                        // ap_message_buff[6] = 0x0f;
                    }

                    raw_hid_send(ap_message_buff, sizeof(ap_message_buff));
                    // for(uint32_t i=0 ; i< sizeof(ap_message_buff) ; i++ ){
		            //     uprintf("%x\r\n",ap_message_buff[i]);
                    // }


                    break;

            case 	0x41:
                    //    rgb_wireless_timer  = timer_read32();
                    if( R_FLAG( kb_screen.flag , PK_MOD_SEM )==NEED_UPDATE ){
                         kb_screen.ap_countdown = 100;
                         sdWrite(&SD3,  ap_message_buff+7,data_len);
                         ap_message_buff[6] = 0x55;
                         wait_us(5);
                    }
                    else{
                        ap_message_buff[6] = 0x0f;
                        // ap_message_buff[6] = 0x0f;
                    }

                     raw_hid_send(ap_message_buff, sizeof(ap_message_buff));
                    //  setPinOutput(ARGB_LEFT_EN);
                    //  writePinLow(ARGB_LEFT_EN);

                    break;
            case 	0x42:
                    //  rgb_wireless_timer  = timer_read32();
                    if( R_FLAG( kb_screen.flag , PK_MOD_SEM )==NEED_UPDATE ){
                        CLEAR_FLAG( kb_screen.flag , PK_MOD_SEM );
                        ap_message_buff[6] = 0x55;
                    }
                    else{
                        ap_message_buff[6] = 0x0f;
                    }

                    raw_hid_send(ap_message_buff, sizeof(ap_message_buff));

                    break;

            case    AP_RGB_TEST:
                    // rgb_wireless_timer  = timer_read32();
                    ap_message_buff[6] = 0x55;
                    test_rgb_r = ap_message_buff[7];
                    test_rgb_g = ap_message_buff[8];
                    test_rgb_b = ap_message_buff[9];
                    raw_hid_send(ap_message_buff, sizeof(ap_message_buff));


                    default:
                    ap_message_buff[6] = 0x55;

                    break;


        }
       return true;

    }

#endif



    return false;
}
// #endif

//作为临时定时器使�?    周期�?1ms
#ifdef Screen_AP
void matrix_scan_kb (void)
{
    time_50ms_count++;
   	if(time_50ms_count >= 50) //50ms
    {
        // if(kb_screen.screen_boot_step==0){
        //     kb_screen.screen_boot_time++;
        // }
        if( R_FLAG( kb_screen.flag , PK_MOD_SEM )==NEED_UPDATE ){
            if( kb_screen.ap_countdown!=0 ){
                kb_screen.ap_countdown--;
            }
            else{
                CLEAR_FLAG( kb_screen.flag , PK_MOD_SEM );
            }
        }
     time_50ms_count = 0;
    }
}

#endif
