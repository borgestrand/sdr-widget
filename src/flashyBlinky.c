/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
#include "compiler.h"
#include "gpio.h"
#include "board.h"
//
#include "conf_isp.h"

#include "flashyBlinky.h"

/*! \name Pin Configuration
 */
//! @{
#define GPIO_PIN_EXAMPLE_1  LED1_GPIO
#define GPIO_PIN_EXAMPLE_2  LED0_GPIO

#define TWI_SCL  AVR32_PIN_PA14
#define TWI_SDA  AVR32_PIN_PA15

//#define GPIO_CW_KEY_1        AVR32_PIN_PB9
#define GPIO_CW_KEY_1        AVR32_PIN_PX00
#define GPIO_CW_KEY_2        AVR32_PIN_PX01
//#define GPIO_PTT_INPUT       AVR32_PIN_PX03

#define PTT_1				 AVR32_PIN_PX45
#define PTT_2				 AVR32_PIN_PX42
#define PTT_3				 AVR32_PIN_PX22

#if BOARD == EVK1104
  #define GPIO_PIN_EXAMPLE_3  GPIO_PUSH_BUTTON_SW2
#elif BOARD == SDRwdgt || BOARD == SDRwdgtLite
  #define GPIO_PIN_EXAMPLE_3  GPIO_PUSH_BUTTON_SW2
#endif

#if !defined(GPIO_PIN_EXAMPLE_1) || \
    !defined(GPIO_PIN_EXAMPLE_2) || \
    !defined(GPIO_PIN_EXAMPLE_3) || \
	!defined(ENCODER_SWITCH)
  #error The pin configuration to use in this example is missing.
#endif
//! @}



/*! \brief This is an example of how to access the gpio.c driver to set, clear, toggle... the pin GPIO_PIN_EXAMPLE.
 */
int flashyBlinky(void)
{

  U32 i;

  gpio_enable_pin_glitch_filter(GPIO_PIN_EXAMPLE_3);
  gpio_enable_pin_pull_up(GPIO_CW_KEY_1);
  gpio_enable_pin_pull_up(GPIO_CW_KEY_2);
  gpio_enable_pin_pull_up(GPIO_PTT_INPUT);
  gpio_enable_pin_pull_up(ENCODER_SWITCH);

  while (1)
  {

      // Note that it is also possible to use the GPIO toggle feature.
      gpio_tgl_gpio_pin(GPIO_PIN_EXAMPLE_1);


    // Poll push button value.
    for (i = 0; i < 200; i++)
    {
      if (gpio_get_pin_value(GPIO_PIN_EXAMPLE_3) == 0 ||
		  gpio_get_pin_value(GPIO_CW_KEY_1) == 0 ||
		  gpio_get_pin_value(GPIO_CW_KEY_2) == 0 ||
		  gpio_get_pin_value(GPIO_PTT_INPUT) == 0 ||
		  gpio_get_pin_value(ENCODER_SWITCH)
      ){
        gpio_clr_gpio_pin(GPIO_PIN_EXAMPLE_2);
		gpio_clr_gpio_pin(TWI_SCL);
		gpio_clr_gpio_pin(TWI_SDA);
		gpio_set_gpio_pin(PTT_1);
		gpio_set_gpio_pin(PTT_2);
		gpio_set_gpio_pin(PTT_3);
      }
      else {
        gpio_set_gpio_pin(GPIO_PIN_EXAMPLE_2);
		gpio_set_gpio_pin(TWI_SCL);
		gpio_set_gpio_pin(TWI_SDA);
		gpio_clr_gpio_pin(PTT_1);
		gpio_clr_gpio_pin(PTT_2);
		gpio_clr_gpio_pin(PTT_3);
        }
    }

  }
}
