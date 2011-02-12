/*
 * Mobo_config.h
 *
 *  Created on: 2010-03-23
 *      Author: TF3LJ
 */

#ifndef MOBO_CONFIG_H_
#define MOBO_CONFIG_H_

#include <stdint.h>

#include "Si570.h"
#include "PCF8574.h"
#include "AD5301.h"
#include "AD7991.h"
#include "TMP100.h"

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
#define COLDSTART_REF		0x02

// DEFS for PA BIAS selection and autocalibration
#define	BIAS_SELECT			1		// Which bias, 0 = Cal, 1 = AB, 2 = A
									// If BIAS_SELECT is set at 0, then the first operation
									// after any Reset will be to autocalibrate
#define	BIAS_LO				2		// PA Bias in 10 * mA, Class B ( 2 =  20mA)
#define	BIAS_HI				35		// PA Bias in 10 * mA, Class A  (35 = 350mA)
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

// DEFS for the Shaft Encoder VFO function
#if ENCODER_INT_STYLE						// Interrupt driven Shaft Encoder
// Useable external interrupt pins on the AT90USB162:
// INT0->PD0, INT1->PD1, INT2->PD2, INT3->PD3,
// INT5->PD4, INT6->PD6, INT7->PD7
// Interrupt Configuration Parameters
#define ENC_A_SIGNAL	INT6_vect			// Interrupt signal name: INTx, x = 0-7
#define ENC_A_IREG		EIMSK				// Interrupt register, always EIMSK
#define ENC_A_ICR		EICRB				// Interrupt Config Register
											// INT0-3: EICRA, INT4-7: EICRB
#define ENC_A_INT		(1 << INT6)			// matching INTx bit in EIMSK
#define ENC_A_ISCX0		(1 << ISC60)		// Interrupt Sense Config bit0 (ISCx0)
#define ENC_A_ISCX1		(1 << ISC61)		// Interrupt Sense Config bit1 (ISCx1)
#endif
//#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Common for both Interrupt and Scan style Encoder
// Configuration of the two input pins, Phase A and Phase B
// They can be set up to use any pin on two separate input ports
// Phase A connects to the pin tied to the Interrupt above
#define ENC_A_PORT		PORTD				// PhaseA port register
#define ENC_A_DDR		DDRD				// PhaseA port direction register
#define ENC_A_PORTIN	PIND				// PhaseA port input register
#define ENC_A_PIN		(1 << PD6)			// PhaseA port pin
#define ENC_B_PORT		PORTD				// PhaseB port register
#define ENC_B_DDR		DDRD				// PhaseB port direction register
#define ENC_B_PORTIN	PIND				// PhaseB port input register
#define ENC_B_PIN		(1 << PD5)			// PhaseB port pin

// Definitions for the Pushbutton Encoder functionality
#define ENC_PUSHB_PORT		PORTD
#define ENC_PUSHB_DDR		DDRD
#define	ENC_PUSHB_INPORT	PIND
#define	ENC_PUSHB_PIN		(1 << PD7)
#define ENC_PULSES			1024			// Number of resolvable Encoder States per revolution.
											// Note that the pulses per revolution parameter is not consistent
											// for all encoders.  For some Rotary Encoders "pulses per revolution"
											// indicates the number of pulses per revolution for each of the two
											// phase outputs, while for others, this parameter indicates the total
											// number of resolvable states.  In case of the former, the total number
											// of resolvable states is 4 times the specified pulses per revolution.
#define	ENC_INCREMENTS		1000/(.11920929 * ENC_PULSES) // One kHz per revolution of Encoder
											// Frequency increments in units of
											// ~0.11920929 Hz (or 8 ~ 1 Hz)
#define ENC_PUSHB_MIN		1				// Min pushdown for valid push (x 10ms)
#define	ENC_PUSHB_MAX		10				// Min pushdown for memory save (x 10ms)
#define ENC_STORE_DISP		20				// Time to display "Memory Stored" on LCD (x 100ms)
//#endif

#if ENCODER_FAST_ENABLE						// Variable speed Rotary Encoder feature
// Definitions for Variable Speed Rotary Encoder function
// The below parameters are hopefully more or less generic for any encoder, down to 32 pulses or so,
// but are optimized for a 1024 state Rotary Encoder
#define ENC_FAST_SENSE		96000/ENC_PULSES// Maximum time threshold (in steps of 1/65536 s) per click to enable fast mode
#define ENC_FAST_TRIG		ENC_PULSES/5	// Number of fast clicks to enable fast Mode
											// Make sure this does not exceed uint8_t
