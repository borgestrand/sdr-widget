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
char lcd_prtdb[20];
char lcd_prtdb2[20];
char lcd_prtdbhpf[20];
char lcd_bar1[21];
char lcd_bar2[21];


/*! \brief Display stuff that goes into the 3rd and 4th line of the LCD display
 * Power & SWR during Transmit and display audio amplitude in dB_full_scale during Receive
 *
 * \retval none
 */
static void vtaskPowerDisplay( void * pcParameters )
{
	// Wait while the Mobo Stuff catches up
	vTaskDelay(10000 );								// defer to other tasks
	xSemaphoreTake( mutexInit, portMAX_DELAY );		// wait for initialization complete
	xSemaphoreGive( mutexInit );					// release and continue

	uint16_t pow_avg[PEP_MAX_PERIOD];		// Power measurement ringbuffer

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
    		static uint8_t tx_print = 0;
   	 		tx_print++;
    		if (tx_print == 21) tx_print = 0;
    		if (tx_print == 0)
    		{
       	 		// Prepare Power readout
       	 		pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT]);// Power in cW

       	 		static uint8_t i = 0;

    			// Store value in ringbuffer
       	 		pow_avg[i] = pow_tot;
       	 		i++;
       	 		if (i >= cdata.PEP_samples) i = 0;

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
            		lcd_q_goto(1,39);							// In case blinking cursor, put it on "A" in "SWR Alarm"
       	    	}
       	 	}
    	}
    	else
    	{
    		//-----------------
    		// Do Receive stuff
    		//-----------------

    		// Clear TX PEP storage during receive
    		uint8_t jj;
    		for (jj = 0; jj < PEP_MAX_PERIOD; jj++)
    		{
    			pow_avg[jj]=0;
    		}

    		#if DISP_RX_DB
    		//
        	// Calculate and display RX Power received in both audio channels
        	//
        	static uint8_t i=0,j, k=0, bufsize;

        	uint32_t audio_sample0, audio_sample1;
        	int32_t audio_sample_buffer[2][200];
        	int32_t audio_max_0_pos, audio_max_0_neg;
        	int32_t audio_max_1_pos, audio_max_1_neg;
           	float audio_max_0_dB, audio_max_1_dB;

        	if(i2c.si570)	// IF RXdb only, then a large ringbuffer, else smaller
        		bufsize = 200;
        	else
        		bufsize = 20;

           	// Normalize values
        	audio_sample0 = audio_buffer_0[0];
        	if(audio_sample0>0x7FFFFF)
         		audio_sample0 = audio_sample0 - 0x1000000;
        		//audio_sample0 = 0xFFFFFF - audio_sample0;

        	audio_sample1 = audio_buffer_0[1];
         	if(audio_sample1>0x7FFFFF)
         		audio_sample1 = audio_sample1 - 0x1000000;
         		//audio_sample1 = 0xFFFFFF - audio_sample1;

        	// Store samples in a Ring Buffer
        	audio_sample_buffer[0][i] = audio_sample0;
        	audio_sample_buffer[1][i] = audio_sample1;
        	i++;
        	if (i >= bufsize) i = 0;


    		// Retrieve the largest pos/neg value out of the measured window
    		audio_max_0_pos = 0;
    		audio_max_0_neg = 0;
    		audio_max_1_pos = 0;
    		audio_max_1_neg = 0;
    		for (j = 0; j < bufsize; j++)
    		{
    			if (audio_sample_buffer[0][j] > audio_max_0_pos)
    				audio_max_0_pos = audio_sample_buffer[0][j];
    			if (audio_sample_buffer[0][j] < audio_max_0_neg)
    				audio_max_0_neg = audio_sample_buffer[0][j];
    			if (audio_sample_buffer[1][j] > audio_max_1_pos)
    				audio_max_1_pos = audio_sample_buffer[0][j];
    			if (audio_sample_buffer[1][j] < audio_max_1_neg)
    				audio_max_1_neg = audio_sample_buffer[1][j];
    		}

			// Do only once every 100ms
			if (k == 0) {

				//
				// If no I2C, then Calculate and display RX & TX Audio Power including bargraphs
				//
				if(!i2c.si570)
					{
						static uint8_t l=0,m;
						uint32_t spk_sample0, spk_sample1;
						int32_t spk_sample_buffer[2][20];
						int32_t spk_max_0_pos, spk_max_0_neg;
						int32_t spk_max_1_pos, spk_max_1_neg;
						float spk_max_0, spk_max_1;

						// Normalize values
						spk_sample0 = spk_buffer_0[0];
						if(spk_sample0>0x7FFFFF)
							spk_sample0 = spk_sample0 - 0x1000000;
						//spk_sample0 = 0xFFFFFF - spk_sample0;

						spk_sample1 = spk_buffer_0[1];
						if(spk_sample1>0x7FFFFF)
							spk_sample1 = spk_sample1 - 0x1000000;
						//spk_sample1 = 0xFFFFFF - spk_sample1;

						// Store samples in a Ring Buffer
						spk_sample_buffer[0][l] = spk_sample0;
						spk_sample_buffer[1][l] = spk_sample1;
						l++;
						if (l >= bufsize) l = 0;
						// Retrieve the largest pos/neg value out of the measured window
						spk_max_0_pos = 0;
						spk_max_0_neg = 0;
						spk_max_1_pos = 0;
						spk_max_1_neg = 0;
						for (m = 0; m < bufsize; m++)
							{
								if (spk_sample_buffer[0][m] > spk_max_0_pos)
									spk_max_0_pos = spk_sample_buffer[0][m];
								if (spk_sample_buffer[0][m] < spk_max_0_neg)
									spk_max_0_neg = spk_sample_buffer[0][m];
								if (spk_sample_buffer[1][m] > spk_max_1_pos)
									spk_max_1_pos = spk_sample_buffer[0][m];
								if (spk_sample_buffer[1][m] < spk_max_1_neg)
									spk_max_1_neg = spk_sample_buffer[1][m];
							}

						// Calculate RX audio in dB, TX audio in % power
						audio_max_0_dB = 20 * log10f(1+audio_max_0_pos-audio_max_0_neg);
						audio_max_1_dB = 20 * log10f(1+audio_max_1_pos-audio_max_1_neg);
						sprintf(lcd_prtdb,"%4.0fdB  RXpwr %4.0fdB",
								audio_max_0_dB-144.0, audio_max_1_dB-144.0);

						// TX audio bargraph in dB or VU-meter style
						#if	TX_BARGRAPH_dB
						spk_max_0_dB = 20 * log10f(1+spk_max_0_pos-spk_max_0_neg);
						spk_max_1_dB = 20 * log10f(1+spk_max_1_pos-spk_max_1_neg);
						sprintf(lcd_prtdb2,"%4.0fdB  TXpwr %4.0fdB",
								spk_max_0_dB-144.0, spk_max_1_dB-144.0);
						#else
						spk_max_0 = pow((float)(spk_max_0_pos-spk_max_0_neg)/(float)0xc0000, 2);
						spk_max_1 = pow((float)(spk_max_1_pos-spk_max_1_neg)/(float)0xc0000, 2);
						if (spk_max_0 > 100) spk_max_0 = 100;
						if (spk_max_1 > 100) spk_max_1 = 100;
						sprintf(lcd_prtdb2,"%4.0f%%   TXpwr %4.0f%% ", spk_max_0, spk_max_1);
						#endif

						// Prepare bargraphs
						// progress, maxprogress, len
						lcdProgressBar(audio_max_0_dB, 144, 9, lcd_bar1);
						*(lcd_bar1+9)=' ';
						*(lcd_bar1+10)=' ';
						lcdProgressBar(audio_max_1_dB, 144, 9, lcd_bar1+11);
						lcdProgressBar(spk_max_0, 100, 9, lcd_bar2);
						*(lcd_bar2+9)=' ';
						*(lcd_bar2+10)=' ';
						lcdProgressBar(spk_max_1, 100, 9, lcd_bar2+11);

						if (!MENU_mode)
							{
								xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
								lcd_q_goto(0,0);
								lcd_q_print(lcd_bar1);
								lcd_q_goto(1,0);
								lcd_q_print(lcd_prtdb);
								lcd_q_goto(2,0);
								lcd_q_print(lcd_bar2);
								lcd_q_goto(3,0);
								lcd_q_print(lcd_prtdb2);
								xSemaphoreGive( mutexQueLCD );
							}
					}
				// Display RX audio power stuff in third line, if Si570
				else
					{
						// Calculate and display dB
						audio_max_0_dB = 20 * log10f(1+audio_max_0_pos-audio_max_0_neg);
						audio_max_1_dB = 20 * log10f(1+audio_max_1_pos-audio_max_1_neg);

						sprintf(lcd_prtdb,"RXpwr %4.0fdB  %4.0fdB",
								audio_max_0_dB-144.0, audio_max_1_dB-144.0);

						if (!MENU_mode)
							{
								xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
								lcd_q_goto(2,0);
								lcd_q_print(lcd_prtdb);
								xSemaphoreGive( mutexQueLCD );
							}

					}
			}
			k++;
			if (k == 41) k = 0;	// Print dB to LCD once every 205ms
    		#endif

    		#if DISP_RX_DB_HPF
        	// Calculate and display RX Power received in both audio channels
    		// Simple High pass filter, by doing two consecutive measurements,
    		// spaced 200us apart, and using the differential for measurement.
    		static uint8_t a=0,b, c=0;
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
	xStatus = xTaskCreate( vtaskPowerDisplay,
						   configTSK_PDISPLAY_NAME,
                           configTSK_PDISPLAY_STACK_SIZE,
                           NULL, 
       					   configTSK_PDISPLAY_PRIORITY,
                         ( xTaskHandle * ) NULL );
}

#endif

