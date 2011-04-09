/*
 * DG8SAQ_cmd.c
 *
 *  Created on: 2010-06-13
 *
 *      Author: Loftur Jonasson, TF3LJ
 */


#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "wdt.h"
#include "flashc.h"
#include "gpio.h"

#include "DG8SAQ_cmd.h"
#include "taskLCD.h"
#include "Mobo_config.h"
#include "Si570.h"
#include "AD7991.h"
#include "TMP100.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "features.h"
#include "widget.h"

// This var is used to pass frequency from USB input command
volatile uint32_t freq_from_usb;		// New Frequency from USB

volatile bool	FRQ_fromusbreg = FALSE;	// Flag: New frequency by Register from USB
volatile bool	FRQ_fromusb = FALSE;	// Flag: New frequency from USB
volatile bool	FRQ_lcdupdate = FALSE;	// Flag: Update LCD frequency printout


/**
 @brief Process USB Host to Device transmissions.  No result is returned.
                 This function processes control of USB commands 0x30 - 0x35
*/
void dg8saqFunctionWrite(uint8_t type, uint16_t wValue, uint16_t wIndex, U8 *Buffer, uint8_t len)
{
	//uint64_t *Buf64;
	uint32_t *Buf32;
	uint16_t *Buf16;
	//Buf64 = (uint64_t*)Buffer;
	Buf32 = (uint32_t*)Buffer;
	Buf16 = (uint16_t*)Buffer;
	int x;

//	LED_Toggle(LED1);

	switch (type)
	{
	case 0x30:
		if (len == 6)
		{
			for (x = 0; x<6;x++)
			{
				si570reg[x] = Buffer[5-x];
			}
			// Calc the freq as a 32bit integer, based on the Si570 register value
			// Writing a non_zero value into freq_from_usb will result in a write to
			// Si570 by the taskMoboCtrl
			// Freq_FRom_Register uses the si570reg[6] as input
			FRQ_fromusbreg = TRUE;
		}
		break;
			#if CALC_FREQ_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
			case 0x31:								// Write the frequency subtract multiply to the eeprom
				if (len == 2*sizeof(uint32_t))
				{
					cdata.FreqSub = Buf32[1];
					cdata.FreqMul = Buf32[0];
					flashc_memset32((void *)&nvram_cdata.FreqSub, cdata.FreqSub, sizeof(uint32_t), TRUE);
					flashc_memset32((void *)&nvram_cdata.FreqMul, cdata.FreqMul, sizeof(uint32_t), TRUE);
				}
				break;
			#endif
			#if CALC_BAND_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
			case 0x31:								// Write the frequency subtract multiply to the eeprom
				if (len == 2*sizeof(uint32_t))
				{
					cdata.BandSub[wIndex & 0x0f] = Buf32[1];
					cdata.BandMul[wIndex & 0x0f] = Buf32[0];
					flashc_memset32((void *)&nvram_cdata.BandSub[wIndex & 0x0f], Buf32[1], sizeof(uint32_t), TRUE);
					flashc_memset32((void *)&nvram_cdata.BandMul[wIndex & 0x0f], Buf32[0], sizeof(uint32_t), TRUE);
				}
				break;
			#endif

			case 0x32:								// Set frequency by value and load Si570
				if (len == 4)
				{
					freq_from_usb = *Buf32;
					FRQ_fromusb = TRUE;
				}
				break;


			case 0x33:								// Write new crystal frequency to EEPROM and use it.
				if (len == 4) {
					cdata.FreqXtal = *Buf32;
					flashc_memset32((void *)&nvram_cdata.FreqXtal, *Buf32, sizeof(uint32_t), TRUE);
					freq_from_usb = cdata.Freq[0];  // Refresh frequency
					FRQ_fromusb = TRUE;
				}
				break;


			case 0x34:								// Write new startup frequency to eeprom
				if (len == 4) {
					if (wIndex < 10)			// Is it a "legal" memory location
					{
						flashc_memset32((void *)&nvram_cdata.Freq[wIndex], *Buf32, sizeof(uint32_t), TRUE);
						flashc_memset8((void *)&nvram_cdata.SwitchFreq, Buffer[0], sizeof(uint8_t), TRUE);
					}
				}
				break;


			case 0x35:								// Write new smooth tune to eeprom and use it.
				if (len == 2)
				{
					cdata.SmoothTunePPM = *Buf16;
					flashc_memset16((void *)&nvram_cdata.SmoothTunePPM, *Buf16, sizeof(uint16_t), TRUE);
				}
	}
}


