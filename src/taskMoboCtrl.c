/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * taskMoboCtrl.c
 *
 * This task takes care of the Mobo specific functions, such as Si570
 * frequency control, A/D inputs, Bias management and control (D/A output),
 * Transmit/Receive switchover and so on.
 * It accepts parameter updates from the USB task through the DG8SAQ_cmd.c/h
 * And it also makes A/D inputs available to the taskPowerDisplay.c/h, which
 * in turn does a best effort LCD update of the lower two LCD lines.
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "features.h"
#include "widget.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "wdt.h"
#include "flashc.h"
#include "rtc.h"
#include "queue.h"
#include "usb_drv.h"

#include "composite_widget.h"
#include "taskMoboCtrl.h"
#include "Mobo_config.h"
#include "rotary_encoder.h"
#include "I2C.h"
#include "AD7991.h"
#include "PCF8574.h"
#include "TMP100.h"
#include "DG8SAQ_cmd.h"
#include "freq_and_filters.h"


#if LCD_DISPLAY			// Multi-line LCD display
#include "taskLCD.h"
#include "LCD_bargraphs.h"
#endif


//#define GPIO_PIN_EXAMPLE_3    GPIO_PUSH_BUTTON_SW2

// Set up NVRAM (EEPROM) storage
#if defined (__GNUC__)
__attribute__((__section__(".userpage")))
#endif
mobo_data_t nvram_cdata;

char lcd_pass1[20];									// Pass data to LCD
char lcd_pass2[20];									// Pass data to LCD
char lcd_pass3[20];									// Pass data to LCD

i2c_avail	i2c;	// Availability of probed i2c devices

// Various flags, may be moved around
volatile bool MENU_mode		= 	FALSE;				// LCD Menu mode, used in conjunction with taskPushButtonMenu.c

bool		TX_state	=	FALSE;					// Keep tabs on current TX status
bool		TX_flag		=	FALSE;					// Request for TX to be set
bool		SWR_alarm	= 	FALSE;					// SWR alarm condition
bool		TMP_alarm	= 	FALSE;					// Temperature alarm condition
bool		PA_cal_lo	= 	FALSE;					// Used by PA Bias auto adjust routine
bool		PA_cal_hi	= 	FALSE;					// Used by PA Bias auto adjust routine
bool		PA_cal		=	FALSE;					// Indicates PA Bias auto adjust in progress
bool		COOLING_fan =	TRUE;					// Power Amplifier Cooling Fan (blower)


uint8_t		biasInit = 0;							// Power Amplifier Bias initiate flag
													// (0 = uninitiated => forces init, 1 = class AB, 2 = class A)

uint16_t	measured_SWR;							// SWR value x 100, in unsigned int format

/*! \brief Probe and report presence of individual I2C devices
 *
 * \retval none
 */
static uint8_t i2c_device_probe_and_log(uint8_t addr, char *addr_report)
{
	uint8_t retval;
	char	report[20];

	retval = (twi_probe(MOBO_TWI,addr)== TWI_SUCCESS);
	#if LCD_DISPLAY				// Multi-line LCD display
	if (retval)
    {
    	// Print and Log the result
		sprintf(report,"%s probed: %02x", addr_report, addr);
		//widget_display_string_scroll_and_log(report);
		widget_startup_log_line(report);
    }
	#endif

	return retval;
}
/*! \brief Probe and report which I2C devices are present
 *
 * \retval none
 */
