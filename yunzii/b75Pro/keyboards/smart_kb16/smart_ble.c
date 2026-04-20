/* Copyright 2023 Jacky Jia www.i-chip.cn
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
/*
    Copyright (C) 2020 Yaotian Feng, Codetector<codetector@codetector.cn>
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


#include "uart.h"
#include "ch.h"
#include "hal.h"
#include "host.h"
#include "host_driver.h"
#include "report.h"
#include "smart_ble.h"
#include "print.h"

#include "quantum.h"

#include "smart_ble.h"
#include "keyboard.h"
#include "analog.h"

#include "usb_main.h"
#include "usb_util.h"
#include "usb_driver.h"
#include "os_detection.h"
#include "raw_hid.h"


/**********

已改：

----- Static Function Prototypes -----
static uint8_t ap2_ble_leds(void);
static void    ap2_ble_mouse(report_mouse_t *report);
static void    ap2_ble_extra(report_extra_t *report);
static void    ap2_ble_keyboard(report_keyboard_t *report);

static void ap2_ble_swtich_ble_driver(void);

-------------------- Static Local Variables ----------------
static host_driver_t ap2_ble_driver = {
    ap2_ble_leds, ap2_ble_keyboard, NULL, ap2_ble_mouse, ap2_ble_extra
};
14:07
BLE的驱动定义变了
14:07

//改定义在C:\qmk_20240305\tmk_core\protocol\host_driver.h
typedef struct {
    uint8_t (*keyboard_leds)(void);
    void (*send_keyboard)(report_keyboard_t *);
    void (*send_nkro)(report_nkro_t *);
    void (*send_mouse)(report_mouse_t *);
    void (*send_extra)(report_extra_t *);
} host_driver_t;


 ***********/

#define WIRELESS_MODULE_WAKE_UP_BYTES_NUM 60
#define WIRELESS_MODULE_SLEEP_TIME 30000

uint8_t ble_led_state =0;
bool wireless_connected=false;
bool suspend = false;
uint32_t last_wireless_mode=1;

uint8_t ble_command[22]= {0x55, 0x20, 0x00, 0x00, 'S', 'm','a','r','t','B','L','E',0,0,0,0,0,0,0,0,0,0};
const char ble_name[] = "YUNZII B75PROMAX";

enum kb_mode_t kb_mode = 4;
bool           first_sleep_flag;
bool           keyboard_no_idle_flag;
uint32_t       packet_send_time;
uint32_t       keyboard_idle_time;
uint32_t       first_sleep_time;
uint32_t       delay_send_code_time;
uint32_t       keyboard_delay_send_code_time;
uint32_t       wireless_no_operation_time;
/* -------------------- Static Function Prototypes -------------------------- */
static uint8_t ap2_ble_leds(void);
static void    ap2_ble_mouse(report_mouse_t *report);
static void    ap2_ble_extra(report_extra_t *report);
static void    ap2_ble_keyboard(report_keyboard_t *report);
static void    ap2_send_nkro(report_nkro_t *report);
static void ap2_ble_swtich_ble_driver(void);

/* -------------------- Static Local Variables ------------------------------ */
static host_driver_t ap2_ble_driver = {
    ap2_ble_leds, ap2_ble_keyboard,ap2_send_nkro,ap2_ble_mouse, ap2_ble_extra
};//注册回调函数


static host_driver_t *last_host_driver = NULL;

/* -------------------- Public Function Implementation ---------------------- */


void smart_ble_wakeup(void)
{
    uint32_t mode_flag;


    if(first_sleep_flag)
    {
        eeconfig_read_kb_datablock(&variable_data);
        mode_flag = variable_data.eeconfig_last_wireless_mode;
        first_sleep_flag = false;
        for (int i=0;i<WIRELESS_MODULE_WAKE_UP_BYTES_NUM;i++)
        {
            uart_write(0x00);

        }

        wait_ms(10);
        if(mode_flag > 3)
        {
            mode_flag = 4;
        }
        ble_command[0]=0x55;  //sync
        ble_command[1]=20;   //len
        ble_command[2]=0;       //ble command
        ble_command[3]=mode_flag;    //ble mode 0~4 usb ble 1 2 3 & 24g
        // strcpy((char * restrict)(ble_command+4),PRODUCT);  //4  5
        // ble_command[strlen(PRODUCT)+4]='-';   //6
        // ble_command[strlen(PRODUCT)+5]='0'+mode_flag;   //7
        // ble_command[strlen(PRODUCT)+6]=0;     //8
        strcpy((char * restrict)(ble_command+4),ble_name);  //4  5
        // ble_command[strlen(ble_name)+4]='0'+mode_flag;   //7
        ble_command[strlen(ble_name)+5]=0;     //8
        uart_transmit(ble_command,sizeof(ble_command));
        wait_ms(5);
    }
}



