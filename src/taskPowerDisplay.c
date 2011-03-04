/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * taskPowerDisplay.c
 *
 * \brief This task does a best effort update of the lower two LCD lines.
 * It uses the A/D input values made available by the taskMoboCtrl, and
 * the audio input values made available by the audio task.  During Transmit
 * it prints Power/SWR, during Receive it prints samples of the audio inputs
 * and/or dB bargraphs showing the total received input power into the full
 * bandwidth of the audio channels.
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#include "board.h"

#if LCD_DISPLAY			// Multi-line LCD display

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//#include "rtc.h"

#include "composite_widget.h"
#include "taskPowerDisplay.h"
#include "taskMoboCtrl.h"
#include "Mobo_config.h"
#include "taskLCD.h"
#include "taskAK5394A.h"
#include "LCD_bargraphs.h"


//#define GPIO_PIN_EXAMPLE_3    GPIO_PUSH_BUTTON_SW2

char lcd_prt1[10];
char lcd_prt2[10];
char lcd_prt3[10];
// some of these were too short and were getting
// concatenated onto the following array when
// 20 characters were printed into them.
char lcd_prtdb1[21];
char lcd_prtdb2[21];
char lcd_prtdbhpf[21];
char lcd_bar1[21];
char lcd_bar2[21];

//
// use small static functions,
// they'll be expanded inline by the compiler
//

// normalize an audio sample
static int normalize_sample(unsigned sample) {
	return abs((((int)sample) << 8) >> 8);
}

//
// compute the integer decibels
// of an unsigned integer value
// using integer operations and
// 256 precomputed values.
// call this once to initialize
// the table before using it
// in time critical code.
//
static int twenty_log10(unsigned bits) {
  // fixed point 8.8 log10 of 9 bit patterns
  // from 0x100 to 0x1FF where the 0x100 is implicit
  static short log10_9bits[256];
  // fixed point 8.8 log10 of 2
  static short log10_2;

  if (log10_2 == 0) {
    // initialize the tables
    unsigned i;
    for (i = 0; i < 256; i += 1) {
      log10_9bits[i] = log10(256+i) * 256;
      // fprintf(stdout, "log10(0x%x): float = %f, fixed = %f\n", 256+i, log10(256+i), log10_9bits[i] / 256.0);
    }
    log10_2 = log10(2) * 256;
  }

  int result = 0;

  // deal with the invalid input
  if (bits == 0)
	  return 0;

  // shift the implicit 1 into position
  if ((bits & 0xFF000000) != 0) {
    bits >>= 16;
    result += 16 * log10_2;
  }
  if ((bits & 0x00FF0000) != 0) {
    bits >>= 8;
    result += 8 * log10_2;
  }
  if ((bits & 0x0000F000) != 0) {
    bits >>= 4;
    result += 4 * log10_2;
  }
  while ((bits & 0x1FF) != bits) {
    bits >>= 1;
    result += log10_2;
  }
  // in case we were less than 0x1FF
  while ((bits & 0x100) == 0) {
    bits <<= 1;
    result -= log10_2;
  }
  // compute the log10 result in 8.8
  result += log10_9bits[bits&0xFF];
  // convert to dB and round the fraction
  // fprintf(stdout, "twenty_log10(0x%x = %f\n", bits, (20 * result) / 256.0);
  // it isn't rounding, and I don't really understand why it isn't
  return (20 * result + 0x80) >> 8;
}

// compute the squared ratio of sample to sample max * 100
static int pow_2_ratio(uint32_t denom, uint32_t numer) {
	// return (int)pow((float)denom / numer, 2);
	// should be able to do it this way
	return (int)(((((unsigned long long)denom) * denom) / numer) / numer);
}

/*! \brief Display stuff that goes into the 3rd and 4th line of the LCD display
 * Power & SWR during Transmit and display audio amplitude in dB_full_scale during Receive
 *
 * \retval none
 */