static void i2c_device_scan(void)
{
	#if Si570
	i2c.si570 = i2c_device_probe_and_log(cdata.Si570_I2C_addr, "SI570");
    #endif

	#if TMP100
	i2c.tmp100 = i2c_device_probe_and_log(cdata.TMP100_I2C_addr, "TMP100");
	// If not found at first address, then test second address
	if (!i2c.tmp100)
	{
    	// Test for the presence of a TMP101
		i2c.tmp100 = i2c_device_probe_and_log(TMP101_I2C_ADDRESS, "TMP101");
			// If found, then write into Flash
    	if (i2c.tmp100)
    	{
        	cdata.TMP100_I2C_addr = TMP101_I2C_ADDRESS;
    		flashc_memset8((void *)&nvram_cdata.TMP100_I2C_addr, TMP101_I2C_ADDRESS, sizeof(uint8_t), TRUE);
    	}
    }
    #endif

	#if AD5301
	i2c.ad5301 = i2c_device_probe_and_log(cdata.AD5301_I2C_addr, "AD5301");
	#endif

	#if AD7991
	i2c.ad7991 = i2c_device_probe_and_log(cdata.AD7991_I2C_addr, "AD7991");
	#endif

	#if PCF8574
	i2c.pcfmobo = i2c_device_probe_and_log(cdata.PCF_I2C_Mobo_addr, "PCF8574");
	i2c.pcflpf1 = i2c_device_probe_and_log(cdata.PCF_I2C_lpf1_addr, "PCFLPF1");
	i2c.pcflpf2 = i2c_device_probe_and_log(cdata.PCF_I2C_lpf2_addr, "PCFLPF2");
	i2c.pcfext = i2c_device_probe_and_log(cdata.PCF_I2C_Ext_addr, "PCF_EXT");
	// Probe for all possible PCF8574 addresses to prevent a later attempt to
	// write to a nonexistent I2C device.  Otherwise the I2C driver would end
	// up in a funk.
	i2c.pcf0x20 = (twi_probe(MOBO_TWI,0x20)== TWI_SUCCESS);
	i2c.pcf0x21 = (twi_probe(MOBO_TWI,0x21)== TWI_SUCCESS);
	i2c.pcf0x22 = (twi_probe(MOBO_TWI,0x22)== TWI_SUCCESS);
	i2c.pcf0x23 = (twi_probe(MOBO_TWI,0x23)== TWI_SUCCESS);
	i2c.pcf0x24 = (twi_probe(MOBO_TWI,0x24)== TWI_SUCCESS);
	i2c.pcf0x25 = (twi_probe(MOBO_TWI,0x25)== TWI_SUCCESS);
	i2c.pcf0x26 = (twi_probe(MOBO_TWI,0x26)== TWI_SUCCESS);
	i2c.pcf0x27 = (twi_probe(MOBO_TWI,0x27)== TWI_SUCCESS);
	i2c.pcf0x38 = (twi_probe(MOBO_TWI,0x38)== TWI_SUCCESS);
	i2c.pcf0x39 = (twi_probe(MOBO_TWI,0x39)== TWI_SUCCESS);
	i2c.pcf0x3a = (twi_probe(MOBO_TWI,0x3a)== TWI_SUCCESS);
	i2c.pcf0x3b = (twi_probe(MOBO_TWI,0x3b)== TWI_SUCCESS);
	i2c.pcf0x3c = (twi_probe(MOBO_TWI,0x3c)== TWI_SUCCESS);
	i2c.pcf0x3d = (twi_probe(MOBO_TWI,0x3d)== TWI_SUCCESS);
	i2c.pcf0x3e = (twi_probe(MOBO_TWI,0x3e)== TWI_SUCCESS);
	i2c.pcf0x3f = (twi_probe(MOBO_TWI,0x3f)== TWI_SUCCESS);
	#endif
}


/*! \brief Print stuff in the second line of the LCD
 *
 * \retval nothing returned.
 */
void lcd_display_V_C_T_in_2nd_line(void)
{
	#if LCD_DISPLAY			// Multi-line LCD display
	// TMP100 dependent printouts
	if (i2c.tmp100)
	{
		#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
		uint16_t tmp_F;							// (threshold still set in deg C)
		tmp_F = ((tmp100_data>>7) * 9)/10 + 32;
		#else
		int16_t tmp_C = tmp100_data/256;		// Signed integer, discard sub-decimal precision
		#endif

		// Display "  Voltage and temperature" in second line
		#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
												// (threshold still set in deg C)
		sprintf(lcd_pass1,"%3uF   ", tmp_F);
    	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
		lcd_q_goto(1,0);						// First char, second line
		lcd_q_print(lcd_pass1);
		xSemaphoreGive( mutexQueLCD );
		#else
		sprintf(lcd_pass1,"%3d%cC  ", tmp_C,0xdf);
    	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
		lcd_q_goto(1,0);						// First char, second line
		lcd_q_print(lcd_pass1);
		xSemaphoreGive( mutexQueLCD );
		#endif
	}

	// AD7991 dependent printouts
	if (i2c.ad7991)
	{
		// Prep Voltage readout.  Max voltage = 15.6V (5V * 14.7k/4.7k)
		uint16_t vdd_tenths = ((uint32_t) ad7991_adc[AD7991_PSU_VOLTAGE] * 156) / 0xfff0;
		uint16_t vdd = vdd_tenths / 10;
		vdd_tenths = vdd_tenths % 10;

		sprintf(lcd_pass2,"%2u.%1uV ", vdd, vdd_tenths);
    	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
		lcd_q_goto(1,7);
		lcd_q_print(lcd_pass2);
		xSemaphoreGive( mutexQueLCD );

		//
		//-------------------------------------------------------
		// During TX, Display PA current in second line of LCD
		//-------------------------------------------------------
		//
		if (TX_state)
		{
			// Fetch and normalize PA current
			uint16_t idd_ca = ad7991_adc[AD7991_PA_CURRENT] / 262;
			uint16_t idd = idd_ca/100; idd_ca = idd_ca%100;

			sprintf(lcd_pass3,"%u.%02uA ", idd, idd_ca);	// Display current while in transmit
	    	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
			lcd_q_goto(1,13);							// Second line, 13th position
			lcd_q_print(lcd_pass3);
			xSemaphoreGive( mutexQueLCD );

		}

		//
		//-------------------------------------------------------
		// Display alternate text during RX in second line of LCD
		//-------------------------------------------------------
		//
		else
		{
	    	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
			lcd_q_goto(1,12);
	    	lcd_q_print("  Bias:");
			xSemaphoreGive( mutexQueLCD );
		}

		//
		//-------------------------------------------------------
		// Display Bias setting in second line of LCD
		//-------------------------------------------------------
		//
		const char bias[]= {'R','L','H'};
    	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
		lcd_q_goto(1,19);								// Second line, 19th position
		lcd_q_putc(bias[biasInit]);						// Print Bias status 'R'educed
														// 'Low' or 'H'igh
		xSemaphoreGive( mutexQueLCD );
	}
	#endif
}