void smart_ble_startup(void) {
    ap2_ble_swtich_ble_driver();
}

void smart_ble_broadcast(uint8_t port) {

}

void smart_ble_connect(uint8_t port) {

}

void smart_ble_disconnect(void) {
    /* Skip if the driver is already enabled */
    if (host_get_driver() != &ap2_ble_driver) {
        return;
    }

    clear_keyboard();
    host_set_driver(last_host_driver);
}

void smart_ble_unpair(void) {
    // sdPut(&SD1, 0x0);
    //sdWrite(&SD1, ble_mcu_unpair, sizeof(ble_mcu_unpair));
}



void WIRELESS_PAIR(uint32_t mode)
{
    smart_ble_startup();
    wireless_connected=false;
    for (int i=0;i<WIRELESS_MODULE_WAKE_UP_BYTES_NUM;i++)
    {
        uart_write(0x00);
    }
    wait_ms(350);  //make sure module power on


    if(mode<1 || mode>4)
    {
        mode=1;
    }
    if((mode>=1) && (mode<=4)) {                      // remember last wireless mode
        if (mode==4)
        {
            uint32_t temp ;
            eeconfig_read_kb_datablock(&variable_data);
            temp = variable_data.eeconfig_last_wireless_mode;
            variable_data.eeconfig_last_wireless_mode = temp|4;
            eeconfig_update_kb_datablock(&variable_data);
            last_wireless_mode=temp|4;
        }
        else
        {
            variable_data.eeconfig_last_wireless_mode = mode;
            eeconfig_update_kb_datablock(&variable_data);
            last_wireless_mode=mode;
        }
    }

    uart_write(0x55);
    uart_write(0x03);
    uart_write(0);  //battery level id
    uart_write(mode);
    uart_write(0x01);
    wait_ms(10);
    uart_write(0x55);
    uart_write(0x03);
    uart_write(0);  //battery level id
    uart_write(mode);
    uart_write(0x01);
    wait_ms(10);
}


void WIRELESS_START(uint32_t mode)
{
    smart_ble_startup();
    wireless_connected=false;
    if(mode<1 || mode>4)
    {
        mode=1;
    }

    if((mode>=1) && (mode<=4)) {                      // remember last wireless mode    切换到蓝牙可以记住上次蓝牙的模式
        if (mode==4)
        {
            uint32_t temp;
            eeconfig_read_kb_datablock(&variable_data);
            temp = variable_data.eeconfig_last_wireless_mode;
            variable_data.eeconfig_last_wireless_mode = temp|4;
            eeconfig_update_kb_datablock(&variable_data);
            last_wireless_mode=temp|4;
        }
        else
        {
            variable_data.eeconfig_last_wireless_mode = mode;
            eeconfig_update_kb_datablock(&variable_data);
            last_wireless_mode=mode;
        }
    }
    for (int i=0;i<WIRELESS_MODULE_WAKE_UP_BYTES_NUM;i++)
    {
        uart_write(0x00);
    }
    wait_ms(350);  //make sure module power on

    ble_command[0]=0x55;  //sync
    ble_command[1]=20;   //len
    ble_command[2]=0;       //ble command
    ble_command[3]=mode;    //ble mode 0~4 usb ble 1 2 3 & 24g
    // strcpy((char * restrict)(ble_command+4),PRODUCT);  //4  5
    // ble_command[strlen(PRODUCT)+4]='-';   //6
    // ble_command[strlen(PRODUCT)+5]='0'+mode;   //7
    // ble_command[strlen(PRODUCT)+6]=0;     //8
    strcpy((char * restrict)(ble_command+4),ble_name);  //4  5
    // ble_command[strlen(ble_name)+4]='0'+mode;   //7
    ble_command[strlen(ble_name)+5]=0;     //8
    uart_transmit(ble_command,sizeof(ble_command));
    wait_ms(10);
    uart_transmit(ble_command,sizeof(ble_command));
    wait_ms(10);
}

