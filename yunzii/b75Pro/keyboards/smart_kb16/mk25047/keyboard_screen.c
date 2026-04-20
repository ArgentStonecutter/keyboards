#include "keyboard_screen.h"
#include "uart_mod.h"
#include "hygui.h"
#include "quantum.h"
#include "uart.h"
#include "smart_ble.h"
#include "keyboard.h"
#include "print.h"
#include "usb_main.h"
#include "usb_util.h"
#include "usb_driver.h"
#include "adc.h"
#include "mk25047.h"
#include "os_detection.h"
#include "raw_hid.h"
#include "process_rgb.h"
#include "action_util.h"
#include "common.h"
#include "rtc.h"
#include "uart.h"
#include "battery.h"

// #include "test_pic.h"

/* ***************************
			EXTERN
*************************** */

/* ***************************
			EXTERN
*************************** */
// static void delay_ms( uint32_t ms ){
// 	while( ms>0 ){
// 		SYS_delay_10nop(2821);
// 		ms--;
// 	}
// }
// static void delay_us( uint32_t us ){
// 	while( us>0 ){
// 		SYS_delay_10nop(3);
// 		us--;
// 	}
// }
extern bool flag_SCREEN_time;
extern enum batt_charge_status_t batt_charge_status;
uint8_t keyboard_screen_buff[32];
KEYBOARD_SCREEN_T kb_screen = {
    .buff = keyboard_screen_buff,
    .flag = { 0Xffffffff },
	.screen_boot_step = 0,
	.screen_boot_time = 0,
    .ap_countdown = 0,
};

#define SCREEN_UART_POOL_SIZE 8
uint8_t screen_pool[SCREEN_UART_POOL_SIZE];
// uint16_t screen_boot_time = 1 ;
// uint8_t screen_boot_step = 0 ;
uint8_t pre_batt_status = batt_default ;
uint8_t f_os_mode = 0 ;
uint8_t light_mode = 0 ;
uint8_t Batt_Baifen = 0 ;
uint8_t Keyboard_mode;
uint32_t ret = 1;
uint8_t temp_u8;



/* ***************************
		FUNC DECLARE
*************************** */
uint32_t uart_mod_dma_send( uint32_t data_size , uint8_t *data );



/* ***************************
		UART MOD INIT
*************************** */
#define SCREEN_UART_BUFF_SIZE    	32
#define SCREEN_HEAD_BYTELEN			2
#define SCREEN_TYPE_BYTELEN	    	1
#define SCREEN_LEN_BYTELEN			2
#define SCREEN_CHECK_BYTELEN		2
#define SCREEN_CRC_POLY		    	0xA001	// CRC-16/MODBUS ~8005~
#define SCREEN_CRC_INIT		    	0xFFFF	// CRC-16/INIT  	0xFFFF
#define SCREEN_CRC_XOROUT	    	0x0000	// CRC-16/XOROUT  0x0000
static uint8_t kb_buff[SCREEN_UART_BUFF_SIZE];
static uint8_t kb_protocol_buff[8];
const uint8_t pk_head[2] = {0xA5,0x5A};
static void kb_mod_crc(uint32_t *compute_check,const uint8_t *data){
 	*compute_check = *data ^ *compute_check;
	for( char i=0;i < 8;i++ ){
		if(*compute_check & 0x01){
			*compute_check = (*compute_check >> 1) ^ SCREEN_CRC_POLY;
		}
		else{
			*compute_check = *compute_check >> 1;
		}
	}
}
static uint32_t uart_debug_send(uint32_t size,const uint8_t *addr){
	//printk("\nUART SEND:");
//	uprintf("send finish\r\n");
	sdWrite(&SD3, addr,size);
   // uprintf("%ld\r\n",size);
	for(uint32_t i=0 ; i<size ; i++ ){
		// uprintf("%x\r\n",addr[i]);
	// printk("%02X",data[i]);
	}
    return 1;
}


 UART_MOD_T screen_uart = {
	.rx = {
		.pool_size = SCREEN_UART_POOL_SIZE,
		.pool = screen_pool,
		.write_pos = 0,
		.read_pos = 0,
	},
	.pk = {
		.buff = kb_buff,
		.buff_size = SCREEN_UART_BUFF_SIZE,

		.pk_step = PK_REC_HEAD,
		.head_Bsize = SCREEN_HEAD_BYTELEN,
		.type_remain = SCREEN_TYPE_BYTELEN,
		.type_Bsize = SCREEN_TYPE_BYTELEN,
		.type_max_size = PK_CTL_SIZE,
		.len_remain = SCREEN_LEN_BYTELEN,
		.len_Bsize = SCREEN_LEN_BYTELEN,
		.check_remain = SCREEN_CHECK_BYTELEN,
		.check_Bsize = SCREEN_CHECK_BYTELEN,

		.head = pk_head,
		.head_step = 0,
		.type = 0,
		.len = 0,
		.len_step = 0,
		.rec_check = 0,
		.compute_check = SCREEN_CRC_INIT,
		.check_init = SCREEN_CRC_INIT,

		.rec_err_number = 0,
		.handle_ret = 0,
		.ret_buff = kb_protocol_buff,

		.crc = kb_mod_crc,
		// .timer_start = kb_rx_timer_start,
		// .timer_load = kb_rx_timer_load,
		// .timer_close = kb_rx_timer_close,
	},
	.attr = {
		.id = SCREEN_ID,
		.timer = MOD_TIMER_OFF,
		.typeover = MOD_TYPEOVER_CONTINUE_OFF,
		.check = PK_CHECK_INFO,
	},
	.tx = {
		// .write = uart_mod_dma_send,
		.write = uart_debug_send,
	},
};