#define ENC_FAST_MULTIPLY	100				// Encoder click multiplier during FAST mode (max 128)
#define ENC_FAST_PATIENCE	5				// Time in 1/10th of seconds.  Time to revert to
											// normal encoder mode if no encoder activity
#endif



//
//-----------------------------------------------------------------------------
// Miscellaneous software defines, functions and variables
//-----------------------------------------------------------------------------
//

// CW input bits, defines which bit goes where in the USB reply to a CW pin poll
#define REG_CWSHORT 		(1 << 5)		// Bits used for CW in reg
#define REG_CWLONG  		(1 << 1)

#if							I2C_TX_FILTER_IO
#define TXF					16
#elif						PCF_FILTER_IO
#define TXF					8
#elif						M0RZF_FILTER_IO
#define TXF					4
#endif


// Various flags, may be moved around
extern bool	FRQ_fromusb;					// New frequency from USB
extern bool	FRQ_fromenc;					// New frequency from Encoder
extern bool	TX_state;						// Keep tabs on current TX status
extern bool	TX_flag;						// Request for TX to be set
extern bool	SWR_alarm;						// SWR alarm condition
extern bool	TMP_alarm;						// Temperature alarm condition
extern bool	PA_cal_lo;						// Used by PA Bias auto adjust routine
extern bool	PA_cal_hi;						// Used by PA Bias auto adjust routine
extern bool	PA_cal;							// Indicates PA Bias auto adjust in progress
extern bool	ENC_stored;						// Shaft Enc pushbutton status flag "STORED"
extern bool	ENC_changed;					// Encoder Changed flag (used with variable rate enc)
extern bool	ENC_fast;						// Encoder FAST mode enabled
extern bool	ENC_dir;						// Encoder Direction Change

#define	_2(x)		((uint32_t)1<<(x))		// Macro: Take power of 2

typedef struct
{
		bool		si570;					// Chip has been probed
		bool		tmp100;					// Chip has been probed
		bool		ad5301;					// Chip has been probed
		bool		ad7991;					// Chip has been probed
		bool		pcfmobo;				// Chip has been probed
} i2c_avail;

extern i2c_avail i2c;

typedef struct
{
		uint8_t		EEPROM_init_check;		// If value mismatch,
		uint8_t		Si570_I2C_addr;			// Si570 I2C addres, default 0x55 (85 dec)
		uint8_t		TMP100_I2C_addr;		// I2C address for the onboard TMP100 temperature sensor
		uint8_t		AD5301_I2C_addr;		// I2C address for the onboard AD5301 8 bit DAC
		uint8_t		AD7991_I2C_addr;		// I2C address for the onboard AD7991 4 x ADC
		uint8_t		PCF_I2C_Mobo_addr;		// I2C address for the onboard PCF8574
		uint8_t		PCF_I2C_lpf1_addr;		// I2C address for the first PCF8574 used in the MegaFilterMobo
		uint8_t		PCF_I2C_lpf2_addr;		// I2C address for the second PCF8574 used in the MegaFilterMobo
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
		#if PCF_FILTER_IO					// 8x BCD control for LPF switching, switches P1 pins 4-6
		uint16_t	TXFilterCrossOver[8];	// 8x Cross Over points for TX Low Pass Filter
		#elif M0RZF_FILTER_IO					// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6
		uint16_t	TXFilterCrossOver[4];	// 4x Cross Over points for TX Low Pass Filter
		#else
		uint16_t	TXFilterCrossOver[16];	// 16x Cross Over points for TX Low Pass Filter
		#endif
		uint8_t		PWR_fullscale;			// Full Scale setting for Power Output Bargraph
		uint8_t		SWR_fullscale;			// Full Scale setting for SWR Bargraph
		uint8_t		PEP_samples;			// Number of samples in PEP measurement

		int32_t		Encoder_Resolution;		// Frequency increments in units of ~0.12 Hz (or 8 ~ 1 Hz)
											// Display a fixed frequency offset during RX only.
		int32_t		LCD_RX_Offset;			// Freq add/subtract value is 0.0MHz (11.21bits)
											// signed integer, 0.000 MHz * 4.0 * _2(21)
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
