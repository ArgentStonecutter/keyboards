MCU_LDSCRIPT = STM32F103xB

DEFERRED_EXEC_ENABLE = yes
UART_DRIVER_REQUIRED = yes
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE

SPI_DRIVER_REQUIRED = yes

SRC +=  analog.c \
		smart_ble.c \
		smart.c\
		adc.c\
		battery.c\
		rtc.c\
		rgblight_custom.c\
        uart_mod.c\
        keyboard_screen.c