/* ***************************
		SEND COMMOND
*************************** */
static void screen_update(void);
void keyboard_screen_process(void){
	switch (kb_screen.screen_boot_step){
		case 0 :
        //增加计时
			if( kb_screen.screen_boot_time<=20 ){
				volatile static uint8_t next_trigger_time = 0;
				if( kb_screen.screen_boot_time%10==0 && kb_screen.screen_boot_time==next_trigger_time){
					next_trigger_time=(kb_screen.screen_boot_time/10+1)*10;
					SET_FLAG( kb_screen.flag , PK_CONN_TYPE );
				}
			}else{
			//	printk("'\n screen_boot_step 0 -> 1");
				pre_batt_status = SCREEN_BATT_UNCHARGING;
				SET_FLAG( kb_screen.flag , PK_OS_TYPE );
				SET_FLAG( kb_screen.flag , PK_CONN_TYPE );
				SET_FLAG( kb_screen.flag , PK_CAPS_STATUS );
				SET_FLAG( kb_screen.flag , PK_BATT_STATUS );
				SET_FLAG( kb_screen.flag , PK_BATT_QUANTITY );
				SET_FLAG( kb_screen.flag , PK_NUMLOCK_STATUS );
				SET_FLAG( kb_screen.flag , PK_WINLOCK_STATUS );
                SET_FLAG( kb_screen.flag , PK_LETTER_OFF );
				kb_screen.screen_boot_step = 1;
			}

		case 1 :
			if( R_FLAG( kb_screen.flag , PK_MOD_SEM )==NEED_UPDATE ){
				break;
			}
			for( char i=0 ; i<1 ; i++ ){
				// if( kb_screen.flag[i]!=0xFFFFFFFF  ){
                 if( kb_screen.flag[0] !=0xFFFFFFFF  ){
					// uprintf("enter here3\r\n");
					// uprintf("enter here3\r\n");
					// uprintf("enter here3\r\n");
					// uprintf("enter here3\r\n");
					screen_update();
				}
			}
		break;

		default :
			kb_screen.screen_boot_step = 1;
		break;
	}
}