/*! \brief Convert AD reading into "Measured Power in centiWatts"
 *
 * \retval Measured Power in centiWatts
 */
// A simplified integer arithmetic version, still with decent accuracy
// (the maximum return value overflows above 655.35W max)
uint16_t measured_Power(uint16_t voltage)
{
	// All standard stuff
	// normalise the measured value from the VSWR bridge
	// Reference voltage is 5V,
	// diode offset ~ .10V
	// R.PWR_Calibrate = Power meter calibration value
	uint32_t measured_P;

	if (voltage > 0) voltage = voltage/0x10 + 82;				// If no input voltage, then we do not add offset voltage
																// as this would otherwise result in a bogus minimum power
																// reading
																// voltage is a MSB adjusted 12 bit value, therefore
																// dividing by 10 does not lose any information
																// 4096 = 5V,
																// 82 = 100mV, compensating for schottky diode loss
	// Formula roughly adjusted for the ratio in the SWR bridge
	measured_P = (uint32_t)voltage * cdata.PWR_Calibrate/84;
	measured_P = (measured_P*measured_P)/500000;
	return measured_P;											// Return power in cW
}


/*! \brief Do SWR calcultions and control the PTT2 output
 *
 * \retval nothing returned.
 */
// Read the ADC inputs.
// The global variable ad7991_adc[AD7991_POWER_OUT] contains a measurement of the
// Power transmitted,
// and the global variable ad7991_adc[AD7991_POWER_REF] contains a measurement of the
// Power reflected,
#if POWER_SWR													// Power and SWR measurement
void Test_SWR(void)
{
	uint16_t swr = 100;											// Initialize SWR = 1.0

	#if SWR_ALARM_FUNC											// SWR alarm function, activates a secondary PTT
	static uint8_t second_pass=0, timer=0;
	#endif//SWR_ALARM_FUNC										// SWR alarm function, activates a secondary PTT

	// There is no point in doing a SWR calculation, unless keyed and on the air
	if(!(TMP_alarm) && (TX_state))
	{
		//-------------------------------------------------------------
		// Calculate SWR
		//-------------------------------------------------------------
		// Quick check for an invalid result
		if (ad7991_adc[AD7991_POWER_OUT] < V_MIN_TRIGGER*0x10)
			swr = 100;											// Too little for valid measurement, SWR = 1.0
		else if (ad7991_adc[AD7991_POWER_REF] >= ad7991_adc[AD7991_POWER_OUT])
			swr = 9990; 										// Infinite (or more than infinite) SWR:
		// Standard SWR formula multiplied by 100, eg 270 = SWR of 2.7
		else
		{
			uint32_t diff = (uint32_t)(ad7991_adc[AD7991_POWER_OUT] - ad7991_adc[AD7991_POWER_REF]);
			uint32_t sum = (uint32_t)ad7991_adc[AD7991_POWER_OUT] + (uint32_t)ad7991_adc[AD7991_POWER_REF];
			swr = 100 * sum / diff;		
		}

		if (swr < 9990)											// Set an upper bound to avoid overrrun.
			measured_SWR = swr;
		else measured_SWR = 9990;

		//-------------------------------------------------------------
		// SWR Alarm function
		// If PTT is keyed, key the PTT2 line according to SWR status
		//-------------------------------------------------------------
		#if SWR_ALARM_FUNC										// SWR alarm function, activates a secondary PTT
			    if (i2c.pcfmobo)
					{
		// On measured Power output and high SWR, force clear RXTX2 and seed timer

		// Compare power measured (in mW) with min Trigger value
		if ((measured_Power(ad7991_adc[AD7991_POWER_OUT]) > cdata.P_Min_Trigger)
			&& (measured_SWR > 10*cdata.SWR_Trigger)) 				// SWR Trigger value is a 10x value,
																// e.g. 27 corresponds to an SWR of 2.7.
		{
			if (!second_pass)									// First time, set flag, no action yet
				second_pass++;
			else												// There have been two or more consecutive measurements
																// with high SWR, take action
			{
				SWR_alarm = TRUE;								// Set SWR alarm flag
				#if  REVERSE_PTT2_LOGIC							// Switch the PTT2 logic
				pcf8574_mobo_clear(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX2);// Clear PTT2 line
				#else//not REVERSE_PTT2_LOGIC					// Normal PTT2 logic
				pcf8574_mobo_set(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX2);// Set PTT2 line
				#endif//REVERSE_PTT2_LOGIC						// end of Switch the PTT2 logic
				timer = cdata.SWR_Protect_Timer;				// Seed SWR Alarm patience timer
			}
		}
		// If SWR OK and timer has been zeroed, set the PTT2 line
		else
		{
			if (timer == 0)
			{
				SWR_alarm = FALSE;								// Clear SWR alarm flag

				#if  REVERSE_PTT2_LOGIC							// Switch the PTT2 logic
				pcf8574_mobo_set(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX2);// Set PTT2 line
				#else//not REVERSE_PTT2_LOGIC					// Normal PTT2 logic
				pcf8574_mobo_clear(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX2);// Clear PTT2 line
				#endif//REVERSE_PTT2_LOGIC						// end of Switch the PTT2 logic
				second_pass = 0;								// clear second pass flag
			}
			else
			{
				timer--;
			}
		}
		#endif//SWR_ALARM_FUNC									// SWR alarm function, activates a secondary PTT
	}
	}

	//-------------------------------------------------------------
	// Not Keyed - Clear PTT2 line
	//-------------------------------------------------------------
	else
	{
		SWR_alarm = FALSE;								// Clear SWR alarm flag
	    if (i2c.pcfmobo)
#if  REVERSE_PTT2_LOGIC							// Switch the PTT2 logic
		pcf8574_mobo_clear(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX2);// Clear PTT2 line
		#else//not REVERSE_PTT2_LOGIC					// Normal PTT2 logic
		pcf8574_mobo_set(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX2);// Set PTT2 line
		#endif//REVERSE_PTT2_LOGIC						// end of Switch the PTT2 logic
	}
}
#endif//POWER_SWR												// Power and SWR measurement


