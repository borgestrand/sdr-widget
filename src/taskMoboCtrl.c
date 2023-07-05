/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * taskMoboCtrl.c
 *
 * This task takes care of the Mobo specific functions, such as 
 * frequency control, A/D inputs, Bias management and control (D/A output),
 * Transmit/Receive switchover and so on.
 * It accepts parameter updates from the USB task through the DG8SAQ_cmd.c/h
 * And it also makes A/D inputs available to the taskPowerDisplay.c/h, which
 * in turn does a best effort LCD update of the lower two LCD lines.
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 *
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

// I2C functions
#include "I2C.h"

#include "composite_widget.h"
#include "taskMoboCtrl.h"
#include "Mobo_config.h"
#include "usb_specific_request.h"
#include "DG8SAQ_cmd.h" 
#include "device_audio_task.h"
#include "taskAK5394A.h"


#if (defined  HW_GEN_RXMOD)
#include "wm8804.h"
#include "pcm5142.h"
#include "device_audio_task.h"
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


// Various flags, may be moved around
volatile bool MENU_mode		= 	FALSE;				// LCD Menu mode, used in conjunction with taskPushButtonMenu.c

bool		SWR_alarm	= 	FALSE;					// SWR alarm condition
bool		TMP_alarm	= 	FALSE;					// Temperature alarm condition
bool		PA_cal_lo	= 	FALSE;					// Used by PA Bias auto adjust routine
bool		PA_cal_hi	= 	FALSE;					// Used by PA Bias auto adjust routine
bool		PA_cal		=	FALSE;					// Indicates PA Bias auto adjust in progress
bool		COOLING_fan =	TRUE;					// Power Amplifier Cooling Fan (blower)


uint8_t		biasInit = 0;							// Power Amplifier Bias initiate flag
													// (0 = uninitiated => forces init, 1 = class AB, 2 = class A)

uint16_t	measured_SWR;							// SWR value x 100, in unsigned int format





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
			biasInit = 1;
			break;
		//-------------------------------------------------------------
		// Set RD16HHF1 Bias to HI setting,  using stored calibrated value
		//-------------------------------------------------------------
		case 2:													// Set RD16HHF1 PA bias for Class A
			if (SWR_alarm)										// Whoops, we have a SWR situation
			{
				biasInit = 0;
			}
			else if(biasInit != 2 )								// Has this been done already?
			{
				biasInit = 2;
			}
			break;

	}
}


static void mobo_ctrl_factory_reset_handler(void) {
	// Force an EEPROM update in the mobo config
	flashc_memset8((void *)&nvram_cdata.EEPROM_init_check, 0xFF, sizeof(uint8_t), TRUE);
}

/*! \brief Initialize and run Mobo functions, including Si570 frequency control, filters and so on
 *
 * \retval none
 */