static void vtaskPowerDisplay( void * pcParameters )
{
#if 0
	// Wait for 9 seconds while the Mobo Stuff catches up
	vTaskDelay(90000 );
#else
	// Wait until the Mobo Stuff, and other initialization, catches up
	vTaskDelay(10000 );								// defer to other tasks
	xSemaphoreTake( mutexInit, portMAX_DELAY );		// wait for initialization complete
	xSemaphoreGive( mutexInit );					// release and continue
#endif

	// set these up here so the compiler knows they don't change
	const Bool have_si570 = i2c.si570;
	const uint8_t bufsize = have_si570 ? 200 : 20; // IF RXdb only, then a large ringbuffer, else smaller

	uint16_t pow_avg[PEP_MAX_PERIOD];		// Power measurement ringbuffer
	uint8_t pow_avg_index = 0;
	int audio_sample_buffer[2][bufsize];	// audio input ringbuffer
	uint8_t audio_sample_buffer_index = 0;
	int spk_sample_buffer[2][bufsize];		// audio output ringbuffer
	uint8_t spk_sample_buffer_index = 0;

	Bool clear_pow_avg = FALSE;				// Whether the ! TX_state needs to clear pow_avg

	uint8_t tx_print = 0;					// tx information print loop count down
	uint8_t print_count = 0;				// audio information print loop count down
/*
#if 1
	// Wait for 9 seconds while the Mobo Stuff catches up
	vTaskDelay(90000 );
#else
	// Wait until the Mobo Stuff, and other initialization, catches up
	vTaskDelay(10000 );								// defer to other tasks
	xSemaphoreTake( mutexInit, portMAX_DELAY );		// wait for initialization complete
	xSemaphoreGive( mutexInit );					// release and continue
#endif
*/

	while( 1 )
    {
    	if (TX_state)
    	{
    		//------------------
    		// Do transmit stuff
    		//------------------

   	 		uint32_t pow_tot, pow, pow_cw;

    		//---------------------------------------
    		// Print to LCD once every 105ms (21*5ms)
    		//---------------------------------------
   	 		tx_print++;
    		if (tx_print == 21) tx_print = 0;
    		if (tx_print == 0)
    		{
       	 		// Prepare Power readout
       	 		pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT]);// Power in cW

    			// Store value in ringbuffer
       	 		pow_avg[pow_avg_index] = pow_tot;
       	 		pow_avg_index++;
       	 		if (pow_avg_index >= cdata.PEP_samples) pow_avg_index = 0;

       	 		// Retrieve the largest value out of the measured window
       	 		pow_tot = 0;
       	 		uint8_t j;
       	 		for (j = 0; j < cdata.PEP_samples; j++)
       	 		{
       	 			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
       	 		}


    			if (!MENU_mode)
       	    	{
           	 		//--------------------------------------------
           	 		// Display Power Output Bargraph in third line
           	 		//--------------------------------------------

           	 		// progress, maxprogress, len
           	 		lcdProgressBar(pow_tot/10, cdata.PWR_fullscale*10, 12, lcd_bar1);

           	 		pow = pow_tot / 100; 				// Watts
           	 		pow_cw = pow_tot % 100;				// centiWatts
           	 		sprintf(lcd_prt1, "P%3lu.%02luW", pow, pow_cw);

           	 		xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
            		lcd_q_goto(2,0);
            		lcd_q_print(lcd_bar1);
            		lcd_q_goto(2,12);
            		lcd_q_print(lcd_prt1);
            		xSemaphoreGive( mutexQueLCD );

           	 		//------------------------------------
           	 		// Display SWR Bargraph in fourth line
           	 		//------------------------------------

           	 		// Prepare SWR readout
           	 		uint16_t swr, swr_hundredths, swr_tenths;
           	 		swr = measured_SWR / 100;					// SWR
           	 		swr_hundredths = measured_SWR % 100;		// sub decimal point, 1/100 accuracy
           	 		swr_tenths = swr_hundredths / 10;			// sub decimal point, 1/10 accuracy

           	 		// progress, maxprogress, len
           	 		lcdProgressBar(measured_SWR - 100, cdata.SWR_fullscale*100, 12, lcd_bar2);
           	 		sprintf(lcd_prt2,"%2u.", swr);				// Print the super decimal portion of the SWR
           	 		xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
            		lcd_q_goto(3,0);
          	     	lcd_q_print(lcd_bar2);
          	     	lcd_q_print("SWR");
          	     	lcd_q_print(lcd_prt2);

           	 		#if SWR_ALARM_FUNC							// SWR alarm function, activates a secondary PTT
           	 		//------------------------------
           	 		// Display a SWR Alarm situation
           	 		//------------------------------
           	 		if(SWR_alarm)								// SWR Alarm flag set
           	 		{
           	 			if(swr >= 100)							// SWR more than 99
           	 				sprintf(lcd_prt3,"  --A");
           	 			else
           	 				// Print the sub-decimal value of the SWR, single digit precision
           	 				sprintf(lcd_prt3,"%1uA",swr_tenths);

           	 			// Todo lcd_command(LCD_DISP_ON_BLINK);	// Blinking block cursor ON
           	 		}
           	 		else
           	 		#endif//SWR_ALARM_FUNC
           	 			sprintf(lcd_prt3,"%02u", swr_hundredths);// Print the sub-decimal value of the SWR, double digit precision

          	     	lcd_q_print(lcd_prt3);
            		xSemaphoreGive( mutexQueLCD );

            		// Todo... a bit off, when many tasks are messing with the LCD
            		//lcd_q_goto(1,39);							// In case blinking cursor, put it on "A" in "SWR Alarm"
       	    	}
       	 	}
			clear_pow_avg = TRUE;
    	}
    	else
    	{
    		//-----------------
    		// Do Receive stuff
    		//-----------------

			if (clear_pow_avg) {
				// Clear TX PEP storage during receive
				// but only do it once per PTT transition
				uint8_t j;
				clear_pow_avg = FALSE;
				for (j = 0; j < PEP_MAX_PERIOD; j++)
				{
					pow_avg[j]=0;
				}
			}

    		#if DISP_RX_DB
    		//
        	// Calculate and display RX Power received in both audio channels
        	//

       		//
            // If no I2C, then Calculate and display RX & TX Audio Power including bargraphs
            //
    		if( ! have_si570)
    		{
				//---------------------------------------
				// Normalize and store samples in a Ring Buffer once every 5ms
				//---------------------------------------
				audio_sample_buffer[0][audio_sample_buffer_index] = normalize_sample(audio_buffer_0[0]);
				audio_sample_buffer[1][audio_sample_buffer_index] = normalize_sample(audio_buffer_0[1]);
				audio_sample_buffer_index++;
				if (audio_sample_buffer_index >= bufsize) audio_sample_buffer_index = 0;

				spk_sample_buffer[0][spk_sample_buffer_index] = normalize_sample(spk_buffer_0[0]);
				spk_sample_buffer[1][spk_sample_buffer_index] = normalize_sample(spk_buffer_0[1]);
   	        	spk_sample_buffer_index++;
   	    		if (spk_sample_buffer_index >= bufsize) spk_sample_buffer_index = 0;

   	    		//---------------------------------------
   	     		// Retrieve values and Print to LCD once every 105ms (21*5ms)
   	     		//---------------------------------------
   	     		print_count++;
   	     		if (print_count == 21) print_count = 0;
   	     		if (print_count == 0)
   	     		{
   	    	     	if (!MENU_mode)
   	       	    	{
						// Retrieve the largest values out of the measured window
						int audio_max_0 = 0, audio_max_1 = 0;
						int spk_max_0 = 0, spk_max_1 = 0;
						uint8_t j;
						for (j = 0; j < bufsize; j++)
						{
							audio_max_0 = max(audio_max_0, audio_sample_buffer[0][j]);
							audio_max_1 = max(audio_max_1, audio_sample_buffer[1][j]);
							spk_max_0 = max(spk_max_0, spk_sample_buffer[0][j]);
							spk_max_1 = max(spk_max_1, spk_sample_buffer[1][j]);
						}

						// Calculate RX audio in dB, TX audio in % power
						int audio_max_0_dB = twenty_log10(2*audio_max_0);
						int audio_max_1_dB = twenty_log10(2*audio_max_1);
						sprintf(lcd_prtdb1,"%4ddB  adc  %4ddB", audio_max_0_dB-144, audio_max_1_dB-144);
						// TX audio bargraph in dB or VU-meter style
						#if	TX_BARGRAPH_dB
						int spk_max_0_dB = twenty_log10(2*spk_max_0);
						int spk_max_1_dB = twenty_log10(2*spk_max_1);
						sprintf(lcd_prtdb2,"%4ddB  dac  %4ddB", spk_max_0_dB-144, spk_max_1_dB-144);
						#else
						int spk_max_0_pct = min(100, pow_2_ratio(2*spk_max_0, 0xc0000));
						int spk_max_1_pct = min(100, pow_2_ratio(2*spk_max_1, 0xc0000));
						sprintf(lcd_prtdb2,"%4d%%   dac   %4d%%", spk_max_0_pct, spk_max_1_pct);
						#endif

						// Prepare bargraphs
						// progress, maxprogress, len
						lcdProgressBar(audio_max_0_dB, 144, 9, lcd_bar1);
						*(lcd_bar1+9)=' ';
						*(lcd_bar1+10)=' ';
						lcdProgressBar(audio_max_1_dB, 144, 9, lcd_bar1+11);
						#if	TX_BARGRAPH_dB
						lcdProgressBar(spk_max_0_dB, 144, 9, lcd_bar2);
						*(lcd_bar2+9)=' ';
						*(lcd_bar2+10)=' ';
						lcdProgressBar(spk_max_1_dB, 144, 9, lcd_bar2+11);
						#else
						lcdProgressBar(spk_max_0_pct, 100, 9, lcd_bar2);
						*(lcd_bar2+9)=' ';
						*(lcd_bar2+10)=' ';
						lcdProgressBar(spk_max_1_pct, 100, 9, lcd_bar2+11);
						#endif

   	        	     	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
   	            		lcd_q_goto(0,0);
   	          	     	lcd_q_print(lcd_bar1);
   	        	     	lcd_q_goto(1,0);
   	       				lcd_q_print(lcd_prtdb1);
   	            		lcd_q_goto(2,0);
   	          	     	lcd_q_print(lcd_bar2);
   	       				lcd_q_goto(3,0);
   	       				lcd_q_print(lcd_prtdb2);
   	       				xSemaphoreGive( mutexQueLCD );
   	       	    	}
   	    		}
    		}

    		// Display RX audio power stuff in third line, if Si570
    		else
	    	{
				// Normalize and store samples in a Ring Buffer once every 5ms
				audio_sample_buffer[0][audio_sample_buffer_index] = normalize_sample(audio_buffer_0[0]);
				audio_sample_buffer[1][audio_sample_buffer_index] = normalize_sample(audio_buffer_0[1]);
				audio_sample_buffer_index++;
				if (audio_sample_buffer_index >= bufsize) audio_sample_buffer_index = 0;

        		// Do only once every 205ms
   	     		print_count++;
   	     		if (print_count == 41) print_count = 0;
   	     		if (print_count == 0)
        		{
           	    	if (!MENU_mode)
           	    	{
						// Retrieve the largest value out of the measured window
						int32_t audio_max_0 = 0, audio_max_1 = 0;
						uint8_t j;
						for (j = 0; j < bufsize; j++)
						{
							audio_max_0 = max(audio_max_0, audio_sample_buffer[0][j]);
							audio_max_1 = max(audio_max_1, audio_sample_buffer[0][j]);
						}

						// Calculate RX audio in dB, TX audio in % power
						int audio_max_0_dB = twenty_log10(2*audio_max_0);
						int audio_max_1_dB = twenty_log10(2*audio_max_1);

						// Format for print
						sprintf(lcd_prtdb1,"%4ddB  RXpwr %4ddB",audio_max_0_dB-144, audio_max_1_dB-144);

						// Prepare bargraphs
						// progress, maxprogress, len
						lcdProgressBar(audio_max_0_dB, 144, 9, lcd_bar1);
						*(lcd_bar1+9)=' ';
						*(lcd_bar1+10)=' ';
						lcdProgressBar(audio_max_1_dB, 144, 9, lcd_bar1+11);

						xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
						lcd_q_goto(2,0);
						lcd_q_print(lcd_prtdb1);
						lcd_q_goto(3,0);
						lcd_q_print(lcd_bar1);
						xSemaphoreGive( mutexQueLCD );
           	    	}
        		}
    		}
    		#endif

    		#if DISP_RX_DB_HPF
        	// Calculate and display RX Power received in both audio channels
    		// Simple High pass filter, by doing two consecutive measurements,
    		// spaced 200us apart, and using the differential for measurement.
			static uint8_t a=0, b, c=0;
    		int32_t first_sample0, first_sample1;
    		int32_t second_sample0, second_sample1;
    		int32_t sample_buffer[2][200];
    		int32_t max_0_pos, max_0_neg;
    		int32_t max_1_pos, max_1_neg;
    		float max_0_dB, max_1_dB;


   			// Get first batch of samples
    		// Sample values
    		first_sample0 = audio_buffer_0[0];
    		first_sample1 = audio_buffer_0[1];

    	    // Pause for 200us
    		vTaskDelay( 2 );

   			// Get second batch of samples
       		// Sample values
       		second_sample0 = audio_buffer_0[0];
       		second_sample1 = audio_buffer_0[1];

      		// Normalize values
       		if(first_sample0>0x7FFFFF)
       			first_sample0 = first_sample0 - 0x1000000;
       		if(first_sample1>0x7FFFFF)
       			first_sample1 = first_sample1 - 0x1000000;

      		// Normalize values
      		if(second_sample0>0x7FFFFF)
      			second_sample0 = second_sample0 - 0x1000000;
      		if(second_sample1>0x7FFFFF)
      			second_sample1 = second_sample1 - 0x1000000;

      		// Subtract and normalize again
       		second_sample0 = second_sample0 - first_sample0;
       		//if (second_sample0<0) second_sample0 *=1;
       		second_sample1 = second_sample1 - first_sample1;
       		//if (second_sample1<0) second_sample1 *=1;

       		// Store samples in a Ring Buffer
       		sample_buffer[0][a] = second_sample0;
       		sample_buffer[1][a] = second_sample1;
       		a++;
       		if (a >= 200) a = 0;

       		// Retrieve the largest pos/neg value out of the measured window
       		max_0_pos = 0;
       		max_0_neg = 0;
       		max_1_pos = 0;
       		max_1_neg = 0;
       		for (b = 0; b < 200; b++)

       		{
       			if (sample_buffer[0][b] > max_0_pos)
       				max_0_pos = sample_buffer[0][b];
       			if (sample_buffer[0][b] < max_0_neg)
       				max_0_neg = sample_buffer[0][b];
       			if (sample_buffer[1][b] > max_1_pos)
       				max_1_pos = sample_buffer[0][b];
       			if (sample_buffer[1][b] < max_1_neg)
       				max_1_neg = sample_buffer[1][b];
       		}

       		// Print dB to LCD once every 210ms
       		c++;
   			if (c == 42) c = 0;
       		if (c == 0)
       		{
       			// Calculate and display dB
       			max_0_dB = 20 * log10f(1+(max_0_pos-max_0_neg)/2);
       			max_1_dB = 20 * log10f(1+(max_1_pos-max_1_neg)/2);

       		 	sprintf(lcd_prtdbhpf,"RXhpf %4.0fdB  %4.0fdB",
       		 			max_0_dB-144.0, max_1_dB-144.0);

       	    	if (!MENU_mode)
       	    	{
       			 	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
       					lcd_q_goto(3,0);
       					lcd_q_print(lcd_prtdbhpf);
       				xSemaphoreGive( mutexQueLCD );
       	    	}

    		}
    		#endif

        	#if ENOB_TEST
    		// Simple LCD display of momentary samples, once every second
			static uint8_t m=0;
          	int32_t x;

     		// Do only once every 1s
     		if (m == 0)
     		{
     	   		if (!MENU_mode)
     	   	    {
   	     			// Display ENOB
  	     			xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
   	     				lcd_q_goto(1,0);
 //  	     				lcd_q_print("ENOB ");
   	     				lcd_q_print("TXDAT ");
   	     	    	xSemaphoreGive( mutexQueLCD );

   	     	     	// Print a snapshot of the first channel
   	     	    	// Contains a 24bit signed integer
  // 	     	     	x = audio_buffer_0[0];
   	     	    	x = spk_buffer_0[0];
   	     	     	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
   	     				lcd_q_goto(3,5);
   	     				if(x>0x7FFFFF)
   	     				{
   	     	     		lcd_q_print("-");
   	     	     		x = 0x1000000 - x;
   	     				}
   	     				else  lcd_q_print(" ");
   	     				sprintf(lcd_prt1,"%06lx ", (uint32_t)x);
   	     				lcd_q_print(lcd_prt1);
   	     	    	xSemaphoreGive( mutexQueLCD );

   	     	     	// Print a snapshot of the second channel
   	     	    	// Contains a 24bit signed integer
   //	     			x = audio_buffer_0[1];
   	     	    	x = spk_buffer_0[1];
   	     	     	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
   	     				lcd_q_goto(3,13);
   	     				if(x>0x7FFFFF)
   	     				{
   	     					lcd_q_print("-");
   	     					x = 0x1000000 - x;
   	     				}
   	     				else  lcd_q_print(" ");
   	     				sprintf(lcd_prt2,"%06lx", (uint32_t)x);
   	     				lcd_q_print(lcd_prt2);
   	     	    	xSemaphoreGive( mutexQueLCD );
   	     		}
   	    	}
	     	m++;
    		if (m == 200) m = 0; // Print ENOB to LCD once every second
    		#endif
    	}
        vTaskDelay(50 );	// 5ms loop pause
    }
}


/*! \brief RTOS initialisation of the PowerDisplay task
 *
 * \retval none
 */
void vStartTaskPowerDisplay(void)
{
	(void)twenty_log10(2);					// initialize the precomputed tables
	xStatus = xTaskCreate( vtaskPowerDisplay,
						   configTSK_PDISPLAY_NAME,
                           configTSK_PDISPLAY_STACK_SIZE,
                           NULL, 
       					   configTSK_PDISPLAY_PRIORITY,
                         ( xTaskHandle * ) NULL );
}

#endif

