// #ifndef HYGUI_H
// #define HYGUI_H

// #include <stdint.h>
// #include "spi_flash.h"
// #include "hyprintf.h"

#pragma once


#include "quantum.h"




#define BLUE 					0x001F
#define MY_BLUE 			0x03de

#define BRED 					0XF81F
#define GRED 					0XFFE0
#define GBLUE					0X07FF

#define RED  					0xF800

#define MAGENTA				0xF81F

#define GREEN					0x07E0
#define GREEN_2				0x3d26

#define CYAN 					0x7FFF

#define YELLOW				0xFFE0
#define YELLOW_2			0xdea4

#define BROWN					0XBC40
#define BRRED					0XFC07

#define GRAY 					0X8430
#define MY_GRAY				0xe696

#define DARKBLUE			0X01CF
#define LIGHTBLUE			0X7D7C
#define GRAYBLUE      0X5458
#define LIGHTGREEN    0X841F
#define LGRAY 			  0XC618
#define LGRAYBLUE     0XA651
#define LBBLUE        0X2B12



#define WHITE					0xFFFF
#define BLACK					0x0000

#define MAIN_MENU_F 	0xf73d
#define MAIN_MENU_B 	BLACK

#define ICON_OFF_F 		0x8410
#define ICON_OFF_B		0x4208
#define ICON_ON_F	 		0xFFFF
#define ICON_ON_B			0x3d26

#define LIST_B				BLACK
#define LIST_SEL_B		WHITE
#define LIST_SEL_F		BLACK
#define LIST_UNSEL_B	MAIN_MENU_B
#define LIST_UNSEL_F	WHITE

#define CLOCK_ICON_B	0xb33a

#define DATA_ICON_B		MAIN_MENU_B
#define	DATA_ICON_F		WHITE

#define CHARGING_FONT_B		BLACK
#define CHARGING_FONT_ON_F		0xf745
#define CHARGING_FONT_OFF_F		BLACK

#define FONT_BATT_CHARGING_B				BLACK
#define FONT_BATT_CHARGING_ON_F			0xf745
#define FONT_BATT_CHARGING_OFF_F		BLACK

#define FONT_BATT_LOW_POWER_B				BLACK
#define FONT_BATT_LOW_POWER_ON_F		0xF800
#define FONT_BATT_LOW_POWER_OFF_F		BLACK

#define FONT_BATT_WARNING_ON_F			0xFFE0

#define BATT_CHARGING_B			0x3d26
#define BATT_LOW_POWER_B		0xF800
#define BATT_WARNING_B			0xFFE0
#define BATT_UNCHARGING_B		0X8430









//#define R_ICON_FLAG(flag,type) 			( flag&(1<<type) )
//#define CLEAR_ICON_FLAG(flag,type) 	( flag |= (1<<type) )
//#define SET_ICON_FLAG(flag,type) 		( flag &= ~(1<<type) )

#define R_GUI_FLAG(flag,type)				( flag[type/32] &   (1<<(type%32)) )
#define CLEAR_GUI_FLAG(flag,type) 	( flag[type/32] |=  (1<<(type%32)) )
#define SET_GUI_FLAG(flag,type) 		( flag[type/32] &= ~(1<<(type%32)) )
#define GUI_NEED_UPDATE						0

enum{
	GUI_MAIN  ,
	GUI_LIST	,
	GUI_PIC 	,
	GUI_GIF		,
	GUI_SLEEP	,
};

enum{

	Caps_ON,
	Caps_OFF,
	Default_Status,
};

/* list id */
enum{
	LIGHT_MODE_LIST_ID,

};

enum{
	FONT_16X08,
	FONT_24X12,
	FONT_24X24,
	FONT_USER,
	FONT_NXN, // ���ó���2048B
} ;

enum{
	GUI_MAIN_CLOCK 			,
	GUI_MAIN_CONN_TYPE 	,
	GUI_MAIN_BATTERY		,
	GUI_MAIN_CAPS				,
	GUI_MAIN_NUMLOCK		,
	GUI_MAIN_DATE				,
	GUI_MAIN_OS_TYPE		,
	GUI_MAIN_WINLOCK		,
};
enum{
	GUI_LIST_LIGHT_MODE	,
};

enum{
	BATT_UNCHARGING,
	BATT_WARNING,
	BATT_LOW_POWER,
	BATT_CHARGING,

	BATT_STATUS_SIZE,
};

enum{
	LIGHT_MODE_BOLANG					,
	LIGHT_MODE_CAIYUN					,
	LIGHT_MODE_NIHONG					,
	LIGHT_MODE_HUIXUAN				,

	LIGHT_MODE_BAIHUA					,
	LIGHT_MODE_FEIXIANG				,
	LIGHT_MODE_CHUANGYUE			,
	LIGHT_MODE_XIAYU					,

	LIGHT_MODE_GUANGPU				,
	LIGHT_MODE_HUXI						,
	LIGHT_MODE_CHANGLIANG			,
	LIGHT_MODE_LIANYI					,

	LIGHT_MODE_JIGUANG	  		,
	LIGHT_MODE_DANDIAN				,
	LIGHT_MODE_KUOSAN					,
	LIGHT_MODE_SAOMIAO				,

