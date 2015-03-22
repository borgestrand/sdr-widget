#include "gpio.h"

int foo(void) {
    int a;

    if (gpio_get_pin_value(AVR32_PIN_PB10) == 0)
        a = 0;
    else
        a = 1;
    
    return a;
}