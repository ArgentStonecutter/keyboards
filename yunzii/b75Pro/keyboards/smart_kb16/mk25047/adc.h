#pragma once

void adc_init(void);
void adc_channel_set(ADC_TypeDef *adcx, uint8_t ch, uint8_t stime);
uint32_t adc_get_result(uint8_t ch);
uint32_t adc_get_result_average(uint8_t ch, uint8_t times);
uint16_t get_adc_value(void);
uint16_t get_adc_vref(void);
void battery_calibration(void);
uint8_t batt_level(void);