	LIGHT_MODE_JIANBIAN	  		,
	LIGHT_MODE_ZIDING					,
	LIGHT_MODE_GUANBI					,

	LIGHT_MODE_SIZE						,
};

enum{
	CAPS_OFF,
	CAPS_ON,
};

enum{
	NUMLOCK_OFF,
	NUMLOCK_ON,
};

enum{
	WIN_LOCK_OFF,
	WIN_LOCK_ON,
};

enum{
	CONN_USB,
	CONN_BT_1,
	CONN_BT_2,
	CONN_BT_3,
	CONN_2D4G,
};

enum{
	WINDOWS_MODE,
	MAC_MODE,
};

typedef struct {
	uint32_t 		 ch;

	uint16_t x0;
	uint16_t y0;

	uint16_t fontcolor;
	uint16_t backcolor;
	char font_type;

	uint16_t width;
	uint16_t height;
	uint16_t cir;

}FONT_T;

typedef struct{

	FONT_T font;

	uint16_t x0;
	uint16_t y0;
	uint16_t width;
	uint16_t height;
	char	H;
	char	M;
	char	S;
	char 	clock_str[6+1];
	char	upd_f;
} CLOCK_ICON_T;

typedef struct{
	uint16_t x0;
	uint16_t y0;
	uint16_t width;
	uint16_t height;
	uint16_t fontcolor;
	uint16_t backcolor;
	char font_type;

	uint16_t		Y;
	char				M;
	char				D;
	char 	date_str[4+1];

} DATE_ICON_T;

typedef struct{
	uint16_t x0;
	uint16_t y0;

	uint16_t fontcolor;
	uint16_t backcolor;
	uint16_t blockcolor;

	char font_type;
	uint16_t width;
	uint16_t height;

	char	quantity;
	char 	batt_status;
	char	last_quantity;
	char	last_quantity_idx;

} BATTERY_ICON_T;

typedef struct{
	uint16_t x0;
	uint16_t y0;
	uint16_t width;
	uint16_t height;
	char cur_enum;
	const uint16_t **array;
} ENUM_ICON_T;

typedef struct {
	char cur_sel;
	char pre_sel;
	char cur_pic_num;
}GUI_MENU_T;

typedef struct{
	uint16_t 	width;
	uint16_t 	height;
	uint16_t	backcolor;
} GUI_MAIN_T;

typedef struct{

	struct HYLIST_T *pre_list;

	uint16_t	x0;
	uint16_t	y0;
	uint16_t 	width;
	uint16_t 	height;
	uint16_t	backcolor;
	uint16_t	fontcolor;
	uint16_t	sel_backcolor;
	uint16_t	sel_fontcolor;
	uint16_t	unsel_backcolor;
	uint16_t	unsel_fontcolor;

	char 	sel_size;

} HYLIST_T;

typedef struct{
	uint16_t x0;
	uint16_t y0;
	uint16_t width;
	uint16_t height;
	char cur_enum;
	const uint32_t *addr;
} ENUM_IMAGE_T;

typedef struct {
	HYLIST_T list;
	ENUM_IMAGE_T enum_image;
	FONT_T font;
}ENUM_IMAGE_LIST_T;

typedef struct {
	uint32_t menu_flag[1];
	uint32_t main_flag[1];
	uint32_t list_flag[1];
}GUI_FLAG_T;



typedef struct{
	char action;
	char attr;
	uint32_t *el_ptr;
	uint32_t el_max;
	uint32_t *option_ptr;
	uint32_t option_max;
	uint32_t option_step;
	uint32_t id;
	void (*func[])(void);

}GUI_EVENT_T;

typedef struct{

	char *buff;
	void (*write_one_pixel)(void);
	void (*write_block_pure_color)( uint32_t Xstart,uint32_t width,uint32_t Ystart,uint32_t height , uint16_t color );
	void (*write_block_pixel)( uint32_t Xstart,uint32_t width,uint32_t Ystart,uint32_t height ,const char *pdata );
	void (*write_block_pixel_from)( uint32_t Xstart,uint32_t width,uint32_t Ystart,uint32_t height , uint32_t img_addr );
	void (*vertical_scrolling)(void);

	GUI_MENU_T	menu;
	GUI_MAIN_T 	main;
	GUI_FLAG_T 	flag;
	GUI_EVENT_T	event;

}HYGUI_T;



void clock_icon_init( HYGUI_T *hygui , CLOCK_ICON_T *clock_icon );
void icon_update( HYGUI_T *hygui , ENUM_ICON_T *enum_icon );
void date_icon_update( HYGUI_T *hygui , DATE_ICON_T *date_icon );
void date_icon_init( HYGUI_T *hygui , DATE_ICON_T *date_icon );
void battery_icon_init( HYGUI_T *hygui , BATTERY_ICON_T *battery_icon );
void battery_icon_update( HYGUI_T *hygui , BATTERY_ICON_T *battery_icon );
void clock_icon_update( HYGUI_T *hygui , CLOCK_ICON_T *clock_icon );
void light_mode_list_init( HYGUI_T *hygui , ENUM_IMAGE_LIST_T *enum_list );
void light_mode_list_update( HYGUI_T *hygui , ENUM_IMAGE_LIST_T *enum_list );
void font_update( HYGUI_T *hygui , FONT_T font );

// #endif /* HYGUI_H */


