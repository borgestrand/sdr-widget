/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * Mobo_config.h
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef MOBO_CONFIG_H_
#define MOBO_CONFIG_H_

#include <stdint.h>


#include "Si570.h"
#include "PCF8574.h"
#include "AD5301.h"
#include "AD7991.h"
#include "TMP100.h"
#include "rotary_encoder.h"


// Hardware control functions

// Low-power sleep for a number of milliseconds by means of RTC
// Use only during init, before any MCU hardware (including application use of RTC) is enabled.
void mobo_sleep_rtc_ms(uint16_t time_ms);

// Audio Widget select oscillator
void mobo_xo_select(U32 frequency, uint8_t source);

// Master clock to DAC's I2S port frequency setup
void mobo_clock_division(U32 frequency);


// Empty the contents of the incoming pdca buffers
void mobo_clear_adc_channel(void);

// Empty the contents of the outgoing pdca buffers
void mobo_clear_dac_channel(void);


// Commands to and from CPU
#ifdef USB_REDUCED_DEBUG
#define CPU_CHAR_BOOT				'H'	// MCU boots up
#define CPU_CHAR_IDLE				'0'	// MCU reports no input
#define CPU_CHAR_SPDIF0				's'	// MCU reports that SPDIF0 is input
#define CPU_CHAR_SPDIF1				'S'	// MCU reports that SPDIF1 is input
#define CPU_CHAR_TOSLINK0			't'	// MCU reports that TOSLINK0 is input
#define CPU_CHAR_TOSLINK1			'T'	// MCU reports that TOSLINK0 is input
#define CPU_CHAR_UAC1_B				'b'	// MCU reports that Rear USB-B USB Audio Class 1 is input
#define CPU_CHAR_UAC2_B				'B'	// MCU reports that Rear USB-B USB Audio Class 2 is input
#define CPU_CHAR_UAC1_C				'c'	// MCU reports that Front USB-C USB Audio Class 1 is input
#define CPU_CHAR_UAC2_C				'C'	// MCU reports that Front USB-C USB Audio Class 2 is input
#define CPU_CHAR_SRC_DEF			'y' // MCU reports that Input is not known
#define CPU_CHAR_44					'1' // MCU requests oscillator for 44.1ksps and outputs on that rate
#define CPU_CHAR_48					'2' // MCU requests oscillator for 48ksps and outputs on that rate
#define CPU_CHAR_88					'3' // MCU requests oscillator for 88.2ksps and outputs on that rate
#define CPU_CHAR_96					'4' // MCU requests oscillator for 96ksps and outputs on that rate
#define CPU_CHAR_176				'5' // MCU requests oscillator for 176.4ksps and outputs on that rate
#define CPU_CHAR_192				'6' // MCU requests oscillator for 192ksps and outputs on that rate
#define CPU_CHAR_REGEN				'X' // MCU requests regenerated clock from SPDIF/TOSLINK receiver. Probably outputs on last reported rate ('1'-'6')
#define CPU_CHAR_RATE_DEF			'Y' // MCU rate request is unknown
#define MCU_CHAR_SI_ENABLE			'I' // CPU instructs MCU to enable sample skip/insert on SPDIF/TOSLINK reception. NOT IMPLEMENTED!
#define MCU_CHAR_SI_DISABLE			'i' // CPU instructs MCU to disable sample skip/insert on SPDIF/TOSLINK reception. NOT IMPLEMENTED!
#define MCU_CHAR_RESET				'R'	// CPU resets MCU over UART
#endif