/*! \brief RD16HHF1 PA Bias management
 *
 * \retval Nothing returned
 */
void PA_bias(void)
{
	uint8_t static calibrate = 0;								// Current calibrate value

	switch (cdata.Bias_Select)
	{
		//-------------------------------------------------------------
		// Set RD16HHF1 Bias to LO setting, using stored calibrated value
		//-------------------------------------------------------------
		case 1:													// Set RD16HHF1 PA bias for Class AB
			if(biasInit != 1)									// Has this been done already?
				ad5301(cdata.AD5301_I2C_addr, cdata.cal_LO);	// No, set bias
			biasInit = 1;
			break;
		//-------------------------------------------------------------
		// Set RD16HHF1 Bias to HI setting,  using stored calibrated value
		//-------------------------------------------------------------
		case 2:													// Set RD16HHF1 PA bias for Class A
			if (SWR_alarm)										// Whoops, we have a SWR situation
			{
				ad5301(cdata.AD5301_I2C_addr, cdata.cal_LO);	// Set lower bias setting
				biasInit = 0;
			}
			else if(biasInit != 2 )								// Has this been done already?
			{
				ad5301(cdata.AD5301_I2C_addr, cdata.cal_HI);	// No, set bias
				biasInit = 2;
			}
			break;
		//-------------------------------------------------------------
		// Calibrate RD16HHF1 Bias
		//-------------------------------------------------------------
		default:												// Calibrate RD16HHF1 PA bias
			if ((!TMP_alarm) && (!TX_state))					// Proceed if there are no inhibits
			{
				TX_flag = TRUE; 								// Ask for transmitter to be keyed on
				PA_cal = TRUE;									// Indicate PA Calibrate in progress
				ad5301(cdata.AD5301_I2C_addr, 0);				// Set bias to 0 in preparation for step up
			}
			else if ((!TMP_alarm) && (TX_flag) && (TX_state))	// We have been granted switch over to TX
			{													// Start calibrating
				// Is current larger or equal to setpoint for class AB?
				if((ad7991_adc[AD7991_PA_CURRENT]/256 >= cdata.Bias_LO) && (!PA_cal_lo))
				{
					PA_cal_lo = TRUE;							// Set flag, were done with class AB
					cdata.cal_LO = calibrate;					// We have bias, store
					flashc_memset8((void *)&nvram_cdata.cal_LO, cdata.cal_LO, sizeof(cdata.cal_LO), TRUE);
				}

				// Is current larger or equal to setpoint for class A?
				if((ad7991_adc[AD7991_PA_CURRENT]/256 >= cdata.Bias_HI) && (!PA_cal_hi))
				{
					PA_cal_hi = TRUE;							// Set flag, we're done with class A
					cdata.cal_HI = calibrate;					// We have bias, store
					flashc_memset8((void *)&nvram_cdata.cal_HI, cdata.cal_HI, sizeof(cdata.cal_HI), TRUE);
				}

				// Have we reached the end of our rope?
				if(calibrate == 0xff)
				{
					PA_cal_hi = TRUE;							// Set flag as if done with class AB
					cdata.cal_HI = 0;
					cdata.cal_LO = 0;							// We have no valid bias setting
					// store 0 for both Class A and Class AB
					flashc_memset8((void *)&nvram_cdata.cal_LO, cdata.cal_LO, sizeof(cdata.cal_LO), TRUE);
					flashc_memset8((void *)&nvram_cdata.cal_HI, cdata.cal_HI, sizeof(cdata.cal_HI), TRUE);
				}

				// Are we finished?
				if (PA_cal_hi)									// We're done, Clear all flags
				{
					PA_cal_hi = FALSE;
					PA_cal_lo = FALSE;
					PA_cal = FALSE;
					TX_flag = FALSE;							// Ask for transmitter to be keyed down

					calibrate = 0;								// Clear calibrate value (for next round)
					cdata.Bias_Select = 2;						// Set bias select for class A and store
					flashc_memset8((void *)&nvram_cdata.Bias_Select, cdata.Bias_Select, sizeof(cdata.Bias_Select), TRUE);
					//implicit, no need:
					//ad5301(cdata.AD5301_I2C_addr, cdata.cal_HI);// Set bias	at class A value
				}

				// Crank up the bias
				else
				{
					calibrate++;								// Crank up the bias by one notch
					ad5301(cdata.AD5301_I2C_addr, calibrate);	// for the next round of measurements
				}
			}
	}
}



