#pragma once

enum batt_charge_status_t {
    batt_no_charge,
    batt_is_charging,
    batt_charge_full
};

// //电量百分比对应的数值
// #define  BATT_OFF  3200
// #define  BATT_5    3300
// #define  BATT_10   3470
// #define  BATT_40   3630
// #define  BATT_60   3760
// #define  BATT_80   3930
// #define  BATT_85   3980
// #define  BATT_99   4150

//电量百分比对应的数值
#define  BATT_OFF  3200
#define  BATT_5    3300
#define  BATT_10   3400
#define  BATT_40   3630
#define  BATT_60   3760
#define  BATT_80   3930
#define  BATT_85   3980
#define  BATT_99   4150

uint8_t batt_level(void);
void battery_level_test(void);
void battery_charge_status(void);
void battery_calibration(void);
void battery_level_test(void);
