#ifndef _SDR_Widget_H_
#define _SDR_Widget_H_

 /*
 * Additions and Modifications to ATMEL AVR32-SoftwareFramework-AT32UC3 are:
 *
 * Copyright (C) Alex Lee
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "compiler.h"

#define FIRMWARE_VERSION	"V:UAC1_087"
// Mobo changes since V:UAC1_069:
//   taskPowerDisplay.c... change calculated pwr value from mW to cW
//   taskMoboControl.c...  change calculated pwr value from mW to cW
//   taskMoboControl.h...  change calculated pwr value from mW to cW
//   taskPushButtonMenu.c...

/*! \name Peripherals to include at compile time. */
//! @{
#define I2C                 1	// I2C driver
#define LCD_DISPLAY         1   // 20x4 Liquid Crystal Display (LCD)
#define SHAFT_ENCODER       1   // Shaft Encoder VFO function
#define Si570               1   // Si570 VXO control funcs (needs I2C driver)
#define AK5394              1   // 24-bit ADC
#define AK4382A             0   // 24-bit DAC
#define TMP100              1   // Temperature measurement device  (needs I2C driver)
#define AD7991              1   //  (needs I2C driver)
#define AD5301              1   //  (needs I2C driver)
#define PCF8574             1   // port expander control of TX/RX and Band Pass filters  (needs I2C driver)

#define DEBUG232            1   // Use the UART debug port
#define USB                 1   // Although it may look odd there may be a free standing mode.
#define LED                 1   // Flashy-Blinky lights
#define SPI                 0   // SPI driver
#define EXTERN_MEM          0   // Use MMC memory slot
#define MENU                0   // A menu system driven by the rotary encoder
//! @}

/*! \name Features to include at compile time. */
//! @{
#define MOBO_FUNCTIONS		1	// AD7991/AD5301/TMP100, P/SWR etc...  (needs I2C driver)
								// Without this, we have a simple Si570 control
// None, or only one of the two, CALC_FREQ_MUL_ADD or CALC_BAND_MUL_ADD should be selected
#define CALC_FREQ_MUL_ADD	0	// Frequency Subtract and Multiply Routines (for smart VFO)
								// normally not needed with Mobo 4.3.   *OR*
#define CALC_BAND_MUL_ADD	1	// Band dependent Frequency Subtract and Multiply Routines
								// (for smart VFO) normally not needed with Mobo 4.3.

#define BPF_LPF_Module		1	// Band Pass and Low Pass filter switcing

#define SCRAMBLED_FILTERS	1	// Enable a non contiguous order of filters

// Low Pass Filters for Transmit ----------------------------------------------------------
#define TX_FILTERS			1	// Enable TX filter selection, including one of the four options below
// Only one of the four below should be selected, if TX_FILTERS is selected
#define	PCF_LPF				1	// External Port Expander Control of Low Pass filters
#define	PCF_16LPF			0	// External Port Expander Control of 16 Low Pass filters
#define PCF_FILTER_IO		0	// 8x BCD control for LPF switching, switches P1 pins 4-6
#define M0RZF_FILTER_IO		0	// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6

#define FRQ_CGH_DURING_TX	1	// Allow Si570 Frequency change during TX
#define FLTR_CGH_DURING_TX	0	// Allow Filter changes when frequency is changed during TX

// Transmit specific features --------------------------------------------------------------
#define POWER_SWR           1   // Measure, and if LCD, display Power and SWR.
                                // If not defined, while LCD is defined, then
                                // LCD displays Vdd and I-Pa
#define SWR_ALARM_FUNC      1   // SWR alarm function, activates a secondary PTT
                                // with auto Hi-SWR shutdown. Is dependent
                                // on POWER_SWR being defined as well
#define REVERSE_PTT2_LOGIC	0	// Reverse the logic of the PTT2 signal pin

#define	FAN_CONTROL			1	// Turn PA Cooling FAN On/Off, based on temperature
// Only one of the two below is selected with the FAN Control
#define	BUILTIN_PCF_FAN		0	// This alternative uses a pin on the builtin PCF8574
								// pin is definable by Cmd 64 index 3, normally header P1, pin 5
#define	EXTERN_PCF_FAN		1	// This alternative uses a pin on an external PCF8574
								// pin is definable by Cmd 64 index 3

// Tests and Debug ------------------------------------------------------------------------
#define FRQ_IN_FIRST_LINE	1	// Normal Frequency display in first line of LCD. Can be disabled for Debug
#define TMP_V_I_SECOND_LINE	1	// Normal Temp/Voltage/Current disp in second line of LCD, Disable for Debug
#define DISP_RX_DB			1	// Display RX level in dB for both input channels, third line (taskPowerDisplay)
#define DISP_RX_DB_HPF		0	// Display RX level in dB for both input channels, simple HPF, fourth line (taskPowerDisplay)
#define ENOB_TEST			1	// Sample A/D input channels for ENOB test, fourth line (taskPowerDisplay)
#define	I2C_LCD_PRINT		0	// Show which values are being sent to the PCF and Si570
#define LCD_CDC				0	// CDC traffic displayed on LCD

//! @}


#endif  // _SDR_Widget_H_
