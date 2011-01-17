/*
 * taskAK5394A.c
 *
 *  Created on: Feb 14, 2010
 *      Author: Alex
 *
 * Modified by Alex Lee and SDR-Widget team for the sdr-widget project.
 * See http://code.google.com/p/sdr-widget/
 * Copyright under GNU General Public License v2
 */


//_____  I N C L U D E S ___________________________________________________

//#include <stdio.h>
#include "usart.h"     // Shall be included before FreeRTOS header files, since 'inline' is defined to ''; leading to
                       // link errors
#include "conf_usb.h"


#include <avr32/io.h>
#if __GNUC__
#  include "intc.h"
#endif
#include "board.h"
#ifdef FREERTOS_USED
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "usb_drv.h"
#include "gpio.h"
#if LCD_DISPLAY				// Multi-line LCD display
#include "taskLCD.h"
#endif
//#include "taskEXERCISE.h"
#include "ssc_i2s.h"
#include "pm.h"
#include "pdca.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "device_audio_task.h"
#include "taskAK5394A.h"

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

//_____ D E C L A R A T I O N S ____________________________________________

static const gpio_map_t SSC_GPIO_MAP =
{
		{SSC_RX_CLOCK, SSC_RX_CLOCK_FUNCTION},
		{SSC_RX_DATA, SSC_RX_DATA_FUNCTION},
		{SSC_RX_FRAME_SYNC, SSC_RX_FRAME_SYNC_FUNCTION},
		{SSC_TX_CLOCK, SSC_TX_CLOCK_FUNCTION},
		{SSC_TX_DATA, SSC_TX_DATA_FUNCTION},
		{SSC_TX_FRAME_SYNC, SSC_TX_FRAME_SYNC_FUNCTION}
};

static const pdca_channel_options_t PDCA_OPTIONS =
	  {
	    .addr = (void *)audio_buffer_0,         // memory address
	    .pid = AVR32_PDCA_PID_SSC_RX,           // select peripheral
	    .size = AUDIO_BUFFER_SIZE,              // transfer counter
	    .r_addr = NULL,                         // next memory address
	    .r_size = 0,                            // next transfer counter
	    .transfer_size = PDCA_TRANSFER_SIZE_WORD  // select size of the transfer - 32 bits
	  };

static const pdca_channel_options_t SPK_PDCA_OPTIONS =
	  {
	    .addr = (void *)spk_buffer_0,         // memory address
	    .pid = AVR32_PDCA_PID_SSC_TX,           // select peripheral
	    .size = SPK_BUFFER_SIZE,              // transfer counter
	    .r_addr = NULL,                         // next memory address
	    .r_size = 0,                            // next transfer counter
	    .transfer_size = PDCA_TRANSFER_SIZE_WORD  // select size of the transfer - 32 bits
	  };



volatile U32 audio_buffer_0[AUDIO_BUFFER_SIZE];
volatile U32 audio_buffer_1[AUDIO_BUFFER_SIZE];
volatile U32 spk_buffer_0[SPK_BUFFER_SIZE];
volatile U32 spk_buffer_1[SPK_BUFFER_SIZE];

volatile avr32_ssc_t *ssc = &AVR32_SSC;

volatile int audio_buffer_in, spk_buffer_out;

void AK5394A_task(void*);


/*! \brief The PDCA interrupt handler.
 *
 * The handler reload the PDCA settings with the correct address and size using the reload register.
 * The interrupt will happen when the reload counter reaches 0
 */
__attribute__((__interrupt__)) static void pdca_int_handler(void)
{
			if (audio_buffer_in == 0)
			  {
				// Set PDCA channel reload values with address where data to load are stored, and size of the data block to load.
			   pdca_reload_channel(PDCA_CHANNEL_SSC_RX, (void *)audio_buffer_1, AUDIO_BUFFER_SIZE);
			   audio_buffer_in = 1;
			  }
			  else
			  {
				pdca_reload_channel(PDCA_CHANNEL_SSC_RX, (void *)audio_buffer_0, AUDIO_BUFFER_SIZE);
				audio_buffer_in = 0;
			  }

}

/*! \brief The PDCA interrupt handler.
 *
 * The handler reload the PDCA settings with the correct address and size using the reload register.
 * The interrupt will happen when the reload counter reaches 0
 */
__attribute__((__interrupt__)) static void spk_pdca_int_handler(void)
{
			if (spk_buffer_out == 0)
			  {
				// Set PDCA channel reload values with address where data to load are stored, and size of the data block to load.
			   pdca_reload_channel(PDCA_CHANNEL_SSC_TX, (void *)spk_buffer_1, SPK_BUFFER_SIZE);
			   spk_buffer_out = 1;
			  }
			  else
			  {
				pdca_reload_channel(PDCA_CHANNEL_SSC_TX, (void *)spk_buffer_0, SPK_BUFFER_SIZE);
				spk_buffer_out = 0;
			  }
}

