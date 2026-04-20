#include "quantum.h"

#ifdef RGBLIGHT_ENABLE

void setleds_custom(rgb_led_t *led, uint16_t led_num) {
    
    //    sc_ws2812_setleds(led,led_num);

}

const rgblight_driver_t rgblight_driver = {
    .setleds = setleds_custom,
};

#endif  // RGBLIGHT_ENABLE