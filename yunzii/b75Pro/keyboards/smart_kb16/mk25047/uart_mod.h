// #ifndef HYSCANF_H
// #define HYSCANF_H


// /*
//  * include
//  */
// #include <zephyr.h>
// #include <soc.h>
// #include <console/console.h>



#pragma once


#include "quantum.h"
/*
 * enum
 */
enum{
    SCREEN_ID,
};
enum {

	PK_PROTOCOL_RET,

	PK_CONN_TYPE,
	PK_OS_TYPE,
	PK_CAPS_STATUS,
	PK_NUMLOCK_STATUS,
	PK_WINLOCK_STATUS,
	PK_BATT_QUANTITY,
	PK_BATT_STATUS,
	PK_LIGHT_MODE,

	PK_TIME,
	PK_DATE,
	PK_GO_HOME,
	PK_ADD_PIC,
	PK_TOGGLE_PIC,
	PK_DEL_PIC,
	PK_GO_GIF,

	PK_GUI_EVENT,

	PK_ADD_GIF,
	PK_GIF_NUM,
	PK_GIF_FRAME,

    PK_VERSION,
    PK_SWITCH_TIME,

	PK_CTL_SIZE,

    PK_TO_TIME,
    PK_LETTER_SHOW,
    PK_LETTER_OFF,
	PK_MOD_SEM
};
enum{
    MOD_ERR,
    MOD_OK,
    MOD_OVERTIME,
    MOD_LENERR,
    MOD_TYPEERR,
    MOD_HEARERR,
    MOD_CHECKERR,
    MOD_OVERSIZE,
};

enum{
    PK_REC_HEAD,
    PK_REC_TYPE,
    PK_REC_LEN,
    PK_REC_CHECK,
    PK_REC_DATA,
};
enum{
    MOD_TIMER_OFF,
    MOD_TIMER_ON,
};
enum{
    PK_CHECK_OFF,
    PK_CHECK_INFO,
    PK_CHECK_ALL
};
enum{
    MOD_TYPEOVER_CONTINUE_OFF,
    MOD_TYPEOVER_CONTINUE_ON,
};

/*
 * typedef
 */
// typedef char mod_fun_ret_t;

typedef struct{
    uint32_t pool_size;
    uint8_t *pool;
    uint32_t write_pos;
    uint32_t read_pos;
}UART_MOD_RX_T;

typedef struct{
	uint8_t *buff;
	uint32_t buff_size;

	uint8_t pk_step;

	uint8_t head_step;
	uint8_t head_Bsize;
	uint8_t type_remain;
	uint8_t type_Bsize;
	uint8_t type_max_size;
	uint8_t len_remain;
	uint8_t len_Bsize;

	uint8_t check_remain;
	uint8_t check_Bsize;

	const uint8_t *head;
	uint32_t type;
	uint32_t len;
	uint32_t len_step;
	uint32_t rec_check;
	uint32_t compute_check;
	uint32_t check_init;

	uint8_t rec_err_number;
	uint8_t handle_ret;
	uint8_t *ret_buff;

	void (*crc)(uint32_t *compute_check,const uint8_t *data);
	void (*timer_start)(void);
	void (*timer_load)(void);
	void (*timer_close)(void);
}UART_MOD_PK_T;

typedef struct{
    uint32_t (*write)(uint32_t size,const uint8_t *addr);
}UART_MOD_TX_T;

typedef struct{
    uint8_t id;
    uint8_t check;
    uint8_t timer;
    uint8_t typeover;
}UART_MOD_ATTR_T;

typedef struct{
    UART_MOD_ATTR_T attr;
    UART_MOD_RX_T rx;
    UART_MOD_TX_T tx;
    UART_MOD_PK_T pk;
}UART_MOD_T;

/*
 * function
 */
void mod_protocol_ret(UART_MOD_T *mod);
char uart_mod_pack(UART_MOD_T *mod, uint8_t type,uint8_t *buff_addr,uint32_t data_size,const uint8_t *data_addr);
void uart_mod_row_data_handle(UART_MOD_T *mod);

// #endif /* KEYBOARD_SCREEN_USART */
