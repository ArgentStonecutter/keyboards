#include "mk25047.h"
#include "adc.h"
#include "battery.h"
#include "quantum.h"

#define CHARGE_STATUS_CHANGE_CNT 50 /* 100ms*50=5s */
enum batt_charge_status_t batt_charge_status = batt_no_charge;
bool                      battery_first_test_flag   = false;    //一旦重新上电就会重新检测一次adc的标志位
bool                      battery_level_report_flag = false;    /*电池待更新标志*/
uint8_t                   battery_level;                        //电池电量百分比
uint8_t                   temp      = 0;                        /*电量百分比*/
uint8_t                   last_temp = 0;                        /*记忆上次电量百分比*/
uint8_t                   temp_change_count;
uint16_t                  adc_value;                            // B1引脚测量出来的adc值
uint16_t                  adc_vref;                             // vref引脚测量出来的adc值
uint16_t                  battery_value;                        //电池电量测量出来的adc值
uint32_t                  battery_test_time;                    //每30s更新一次adc值计时变量

/*电池充电指示位*/
void battery_charge_status(void) {
    static uint8_t batt_charge_cnt    = CHARGE_STATUS_CHANGE_CNT;
    static uint8_t batt_full_cnt      = CHARGE_STATUS_CHANGE_CNT;
    static uint8_t batt_no_charge_cnt = 0;
    if (get_plug_mode() == true) //连接电源, 充电指示, 0未充电, 1充电中, 2充满
    {
        batt_no_charge_cnt = 0;
        if (batt_charge_status != batt_no_charge && !readPin(B1) && battery_value <= 900) //充满
        {
            batt_charge_cnt = 0;
            if (batt_full_cnt < CHARGE_STATUS_CHANGE_CNT) //连续检测到CHARGE_STATUS_CHANGE_CNT次充满才切换状态
                batt_full_cnt++;
            else
                batt_charge_status = batt_charge_full;
        } else //充电中
        {
            batt_full_cnt = 0;
            if (batt_charge_cnt < CHARGE_STATUS_CHANGE_CNT) //连续检测到CHARGE_STATUS_CHANGE_CNT次充电中才切换状态
                batt_charge_cnt++;
            else
                batt_charge_status = batt_is_charging;
        }
    } else {
        if (batt_no_charge_cnt < 2) //连续检测到2次未充电才切换为未充电状态(解决USB拔掉后充电指示灯会闪几次的问题)
        {
            batt_no_charge_cnt++;
            return;
        }
        batt_charge_status = batt_no_charge;
        batt_full_cnt      = CHARGE_STATUS_CHANGE_CNT;
        batt_charge_cnt    = CHARGE_STATUS_CHANGE_CNT;
    }
}


/*电池电量百分比*/
uint8_t batt_level(void) {
    if (battery_value > BATT_99 || (battery_value < 900 && get_plug_mode() == true)) {
        battery_level = 100;
    }
    /*0~5%*/
    else if (battery_value > BATT_OFF && battery_value <= BATT_5) {
        battery_level = ((battery_value - BATT_OFF) * 5) / (BATT_5 - BATT_OFF);
    }
    /*5%~10%*/
    else if (battery_value > BATT_5 && battery_value <= BATT_10) {
        battery_level = ((battery_value - BATT_5) * 5) / (BATT_10 - BATT_5) + 5;
    }
    /*10%~40%*/
    else if (battery_value > BATT_10 && battery_value <= BATT_40) {
        battery_level = ((battery_value - BATT_10) * 30) / (BATT_40 - BATT_10) + 10;
    }
    /*40%~60%*/
    else if (battery_value > BATT_40 && battery_value <= BATT_60) {
        battery_level = ((battery_value - BATT_40) * 20) / (BATT_60 - BATT_40) + 40;
    }
    /*60%~80%*/
    else if (battery_value > BATT_60 && battery_value <= BATT_80) {
        battery_level = ((battery_value - BATT_60) * 20) / (BATT_80 - BATT_60) + 60;
    }
    /*80%~85%*/
    else if (battery_value > BATT_80 && battery_value <= BATT_85) {
        battery_level = ((battery_value - BATT_80) * 5) / (BATT_85 - BATT_80) + 80;
    }
    /*85%~99%*/
    else if (battery_value > BATT_85 && battery_value <= BATT_99) {
        battery_level = ((battery_value - BATT_85) * 14) / (BATT_99 - BATT_85) + 85;
    } else if (battery_value < BATT_OFF) {
        battery_level = 0;
    }
    return battery_level;
}
/*电池电量校准*/
void battery_calibration(void) {
    if (batt_charge_status == batt_is_charging) {
        if (temp <= last_temp) {
            temp_change_count = 0;
            temp              = last_temp;
        } else if (temp > last_temp) {
            temp_change_count++;
            if (temp_change_count >= 3) // 30s才加1
            {
                temp_change_count = 0;
                last_temp++;
                temp                      = last_temp;
                battery_level_report_flag = true;
            } else {
                temp = last_temp;
            }
        }
    } else if (batt_charge_status == batt_charge_full) {
        temp_change_count++;
        if (temp_change_count >= 2) {
            temp_change_count         = 0;
            temp                      = 100;
            last_temp                 = 100;
            battery_level_report_flag = true;
        }
    } else {
        if (temp >= last_temp) {
            temp_change_count = 0;
            temp              = last_temp;
        } else if (temp < last_temp) //跳变太快逐步减
        {
            temp_change_count++;
            if (temp_change_count >= 3) // 30s才减1
            {
                temp_change_count = 0;
                last_temp--;
                temp                      = last_temp;
                battery_level_report_flag = true;
            } else {
                temp = last_temp;
            }
        }
    }
}
/*电池电量的adc检测*/
void battery_level_test(void) {
    if (battery_first_test_flag == false) {
        adc_value                 = get_adc_value(); //获取adc值
        adc_vref                  = get_adc_vref();
        battery_value             = (adc_value * 1764 / adc_vref);
        temp                      = batt_level();
        last_temp                 = temp;
        battery_test_time         = timer_read32();
        battery_level_report_flag = true;
        battery_first_test_flag   = true;
    }
    if (timer_elapsed32(battery_test_time) > 10000) {
        adc_value     = get_adc_value(); //获取adc值
        adc_vref      = get_adc_vref();
        battery_value = (adc_value * 1764 / adc_vref);
        temp          = batt_level();
        battery_calibration();
        battery_test_time = timer_read32();
    }
}
