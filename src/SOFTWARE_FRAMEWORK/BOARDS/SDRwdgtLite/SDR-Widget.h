#ifndef _SDR_Widget_H_
#define _SDR_Widget_H_

#include "compiler.h"


/*! \name Peripherals to include at compile time. */
//! @{
#define LCD_DISPLAY         1   // Multi-line LCD display
#define SHAFT_ENCODER       1   // Shaft Encoder VFO function
#define Si570               1   // Si570 VXO control funcs
#define AK5394              1   // 24-bit ADC
#define AK4382A             0   // 24-bit DAC
#define POWER_SWR           0   // Measure, and if LCD, display Power and SWR.
                                // If not defined, while LCD is defined, then
                                // LCD displays Vdd and I-Pa
#define SWR_ALARM_FUNC      0   // SWR alarm function, activates a secondary PTT
                                // with auto Hi-SWR shutdown. Is dependent
                                // on POWER_SWR being defined as well
#define TMP100              1   // Temperature measurement device
#define AD7991              1   //
#define AD5301              1   //
#define PCF8574             1   // port expander control of TX/RX and Band Pass filters

#define DEBUG232            1   // Use the UART debug port
#define USB                 1   // Although it may look odd there may be a free standing mode.
#define LED                 1   // Flashy-Blinky lights
#define SPI                 0   // SPI driver
#define I2C                 1   // I2C driver
#define EXTERN_MEM          0   // Use MMC memory slot
#define MENU                0   // A menu system driven by the rotary encoder
//! @}

/*! \name Features to include at compile time. */
//! @{
// None, or only one of the two, CALC_FREQ_MUL_ADD or CALC_BAND_MUL_ADD should be selected
#define CALC_FREQ_MUL_ADD	0	// Frequency Subtract and Multiply Routines (for smart VFO)
								// normally not needed with Mobo 4.3.   *OR*
#define CALC_BAND_MUL_ADD	1	// Band dependent Frequency Subtract and Multiply Routines
								// (for smart VFO) normally not needed with Mobo 4.3.

#define BPF_LPF_Module		1	// Band Pass and Low Pass filter switcing

#define SCRAMBLED_FILTERS	0	// Enable a non contiguous order of filters

// None, or only one of the three below should be selected
#define	PCF_LPF				0	// External Port Expander Control of Low Pass filters
#define PCF_FILTER_IO		1	// 8x BCD control for LPF switching, switches P1 pins 4-6
#define M0RZF_FILTER_IO		0	// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6

#define FRQ_CGH_DURING_TX	1	// Allow Si570 Frequency change during TX
#define FLTR_CGH_DURING_TX	0	// Allow Filter changes when frequency is changed during TX
//! @}


#endif  // _SDR_Widget_H_