void WIRELESS_STOP(void) {
    wireless_connected = false;
    for (int i = 0; i < WIRELESS_MODULE_WAKE_UP_BYTES_NUM; i++) {
        uart_write(0x00);
    }
    wait_ms(100);
    smart_ble_disconnect();
    wait_ms(20);
    ble_command[0] = 0x55; // sync
    ble_command[1] = 2;    // len
    ble_command[2] = 0;    // ble command
    ble_command[3] = 0;    // ble mode 0~4 usb ble 1 2 3 & 24g
    uart_transmit(ble_command, 4);
}

/* ------------------- Static Function Implementation ----------------------- */
static void ap2_ble_swtich_ble_driver(void) {

    if (host_get_driver() == &ap2_ble_driver) {
        return;
    }
    clear_keyboard();
    last_host_driver = host_get_driver();
    ble_led_state = host_keyboard_leds();
    host_set_driver(&ap2_ble_driver);
}

void sc_ble_battary(uint8_t batt_level) {
    if(wireless_connected)
    {
       // smart_ble_wakeup();//从睡眠模式中唤醒
        uart_write(0x55);
        uart_write(0x02);
        uart_write(0x09);  //battery level id
        uart_write(batt_level);
        wait_ms(10);
    }
}

 //#ifdef Module_Updata
void Module_UpdataHandle(void)
{
    static enum {UART_READY, UART_0X55_RECEIVED, UART_LENS_REVEIVED,UART_Module, UART_REPORT_ID_RECEIVED} uart_state = UART_READY;
    static uint8_t 	uart_command[40];
    static uint8_t uart_lens =0;
    static uint8_t uart_buff_index=0;
    uint8_t c;
    uint8_t Pc_updata[64];
   //当前在usb模式下且在升级的过程中
   if( kb_mode ==KB_MODE_USB)
   {
        if (uart_available())
        {
            c= uart_read();     // 0x55 0x03 0x01 mode data  根据command判断
            switch (uart_state) {
                case UART_READY:
                    if (c==0x55)
                    {
                        uart_state=UART_0X55_RECEIVED;
                        uart_buff_index=0;
                        uart_command[uart_buff_index++]=c;
                    }
                    break;

                case UART_0X55_RECEIVED:
                    if (c ==0x55)
                        break;
                    else if((c >= 0x02) && (c <= 0x20))
                    {
                        uart_lens=c;
                        uart_command[uart_buff_index++]=c;
                        uart_state= UART_LENS_REVEIVED;
                    }
                    else
                        uart_state= UART_READY;
                    break;

                case UART_LENS_REVEIVED:
                        if(c == 0x0a)//模组升级指令
                        {
                            uart_command[uart_buff_index++]=c;
                            uart_state= UART_Module;
                        }
                    else
                        uart_state= UART_READY;
                    break;
    //question
                case UART_Module:    //这里逻辑异常，更改点20240226
                    uart_command[uart_buff_index++]=c;
                    if (uart_buff_index >= uart_lens + 2)//所有的数据存储完毕，开始补包   //20240226
                    {
                        memset(Pc_updata, 0, sizeof(Pc_updata));
                        for(uint8_t i = 0; i < uart_lens; i++)
                        {
                          Pc_updata[i] = uart_command[i+2];//拆包前两个字节

                        }
                        //剩余的字节全部用FF填充
                        for(uint8_t i = uart_lens; i < (64-uart_lens); i++)
                        {
                          Pc_updata[i] = 0xff;

                        }
                        raw_hid_send(Pc_updata, sizeof(Pc_updata));
                        uart_state= UART_READY;    //20240226 数据全部发送完成后状态切换到初始状态
                    }
                    break;
                default:
                    uart_state = UART_READY;
                    break;
            }


        }
   }


}
 //#endif