#ifdef HW_GEN_RXMOD
// USB multiplexer definitions
#define USB_DATA_ENABLE_PIN_INV		AVR32_PIN_PA31		// USB_OE0, inverted MUX output enable pin
#define USB_DATA_C0_B1_PIN			AVR32_PIN_PA02		// USB_C0_B1, MUX address control
#define USB_VBUS_C_PIN				AVR32_PIN_PB06		// Active high enable A's VBUS
#define USB_VBUS_B_PIN				AVR32_PIN_PA28		// Active high enable B's VBUS
#define	MOBO_I2S_ENABLE				1
#define MOBO_I2S_DISABLE			0
// RXMODFIX IO is available on TP, currently not in use
/*
#define MOBO_HP_KM_ENABLE			1					// Enable power to headphones
#define MOBO_HP_KM_DISABLE			0					// Disable power to headphones

void mobo_km(uint8_t enable);
*/

// Control USB multiplexer in HW_GEN_RXMOD
void mobo_usb_select(uint8_t usb_ch);

// Quick and dirty detect of whether front USB (A) is plugged in. No debounce here!
uint8_t mobo_usb_detect(void);
#endif


#ifdef HW_GEN_AB1X
// Front panel RG LED control
void mobo_led(uint8_t fled);
#endif


#ifdef HW_GEN_RXMOD
// LED control
void mobo_led(uint8_t fled0);
#endif


#if (defined  HW_GEN_RXMOD)
// RXmod SPDIF mux control
void mobo_rxmod_input(uint8_t input_sel);
#endif

// Sample rate detection on ADC interface
uint32_t mobo_srd(void);
uint32_t mobo_srd_asm2(void);
uint32_t mobo_wait_LRCK_asm(void);

#ifdef HW_GEN_RXMOD
// Process spdif and toslink inputs
void mobo_handle_spdif(uint8_t width);

// Front panel RGB LED control
void mobo_led_select(U32 frequency, uint8_t source);

// I2S hard mute control
void  mobo_i2s_enable(uint8_t i2s_mode);

#endif // HW_GEN_RXMOD




//
//-----------------------------------------------------------------------------
// Implementation dependent definitions (user tweak stuff)
//-----------------------------------------------------------------------------
//

#define VERSION_MAJOR 16
#define VERSION_MINOR 100

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM (NVRAM)at first boot after
// an upgrade
#define COLDSTART_REF		0x0a

// DEFS for PA BIAS selection and autocalibration
#define	BIAS_SELECT			1		// Which bias, 0 = Cal, 1 = AB, 2 = A
									// If BIAS_SELECT is set at 0, then the first operation
									// after any Reset will be to autocalibrate
#define	BIAS_LO				2		// PA Bias in 10 * mA, Class B ( 2 =  20mA)
#define	BIAS_HI				35		// PA Bias in 10 * mA, Class A  (35 = 350mA)
#define	BIAS_MAX			100		// Max allowable PA Bias in 10 * mA, Class A  (100 = 1A)
#define	CAL_LO				0		// PA Bias setting, Class B
#define	CAL_HI				0		// PA Bias setting, Class A

// Defs for Power and SWR measurement (values adjustable through USB command 0x44)
#define SWR_TRIGGER			30				// Max SWR threshold (10 x SWR, 30 = 3.0)
#define SWR_PROTECT_TIMER	200				// Timer loop value in increments of 10ms
#define P_MIN_TRIGGER 		49				// Min P out in mW for SWR trigger
#define V_MIN_TRIGGER		0x20			// Min Vin in 1/4096 Full Scale, for valid SWR measurement
											// (SWR = 1.0 if lower values measured)
#define PWR_CALIBRATE		1000			// Power meter calibration value
#define PEP_MAX_PERIOD		20				// Max Time period for PEP measurement (in 100ms)
#define PEP_PERIOD			10				// Time period for PEP measurement (in 100ms)
											// PEP_PERIOD is used with PWR_PEAK_ENVELOPE func
											// under LCD display.

#define PWR_FULL_SCALE		4				// Bargraph Power fullscale in Watts
#define SWR_FULL_SCALE		4				// Bargraph SWR fullscale: Max SWR = Value + 1 (4 = 5.0:1)

#define	HI_TMP_TRIGGER		55				// If measured PA temperature goes above this point
											// then disable transmission. Value is in deg C
											// even if the LCD is set to display temp in deg F