static void vtaskMoboCtrl( void * pcParameters )
{

	uint32_t time;					// Time management

#ifdef  HW_GEN_RXMOD
	uint8_t usb_ch_counter = 0;						// How many poll periods have passed since a USB change detection?
#endif

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

	
	// Initialize Real Time Counter
	//rtc_init(&AVR32_RTC, RTC_OSC_RC, 0);	// RC clock at 115kHz
	//rtc_disable_interrupt(&AVR32_RTC);
	//rtc_set_top_value(&AVR32_RTC, RTC_COUNTER_MAX);	// Counter reset once per 10 seconds
	//rtc_enable(&AVR32_RTC);


	//Todo! may want a better name for function, function has changed
	features_display_all();


	// Create I2C comms semaphore
	mutexI2C = xSemaphoreCreateMutex();

 	// Initialize I2C communications
	#if I2C
		twi_init(); // RXMODFIX vs. WM8804 config!! // i2c_init() <- this is where you should come if you search for this!

		#if (defined HW_GEN_FMADC)
			I2C_busy = xSemaphoreCreateMutex();		// Separate whole I2C packets
			
			mobo_pcm1863_init();					// Enable ADC over I2C *** Do we have I2C yet?
			mobo_fmadc_gain(0x01, 0x02);			// Channel 1, gain setting 2
			mobo_fmadc_gain(0x02, 0x02);			// Channel 2, gain setting 2
		#endif

		#if (defined HW_GEN_RXMOD)
			input_select_semphr = xSemaphoreCreateMutex();		// Tasks may take input select semaphore after init
			I2C_busy = xSemaphoreCreateMutex();		// Separate whole I2C packets

			// Initialie PCM5142
			pcm5142_filter(02); // Selected from listening 20230429

			// FIX: Why must this code be here and not in device_mouse_hid_task.c:device_mouse_hid_task_init ?
//			print_dbg_char('p');
//			wm8804_init();							// Start up the WM8805 in a fairly dead mode
			wm8804_task_init();
			wm8804_sleep();
//			print_dbg_char('r'); // Skipping 'q'
		#endif

	#endif




	// Fetch last frequency stored
	cdata.Freq[0] = cdata.Freq[cdata.SwitchFreq];
	// Initialize Startup frequency
	freq_from_usb = cdata.Freq[0];
	// Indicate new frequency for Si570
	FRQ_fromusb = TRUE;
	



	//----------------------------------------------------
	// Mobo Functions Loop *******************************
	//----------------------------------------------------

	// Prog button poll stuff BSB 20110903
	gpio_enable_pin_glitch_filter(PRG_BUTTON);



	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while (1) {
		vTaskDelayUntil(&xLastWakeTime, configTSK_MoboCtrl_PERIOD); // This is the delay method used in other tasks.

		// Prog button poll stuff BSB 20110903

		//-------------------------------------------
   		// Routines accessed every 0.5s on 115kHz timer
   		//-------------------------------------------
		static uint8_t btn_poll_temp=0;
		static uint32_t btn_poll_lastIteration=0, btn_poll_Timerval; // Counters to keep track of time

/*	// Only needed with working flash writes, see below
		static S16 spk_vol_usb_L_local = VOL_INVALID;
		static S16 spk_vol_usb_R_local = VOL_INVALID;
*/

		// Poll Real Time Clock, used for 0.5s, 100ms and 10s timing below
		time = 	rtc_get_value(&AVR32_RTC);
		btn_poll_Timerval = time/57000; // RTC on 115kHz, divide by 57000 for about 0.5s poll time

		if (btn_poll_Timerval != btn_poll_lastIteration) {	// Once every 0.5 second, do stuff
			btn_poll_lastIteration = btn_poll_Timerval;			// Make ready for next iteration



/*		// FIX: Flash writes creates ticks. Much faster (or interruptable code) is needed!
    		// Has volume setting changed recently? If so store it to flash
    		if (spk_vol_usb_L_local == VOL_INVALID) {
    			spk_vol_usb_L_local = spk_vol_usb_L;		// 1st time, establish history
    		}
    		if (spk_vol_usb_R_local == VOL_INVALID) {
    			spk_vol_usb_R_local = spk_vol_usb_R;		// 1st time, establish history
    		}
    		else if (spk_vol_usb_L_local != spk_vol_usb_L) {
    			spk_vol_usb_L_local = spk_vol_usb_L;
            	usb_volume_flash(CH_LEFT, spk_vol_usb_L, VOL_WRITE);
    		}
    		else if (spk_vol_usb_R_local != spk_vol_usb_R) {	// Not both in a row! These suckers seem to take TIME away from scheduler!
    			spk_vol_usb_R_local = spk_vol_usb_R;
            	usb_volume_flash(CH_RIGHT, spk_vol_usb_R, VOL_WRITE);
    		}
*/

    		if (gpio_get_pin_value(PRG_BUTTON) == 0) {
				#ifdef USB_STATE_MACHINE_DEBUG
					print_dbg_char('p');
				#endif
    		}

    		if ( (gpio_get_pin_value(PRG_BUTTON) == 0) && (btn_poll_temp != 100) ) {	// If Prog button pressed and not yet handled..
    			// At first detection of Prog pin change AB-1.x / USB DAC 128 mkI/II front LEDs for contrast:
    			// RED->GREEN / GREEN->RED depending on LED_AB_FRONT
    			// Historical note: Here used to be a pink definition and a bunch of defines. Removed 20150403
				#if defined(HW_GEN_AB1X)
					if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
					{										// With UAC1:
						mobo_led(FLED_RED);
					}
					else
					{										// With UAC != 1
						mobo_led(FLED_GREEN);
					}

				#elif (defined HW_GEN_RXMOD)
					if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
						mobo_led(FLED_RED);							// With UAC1
					else
						mobo_led(FLED_GREEN);						// With UAC != 1
				
				#elif (defined HW_GEN_FMADC)
					// Don't do anything with LEDs, we're not running UAC1 on this hardware
				#else
				#error undefined hardware
				#endif

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

					if (btn_poll_temp == 100) {
						#if defined(HW_GEN_AB1X)
							mobo_led(FLED_DARK);
						#elif (defined HW_GEN_RXMOD)
							mobo_led(FLED_DARK);
							// FIX: Make sure automatic sample rate or source change doesn't turn LEDs back on!
						#elif (defined HW_GEN_FMADC)
							// Do nothing with LEDs
						#else
							#error undefined hardware
						#endif
					}
    			}
    			else
    				btn_poll_temp++;
    		} // if ( (gpio_get_pin_value(PRG_BUTTON) == 0) && (btn_poll_temp != 100) ) 	// If Prog button pressed and not yet handled..
    		else if ( (gpio_get_pin_value(PRG_BUTTON) != 0) && (btn_poll_temp > 0) ) {	// If Prog button released..
//    			if (btn_poll_temp == 100)		// Only reset after Prog button is released and successfull nvram change.
//					widget_reset();		 		// If Prog were still pressed, device would go to bootloader
					// Doesn't seem to reset Audio Widget.....

    			if (btn_poll_temp != 100)		// Prog released without nvram change -> default front LED color
    			{								// Keep front LEDs dark after nvram change

					#if (defined HW_GEN_AB1X)
						if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio) {
							mobo_led(FLED_GREEN);					// With UAC1
						}
						else {										// With UAC != 1
							mobo_led(FLED_RED);
						}
					#elif (defined HW_GEN_RXMOD)
						if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio) {
							mobo_led(FLED_YELLOW);	// With UAC1:
						}
						else {
							mobo_led(FLED_PURPLE);	// With UAC != 1
						}
					#elif (defined HW_GEN_FMADC)
						// Do nothing with LEDs
					#else
						#error undefined hardware
					#endif
    			}
    			btn_poll_temp = 0;
    		}

    		// Is the task switcher running???