/*! \brief Init interrupt controller and register pdca_int_handler interrupt.
 */
void pdca_set_irq(void)
{
  // Disable all interrupt/exception.
  Disable_global_interrupt();

  // Register the compare interrupt handler to the interrupt controller
  // and enable the compare interrupt.
  // (__int_handler) &pdca_int_handler The handler function to register.
  // AVR32_PDCA_IRQ_0 The interrupt line to register to.
  // AVR32_INTC_INT2  The priority level to set for this interrupt line.  INT0 is lowest.
  // INTC_register_interrupt(__int_handler handler, int line, int priority);
  INTC_register_interrupt( (__int_handler) &pdca_int_handler, AVR32_PDCA_IRQ_0, AVR32_INTC_INT2);
  INTC_register_interrupt( (__int_handler) &spk_pdca_int_handler, AVR32_PDCA_IRQ_1, AVR32_INTC_INT1);
  // Enable all interrupt/exception.
  Enable_global_interrupt();
}


//!
//! @brief This function initializes the hardware/software resources
//! required for device CDC task.
//!
void AK5394A_task_init(void)
{
	// Set up CS4344
	// Set up GLCK1 to provide master clock for CS4344
	gpio_enable_module_pin(GCLK1, GCLK1_FUNCTION);	// for DA_SCLK
													// LRCK is SCLK / 64 generated by TX_SSC
													// so SCLK of 6.144Mhz ===> 96khz

	pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
					  0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
					  1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
					  1,                  // diven - enabled
					  0);                 // divided by 2.  Therefore GCLK1 = 6.144Mhz
	pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

	pm_enable_osc1_ext_clock(&AVR32_PM);	// OSC1 is clocked by 12.288Mhz Osc
												// from AK5394A Xtal Oscillator
	pm_enable_clk1(&AVR32_PM, OSC1_STARTUP);

	// Set up AK5394A
	gpio_clr_gpio_pin(AK5394_RSTN);		// put AK5394A in reset
	gpio_clr_gpio_pin(AK5394_DFS0);		// L L  -> 48khz
	gpio_clr_gpio_pin(AK5394_DFS1);
	gpio_set_gpio_pin(AK5394_HPFE);		// enable HP filter
	gpio_clr_gpio_pin(AK5394_ZCAL);		// use VCOML and VCOMR to cal
	gpio_set_gpio_pin(AK5394_SMODE1);	// SMODE1 = H for Master i2s
	gpio_set_gpio_pin(AK5394_SMODE2);	// SMODE2 = H for Master/Slave i2s

    gpio_set_gpio_pin(AK5394_RSTN);		// start AK5394A
    while (gpio_get_pin_value(AK5394_CAL)); // wait till CAL goes low

		// Assign GPIO to SSC.
	  gpio_enable_module(SSC_GPIO_MAP, sizeof(SSC_GPIO_MAP) / sizeof(SSC_GPIO_MAP[0]));
	  gpio_enable_pin_glitch_filter(SSC_RX_CLOCK);
	  gpio_enable_pin_glitch_filter(SSC_RX_DATA);
	  gpio_enable_pin_glitch_filter(SSC_RX_FRAME_SYNC);
	  gpio_enable_pin_glitch_filter(SSC_TX_CLOCK);
	  gpio_enable_pin_glitch_filter(SSC_TX_DATA);
	  gpio_enable_pin_glitch_filter(SSC_TX_FRAME_SYNC);

	  current_freq.frequency = 96000;

	  // set up SSC
	  ssc_i2s_init(ssc, 96000, 24, 32, SSC_I2S_MODE_STEREO_OUT_STEREO_IN, FPBA_HZ);

	  // set up PDCA
	  // In order to avoid long slave handling during undefined length bursts (INCR), the Bus Matrix
	  // provides specific logic in order to re-arbitrate before the end of the INCR transfer.
	  //
	  // HSB Bus Matrix: By default the HSB bus matrix mode is in Undefined length burst type (INCR).
	  // Here we have to put in single access (the undefined length burst is treated as a succession of single
	  // accesses, allowing re-arbitration at each beat of the INCR burst.
	  // Refer to the HSB bus matrix section of the datasheet for more details.
	  //
	  // HSB Bus matrix register MCFG1 is associated with the CPU instruction master interface.
	  AVR32_HMATRIX.mcfg[AVR32_HMATRIX_MASTER_CPU_INSN] = 0x1;

	  audio_buffer_in = 0;
	  spk_buffer_out = 0;
	  // Register PDCA IRQ interrupt.
	  pdca_set_irq();

	  // Init PDCA channel with the pdca_options.
	  pdca_init_channel(PDCA_CHANNEL_SSC_RX, &PDCA_OPTIONS); // init PDCA channel with options.
      pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
      pdca_init_channel(PDCA_CHANNEL_SSC_TX, &SPK_PDCA_OPTIONS); // init PDCA channel with options.
       pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_TX);

       //////////////////////////////////////////////
       // Enable now the transfer.
       pdca_enable(PDCA_CHANNEL_SSC_TX);


	  xTaskCreate(AK5394A_task,
              configTSK_AK5394A_NAME,
              configTSK_AK5394A_STACK_SIZE,
              NULL,
              configTSK_AK5394A_PRIORITY,
              NULL);

}