static uint8_t ap2_ble_leds(void) { //返回的就是灯的状态
    static enum { UART_READY, UART_0X55_RECEIVED, UART_LENS_REVEIVED, UART_WORKMODE, UART_REPORT_ID_RECEIVED } uart_state = UART_READY;
    static uint8_t  uart_command[40];
    static uint8_t  uart_lens = 0;

    static uint8_t  uart_workmode   = 0;
    static uint8_t  uart_buff_index = 0;
    uint8_t    c;

    if (uart_available()) {
        c = uart_read(); // 0x55 0x03 0x01 mode data  根据command判断
        switch (uart_state) {
            case UART_READY:
                if (c == 0x55) {
                    uart_state                      = UART_0X55_RECEIVED;
                    uart_buff_index                 = 0;
                    uart_command[uart_buff_index++] = c;
                }
                break;

            case UART_0X55_RECEIVED:
                if (c == 0x55)
                    break;
                else if (c == 0x03) // UART_COMMAND_LENS)
                {
                    uart_lens                       = c;
                    uart_command[uart_buff_index++] = c;
                    uart_state                      = UART_LENS_REVEIVED;
                } else
                    uart_state = UART_READY;
                break;
            case UART_LENS_REVEIVED:
                if (c <= 2) // UART_command
                {
                    uart_command[uart_buff_index++] = c;
                    uart_state                      = UART_WORKMODE;
                } else
                    uart_state = UART_READY;
                break;
            case UART_WORKMODE:
                if (c <= 4) // UART_mode
                {
                    uart_workmode                   = c;
                    uart_command[uart_buff_index++] = c;
                    uart_state                      = UART_REPORT_ID_RECEIVED;
                } else
                    uart_state = UART_READY;
                break;
            case UART_REPORT_ID_RECEIVED:

                uart_command[uart_buff_index++] = c;
                if (uart_buff_index >= uart_lens + 2) {
                    switch (uart_command[2]) {
                        case 0:
                            //获取的模式与当前的模式一致,2.4G和蓝牙分两种
                            if (last_wireless_mode <= 3) {
                                if (last_wireless_mode == uart_workmode) {
                                    wireless_connected = (c == 0);
                                }

                            } else {
                                if (uart_workmode == 4) {
                                    wireless_connected = (c == 0);
                                }
                            }

                            break;
                        case 1:
                            if (last_wireless_mode <= 3) {
                                if (last_wireless_mode == uart_workmode) {
                                    ble_led_state = c;
                                }

                            } else {
                                if (uart_workmode == 4) {
                                    ble_led_state = c;
                                }
                            }
                            break;
                        case 2:
                            if (last_wireless_mode > 3 && (uart_workmode == 4)) {
                                if (c == 0xaa) {
                                    suspend = true;

                                } else if (c == 0xbb) {
                                    suspend = false;
                                }
                            }

                            break;
                        default:
                            break;
                    }

                    uart_state = UART_READY;
                }
                break;
            default:
                uart_state = UART_READY;
                break;
        }
    }
    return ble_led_state;
}

static void ap2_ble_mouse(report_mouse_t *report) {
    static uint8_t last_report[sizeof(report_mouse_t)] = {0};
    keyboard_idle_time                                 = timer_read32();
    keyboard_no_idle_flag                              = true;
    if (wireless_connected) {
        if (memcmp(last_report,report,sizeof(report_mouse_t)))
        {
            smart_ble_wakeup();
            memcpy(last_report,report,sizeof(report_mouse_t));
            uart_write(0x55);
            uart_write(0x06);
            uart_transmit(last_report,sizeof(report_mouse_t));
        }

        if (timer_elapsed32(keyboard_delay_send_code_time) < 8 && (timer_elapsed32(keyboard_delay_send_code_time) > 0)) {
            delay_send_code_time = 8 - (timer_elapsed32(keyboard_delay_send_code_time));
            if (kb_mode != KB_MODE_24G)
                wait_ms(delay_send_code_time);
            else {
                wait_ms(2);
            }

        } else {
            if (kb_mode != KB_MODE_24G)
                wait_ms(8);
            else {
                wait_ms(2);
            }
        }
        keyboard_delay_send_code_time = timer_read32();
        //防止宏录制异常，更新心跳包计数以及一二级时间
        wireless_no_operation_time = timer_read32();
        first_sleep_time           = timer_read32();
        packet_send_time           = 0;
    }
}