static void screen_update(void){
//	printk("\n screen_update");
	//printk("\n flag=%X",kb_screen.flag[0]);

	uint8_t cur_flag;
	uint16_t data_len=1;



	if( R_FLAG( kb_screen.flag , PK_PROTOCOL_RET 				)==NEED_UPDATE ){
		cur_flag = PK_PROTOCOL_RET;
		uart_mod_pack( &screen_uart ,PK_PROTOCOL_RET,kb_screen.buff,data_len,&temp_u8 );
	}
    else if( R_FLAG( kb_screen.flag , PK_LETTER_OFF 			)==NEED_UPDATE){
        cur_flag = PK_LETTER_OFF;
        if(flag_SCREEN_switch){temp_u8 = 0;}else{temp_u8 = 1;}
        uart_mod_pack( &screen_uart ,PK_LETTER_OFF,kb_screen.buff,data_len,&temp_u8 );
    }
    else if( R_FLAG( kb_screen.flag , PK_LETTER_SHOW 			)==NEED_UPDATE){
        cur_flag = PK_LETTER_SHOW;
        // temp_u8 = flag_press_letter;
        uart_mod_pack( &screen_uart ,PK_LETTER_SHOW,kb_screen.buff,data_len,&temp_u8 );
    }
    else if( R_FLAG( kb_screen.flag , PK_TO_TIME 			)==NEED_UPDATE){
        cur_flag = PK_TO_TIME;

        uart_mod_pack( &screen_uart ,PK_TO_TIME,kb_screen.buff,0,&temp_u8 );
    }
	else if( R_FLAG( kb_screen.flag , PK_CONN_TYPE 				)==NEED_UPDATE ){
		cur_flag = PK_CONN_TYPE;
		uart_mod_pack( &screen_uart ,PK_CONN_TYPE,kb_screen.buff,data_len,&Keyboard_mode );
	}
	else if( R_FLAG( kb_screen.flag , PK_OS_TYPE 				)==NEED_UPDATE ){
		cur_flag = PK_OS_TYPE;
		uart_mod_pack( &screen_uart ,PK_OS_TYPE,kb_screen.buff,data_len,&f_os_mode );
	}
	else if( R_FLAG( kb_screen.flag , PK_CAPS_STATUS			)==NEED_UPDATE ){
		cur_flag = PK_CAPS_STATUS;
		// if( host_keyboard_status&0x02 ){ temp_u8=1; }else{ temp_u8=0; }
        //
    	// uprintf("capslock\r\n");
		// uprintf("capslock\r\n");
		// uprintf("capslock\r\n");
		// uprintf("capslock\r\n");
        if(host_keyboard_leds()&0x02 ){ temp_u8=1; }else{ temp_u8=0; }
        if (!gpio_read_pin(LED_CAP)) {
            temp_u8 = 1;
        }
        else {
            temp_u8 = 0;
        }
		uart_mod_pack( &screen_uart ,PK_CAPS_STATUS,kb_screen.buff,data_len,&temp_u8 );
	}
	else if( R_FLAG( kb_screen.flag , PK_NUMLOCK_STATUS			)==NEED_UPDATE ){
		// uprintf("numslock\r\n");
		// uprintf("numslock\r\n");
		// uprintf("numslock\r\n");
		// uprintf("numslock\r\n");
		cur_flag = PK_NUMLOCK_STATUS;
		if( host_keyboard_leds()&0x01 ){ temp_u8=1; }else{ temp_u8=0; }
		uart_mod_pack( &screen_uart ,PK_NUMLOCK_STATUS,kb_screen.buff,data_len,&temp_u8 );
	}
	else if( R_FLAG( kb_screen.flag , PK_WINLOCK_STATUS			)==NEED_UPDATE ){
		cur_flag = PK_WINLOCK_STATUS;
		//if( win_flag==0 ){ temp_u8=0; }else{ temp_u8=1; }//
        // if ( win_lock_flag==1 ){ temp_u8=1; }else{ temp_u8=0; }
        if (!gpio_read_pin(LED_WIN) || win_lock_flag) {
            temp_u8 = 1;
        }
        else {
            temp_u8 = 0;
        }
		uart_mod_pack( &screen_uart ,PK_WINLOCK_STATUS,kb_screen.buff,data_len,&temp_u8 );
	}
	else if( R_FLAG( kb_screen.flag , PK_BATT_QUANTITY 			)==NEED_UPDATE ){
		cur_flag = PK_BATT_QUANTITY;
		uart_mod_pack( &screen_uart ,PK_BATT_QUANTITY,kb_screen.buff,data_len,&Batt_Baifen );
	}
	else if( R_FLAG( kb_screen.flag , PK_BATT_STATUS			)==NEED_UPDATE ){
		cur_flag = PK_BATT_STATUS;
        if( batt_charge_status==batt_charge_full){
			pre_batt_status = SCREEN_BATT_CHARGING;
			temp_u8 = 0;
		}
		else if( batt_charge_status==batt_no_charge){
			pre_batt_status = SCREEN_BATT_UNCHARGING;
			temp_u8 = 0;
		}
		else if( batt_charge_status==batt_is_charging){
			pre_batt_status = SCREEN_BATT_WARNGING;
			temp_u8 = 1;
		}
		// if( batt_charge_status==batt_charge_full && pre_batt_status!=SCREEN_BATT_CHARGING ){
		// 	pre_batt_status = SCREEN_BATT_CHARGING;
		// 	temp_u8 = 0;
		// }
		// else if( batt_charge_status==batt_no_charge && pre_batt_status!=SCREEN_BATT_UNCHARGING ){
		// 	pre_batt_status = SCREEN_BATT_UNCHARGING;
		// 	temp_u8 = 0;
		// }
		// else if( batt_charge_status==batt_is_charging && pre_batt_status!=SCREEN_BATT_WARNGING ){
		// 	pre_batt_status = SCREEN_BATT_WARNGING;
		// 	temp_u8 = 1;
		// }
		//  temp_u8 = 0;
		// uprintf("enter here3\r\n");
		// uprintf("enter here3\r\n");
		// uprintf("enter here3\r\n");
		// uprintf("enter here3\r\n");
		uart_mod_pack( &screen_uart ,PK_BATT_STATUS,kb_screen.buff,data_len,&temp_u8 );
	}
	else if( R_FLAG( kb_screen.flag , PK_LIGHT_MODE 					)==NEED_UPDATE ){
		cur_flag = PK_LIGHT_MODE;
		uart_mod_pack( &screen_uart ,PK_LIGHT_MODE,kb_screen.buff,data_len,&light_mode );
	}

	else if( R_FLAG( kb_screen.flag , PK_GO_HOME 					)==NEED_UPDATE ){
		// uprintf("enter here1\r\n");
		// uprintf("enter here1\r\n");
		// uprintf("enter here1\r\n");
		// uprintf("enter here1\r\n");
		cur_flag = PK_GO_HOME;
		uart_mod_pack( &screen_uart ,PK_GO_HOME,kb_screen.buff,0,&temp_u8 );
	}
	else if( R_FLAG( kb_screen.flag , PK_ADD_PIC 					)==NEED_UPDATE ){
		cur_flag = PK_ADD_PIC;
		CLEAR_FLAG( kb_screen.flag , cur_flag );
		return ;
	}
	else if( R_FLAG( kb_screen.flag , PK_TOGGLE_PIC 					)==NEED_UPDATE ){
		// uprintf("enter here2\r\n");
		// uprintf("enter here2\r\n");
		// uprintf("enter here2\r\n");
		// uprintf("enter here2\r\n");
		cur_flag = PK_TOGGLE_PIC;
		uart_mod_pack( &screen_uart ,PK_TOGGLE_PIC,kb_screen.buff,0,&temp_u8 );
	}
	else if( R_FLAG( kb_screen.flag , PK_DEL_PIC 					)==NEED_UPDATE ){
		cur_flag = PK_DEL_PIC;
		uart_mod_pack( &screen_uart ,PK_DEL_PIC,kb_screen.buff,0,&temp_u8 );
	}
    else if( R_FLAG( kb_screen.flag , PK_GO_GIF 					)==NEED_UPDATE ){
		// uprintf("enter here3\r\n");
		// uprintf("enter here3\r\n");
		// uprintf("enter here3\r\n");
		// uprintf("enter here3\r\n");
		cur_flag = PK_GO_GIF;
		uart_mod_pack( &screen_uart ,PK_GO_GIF,kb_screen.buff,0,&temp_u8 );
	}

	else if( R_FLAG( kb_screen.flag , PK_TIME 					)==NEED_UPDATE ){
		cur_flag = PK_TIME;
		data_len=3;
		uart_mod_pack( &screen_uart ,PK_TIME,kb_screen.buff,data_len,&Keyboard_mode );
	}
	else if( R_FLAG( kb_screen.flag , PK_DATE 					)==NEED_UPDATE ){
		cur_flag = PK_DATE;
		data_len=4;
		uart_mod_pack( &screen_uart ,PK_DATE,kb_screen.buff,data_len,&Keyboard_mode );
	}
    else if ( R_FLAG( kb_screen.flag , PK_SWITCH_TIME 					)==NEED_UPDATE ) {
        cur_flag = PK_SWITCH_TIME;
        if (!variable_data.eeconfig_SCREEN_flag_time) {
            temp_u8 = 0;
        }
        else {
            temp_u8 = 1;
        }
        uart_mod_pack( &screen_uart ,PK_SWITCH_TIME,kb_screen.buff,1,&temp_u8 );
    }
	else{
	//	printk("\n SCREEN_SCREEN_FLAG_ERR");
		while(1){}
	}

	ret = 1;
	if( screen_uart.attr.check!=PK_CHECK_OFF ){
		ret=screen_uart.tx.write(screen_uart.pk.head_Bsize+screen_uart.pk.type_Bsize+screen_uart.pk.len_Bsize+screen_uart.pk.check_Bsize+data_len,kb_screen.buff);
	}
	else{
		ret=screen_uart.tx.write(screen_uart.pk.head_Bsize+screen_uart.pk.type_Bsize+screen_uart.pk.len_Bsize+data_len,kb_screen.buff);
	}
	CLEAR_FLAG( kb_screen.flag , cur_flag );
}