#ifdef USB_STATE_MACHINE_DEBUG
    		else {
    			// print_dbg_char_char(',');
    			// print_dbg_char_nibble(input_select);
    		}
#endif


   		} // if (btn_poll_Timerval != btn_poll_lastIteration)	// Once every 1second, do stuff

		// End Prog button poll stuff BSB 20110903



   		//-----------------------------
   		// Routines accessed every 10ms Now probably every 12ms due to vTaskDelay(120) below...
   		//-----------------------------

		

#ifdef HW_GEN_RXMOD
		if (mobo_usb_detect() != usb_ch) {
			print_dbg_char('#');

			if (usb_ch_counter++ > 2) {					// Different USB plug for some time:
				usb_ch_swap = USB_CH_SWAPDET;			// Signal USB audio tasks to take mute and mutex action
				vTaskDelay(200);						// Chill for a while, at least one execution of uac?_device_audio_task
				mobo_usb_select(USB_CH_NONE);			// Disconnect USB cables. Various house keeping in other tasks...
				vTaskDelay(500);						// Chill for a while, at least one execution of uac?_device_audio_task
				usb_ch = mobo_usb_detect();

				#ifdef USB_STATE_MACHINE_DEBUG			// Report what just happened
					if (usb_ch == USB_CH_C)
						print_dbg_char('c');
					else if (usb_ch == USB_CH_B)
						print_dbg_char('b');
				#endif

				mobo_usb_select(usb_ch);

				if ( (input_select == MOBO_SRC_UAC1) || (input_select == MOBO_SRC_UAC2) || (input_select == MOBO_SRC_NONE) ) {
					//					print_dbg_char('X');
					mobo_led_select(FREQ_44, input_select);	// Change LED according to recently plugged in USB cable. Assume 44.1
				}

				usb_ch_swap = USB_CH_NOSWAP;
				usb_ch_counter = 0;
			}
		}
#endif
		
//        vTaskDelay(120);						// Changed from 100 to 120 to match device_mouse_hid_task and wm8805_poll()
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