#define RTC_COUNTER_MAX		1150000			// Max count for the 115kHz Real Time Counter (10 seconds)
#define RTC_COUNTER_FREQ	115000			// Nominal frequency of Real Time Counter in Hz



//
//-----------------------------------------------------------------------------
// Miscellaneous software defines, functions and variables
//-----------------------------------------------------------------------------
//

// CW input bits, defines which bit goes where in the USB reply to a CW pin poll
#define REG_CWSHORT 		(1 << 5)		// Bits used for CW in reg
#define REG_CWLONG  		(1 << 1)
#define REG_PTT_1			(1 << 2)
#define REG_PTT_2			(1 << 3)
#define REG_PTT_3			(1 << 4)
#define REG_TX_state		(1 << 6)
#define REG_PTT_INPUT		(1 << 7)

// Conditional def based on the above, do not touch:
#if PCF_LPF
#define TXF  8
#elif PCF_16LPF
#define TXF  16
#elif PCF_FILTER_IO
#define TXF  8
#elif M0RZF_FILTER_IO
#define TXF  4
#else
#define TXF  16
#endif

// Various flags, may be moved around
extern volatile bool MENU_mode;				// LCD Menu mode.  Owned by taskPushButtonMenu, used by all LCD users
extern bool	TX_state;						// Keep tabs on current TX status
extern bool	TX_flag;						// Request for TX to be set
extern bool	SWR_alarm;						// SWR alarm condition
extern bool	TMP_alarm;						// Temperature alarm condition
extern bool	PA_cal_lo;						// Used by PA Bias auto adjust routine
extern bool	PA_cal_hi;						// Used by PA Bias auto adjust routine
extern bool	PA_cal;							// Indicates PA Bias auto adjust in progress

#define	_2(x)		((uint32_t)1<<(x))		// Macro: Take power of 2

typedef struct
{
		bool		si570;					// Chip has been probed
		bool		tmp100;					// Chip has been probed
		bool		ad5301;					// Chip has been probed
		bool		ad7991;					// Chip has been probed
		bool		pcfmobo;				// Chip has been probed
		bool		pcflpf1;				// Chip has been probed
		bool		pcflpf2;				// Chip has been probed
		bool		pcfext;					// Chip has been probed
		bool		pcf0x20;				// Chip has been probed (all possible PCF8574 addresses)
		bool		pcf0x21;				// Chip has been probed
		bool		pcf0x22;				// Chip has been probed
		bool		pcf0x23;				// Chip has been probed
		bool		pcf0x24;				// Chip has been probed
		bool		pcf0x25;				// Chip has been probed
		bool		pcf0x26;				// Chip has been probed
		bool		pcf0x27;				// Chip has been probed
		bool		pcf0x38;				// Chip has been probed
		bool		pcf0x39;				// Chip has been probed
		bool		pcf0x3a;				// Chip has been probed
		bool		pcf0x3b;				// Chip has been probed
		bool		pcf0x3c;				// Chip has been probed
		bool		pcf0x3d;				// Chip has been probed
		bool		pcf0x3e;				// Chip has been probed
		bool		pcf0x3f;				// Chip has been probed
} i2c_avail;

extern i2c_avail i2c;

