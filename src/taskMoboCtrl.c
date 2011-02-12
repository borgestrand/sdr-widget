#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "board.h"

#include "gpio.h"

#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
#include "wdt.h"
#include "flashc.h"

#include "taskMoboCtrl.h"
#include "taskLCD.h"

#include "Mobo_config.h"
#include "DG8SAQ_cmd.h"
//#include "rotary_encoder.h"
#include "PCF8574.h"
#include "freq_and_filters.h"


#define GPIO_PIN_EXAMPLE_3    GPIO_PUSH_BUTTON_SW2

// Set up NVRAM (EEPROM) storage
#if defined (__GNUC__)
__attribute__((__section__(".userpage")))
#endif
mobo_data_t nvram_cdata;

int 		i;
uint16_t 	y;

i2c_avail	i2c;	// Availability of probed i2c devices

// Various flags, may be moved around
bool	FRQ_fromusb	= FALSE;				// New frequency from USB
bool	FRQ_fromenc = FALSE;				// New frequency from Encoder
bool	TX_state = FALSE;					// Keep tabs on current TX status
bool	TX_flag;							// Request for TX to be set
bool	SWR_alarm;							// SWR alarm condition
bool	TMP_alarm;							// Temperature alarm condition
bool	PA_cal_lo;							// Used by PA Bias auto adjust routine
bool	PA_cal_hi;							// Used by PA Bias auto adjust routine
bool	PA_cal;								// Indicates PA Bias auto adjust in progress
bool	ENC_stored;							// Shaft Enc pushbutton status flag "STORED"
bool	ENC_changed;						// Encoder Changed flag (used with variable rate enc)
bool	ENC_fast;							// Encoder FAST mode enabled
bool	ENC_dir;							// Encoder Direction Change



/*! \brief Probe and report which I2C devices are present
 *
 * \retval bool values indicating if I2C devices present
 */
static void i2c_probe(void)
{
	lcd_q_clear();

	#if Si570
	i2c.si570 = (twi_probe(MOBO_TWI, cdata.Si570_I2C_addr)== TWI_SUCCESS);
    if (i2c.si570)
    {
    	lcd_q_goto(0,0);
    	lcd_q_print("SI570  OK");
    	vTaskDelay( 250/portTICK_RATE_MS );
    }
    i2c.tmp100 = (twi_probe(MOBO_TWI,cdata.TMP100_I2C_addr)== TWI_SUCCESS);
	#endif
	#if TMP100
    if (i2c.tmp100)
    {
    	lcd_q_goto(1,0);
    	lcd_q_print("TMP100 OK");
    	vTaskDelay( 250/portTICK_RATE_MS );
    }
	#endif
	#if AD5301
    i2c.ad5301 = (twi_probe(MOBO_TWI,cdata.AD5301_I2C_addr)== TWI_SUCCESS);
	if (i2c.ad5301)
    {
    	lcd_q_goto(2,0);
    	lcd_q_print("AD5301 OK");
    	vTaskDelay( 250/portTICK_RATE_MS );
    }
	#endif
	#if AD7991
	i2c.ad7991 = (twi_probe(MOBO_TWI,cdata.AD7991_I2C_addr)== TWI_SUCCESS);
    if (i2c.ad7991)
    {
    	lcd_q_goto(3,0);
    	lcd_q_print("AD7991 OK");
    	vTaskDelay( 250/portTICK_RATE_MS );
    }
	#endif
	#if PCF8574
	i2c.pcfmobo = (twi_probe(MOBO_TWI,cdata.PCF_I2C_Mobo_addr)== TWI_SUCCESS);
    if (i2c.pcfmobo)
    {
    	lcd_q_goto(0,10);
    	lcd_q_print("PCF8574 OK");
    	vTaskDelay( 250/portTICK_RATE_MS );
    }
	#endif

	#if Si570
    lcd_q_goto(3,10);
	// All devices present
	if (i2c.si570 && i2c.tmp100 && i2c.ad5301 && i2c.ad7991 && i2c.pcfmobo)
	{
	    lcd_q_print("AllInit OK");
	}
	// Si570 present
	else if (i2c.si570)
	{
	    lcd_q_print("   Init OK");
	}
	// I2C device problem
	else
	{
	    lcd_q_print("I2Cbus NOK");
	}
	#endif
}


/*! \brief Initialize and run Mobo functions, including Si570 frequency control, filters and so on
 *
 * \retval none
 */
static void vtaskMoboCtrl( void * pcParameters )
{
	// Enforce "Factory default settings" when a mismatch is detected between the
	// COLDSTART_REF defined serial number and the matching number in the NVRAM storage.
	// This can be the result of either a fresh firmware upload, or cmd 0x41 with data 0xff
	if(nvram_cdata.EEPROM_init_check != cdata.EEPROM_init_check)
	{
		flashc_memcpy((void *)&nvram_cdata, &cdata, sizeof(cdata), TRUE);
	    //lcd_q_goto(1,0);
		//lcd_q_print("Flash written");
	}
	else
	{
		memcpy(&cdata, &nvram_cdata, sizeof(nvram_cdata));
	    //lcd_q_goto(1,0);
		//lcd_q_print("Flash read");
	}

    freq_from_usb = cdata.Freq[0];		// Initialize Startup frequency
    FRQ_fromusb = TRUE;					// Indicate new frequency for Si570

    i2c_probe();						// Probe for I2C devices present

	vTaskDelay( 2000/portTICK_RATE_MS );// Wait a bit and then clear display
    lcd_q_clear();

	// Indicate Receive as an initial state
    lcd_q_goto(0,0);
	lcd_q_print("RX");

    //wdt_enable(1000000);				// Watchdog with 1s patience
    for( ;; )
    {
    	// Si570 Control
		#if Si570
       	freq_and_filter_control();
		#endif

    	// TX PTT Control
		#if PCF8574
    	if(i2c.pcfmobo)
    	{
			if ((TX_flag) && !TX_state)		// Asked for TX on, TX not yet on
			{
		   		// Set PTT if there are no inhibits
				if (!(TMP_alarm | PA_cal))
				{
					TX_state = TRUE;
					// Todo biasInit = 0;				// Ensure that correct bias is set by PA_bias()
					// Switch to Transmit mode, set TX out
					pcf8574_mobo_clear(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX);
					LED_Off(LED0);
		    		lcd_q_goto(0,0);
		    		lcd_q_print("TX");
				}
			}
			else if (!TX_flag && TX_state)		// Asked for TX off, TX still on
			{
				TX_state = FALSE;
				pcf8574_mobo_set(cdata.PCF_I2C_Mobo_addr, Mobo_PCF_TX);
				LED_On(LED0);
	    		lcd_q_goto(0,0);
	    		lcd_q_print("RX");
			}
    	}
		#endif

		LED_Toggle(LED2);
        vTaskDelay(10/portTICK_RATE_MS );
        wdt_clear();
    }
}


/*! \brief RTOS initialisation of the Mobo task
 *
 * \retval none
 */
void vStartTaskMoboCtrl( unsigned portBASE_TYPE uxPriority )
{
	xStatus = xTaskCreate( vtaskMoboCtrl,
                         ( signed char * ) "taskMoboCtrl",
                           configMINIMAL_STACK_SIZE,
                           NULL,
                           uxPriority,
                         ( xTaskHandle * ) NULL );
}