static void mobo_ctrl_factory_reset_handler(void) {
	// Force an EEPROM update in the mobo config
	flashc_memset8((void *)&nvram_cdata.EEPROM_init_check, 0xFF, sizeof(uint8_t), TRUE);
}
#define LOGGING 1
/*! \brief Initialize and run Mobo functions, including Si570 frequency control, filters and so on
 *
 * \retval none
 */
static void vtaskMoboCtrl( void * pcParameters )
{

	uint32_t time, ten_s_counter=0;					// Time management
	uint32_t lastIteration=0, Timerval;				// Counters to keep track of time

	widget_initialization_start();
	widget_factory_reset_handler_register(mobo_ctrl_factory_reset_handler);

	//----------------------------------------------------
	// Initialize all Mobo Functions *********************
	//----------------------------------------------------

	// Enforce "Factory default settings" when a mismatch is detected between the
	// COLDSTART_REF defined serial number and the matching number in the NVRAM storage.
	// This can be the result of either a fresh firmware upload, or cmd 0x41 with data 0xff
	if(nvram_cdata.EEPROM_init_check != cdata.EEPROM_init_check)
	{
		widget_startup_log_line("reset mobo nvram");
		flashc_memcpy((void *)&nvram_cdata, &cdata, sizeof(cdata), TRUE);
	}
	else
	{
		memcpy(&cdata, &nvram_cdata, sizeof(nvram_cdata));
	}

	// Enable Pin Pullups for Input Pins
	gpio_enable_pin_pull_up(GPIO_CW_KEY_1);
	gpio_enable_pin_pull_up(GPIO_CW_KEY_2);

	// Initialize Real Time Counter
	//rtc_init(&AVR32_RTC, RTC_OSC_RC, 0);	// RC clock at 115kHz
	//rtc_disable_interrupt(&AVR32_RTC);
	//rtc_set_top_value(&AVR32_RTC, RTC_COUNTER_MAX);	// Counter reset once per 10 seconds
	//rtc_enable(&AVR32_RTC);

	#if LCD_DISPLAY			// Multi-line LCD display
	// Initialize LCD
	gpio_set_gpio_pin(LCD_BL_PIN);	// Turn on LCD backlight
	lcd_q_init();
	lcd_q_clear();
	lcd_bargraph_init();
	#endif

	//Todo! may want a better name for function, function has changed
	features_display_all();

	#if ! LOGGING
	#if LCD_DISPLAY			// Multi-line LCD display
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
	lcd_q_clear();
	lcd_q_goto(3,10);
    lcd_q_print(FIRMWARE_VERSION);
	xSemaphoreGive( mutexQueLCD );
	#endif
	#endif

	// Create I2C comms semaphore
	mutexI2C = xSemaphoreCreateMutex();

 	// Initialize I2C communications
	#if I2C
	twi_init();
    // Probe for I2C devices present and report on LCD
	i2c_device_scan();
	#endif


	#if ! LOGGING
	#if LCD_DISPLAY			// Multi-line LCD display
    // Clear LCD and Print Firmware version again
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
    lcd_q_clear();
	lcd_q_goto(3,10);
    lcd_q_print(FIRMWARE_VERSION);
	xSemaphoreGive( mutexQueLCD );
	#endif
	#endif

	#if Si570
    // Print capabilities on LCD
	#if ! LOGGING
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
	lcd_q_goto(0,0);
	#endif
	// A Full house
	if (i2c.si570 && i2c.tmp100 && i2c.ad5301 && i2c.ad7991 && i2c.pcfmobo && i2c.pcflpf1)
	{
		#if LOGGING
		widget_startup_log_line("RX&TX&LPF Init OK");
		#else
		lcd_q_print("RX&TX&LPF Init OK");
		#endif
	}
	else if (i2c.si570 && i2c.tmp100 && i2c.ad5301 && i2c.ad7991 && i2c.pcfmobo)
	{
		#if LOGGING
		widget_startup_log_line("RX & TX Init OK");
		#else
	    lcd_q_print("RX & TX Init OK");
		#endif
	}
	else if (i2c.si570 && i2c.pcfmobo)
	{
		#if LOGGING
		widget_startup_log_line("RX Init OK");
		#else
	    lcd_q_print("RX Init OK");
		#endif
	}
	// Si570 present
	else if (i2c.si570)
	{
		#if LOGGING
		widget_startup_log_line("Si570 Init OK");
		#else
	    lcd_q_print("Si570 Init OK");
		#endif
	}
	// I2C device problem
	else
	{
		#if LOGGING
		widget_startup_log_line("I2Cbus NOK");
		#else
	    lcd_q_print("I2Cbus NOK");
		#endif//Si570
	}
	#if ! LOGGING
	// Keep init info on LCD for 2 seconds
	vTaskDelay( 20000 );
 	lcd_q_clear();
	xSemaphoreGive( mutexQueLCD );
	#endif
 	#endif

	// Fetch last frequency stored
	cdata.Freq[0] = cdata.Freq[cdata.SwitchFreq];
	// Initialize Startup frequency
	freq_from_usb = cdata.Freq[0];
	// Indicate new frequency for Si570
	FRQ_fromusb = TRUE;

	// Initialise Rotary Encoder Function
	encoder_init();

	// Force an initial reading of AD7991 etc
	#if I2C
	TX_state = TRUE;
	#endif

	widget_initialization_finish();

	//----------------------------------------------------
	// Mobo Functions Loop *******************************
	//----------------------------------------------------

	// Prog button poll stuff BSB 20110903
	gpio_enable_pin_glitch_filter(PRG_BUTTON);


	while( 1 )
    {
		// Poll Real Time Clock, used for 100ms and 10s timing below
		time = 	rtc_get_value(&AVR32_RTC);

/*
		// Prog button poll stuff BSB 20110903

		//-------------------------------------------
   		// Routines accessed every 0.5s on 115kHz timer
   		//-------------------------------------------
		static uint8_t btn_poll_temp=0;
		static uint32_t btn_poll_lastIteration=0, btn_poll_Timerval; // Counters to keep track of time
		btn_poll_Timerval = time/57000; // RTC on 115kHz, divide by 57000 for about 0.5s poll time

		if (btn_poll_Timerval != btn_poll_lastIteration)			// Once every 1second, do stuff
   		{
    		btn_poll_lastIteration = btn_poll_Timerval;			// Make ready for next iteration

    		if ( (gpio_get_pin_value(PRG_BUTTON) == 0) && (btn_poll_temp != 100) ) 	// If Prog button pressed and not yet handled..
    		{
    			// At first detection of Prog pin change AB-1.1 front LEDs for contrast:
    			// PINK->GREEN / RED->GREEN / GREEN->RED depending on LED_AB_FRONT
    			if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
    			{											// With UAC1:
    				#if LED_AB_FRONT_UAC1 == LED_AB_RED
						gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
						gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
    				#endif
    				#if LED_AB_FRONT_UAC1 == LED_AB_GREEN
						gpio_set_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
						gpio_clr_gpio_pin(AVR32_PIN_PX32);	// Clear GREEN light on external AB-1.1 LED
    				#endif
    				#if LED_AB_FRONT_UAC1 == LED_AB_PINK
						gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
						gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
    				#endif
    			}
    			else
    			{											// With UAC != 1
    				#if LED_AB_FRONT == LED_AB_RED
						gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
						gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
    				#endif
    				#if LED_AB_FRONT == LED_AB_GREEN
						gpio_set_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
						gpio_clr_gpio_pin(AVR32_PIN_PX32);	// Clear GREEN light on external AB-1.1 LED
    				#endif
    				#if LED_AB_FRONT == LED_AB_PINK
						gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
						gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
    				#endif
    			}

				if (btn_poll_temp > 2)  // If button pressed during at least 2 consecutive 2Hz polls...
    			{
					// Perform feature swap between UAC1 audio and UAC2 audio
					if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
					{
						feature_set_nvram(feature_image_index, feature_image_uac2_audio);
						if (feature_get_nvram(feature_image_index) == feature_image_uac2_audio)
							btn_poll_temp = 100;	// Ready reset after change and Prog release
					}
					else if (feature_get_nvram(feature_image_index) == feature_image_uac2_audio)
					{
						feature_set_nvram(feature_image_index, feature_image_uac1_audio);
						if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
							btn_poll_temp = 100;	// Ready reset after change and Prog release
					}

					if (btn_poll_temp == 100)
					{
						gpio_clr_gpio_pin(AVR32_PIN_PX32);	// GREEN OFF after performed change in nvram
						gpio_clr_gpio_pin(AVR32_PIN_PX29);	// RED OFF after performed change in nvram
					}
    			}
    			else
    				btn_poll_temp++;
    		} // if ( (gpio_get_pin_value(PRG_BUTTON) == 0) && (btn_poll_temp != 100) ) 	// If Prog button pressed and not yet handled..
    		else if ( (gpio_get_pin_value(PRG_BUTTON) != 0) && (btn_poll_temp > 0) ) // If Prog button released..
    		{
//    			if (btn_poll_temp == 100)		// Only reset after Prog button is released and successfull nvram change.
//					widget_reset();		 		// If Prog were still pressed, device would go to bootloader
					// Doesn't seem to reset Audio Widget.....

				// Modified BSB 20111016
    			if (btn_poll_temp != 100)		// Prog released without nvram change -> default front LED color
    			{								// Keep front LEDs dark after nvram change
    				// Set initial status of LEDs on the front of AB-1.1. BSB 20110903, 20111016
    				// Overriden by #if LED_STATUS == LED_STATUS_AB in SDRwdgt.h
    				if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
    				{											// With UAC1:
    					#if LED_AB_FRONT_UAC1 == LED_AB_RED
    						gpio_set_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
    						gpio_clr_gpio_pin(AVR32_PIN_PX32);	// Clear GREEN light on external AB-1.1 LED
    					#endif
    					#if LED_AB_FRONT_UAC1 == LED_AB_GREEN
    						gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
    						gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
    					#endif
    					#if LED_AB_FRONT_UAC1 == LED_AB_PINK
    						gpio_set_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
    						gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED Both -> PINK-ish!
    					#endif
    				}
    				else
    				{											// With UAC != 1
    					#if LED_AB_FRONT == LED_AB_RED
    						gpio_set_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
    						gpio_clr_gpio_pin(AVR32_PIN_PX32);	// Clear GREEN light on external AB-1.1 LED
    					#endif
    					#if LED_AB_FRONT == LED_AB_GREEN
    						gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
    						gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
    					#endif
    					#if LED_AB_FRONT == LED_AB_PINK
    						gpio_set_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
    						gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED Both -> PINK-ish!
    					#endif
    				}
    			}
    			btn_poll_temp = 0;
    		}
   		} // if (btn_poll_Timerval != btn_poll_lastIteration)	// Once every 1second, do stuff

		// End Prog button poll stuff BSB 20110903

*/

   		//-----------------------------
   		// Routines accessed every 10ms
   		//-----------------------------

		// The below is only applicable if I2C bus is available
		#if I2C

		#if MOBO_FUNCTIONS	// AD7991/AD5301/TMP100, P/SWR etc...
		//--------------------------
   		// TX stuff, once every 10ms
   		//--------------------------
		//---------------------------------
		// Bias management poll, every 10ms
		//---------------------------------
		// RD16HHF1 PA Bias management
		if (i2c.ad7991 && i2c.ad5301)					// Test for presence of required hardware
			PA_bias();									// Autobias and other bias management functions
														// This generates no I2C traffic unless bias change or
														// autobias measurement
		if (TX_state)
       	{
			if (i2c.ad7991)
			{
   				if (ad7991_poll(cdata.AD7991_I2C_addr) == 0){		// Poll the AD7991 for all four values, success == 0

				#if	POWER_SWR							// Power/SWR measurements and related actions
   				// SWR Protect
   				Test_SWR();								// Calculate SWR and control the PTT2 output
   														// (SWR protect).  Updates measured_SWR variable (SWR*100)
   														// Writes to the PCF8574 every time (2 bytes)
   														// => constant traffic on I2C (can be improved to slightly
				#endif									// reduce I2C traffic, at the cost of a few extra bytes)
				}
			}
       	}
		//--------------------------
      	// RX stuff, once every 10ms
      	//--------------------------
		else
   		{
			// Hmm
   		}

      	//------------------------------
      	// Routines accessed every 100ms
      	//------------------------------
		Timerval = time/1150; // get current Timer1 value, changeable every ~1/10th sec
   		if (Timerval != lastIteration)			// Once every 1/10th of a second, do stuff
   		{
    		lastIteration = Timerval;			// Make ready for next iteration

    		//
    		// Temperature Alarm
    		//
    		// Test for prerequisite hardware
    		if(i2c.tmp100 && i2c.pcfmobo)
    		{
    			if(TMP_alarm && !TX_flag)		// Can only clear alarm if not transmitting
        		{
    				if(tmp100_data/256 < cdata.hi_tmp_trigger)
    					TMP_alarm = FALSE;
        		}
    			else
        		{
    				// Test and set alarm if appropriate
    				if(tmp100_data/256 >= cdata.hi_tmp_trigger)
        				TMP_alarm = TRUE;
        		}
    		}

    		#if	FAN_CONTROL				// Turn PA Cooling FAN On/Off, based on temperature
    		//
    		// Activate Cooling Fan for the Transmit Power Amplifier, if needed
    		//
    		// Are we cool?
    		if(COOLING_fan)
    		{
    			if(tmp100_data/256 <= cdata.Fan_Off)
    			{
    				COOLING_fan = FALSE;				// Set FAN Status Off

    				#if	BUILTIN_PCF_FAN
    				// Builtin PCF, set fan bit low
    			    if (i2c.pcfmobo)
    			    	pcf8574_mobo_clear(cdata.PCF_I2C_Mobo_addr, PCF_MOBO_FAN_BIT);
    				#elif	EXTERN_PCF_FAN
    				//Read current status of the PCF
    				uint8_t x;
    			    if (i2c.pcfext)
    			    	pcf8574_in_byte(cdata.PCF_I2C_Ext_addr, &x);
    				//and turn off the FAN bit
    				x &= ~cdata.PCF_fan_bit;			// Extern PCF, set fan bit low
    			    if (i2c.pcfext)
    			    	pcf8574_out_byte(cdata.PCF_I2C_Ext_addr, x);
    				#endif
    			}
    		}
    		// Do we need to start the cooling fan?
    		else
    		{
    			if(tmp100_data/256 >= cdata.Fan_On)
    			{
    				COOLING_fan = TRUE;					// Set FAN Status ON

    				#if	BUILTIN_PCF_FAN
    				// Builtin PCF, set fan bit high
    			    if (i2c.pcfmobo)
    			    	pcf8574_mobo_set(cdata.PCF_I2C_Mobo_addr, PCF_MOBO_FAN_BIT);
    				#elif	EXTERN_PCF_FAN
    				//Read current status of the PCF
					uint8_t x;
    			    if (i2c.pcfext)
    			    	pcf8574_in_byte(cdata.PCF_I2C_Ext_addr, &x);
    				//and turn on the FAN bit
    				x |= cdata.PCF_fan_bit;					// Extern PCF, set fan bit high
    			    if (i2c.pcfext)
    			    	pcf8574_out_byte(cdata.PCF_I2C_Ext_addr, x);
    				#endif
    			}
    		}
			#endif

    		//---------------------------
   			// TX stuff, once every 100ms
   			//---------------------------
   			if (TX_state)
   	       	{
       	    	if (i2c.tmp100)
       	    		tmp100_read(cdata.TMP100_I2C_addr);	// Update temperature reading,
														// value read into tmp100data variable
				#if TMP_V_I_SECOND_LINE					// Normal Temp/Voltage/Current disp in second line of LCD, Disable for Debug
       	    	if (!MENU_mode)
       	    	{
           	    	lcd_display_V_C_T_in_2nd_line();	// Print LCD 2nd line stuff
       	    	}
				#endif
   	       	}
   	  		//---------------------------
   	   		// RX stuff, once every 100ms
   	   		//---------------------------
   			else
   	       	{
				// Nothing here
   	       	}
		}

      	//----------------------------
      	// Routines accessed every 10s
      	//----------------------------
       	//
		//-------------------------
   		// RX stuff, once every 10s
   		//-------------------------
		if (ten_s_counter > time)						// When the timer overflows, do stuff
   		{
			if (!TX_state)								// Do things specific to Receive
           	{
           		if (i2c.tmp100)
       	    		tmp100_read(cdata.TMP100_I2C_addr);	// Update temperature reading,
       													// value read into tmp100data variable
       			if (i2c.ad7991)
       				ad7991_poll(cdata.AD7991_I2C_addr);	// Poll the AD7991 for all four values

				#if TMP_V_I_SECOND_LINE					// Normal Temp/Voltage/Current disp in second line of LCD, Disable for Debug
       			if (!MENU_mode)
       	    	{
           	    	lcd_display_V_C_T_in_2nd_line();	// Print LCD 2nd line stuff
       	    	}
				#endif
       		}
       	}
   		ten_s_counter = time;							// Make ready for next time

		#endif

       	//-------------------------
   		// PTT Control, every 10ms
   		//-------------------------
		// Asked for TX on, TX not yet on and no Temperature alarm
   		if (TX_flag && !TX_state && !TMP_alarm)
		{
			LED_Off(LED0);

			TX_state = TRUE;
			// Switch to Transmit mode, set TX out
			#if PCF8574
			if(i2c.pcfmobo)				// Make sure the Mobo PCF is present
   	    	{
				pcf8574_mobo_clear(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX);
				if(i2c.pcflpf1)			// If the PCF for Low Pass switching is
				{						// also present, then we can use Widget PTT_1
										// for additional PTT control
					gpio_set_gpio_pin(PTT_1);
				}
   	    	}
			else
			#endif
				gpio_set_gpio_pin(PTT_1);

			#if LCD_DISPLAY				// Multi-line LCD display
			#if FRQ_IN_FIRST_LINE		// Normal Frequency display in first line of LCD. Can be disabled for Debug
			if (!MENU_mode)
       	    {
				xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
				lcd_q_goto(0,0);
		    	lcd_q_print("TX ");
		    	xSemaphoreGive( mutexQueLCD );
       	    }
			#endif
			#endif
		}
		// Asked for TX off, TX still on, or if Temperature Alarm
		else if ((!TX_flag && TX_state) || (TMP_alarm && TX_state))
		{
			LED_On(LED0);

			TX_state = FALSE;
			#if PCF8574
			if(i2c.pcfmobo)				// Make sure the Mobo PCF is present
   	    	{
				pcf8574_mobo_set(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX);
				if(i2c.pcflpf1)			// If the PCF for Low Pass switching is
				{						// also present, then we can use Widget PTT_1
										// for additional PTT control
					gpio_clr_gpio_pin(PTT_1);
				}
   	    	}
			else
			#endif
				gpio_clr_gpio_pin(PTT_1);

   	    	if (!MENU_mode)
   	    	{
				#if LCD_DISPLAY				// Multi-line LCD display
   	    		#if FRQ_IN_FIRST_LINE		// Normal Frequency display in first line of LCD. Can be disabled for Debug
   	    		xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
   				//lcd_q_clear();
   				lcd_q_goto(0,0);
   	    		lcd_q_print("RX ");
   	    		xSemaphoreGive( mutexQueLCD );
				#endif
				#endif
				#if TMP_V_I_SECOND_LINE			// Normal Temp/Voltage/Current disp in second line of LCD, Disable for Debug
   	    		lcd_display_V_C_T_in_2nd_line();// Print LCD 2nd line stuff
				#endif

				FRQ_lcdupdate = TRUE;			// Update Frequency on LCD upon return from Menu
												// Side effect:  Also upon return from TX
   	    	}
		}

		// Si570 Control
		#if Si570
		freq_and_filter_control();
		#endif

		#endif

        LED_Toggle(LED2);
        vTaskDelay(100 );
    }
}


/*! \brief RTOS initialisation of the Mobo task
 *
 * \retval none
 */
void vStartTaskMoboCtrl(void)
{
	xTaskCreate( vtaskMoboCtrl,
					configTSK_MoboCtrl_NAME,
					configTSK_MoboCtrl_STACK_SIZE,
					NULL,
					configTSK_MoboCtrl_PRIORITY,
					NULL);

}