static void ap2_ble_extra(report_extra_t *report) {
    keyboard_idle_time   = timer_read32();
    keyboard_no_idle_flag = true;


    if(wireless_connected)
    {
        smart_ble_wakeup();
        uart_write(0x55);
        uart_write(sizeof(report_extra_t));
        uart_transmit((uint8_t *)report,sizeof(report_extra_t));


          if(timer_elapsed32(keyboard_delay_send_code_time) < 8  && (timer_elapsed32(keyboard_delay_send_code_time) > 0))
          {
             delay_send_code_time =  8-(timer_elapsed32(keyboard_delay_send_code_time));

             //蓝牙下才延时
               if(kb_mode != KB_MODE_24G)
               wait_ms(delay_send_code_time);
                 else{
                wait_ms(2);
               }
               // uprintf("%ld\r\n",delay_send_code_time);

          }
          else{
              if(kb_mode != KB_MODE_24G)
              wait_ms(8);
              else{
                wait_ms(2);
               }
          //   uprintf("%d\r\n",222);
          }
          keyboard_delay_send_code_time = timer_read32();

          //防止宏录制异常，更新心跳包计数以及一二级时间
         wireless_no_operation_time  = timer_read32();
         first_sleep_time   = timer_read32();
         packet_send_time = 0;
    }

}

/*!
 * @brief  Send keyboard HID report for Bluetooth driver
 */


static void ap2_ble_keyboard(report_keyboard_t *report) {

    keyboard_idle_time   = timer_read32();
    keyboard_no_idle_flag = true;


    //   uprintf("keyboard_delay_send_code_time:%ld\r\n",timer_elapsed32(keyboard_delay_send_code_time));

    if(wireless_connected)
    {
        //
        smart_ble_wakeup();


        // if (keymap_config.nkro)
        // {
        //     uart_write(0x55);
        //     uart_write(0x12);  //nkro_report should be 0x20(32)
        //     uart_transmit((uint8_t *)report,0x12);
        // }
        // else
        {
            uart_write(0x55);
            uart_write(0x09);
            uart_write(0x01);
            uart_transmit((uint8_t *)report,KEYBOARD_REPORT_SIZE);
        }

//防止宏录制异常，更新心跳包计数以及一二级时间
         wireless_no_operation_time  = timer_read32();
         first_sleep_time   = timer_read32();
         packet_send_time = 0;


          if(timer_elapsed32(keyboard_delay_send_code_time) < 8  && (timer_elapsed32(keyboard_delay_send_code_time) > 0))
          {
             delay_send_code_time =  8-(timer_elapsed32(keyboard_delay_send_code_time));

             //蓝牙下才延时
               if(kb_mode != KB_MODE_24G)
                wait_ms(delay_send_code_time);
               else{
                wait_ms(2);
               }
          }
          else{
              if(kb_mode != KB_MODE_24G)
              wait_ms(8);
              else{
                wait_ms(2);
               }
          }
          keyboard_delay_send_code_time = timer_read32();
    }



}

static void ap2_send_nkro(report_nkro_t *report)
{

    keyboard_idle_time   = timer_read32();
    keyboard_no_idle_flag = true;


    //   uprintf("keyboard_delay_send_code_time:%ld\r\n",timer_elapsed32(keyboard_delay_send_code_time));

    if(wireless_connected)
    {
        //
        smart_ble_wakeup();



            uart_write(0x55);
            uart_write(0x15);  //nkro_report should be 0x20(32)
            uart_transmit((uint8_t *)report,0x15);


//防止宏录制异常，更新心跳包计数以及一二级时间
         wireless_no_operation_time  = timer_read32();
         first_sleep_time   = timer_read32();
         packet_send_time = 0;


          if(timer_elapsed32(keyboard_delay_send_code_time) < 8  && (timer_elapsed32(keyboard_delay_send_code_time) > 0))
          {
             delay_send_code_time =  8-(timer_elapsed32(keyboard_delay_send_code_time));

             //蓝牙下才延时
               if(kb_mode != KB_MODE_24G)
                wait_ms(delay_send_code_time);
               else{
                wait_ms(2);
               }
          }
          else{
              if(kb_mode != KB_MODE_24G)
              wait_ms(8);
              else{
                wait_ms(2);
               }
          }
          keyboard_delay_send_code_time = timer_read32();
    }

}



