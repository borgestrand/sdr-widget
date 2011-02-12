/*
 * DG8SAQ_cmd.c
 *
 *  Created on: 2010-03-20
 *      Author: TF3LJ
 */


#include <stdint.h>
#include <stdio.h>

#include "wdt.h"
#include "flashc.h"
#include "gpio.h"

#include "DG8SAQ_cmd.h"
#include "taskLCD.h"
#include "Mobo_config.h"
#include "Si570.h"

// This var is used to pass frequency from USB input command
volatile uint32_t freq_from_usb;		// New Frequency from USB


/**
 @brief Process USB Host to Device transmissions.  No result is returned.
                 This function processes control of USB commands 0x30 - 0x35
*/
void dg8saqFunctionWrite(uint8_t type, uint16_t wValue, uint16_t wIndex, U8 *Buffer, uint8_t len)
{
	uint32_t *Buf32;
	uint16_t *Buf16;
	Buf32 = (uint32_t*)Buffer;
	Buf16 = (uint16_t*)Buffer;
	int x;

	LED_Toggle(LED1);

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
				freq_from_usb = Freq_From_Register((double)cdata.FreqXtal/_2(24))*_2(21);
				FRQ_fromusb = TRUE;
			}
			break;

			// TODO
			/********************************
			#if CALC_FREQ_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
			case 0x31:								// Write the frequency subtract multiply to the eeprom
				if (len == 8)
				{
					memcpy(&R.FreqSub, data, 2*sizeof(uint32_t));
					eeprom_write_block(data, &E.FreqSub, 2*sizeof(uint32_t));
				}
				break;
			#endif
			#if CALC_BAND_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
			case 0x31:								// Write the frequency subtract multiply to the eeprom
				if (len == 2*sizeof(uint32_t))
				{
					memcpy(&Rm.BandSub[rq->wIndex.b0 & 0x0f], data, sizeof(uint32_t));
					memcpy(&Rm.BandMul[rq->wIndex.b0 & 0x0f], data+4, sizeof(uint32_t));
					eeprom_write_block(data, &Em.BandSub[rq->wIndex.b0], sizeof(uint32_t));
					eeprom_write_block(data+4, &Em.BandMul[rq->wIndex.b0], sizeof(uint32_t));
				}
				break;
			#endif
			********************************/

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

			case 0x36:	// Modify Rotary Encoder Increment Resolution & Display a fixed Frequency Offset
						// during Receive only (PSDR-IQ Offset)
						// Frequency increments are in units of ~0.12 Hz (or 8 ~ 1 Hz)
						// Also note that if this value is set too high,
						// or more than 3500ppm of the operating frequency, then
						// "smoothtune does not work"
				if (len == 4)
				{
					// Display a fixed frequency offset during RX only.
					if (wIndex == 10)					// Is this a RX frequency Offset input
					{									// This can be used always display the actual
						cdata.LCD_RX_Offset = *Buf32;	// used frequency, when using PowerSDR-IQ
						flashc_memset32((void *)&nvram_cdata.LCD_RX_Offset, *Buf32, sizeof(uint32_t), TRUE);
						freq_from_usb = cdata.Freq[0];  // Refresh frequency
						FRQ_fromusb = TRUE;
					}

					{
						cdata.Encoder_Resolution = *Buf32;
						flashc_memset32((void *)&nvram_cdata.Encoder_Resolution, *Buf32, sizeof(uint32_t), TRUE);
					}
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

	LED_Toggle(LED1);

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

		// Todo
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

	case 0x0f:								// Watchdog reset
		wdt_reset_mcu();					// Reboot by Watchdog timer

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
					flashc_memset16((void *)&nvram_cdata.FilterCrossOver, wValue, sizeof(uint16_t), TRUE);
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
					flashc_memset16((void *)&nvram_cdata.TXFilterCrossOver, wValue, sizeof(uint16_t), TRUE);
				}
				#if PCF_FILTER_IO			// 8x BCD TX filter control, switches P1 pins 4-6
				for (x=0;x<8;x++)
				{
					Buf16[7-x] = cdata.TXFilterCrossOver[x];
				}
				return 8 * sizeof(uint16_t);
				#elif M0RZF_FILTER_IO		// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6
				for (x=0;x<4;x++)
				{
					Buf16[3-x] = cdata.TXFilterCrossOver[x];
				}
				return 4 * sizeof(uint16_t);
				#else
				for (x=0;x<16;x++)
				{
					Buf16[15-x] = cdata.TXFilterCrossOver[x];
				}
				return 16 * sizeof(uint16_t);
				#endif
			}
		}

	#if SCRAMBLED_FILTERS					// Enable a non contiguous order of filters
	case 0x18:								// Set the Band Pass Filter Address for one band: 0,1,2...7
		flashc_memset8((void *)&nvram_cdata.FilterNumber[wIndex & 0x03], wValue, sizeof(uint8_t), TRUE);
		cdata.FilterNumber[wIndex & 0x03] = wValue;
		// passthrough to case 0x19

	case 0x19:								// Read the Band Pass Filter Addresses for bands 0,1,2...7
		for (x=0;x<8;x++)
		{
			Buffer[8-x] = cdata.FilterNumber[x];
		}
		return 8 * sizeof(uint8_t);

	case 0x1a:								// Set the Low Pass Filter Address for one band: 0,1,2...15
		flashc_memset8((void *)&nvram_cdata.TXFilterNumber[wIndex & 0x07], wValue, sizeof(uint8_t), TRUE);
		cdata.TXFilterNumber[wIndex & 0x03] = wValue;
		// passthrough to case 0x1b

	case 0x1b:								// Read the Low Pass Filter Addresses for bands 0,1,2...15
		for (x=0;x<TXF;x++)
		{
			Buffer[TXF-x] = cdata.TXFilterNumber[x];
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

	case 0x3a:								// Return running frequnecy
			//*(uint32_t*)Buffer = cdata.Freq[0];
			*Buf32 = cdata.Freq[0];
			return sizeof(uint32_t);

		case 0x3b:								// Return smooth tune ppm value
			*Buf16  = cdata.SmoothTunePPM;
        	return sizeof(uint16_t);


		case 0x3c:								// Return the startup frequency
			// Todo: eeprom_read_block(replyBuf, &E.Freq[rq->wIndex.b0], sizeof(E.Freq[rq->wIndex.b0]));
			*Buf32 = cdata.Freq[0];	// Temporary
			return sizeof(uint32_t);


		case 0x3d:								// Return the XTal frequnecy
			*Buf32  = cdata.FreqXtal;
			return sizeof(uint32_t);

		case 0x3f:								// Return the Si570 chip frequency control registers
			for (x = 0; x<6;x++)
			{
				Buffer[5-x]= si570reg[x];
			}
			return 6*sizeof(uint8_t);

		// Todo This appears to be fairly meaningless. May want to revise
		/***********
		case 0x40:								// return I2C transmission error status
			Buffer[0] = (flags & I2CERRORS)? 0 : 1;
			return sizeof(uint8_t);
		************/

		case 0x41:		// Set a new i2c address for a device, or reset the EEPROM to factory default
						// if Value contains 0xff, then factory defaults will be loaded on reset.

						// Index 0 reads/modifies the Si570_I2C_addr
						// Index 1 reads/modifies the I2C address for the onboard PCF8574
						// Index 2 reads/modifies the I2C address for the first PCF8574 used in the LPF Mobo
						// Index 3 reads/modifies the I2C address for the second PCF8574 used in the LPF Mobo
						// Index 4 reads/modifies the I2C address for the onboard TMP100 temperature sensor
						// Index 5 reads/modifies the I2C address for the onboard AD5301 8 bit DAC
						// Index 6 reads/modifies the I2C address for the onboard AD7991 4 x ADC

			if (wValue == 0xff)
        	{
				// Force an EEPROM update:
				flashc_memset8((void *)&nvram_cdata.EEPROM_init_check, wValue, sizeof(uint8_t), TRUE);
				wdt_reset_mcu();  			// Reboot by watchdog timer
			}

			if (wValue)						// If value field > 0, then update EEPROM settings
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
			}
			else
			{
				// Set PTT flag, ask for state change to TX
				TX_flag = TRUE;
			}
			// Passthrough to Cmd 0x51

		case 0x51:								// read CW key levels
			Buffer[0] = 0x00;
			// read pin and set regbit accordingly
			// Todo if (IO_PIN_PTT_CWKEY & IO_CWKEY1) Buffer[0] |= REG_CWSHORT;
			if (gpio_get_pin_value(GPIO_CW_KEY_1)) Buffer[0] |= REG_CWSHORT;
			// read pin and set regbit accordingly
			if (gpio_get_pin_value(GPIO_CW_KEY_2)) Buffer[0] |= REG_CWLONG;
			// Todo if (IO_PIN_PTT_CWKEY & IO_CWKEY2) Buffer[0] |= REG_CWLONG;
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

			if (wValue)
			{	// New value
				flashc_memset8((void *)&nvram_cdata.hi_tmp_trigger, wValue, sizeof(uint8_t), TRUE);
				cdata.hi_tmp_trigger = wValue;
			}
			// Return current value
			Buffer[0] = cdata.hi_tmp_trigger;
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
						// Todo eeprom_write_block(&rq->wValue.b0, &E.PWR_fullscale, sizeof (E.PWR_fullscale));
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

		// direct control of PCF8574 extenders
		case 0x6e:								// Send byte to (PCF8574) GPIO Extender
			pcf8574_out_byte(wIndex, wValue);

		case 0x6f:								// Read byte from (PCF8574) GPIO Extender
			pcf8574_in_byte(wIndex, Buffer);
			return sizeof(uint8_t);

		default:
			return 1; //break;
	}
	return 1;
}