/**
 @brief Process USB query commands and return a result (flexible size data payload)
		This function processes control of all USB commands except for 0x30 - 0x35
*/
uint8_t dg8saqFunctionSetup(uint8_t type, uint16_t wValue, uint16_t wIndex, U8* Buffer)
{
	int x;

	uint32_t *Buf32;
	uint16_t *Buf16;
	Buf32 = (uint32_t*)Buffer;
	Buf16 = (uint16_t*)Buffer;

//	LED_Toggle(LED1);

	switch (type)
	{
	case 0x00:								// Return software version number
		*Buf16 = (VERSION_MAJOR<<8)|(VERSION_MINOR);
		return sizeof(uint16_t);


	// Todo -- may all go:
	/***********************************
	#if LEGACY_PORT_CMD						// Legacy Commands 0x01 - 0x04. Normally not needed
	case 0x01:								// set port pin directions (PORTD)
		//
		// Here we may want to insert a #define-able protection for the GPIO style
		// Low Pass filter select signals.  A later project, if needed.
		//

		#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Shaft Encoder VFO function
											// Protect Shaft encoder bits
		IO_DDR_MP = rq ->wValue.b0 & ~(ENC_A_PIN | ENC_B_PIN | ENC_PUSHB_PIN);
		#else
		IO_DDR_MP = rq->wValue.b0;
		#endif
		break;

	case 0x02:								// read PORTD
		Buffer[0] = IO_PIN_MP;
		return sizeof(uint8_t);

	case 0x03:								// read port states, PORTD
		Buffer[0] = IO_PORT_MP;
		return sizeof(uint8_t);

	case 0x04:								// set outputs, PORTD
		//
		// Here we may want to insert a #define-able protection for the GPIO style
		// Low Pass filter select signals.  A later project, if needed.
		//
		#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Shaft Encoder VFO function
											// Protect Shaft encoder bits
		IO_PORT_MP = wValue | (ENC_A_PIN | ENC_B_PIN | ENC_PUSHB_PIN);
		#else
		IO_PORT_MP = wValue;
		#endif
		break;
	#endif//LEGACY_PORT_CMD
	***********************************/

	case 0x0f:								// Reboot Widget
		widget_reset();

	// Todo -- may go, unless we have GPIOs to work with
	/***************************************************
	case 0x15:								// Set IO port with mask and data bytes
		//
		// Here we may want to insert a #define-able protection for the GPIO style
		// Low Pass filter select signals.  A later project, if needed.
		//
		#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Shaft Encoder VFO function
											// Protect Shaft encoder bits
		IO_DDR_MP  = wValue & ~(ENC_A_PIN | ENC_B_PIN | ENC_PUSHB_PIN);
		IO_PORT_MP = wIndex |  (ENC_A_PIN | ENC_B_PIN | ENC_PUSHB_PIN);
		#else
		IO_DDR_MP  = wValue;
		IO_PORT_MP = wIndex;
		#endif
		// passthrough to case 0x16

	case 0x16:								// Read I/O bits
		*Buf16 = IO_PIN_MP;
    	return sizeof(uint16_t);
	****************************************************/

	case 0x17:								// Read and Write the Filter Cross over point's and use it.
		{
			// RX Filter cross over point table.
			if (wIndex < 0x100)
			{
				if (wIndex < 8)		// Make sure we don't overwrite other parts of table
				{
					cdata.FilterCrossOver[wIndex] = wValue;
					flashc_memset16((void *)&nvram_cdata.FilterCrossOver[wIndex], wValue, sizeof(uint16_t), TRUE);
				}
				for (x=0;x<8;x++)
				{
					Buf16[7-x] = cdata.FilterCrossOver[x];
				}
				return 8 * sizeof(uint16_t);
			}

			// TX Filter cross over point table.
			else
			{
				wIndex = wIndex & 0xff;	    // Remove high byte

				#if PCF_FILTER_IO			// 8x BCD TX filter control, switches P1 pins 4-6
				if (wIndex < 8)				// Make sure we don't overwrite other parts of table
				#elif M0RZF_FILTER_IO		// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6
				if (wIndex < 4)				// Make sure we don't overwrite other parts of table
				#else
				if (wIndex < 16)	// Make sure we don't overwrite other parts of table
				#endif
				{
					cdata.TXFilterCrossOver[wIndex] = wValue;
					flashc_memset16((void *)&nvram_cdata.TXFilterCrossOver[wIndex], wValue, sizeof(uint16_t), TRUE);
				}
				for (x=0;x<TXF;x++)
				{
					Buf16[(TXF-1)-x] = cdata.TXFilterCrossOver[x];
				}
				return TXF * sizeof(uint16_t);
			}
		}

	#if SCRAMBLED_FILTERS					// Enable a non contiguous order of Filters
	case 0x18:								// Set the Band Pass Filter Address for one band: 0,1,2...7
		cdata.FilterNumber[wIndex] = wValue;
		flashc_memset8((void *)&nvram_cdata.FilterNumber[wIndex], wValue, sizeof(uint8_t), TRUE);
		// passthrough to case 0x19

	case 0x19:								// Read the Band Pass Filter Addresses for bands 0,1,2...7
		for (x=0;x<8;x++)
		{
			Buffer[7-x] = cdata.FilterNumber[x];
		}
		return 8 * sizeof(uint8_t);

	case 0x1a:								// Set the Low Pass Filter Address for one band: 0,1,2...15
		cdata.TXFilterNumber[wIndex] = wValue;
		flashc_memset8((void *)&nvram_cdata.TXFilterNumber[wIndex], wValue, sizeof(uint8_t), TRUE);
		// passthrough to case 0x1b

	case 0x1b:								// Read the Low Pass Filter Addresses for bands 0,1,2...15
		for (x=0;x<TXF;x++)
		{
			Buffer[(TXF-1)-x] = cdata.TXFilterNumber[x];
		}
		return TXF * sizeof(uint8_t);
	#endif

	// Todo -- delete, most likely
	//case 0x20:								// [DEBUG] Write byte to Si570 register
	//	Si570CmdReg(rq->wValue.b1, rq->wIndex.b0);  // Value high byte and Index low byte
	//
	//	Status2 |= SI570_OFFL;				// Next SetFreq call no smoothtune
	//
	//	replyBuf[0].b0 = I2CErrors;			// return I2C transmission error status
	//  return sizeof(uint8_t);

	//case 0x30:							// Set frequnecy by register and load Si570
	//case 0x31:							// Write the FREQ mul & add to the eeprom
	//case 0x32:							// Set frequency by value and load Si570
	//case 0x33:							// write new crystal frequency to EEPROM and use it.
	//case 0x34:							// Write new startup frequency to eeprom
	//case 0x35:							// Write new smooth tune to eeprom and use it.
	//case 0x36:
	//case 0x37:
	//	return 0		;					// Hey we're not supposed to be here
											// 	we use usbFunctionWrite() to transfer data

	#if CALC_FREQ_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
	case 0x39:								// Return the current Subtract and Multiply values
		Buf32[1] = cdata.FreqSub;
		Buf32[0] = cdata.FreqMul;
		return 2*sizeof(uint32_t);
	#endif
	#if CALC_BAND_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
	case 0x39:								// Return the current Subtract and Multiply values
		Buf32[1] = cdata.BandSub[wIndex & 0x0f];
		Buf32[0] = cdata.BandMul[wIndex & 0x0f];
		return 2*sizeof(uint32_t);
	#endif

	case 0x3a:								// Return running frequnecy
			*Buf32 = cdata.Freq[0];
			return sizeof(uint32_t);

		case 0x3b:							// Return smooth tune ppm value
			*Buf16  = cdata.SmoothTunePPM;
        	return sizeof(uint16_t);

		case 0x3c:							// Return the startup frequency
			*Buf32 = nvram_cdata.Freq[wIndex];
			return sizeof(uint32_t);

		case 0x3d:							// Return the XTal frequnecy
			*Buf32  = cdata.FreqXtal;
			return sizeof(uint32_t);

		case 0x3f:							// Return the Si570 chip frequency control registers
			GetRegFromSi570(cdata.Si570_I2C_addr);
			for (x = 0; x<6;x++)
			{
				Buffer[5-x]= si570reg[x];
			}
			return 6*sizeof(uint8_t);


		case 0x41:		// Set a new i2c address for a device, or reset the EEPROM to factory default
						// if Value contains 0xff, then factory defaults will be loaded on reset.

						// Index 0 reads/modifies the Si570_I2C_addr
						// Index 1 reads/modifies the I2C address for the onboard PCF8574
						// Index 2 reads/modifies the I2C address for the first PCF8574 used in the LPF Mobo
						// Index 3 reads/modifies the I2C address for the second PCF8574 used in the LPF Mobo
						// Index 4 reads/modifies the I2C address for the onboard TMP100 temperature sensor
						// Index 5 reads/modifies the I2C address for the onboard AD5301 8 bit DAC
						// Index 6 reads/modifies the I2C address for the onboard AD7991 4 x ADC
						// Index 7 reads/modifies the I2C address for the external PCF8574 used for FAN, attenuators etc

			if (wValue == 0xff)
        	{
				widget_factory_reset();
			}

			if (wValue)							// If value field > 0, then update EEPROM settings
			{
				Buffer[0] = wValue;

				switch (wIndex)
				{
					case 0:
						flashc_memset8((void *)&nvram_cdata.Si570_I2C_addr, wValue, sizeof(uint8_t), TRUE);
						break;
					case 1:
						flashc_memset8((void *)&nvram_cdata.PCF_I2C_Mobo_addr, wValue, sizeof(uint8_t), TRUE);
						break;
					case 2:
						flashc_memset8((void *)&nvram_cdata.PCF_I2C_lpf1_addr, wValue, sizeof(uint8_t), TRUE);
						break;
					case 3:
						flashc_memset8((void *)&nvram_cdata.PCF_I2C_lpf2_addr, wValue, sizeof(uint8_t), TRUE);
						break;
					case 4:
						flashc_memset8((void *)&nvram_cdata.TMP100_I2C_addr, wValue, sizeof(uint8_t), TRUE);
						break;
					case 5:
						flashc_memset8((void *)&nvram_cdata.AD5301_I2C_addr, wValue, sizeof(uint8_t), TRUE);
						break;
					case 6:
						flashc_memset8((void *)&nvram_cdata.AD7991_I2C_addr, wValue, sizeof(uint8_t), TRUE);
						break;
					case 7:
						flashc_memset8((void *)&nvram_cdata.PCF_I2C_Ext_addr, wValue, sizeof(uint8_t), TRUE);
						break;
				}
			}
			else								// Else just read and return the current value
			{
				switch (wIndex)
				{
					case 0:
						Buffer[0] = cdata.Si570_I2C_addr;
						break;
					case 1:
						Buffer[0] = cdata.PCF_I2C_Mobo_addr;
						break;
					case 2:
						Buffer[0] = cdata.PCF_I2C_lpf1_addr;
						break;
					case 3:
						Buffer[0] = cdata.PCF_I2C_lpf2_addr;
						break;
					case 4:
						Buffer[0] = cdata.TMP100_I2C_addr;
						break;
					case 5:
						Buffer[0] = cdata.AD5301_I2C_addr;
						break;
					case 6:
						Buffer[0] = cdata.AD7991_I2C_addr;
						break;
					case 7:
						Buffer[0] = cdata.PCF_I2C_Ext_addr;
						break;
				}
			}
			return sizeof(uint8_t);


		// Todo
		//#if USB_SERIAL_ID						// A feature to change the last char of the USB Serial  number
		//case 0x43:							// Get/Set the USB SeialNumber ID
		//	replyBuf[0].b0 = R.SerialNumber;
		//	if (rq->wValue.b0 != 0) {			// Only set if Value != 0
		//		R.SerialNumber = rq->wValue.b0;
		//		eeprom_write_byte(&E.SerialNumber, R.SerialNumber);
		//	}
		//	return sizeof(R.SerialNumber);
		//#endif


		case 0x50:								//Set/Release PTT and get cw-key status
			if (wValue == 0)
			{
				// Clear PTT flag, ask for state change to RX
				TX_flag = FALSE;
			    FRQ_lcdupdate = TRUE;				// Force LCD update, Indicate new frequency for Si570
			}
			else
			{
				// Set PTT flag, ask for state change to TX
				TX_flag = TRUE;
			    FRQ_lcdupdate = TRUE;				// Force LCD update, Indicate new frequency for Si570
			}
			// Passthrough to Cmd 0x51

		case 0x51:								// read CW & PTT key levels
		case 0x52:
			Buffer[0] = 0x00;
			// read pin and set regbit accordingly
			if (gpio_get_pin_value(GPIO_CW_KEY_1)) Buffer[0] |= REG_CWSHORT;
			if (gpio_get_pin_value(GPIO_CW_KEY_2)) Buffer[0] |= REG_CWLONG;
			if (gpio_get_pin_value(GPIO_PTT_INPUT)) Buffer[0] |= REG_PTT_INPUT;
			if (gpio_get_pin_value(PTT_1)) Buffer[0] |= REG_PTT_1;
			if (gpio_get_pin_value(PTT_2)) Buffer[0] |= REG_PTT_2;
			if (gpio_get_pin_value(PTT_3)) Buffer[0] |= REG_PTT_3;
			if (TX_state) Buffer[0] |= REG_TX_state;

        	return sizeof(uint8_t);


		case 0x61:		// Read ADC inputs,
						// Index byte points to which ADC input to read.

						// Index 0 = PA current, full scale 0xfff0 = 2.5A
						// Index 1 = Power Output
						// Index 2 = Power reflected
						// Index 3 = Supply voltage, full scale 0xfff0 = 15.64V
						// Index 4 = Temperature in degC.Signed Int.  0 = 0 deg C
						// 			 32640 =  128 deg C, 32768 = -128 deg C

			if (wIndex < 4)						// Values from AD7991
			{
				*Buf16 = ad7991_adc[wIndex];
			}
			else								// Read current temperature
			{
				*Buf16 = tmp100_data;
			}
			return sizeof(uint16_t);


		case 0x64:		// Read/Modify the PA High Temperature limit
						// If wValue contains a value higher than 0,
						// then hi_tmp_trigger is updated with this value.
						// If 0, then read current value.

						// If Fan Control is enabled, then also two trigger point values
						// Value selected by using Index
						// Index 0:	PA High Temperature limit (deg C)
						// Index 1:	PA Fan On trigger point (deg C)
						// Index 2: PA Fan Off trigger point (deg C)
						// Index 3: Which bit is used to control the Cooling Fan

			if (wValue)									// New value
			{
				switch (wIndex)
				{
					case 0:
						flashc_memset8((void *)&nvram_cdata.hi_tmp_trigger, wValue, sizeof(uint8_t), TRUE);
						cdata.hi_tmp_trigger = wValue;
						break;
					case 1:
						flashc_memset8((void *)&nvram_cdata.Fan_On, wValue, sizeof(uint8_t), TRUE);
						cdata.Fan_On = wValue;
						break;
					case 2:
						flashc_memset8((void *)&nvram_cdata.Fan_Off, wValue, sizeof(uint8_t), TRUE);
						cdata.Fan_Off = wValue;
						break;
					case 3:
						flashc_memset8((void *)&nvram_cdata.PCF_fan_bit, wValue, sizeof(uint8_t), TRUE);
						cdata.PCF_fan_bit = wValue;
						break;
				}
			}

			else										// Return current value
			{
				switch (wIndex)
				{
					case 0:
						Buffer[0] = cdata.hi_tmp_trigger;
						break;
					case 1:
						Buffer[0] = cdata.Fan_On;
						break;
					case 2:
						Buffer[0] = cdata.Fan_Off;
						break;
					case 3:
						Buffer[0] = cdata.PCF_fan_bit;
						break;
				}
			}
			return sizeof(uint8_t);


		case 0x65:		// Read/Modify five settings (all 8 bit values).
						// If Value = 0 then read, else modify and read back:

						// Index 0:	Bias_Select; 0xff = Force Calibrate, 1 = LO, 2 = HI
						// Index 1:	PA Bias in 10 * mA, typically 2 = 20ma = Class B
						// Index 2: PA Bias in 10 * mA, typically 35 = 350ma = Class A
						// Index 3: PA Bias setting, LO (normally an auto adjusted value)
						// Index 4: PA Bias setting, HI  (normally an auto adjusted value)

			if (wValue)							// If value field > 0, then update EEPROM settings
			{
				switch (wIndex)
				{
					case 0:						// Which bias, 0 = Cal, 1 = LO, 2 = HI
						flashc_memset8((void *)&nvram_cdata.Bias_Select, wValue, sizeof(uint8_t), TRUE);
						Buffer[0] = cdata.Bias_Select = wValue;
						break;

					case 1:						// PA Bias in 10 * mA, Low bias setting
						flashc_memset8((void *)&nvram_cdata.Bias_LO, wValue, sizeof(uint8_t), TRUE);
						Buffer[0] = cdata.Bias_LO = wValue;
						break;
					case 2:						// PA Bias in 10 * mA, High bias setting
						flashc_memset8((void *)&nvram_cdata.Bias_HI, wValue, sizeof(uint8_t), TRUE);
						Buffer[0] = cdata.Bias_HI = wValue;
						break;

					case 3:						// PA Bias setting, Low bias setting
						flashc_memset8((void *)&nvram_cdata.cal_LO, wValue, sizeof(uint8_t), TRUE);
						Buffer[0] = cdata.cal_LO = wValue;
						break;

					case 4:						// PA Bias setting, High bias setting
						flashc_memset8((void *)&nvram_cdata.cal_HI, wValue, sizeof(uint8_t), TRUE);
						Buffer[0]= cdata.cal_HI = wValue;
						break;
				}
			}
			else								// Else just read and return the current value
			{
				switch (wIndex)
				{
					case 0:						// Which bias, 0 = Cal, 1 = LO, 2 = HI
						Buffer[0] = cdata.Bias_Select;
						break;

					case 1:						// PA Bias in mA, LO
						Buffer[0] = cdata.Bias_LO;
						break;
					case 2:						// PA Bias in mA, HI
						Buffer[0] = cdata.Bias_HI;
						break;
					case 3:						// PA Bias setting, LO
						Buffer[0] = cdata.cal_LO;
						break;
					case 4:						// PA Bias setting, HI
						Buffer[0] = cdata.cal_HI;
						break;
				}
			}
			return sizeof(uint8_t);


		case 0x66:		// Read/Modify four settings (all 16 bit values).
						// If Value = 0 then read, else modify and read back:

						// 	Index 0: Min P out measurement for SWR trigger
						// 	Index 1: SWR Timer expiry value (in units of 10ms, can be set awfully long:)
						// 	Index 2: Max SWR threshold in units of 10x SWR (SWR of 2.7 = 27)
						//	Index 3: Power meter calibration value
						//	Index 4: Power Meter bargraph Fullscale in W
						//	Index 5: SWR Meter bargraph Fullscale in SWR - 1 (if enabled by #define)
						//  Index 6: Number of PEP measurement samples for LCD power display (1-20)

			if (wValue)							// If value field > 0, then update EEPROM settings
			{
				switch (wIndex)
				{
					case 0:						// Min P out measurement for SWR trigger
						flashc_memset16((void *)&nvram_cdata.P_Min_Trigger, wValue, sizeof(uint16_t), TRUE);
						*Buf16 = cdata.P_Min_Trigger = wValue;
						break;
					case 1:						// Timer loop value
						flashc_memset16((void *)&nvram_cdata.SWR_Protect_Timer, wValue, sizeof(uint16_t), TRUE);
						*Buf16 = cdata.SWR_Protect_Timer = wValue;
						break;
					case 2:						// Max SWR threshold
						flashc_memset16((void *)&nvram_cdata.SWR_Trigger, wValue, sizeof(uint16_t), TRUE);
						*Buf16 = cdata.SWR_Trigger = wValue;
						break;
					case 3:						// Max SWR threshold
						flashc_memset16((void *)&nvram_cdata.PWR_Calibrate, wValue, sizeof(uint16_t), TRUE);
						*Buf16 = cdata.PWR_Calibrate = wValue;
						break;
					case 4:						// Fullscale Power Bargraph value
						flashc_memset8((void *)&nvram_cdata.PWR_fullscale, wValue, sizeof(uint8_t), TRUE);
						*Buf16 = cdata.PWR_fullscale = wValue;
						break;
					case 5:						// Fullscale SWR Bargraph value
						flashc_memset8((void *)&nvram_cdata.SWR_fullscale, wValue, sizeof(uint8_t), TRUE);
						*Buf16 = cdata.SWR_fullscale = wValue;
						break;
					case 6:						// Number of samples in PEP measurement
						flashc_memset8((void *)&nvram_cdata.PEP_samples, wValue, sizeof(uint8_t), TRUE);
						*Buf16 = cdata.PEP_samples = wValue;
						break;
				}
			}

			else								// Else just read and return the current value
			{
				switch (wIndex)
				{
					case 0:						// Min P out measurement for SWR trigger
						*Buf16 = cdata.P_Min_Trigger;
						break;

					case 1:						// Timer loop value
						*Buf16 = cdata.SWR_Protect_Timer;
						break;
					case 2:						// Max SWR threshold
						*Buf16 = cdata.SWR_Trigger;
						break;
					case 3:						// Max SWR threshold
						*Buf16 = cdata.PWR_Calibrate;
						break;
					case 4:						// Fullscale Power Bargraph value
						*Buf16 = cdata.PWR_fullscale;
						break;
					case 5:						// Fullscale SWR Bargraph value
						*Buf16 = cdata.SWR_fullscale;
						break;
					case 6:						// Number of samples in PEP measurement
						*Buf16 = cdata.PEP_samples;
						break;
				}
			}
			return sizeof(uint16_t);


			case 0x67:					// Read/Modify Rotary Encoder Increment Resolution Setting
										// Normally set to the number of resolvable states per revolution
			if (wValue)					// New value
			{
				flashc_memset16((void *)&nvram_cdata.Resolvable_States, wValue, sizeof(uint8_t), TRUE);
				cdata.Resolvable_States = wValue;
			}
			// Return current value
			*Buf16 = cdata.Resolvable_States;
			return sizeof(uint16_t);

			case 0x68:					// Display a fixed frequency offset during RX only.
				if (wIndex)				// If Index>0, then New value contained in Value
				{
					flashc_memset8((void *)&nvram_cdata.LCD_RX_Offset, wValue, sizeof(uint8_t), TRUE);
					cdata.LCD_RX_Offset = wValue;	// RX frequency offset, when using PowerSDR-IQ
				    FRQ_fromusb = TRUE;				// Indicate new frequency for Si570
				}
				// Return current value
				*Buffer = cdata.LCD_RX_Offset;
				return sizeof(uint8_t);


		// direct control of PCF8574 extenders
		case 0x6e:								// Send byte to (PCF8574) GPIO Extender
												// Check for a device that has been probed previously
												// If we try to write to a nonexistent I2C device, then
												// the I2C driver will end up in a funk.
			if ((wIndex == 0x20) && (i2c.pcf0x20));
			else if ((wIndex == 0x21) && (i2c.pcf0x21));
			else if ((wIndex == 0x22) && (i2c.pcf0x22));
			else if ((wIndex == 0x23) && (i2c.pcf0x23));
			else if ((wIndex == 0x24) && (i2c.pcf0x24));
			else if ((wIndex == 0x25) && (i2c.pcf0x25));
			else if ((wIndex == 0x26) && (i2c.pcf0x26));
			else if ((wIndex == 0x27) && (i2c.pcf0x27));
			else if ((wIndex == 0x38) && (i2c.pcf0x38));
			else if ((wIndex == 0x39) && (i2c.pcf0x39));
			else if ((wIndex == 0x3a) && (i2c.pcf0x3a));
			else if ((wIndex == 0x3b) && (i2c.pcf0x3b));
			else if ((wIndex == 0x3c) && (i2c.pcf0x3c));
			else if ((wIndex == 0x3d) && (i2c.pcf0x3d));
			else if ((wIndex == 0x3e) && (i2c.pcf0x3e));
			else if ((wIndex == 0x3f) && (i2c.pcf0x3f));
			else
			{
				*Buffer = 42;					// For a lack of better number, 42 = Error, nonexistent device
				return sizeof(uint8_t);
			}
			pcf8574_out_byte(wIndex, wValue);
			// Passthrough to next command and do a read.

		case 0x6f:								// Read byte from (PCF8574) GPIO Extender
												// Check for a device that has been probed previously
												// If we try to write to a nonexistent I2C device, then
												// the I2C driver will end up in a funk.
			if ((wIndex == 0x20) && (i2c.pcf0x20));
			else if ((wIndex == 0x21) && (i2c.pcf0x21));
			else if ((wIndex == 0x22) && (i2c.pcf0x22));
			else if ((wIndex == 0x23) && (i2c.pcf0x23));
			else if ((wIndex == 0x24) && (i2c.pcf0x24));
			else if ((wIndex == 0x25) && (i2c.pcf0x25));
			else if ((wIndex == 0x26) && (i2c.pcf0x26));
			else if ((wIndex == 0x27) && (i2c.pcf0x27));
			else if ((wIndex == 0x38) && (i2c.pcf0x38));
			else if ((wIndex == 0x39) && (i2c.pcf0x39));
			else if ((wIndex == 0x3a) && (i2c.pcf0x3a));
			else if ((wIndex == 0x3b) && (i2c.pcf0x3b));
			else if ((wIndex == 0x3c) && (i2c.pcf0x3c));
			else if ((wIndex == 0x3d) && (i2c.pcf0x3d));
			else if ((wIndex == 0x3e) && (i2c.pcf0x3e));
			else if ((wIndex == 0x3f) && (i2c.pcf0x3f));
			else
			{
				*Buffer = 42;					// For lack of better number, 42 = Error, nonexistent device
				return sizeof(uint8_t);
			}
			pcf8574_in_byte(wIndex, Buffer);
			return sizeof(uint8_t);

		case 0x71:
			switch (wValue){
			case 0:				// set frequency
				// I think this should just pass the sample rate in Hertz, 32 bits
				switch (wIndex) {
				case 0:
					if (current_freq.frequency != 48000) {
						current_freq.frequency = 48000;
						freq_changed = TRUE;
					}
					break;
				case 1:
					if (current_freq.frequency != 96000){
						current_freq.frequency = 96000;
						freq_changed = TRUE;
					}
					break;
				case 2:
					if (current_freq.frequency != 192000){
						current_freq.frequency = 192000;
						freq_changed = TRUE;
					}
					break;
				default:
					break;
				}
				*Buffer = 0;
				return sizeof(uint8_t);
			case 1: 			// get frequency
				*Buffer = 0;
				return sizeof(uint8_t);

			// access to sdr-widget feature api
			case 3:				// set feature in nvram
				Buffer[0] = feature_set_nvram(wIndex&0xFF, (wIndex>>8)&0xFF);
				return sizeof(uint8_t);
			case 4:				// get feature from nvram
				Buffer[0] = feature_get_nvram(wIndex);
				return sizeof(uint8_t);
			case 5:				// set feature in memory (may or may not work depending on feature)
				Buffer[0] = feature_set(wIndex&0xFF, (wIndex>>8)&0xFF);
				return sizeof(uint8_t);
			case 6:				// get feature from memory
				Buffer[0] = feature_get(wIndex);
				return sizeof(uint8_t);
			}
		default:
			return 1; //break;
	}
	return 1;
}
