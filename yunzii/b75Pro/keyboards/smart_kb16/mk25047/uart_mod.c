#include "uart_mod.h"
#include "keyboard_screen.h"
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
// #include "common.h"
#include "rtc.h"
#include "uart.h"
// #include "hygui.h"





// buff_addr 包头+包类型+数据长度+校验+数据      2+1+2+2
//uart_mod_pack( &screen_uart ,PK_OS_TYPE,uart_mod_pack( &screen_uart ,PK_OS_TYPE,kb_screen.buff,data_len,&f_os_mode );,data_len,&f_os_mode );
char uart_mod_pack(UART_MOD_T *mod, uint8_t type,uint8_t *buff_addr,uint32_t data_size,const uint8_t *data_addr){

	uint32_t i;
	uint32_t len_fill_start_offset=mod->pk.head_Bsize+mod->pk.type_Bsize;
    uint32_t compute_check = mod->pk.check_init;
    uint32_t check_fill_start_offset = 0;
    uint32_t data_fill_start_offset = 0;

	for( i=0 ; i<mod->pk.head_Bsize ; i++ ){
		buff_addr[i] = (uint8_t)(mod->pk.head[i]);
	}
	for( i=0 ; i<mod->pk.type_Bsize ; i++ ){
		buff_addr[i+mod->pk.head_Bsize] = (uint8_t)(type>>(mod->pk.type_Bsize-i-1)*8);
	}
	for( i=0 ; i<mod->pk.len_Bsize ; i++ ){
		buff_addr[i+len_fill_start_offset] = (uint8_t)(data_size>>(mod->pk.len_Bsize-i-1)*8);
	}


	if( mod->attr.check!=PK_CHECK_OFF ){
		char check_start_offset = mod->pk.head_Bsize;
		char check_no_data_size = mod->pk.type_Bsize+mod->pk.len_Bsize;
		// uint32_t compute_check = mod->pk.check_init;

		// uint32_t check_fill_start_offset;
		// uint32_t data_fill_start_offset;
		if( mod->attr.check==PK_CHECK_INFO ){
			check_fill_start_offset = check_start_offset+check_no_data_size;
			data_fill_start_offset = check_fill_start_offset+mod->pk.check_Bsize;
		}
		else if( mod->attr.check==PK_CHECK_ALL ){
			data_fill_start_offset = check_start_offset+mod->pk.check_Bsize;
			check_fill_start_offset = data_fill_start_offset+data_size;
		}

		for( i=check_start_offset ; i<check_start_offset+check_no_data_size ; i++ ){
			mod->pk.crc( &compute_check,&buff_addr[i] );
		}

		if( mod->attr.check==PK_CHECK_INFO ){
			for( i=data_fill_start_offset ; i<data_fill_start_offset+data_size ; i++ ){
				buff_addr[i] = data_addr[i-data_fill_start_offset];
			}
		}
		else if( mod->attr.check==PK_CHECK_ALL ){
			for( i=data_fill_start_offset ; i<data_fill_start_offset+data_size ; i++ ){
				buff_addr[i] = data_addr[i-data_fill_start_offset];
				mod->pk.crc( &compute_check,& buff_addr[i] );
			}
		}

		for( i=0 ; i<mod->pk.check_Bsize ; i++ ){
			buff_addr[i+check_fill_start_offset] = (uint8_t)(compute_check>>(mod->pk.check_Bsize-i-1)*8);
		}
	}
	return MOD_OK;
}