//!
//! @brief Entry point of the AK5394A task management
//!
void AK5394A_task(void *pvParameters)
{
  portTickType xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  int i;

  while (TRUE)
  {
	  // All the hardwork is done by the pdca and the interrupt handler.
	  // Just check whether sampling freq is changed, to do rate change etc.

    vTaskDelayUntil(&xLastWakeTime, configTSK_AK5394A_PERIOD);


    if (freq_changed){


    	if (current_freq.frequency == 96000){
	   	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
	    pdca_disable(PDCA_CHANNEL_SSC_RX);

    		gpio_set_gpio_pin(AK5394_DFS0);		// L H  -> 96khz
    		gpio_clr_gpio_pin(AK5394_DFS1);

    		pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
    		pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
    						  0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
    						  1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
    						  1,                  // diven - enabled
    						  0);                 // divided by 2.  Therefore GCLK1 = 6.144Mhz
    		pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

    	   	if (Is_usb_full_speed_mode()) FB_rate = 96 << 14;
    	    	else FB_rate = (96) << 13;

    	}
    	else if (current_freq.frequency == 192000)
    	{
    	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
        pdca_disable(PDCA_CHANNEL_SSC_RX);

				gpio_clr_gpio_pin(AK5394_DFS0);		// H L -> 192khz
        		gpio_set_gpio_pin(AK5394_DFS1);

           		pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
            	pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
            						  0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
            						  1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
            						  0,                  // diven - disabled
            						  0);                 // GCLK1 = 12.288Mhz
            	pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

              	if (Is_usb_full_speed_mode()) FB_rate = 192 << 14;
                	else FB_rate = (192) << 13;

    	}
    	else if (current_freq.frequency == 48000) // 48khz
    	{

        	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
            pdca_disable(PDCA_CHANNEL_SSC_RX);

				gpio_clr_gpio_pin(AK5394_DFS0);		// L L  -> 48khz
        		gpio_clr_gpio_pin(AK5394_DFS1);

           		pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
            	pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
            						  0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
            						  1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
            						  1,                  // diven - enabled
            						  1);                 // divided by 4.  Therefore GCLK1 = 3.072Mhz
            	pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

            	if (Is_usb_full_speed_mode()) FB_rate = 48 << 14;
                	else FB_rate = (48) << 13;


        }

    	// re-sync SSC to LRCK
        // Wait for the next frame synchronization event
        // to avoid channel inversion.  Start with left channel - FS goes low
    	// However, the channels are reversed at 192khz

    	if (current_freq.frequency == 192000) {
			while (gpio_get_pin_value(AK5394_LRCK));
			while (!gpio_get_pin_value(AK5394_LRCK));	// exit when FS goes high
    	}
    	else {
			while (!gpio_get_pin_value(AK5394_LRCK));
			while (gpio_get_pin_value(AK5394_LRCK));	// exit when FS goes low
    	}
        // Enable now the transfer.
        pdca_enable(PDCA_CHANNEL_SSC_RX);

        // Init PDCA channel with the pdca_options.
        pdca_init_channel(PDCA_CHANNEL_SSC_RX, &PDCA_OPTIONS); // init PDCA channel with options.
        pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);

        // reset freq_changed flag
        freq_changed = FALSE;
    }
    if (usb_alternate_setting_out_changed){
    	if (usb_alternate_setting_out != 1){
    		for (i = 0; i < SPK_BUFFER_SIZE; i++){
    			spk_buffer_0[i] = 0;
    			spk_buffer_1[i] = 0;
    		}
    	};

    	usb_alternate_setting_out_changed = FALSE;
    }
  }
}


