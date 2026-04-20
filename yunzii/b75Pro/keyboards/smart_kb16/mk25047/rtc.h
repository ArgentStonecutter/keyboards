
#pragma once
void    exti_init_start(void);
void    rcu_backup_clock_Enable(void);
void    rcu_power_clock_Enable(void);
void    backup_write_Enable(void);
void    bkp_reset(void);
void    rcu_inlowclock_on(void);
void    wait_inlowclock_sta(void);
void    rcu_clock_source(void);
void    RCU_Clock_enable(void);
void    rtc_register_sync_wait(void);
void    rtc_lastoper_finish(void);
void    rtc_clock_Enable(void);
void    config_time(uint32_t alarm_count);
void    rtc_ainterrupt_enable(void);
void    rtc_prescaler_set(void);
void    config_time_alarm(void);
void    exti_stop_config(void);
uint8_t Rtc_Config_Api(void);