typedef struct
{
		uint8_t		EEPROM_init_check;		// If value mismatch,
		uint8_t		UAC2_Audio;				// UAC1 if FALSE, UAC2 if TRUE
		uint8_t		Si570_I2C_addr;			// Si570 I2C addres, default 0x55 (85 dec)
		uint8_t		TMP100_I2C_addr;		// I2C address for the onboard TMP100 temperature sensor
		uint8_t		AD5301_I2C_addr;		// I2C address for the onboard AD5301 8 bit DAC
		uint8_t		AD7991_I2C_addr;		// I2C address for the onboard AD7991 4 x ADC
		uint8_t		PCF_I2C_Mobo_addr;		// I2C address for the onboard PCF8574
		uint8_t		PCF_I2C_lpf1_addr;		// I2C address for the first PCF8574 used in the MegaFilterMobo
		uint8_t		PCF_I2C_lpf2_addr;		// I2C address for the second PCF8574 used in the MegaFilterMobo
		uint8_t		PCF_I2C_Ext_addr;		// I2C address for an external PCF8574 used for FAN, attenuators etc
		uint8_t		hi_tmp_trigger;			// If PA temperature goes above this point, then
											// disable transmission
		uint16_t	P_Min_Trigger;			// Min P out measurement for SWR trigger
		uint16_t	SWR_Protect_Timer;		// Timer loop value
		uint16_t	SWR_Trigger;			// Max SWR threshold
		uint16_t	PWR_Calibrate;			// Power meter calibration value
		uint8_t		Bias_Select;			// Which bias, 0 = Cal, 1 = LO, 2 = HI
		uint8_t		Bias_LO;				// PA Bias in 10 * mA, typically  20mA or Class B
		uint8_t		Bias_HI;				// PA Bias in 10 * mA, typically 350mA or Class A
		uint8_t		cal_LO;					// PA Bias setting, LO
		uint8_t		cal_HI;					// PA Bias setting, HI
		uint32_t	FreqXtal;				// crystal frequency[MHz] (8.24bits)
		uint16_t	SmoothTunePPM;			// Max PPM value for the smooth tune
		uint32_t	Freq[10];				// Running frequency[MHz] (11.21bits)
											// The first one is the running frequency
											// the next nine are memory stores, used with
											// the Shaft Encoder function, one for each
											// band, 1.8, 3.5, 7,... 28 MHz
		uint8_t		SwitchFreq;				// Which freq is in use (used with ShaftEncoder)
		uint16_t	FilterCrossOver[8];		// 8x Cross Over points for Band Pass Filter (11.5bits)
		uint16_t	TXFilterCrossOver[TXF];	// TXF (4, 8 or 16) x Cross Over points for TX Low Pass Filter
		uint8_t		PWR_fullscale;			// Full Scale setting for Power Output Bargraph
		uint8_t		SWR_fullscale;			// Full Scale setting for SWR Bargraph
		uint8_t		PEP_samples;			// Number of samples in PEP measurement
		uint16_t	Resolvable_States;		// Number of Encoder Resolvable States per Revolution
		uint8_t		VFO_resolution;			// VFO Resolution 1/2/5/10/50/100kHz per revolution
		int8_t		LCD_RX_Offset;			// Freq add/subtract value is in kHz
											// signed integer, 0.000 MHz * 4.0 * _2(21)
		uint8_t		Fan_On;					// Fan On trigger temp
		uint8_t		Fan_Off;				// Fan Off trigger temp
		uint8_t		PCF_fan_bit;			// Which bit is used to control the Cooling Fan
		#if SCRAMBLED_FILTERS				// Enable a non contiguous order of filters
		uint8_t		FilterNumber[8];		// Which Band Pass filter to select at each crossover
		uint8_t		TXFilterNumber[16];		// Which TX Low Pass filter to select at each crossover
		#endif
		#if CALC_FREQ_MUL_ADD				// Frequency Subtract and Multiply Routines
		uint32_t	FreqSub;				// Freq subtract value[MHz] (11.21bits)
		uint32_t	FreqMul;				// Freq multiply value (11.21bits)
		#endif
		#if CALC_BAND_MUL_ADD				// Band dependent Frequency Subtract and Multiply
		uint32_t	BandSub[8];				// Freq Subtract values [MHz] (11.21bits) for each of
											// the 8 (BPF) Bands
		uint32_t	BandMul[8];				// Freq Multiply values [MHz] (11.21bits) for each of
											// the 8 (BPF) Bands
		#endif
} mobo_data_t;

extern mobo_data_t	cdata;					// Variables in ram/flash rom (default)
extern mobo_data_t	nvram_cdata;			// Contain a number of default config parameters

#endif /* MOBO_CONFIG_H_ */
