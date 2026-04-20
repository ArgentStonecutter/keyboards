#include "rtc.h"
#include "quantum.h"

#define RTC_LOW_BITS_MASK ((uint32_t)0x0000FFFFU)
static uint16_t count_flag;

void exti_init_start(void) {
    EXTI->IMR |= 1 << 17;
    EXTI->EMR &= ~(1 << 17);
    EXTI->FTSR &= ~(1 << 17);
    EXTI->RTSR |= 1 << 17;
}

//备份区域的时钟使能
void rcu_backup_clock_Enable(void) {
    RCC->APB1ENR |= RCC_APB1ENR_BKPEN;
}

//电源管理时钟使能
void rcu_power_clock_Enable(void) {
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
}

//备份区允许访问
void backup_write_Enable(void) {
    PWR->CR |= PWR_CR_DBP;
}

//备份域复位
void bkp_reset(void) {
    RCC->APB1RSTR |= RCC_APB1RSTR_BKPRST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_BKPRST;
}

//使能内部低速时钟
void rcu_inlowclock_on(void) {
    RCC->CSR |= 1 << 0;
}

//等待内部时钟稳定
void wait_inlowclock_sta(void) {
    //该处需增加超时机制
    while (!(RCC->CSR & RCC_BDCR_LSERDY))
        ;
}

// RTC时钟源选择
void rcu_clock_source(void) {
    RCC->BDCR |= RCC_BDCR_RTCSEL_1;
}

//使能RTC时钟
void RCU_Clock_enable(void) {
    RCC->BDCR |= RCC_BDCR_RTCEN;
}

//等待寄存器域APB1时钟同步
void rtc_register_sync_wait(void) {
    RTC->CRL &= ~RTC_CRL_RSF;
    while (!(RTC->CRL & RTC_CRL_RSF))
        ;
}

//等待最后一次操作完成
void rtc_lastoper_finish(void) {
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ;
}

//使能RTC中断
void rtc_clock_Enable(void) {
    RCC->BDCR |= RCC_BDCR_RTCEN;
}

//设置闹钟时间
void config_time(uint32_t alarm_count) {
    RTC->CRL |= RTC_CRL_CNF;
    RTC->ALRH = 0;
    RTC->ALRL = alarm_count & RTC_LOW_BITS_MASK;
    RTC->CRL &= ~RTC_CRL_CNF;
}

//使能RTC闹钟中断
void rtc_ainterrupt_enable(void) {
    RTC->CRH |= RTC_CRH_ALRIE;
}

//设置闹钟时间
//设置预分屏
void rtc_prescaler_set(void) {
    RTC->CRL |= RTC_CRL_CNF;
    RTC->PRLL = 0x9C3F;
    RTC->CRL &= ~RTC_CRL_CNF;
}

void config_time_alarm(void) {
    RTC->CRL |= RTC_CRL_CNF; // Configuration Flag
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ;

    rcu_backup_clock_Enable(); //备份区域的时钟使能
    rcu_power_clock_Enable();  //电源管理时钟使能
    backup_write_Enable();     //备份区允许访问
    RTC->CRL |= RTC_CRL_CNF;   // Configuration Flag
    RTC->CNTL = 0x00;
    RTC->CRL &= ~RTC_CRL_CNF;
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ; // RTC operation OFF

    RTC->CRL &= ~RTC_CRL_CNF;
    exti_init_start();
    nvicEnableVector(41, 0);
}

void exti_stop_config(void) {
    EXTI->IMR &= ~1 << 17;
    EXTI->EMR &= ~(1 << 17);
    EXTI->FTSR &= ~(1 << 17);
    EXTI->RTSR &= ~(1 << 17);

    RTC->CRL |= RTC_CRL_CNF;
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ;
    rcu_backup_clock_Enable(); //备份区域的时钟使能
    rcu_power_clock_Enable();  //电源管理时钟使能
    backup_write_Enable();     //备份区允许访问
    RTC->CRL |= RTC_CRL_CNF;   // Configuration Flag
    RTC->CNTL = 0x00;
    RTC->CRL &= ~RTC_CRL_CNF;
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ; // RTC operation OFF

    RTC->CRL &= ~RTC_CRL_CNF;
}

uint8_t Rtc_Config_Api(void) {
    rcu_backup_clock_Enable(); //备份区域的时钟使能
    rcu_power_clock_Enable();  //电源管理时钟使能
    backup_write_Enable();     //备份区允许访问

    rcu_inlowclock_on(); //使能内部低速时钟

    while (!(RCC->CSR & RCC_BDCR_LSERDY) && count_flag < 250) {
        count_flag++;
        wait_ms(10);
    }
    if (count_flag >= 250) {
        return 1;
    }

    rcu_clock_source();       // RTC时钟源选择
    RCU_Clock_enable();       //使能RTC时钟
    rtc_register_sync_wait(); //等待寄存器域APB1时钟同步

    RTC->CRH |= RTC_CRH_ALRIE;
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ; // RTC operation OFF

    RTC->CRL |= RTC_CRL_CNF; // Configuration Flag

    RTC->CRL |= RTC_CRL_CNF;
    RTC->PRLL = 0x9C3F;       // 0x9C3F;  //RTC Prescaler Reload Value Low
    RTC->CRL &= ~RTC_CRL_CNF; // clear reset Flag
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ; // RTC operation OFF

    rcu_backup_clock_Enable(); //备份区域的时钟使能
    rcu_power_clock_Enable();  //电源管理时钟使能
    backup_write_Enable();     //备份区允许访问
    RTC->CRL |= RTC_CRL_CNF;   // Configuration Flag
    RTC->CNTL = 0x00;
    RTC->CRL &= ~RTC_CRL_CNF;
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ; // RTC operation OFF

    rcu_backup_clock_Enable(); //备份区域的时钟使能
    rcu_power_clock_Enable();  //电源管理时钟使能
    backup_write_Enable();     //备份区允许访问
    config_time(0x5DC);        //设置闹钟时间
    while (!(RTC->CRL & RTC_CRL_RTOFF))
        ; // RTC operation OFF
    RTC->CRL &= ~RTC_CRL_CNF;
    exti_stop_config();
    return 0;
}
