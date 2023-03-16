/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * taskAK5394A.c
 *
 *  Created on: Feb 14, 2010
 *  Refactored on: Feb 26, 2011
 *      Author: Alex
 *
 * Copyright (C) Alex Lee
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
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
#include "ssc_i2s.h"
#include "pm.h"
#include "Mobo_config.h"
#include "pdca.h"
#include "usb_standard_request.h"
#include "features.h"
#include "device_audio_task.h"
#include "taskAK5394A.h"
#include "cycle_counter.h"

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N S ______________________________________________

//_____ D E C L A R A T I O N S ____________________________________________

volatile U32 spk_usb_heart_beat = 0, old_spk_usb_heart_beat = 0;
volatile U32 spk_usb_sample_counter = 0, old_spk_usb_sample_counter = 0;
xSemaphoreHandle mutexSpkUSB;

static const gpio_map_t SSC_GPIO_MAP = {
	{SSC_RX_CLOCK, SSC_RX_CLOCK_FUNCTION},
	{SSC_RX_DATA, SSC_RX_DATA_FUNCTION},
	{SSC_RX_FRAME_SYNC, SSC_RX_FRAME_SYNC_FUNCTION},
	{SSC_TX_CLOCK, SSC_TX_CLOCK_FUNCTION},
	{SSC_TX_DATA, SSC_TX_DATA_FUNCTION},
	{SSC_TX_FRAME_SYNC, SSC_TX_FRAME_SYNC_FUNCTION}
};

static const pdca_channel_options_t PDCA_OPTIONS = {
	.addr = (void *)audio_buffer_0,         // memory address
	.pid = AVR32_PDCA_PID_SSC_RX,           // select peripheral
	.size = ADC_BUFFER_SIZE,              // transfer counter
	.r_addr = NULL,                         // next memory address // Is this safe?? What about using audio_buffer_1 here?
	.r_size = 0,                            // next transfer counter // Is this to force an immediate interrupt?
	.transfer_size = PDCA_TRANSFER_SIZE_WORD  // select size of the transfer - 32 bits
};

static const pdca_channel_options_t SPK_PDCA_OPTIONS = {
	.addr = (void *)spk_buffer_0,         // memory address
	.pid = AVR32_PDCA_PID_SSC_TX,           // select peripheral
	.size = DAC_BUFFER_SIZE,              // transfer counter
	.r_addr = NULL,                         // next memory address
	.r_size = 0,                            // next transfer counter
	.transfer_size = PDCA_TRANSFER_SIZE_WORD  // select size of the transfer - 32 bits
};

volatile S32 audio_buffer_0[ADC_BUFFER_SIZE]; // BSB 20170324 changed to signed
volatile S32 audio_buffer_1[ADC_BUFFER_SIZE];
volatile S32 spk_buffer_0[DAC_BUFFER_SIZE];
volatile S32 spk_buffer_1[DAC_BUFFER_SIZE];

volatile avr32_ssc_t *ssc = &AVR32_SSC;

volatile int ADC_buf_DMA_write = 0;	// Written by interrupt handler, initiated by sequential code
volatile int DAC_buf_DMA_read = 0;	// Written by interrupt handler, initiated by sequential code
volatile int ADC_buf_I2S_IN = 0; 	// Written by sequential code, handles only data coming in from I2S interface (ADC or SPDIF rx)
volatile int ADC_buf_USB_IN = 0;	// Written by sequential code, handles only data IN-to USB host
volatile int DAC_buf_OUT = 0; 		// Written by sequential code, handles both USB OUT -> spk_buffer_0/1 -and- I2S input -> spk_buffer_0/1
volatile avr32_pdca_channel_t *pdca_channel; // Initiated below
volatile avr32_pdca_channel_t *spk_pdca_channel; // Initiated below
volatile int dac_must_clear;	// uacX_device_audio_task.c must clear the content of outgoing DAC buffers

#ifdef FEATURE_ADC_EXPERIMENTAL
	volatile U8 I2S_consumer = I2S_CONSUMER_NONE;	// Initially, no I2S consumer is active
#endif


// BSB 20131201 attempting improved playerstarted detection
volatile S32 usb_buffer_toggle;

// BSB 20140917 attempting to help uacX_device_audio_task.c synchronize to DMA
volatile U8 audio_OUT_alive;
volatile U8 audio_OUT_must_sync;

// BSB 20170324 SPDIF buffer processor detects silence
volatile U8 dig_in_silence;



/*! \brief The PDCA interrupt handler for the ADC interface.
 *
 * The handler reload the PDCA settings with the correct address and size using the reload register.
 * The interrupt will happen when the reload counter reaches 0
 */
__attribute__((__interrupt__)) static void pdca_int_handler(void) {
	
			
	if (ADC_buf_DMA_write == 0) {
		// Set PDCA channel reload values with address where data to load are stored, and size of the data block to load.
		// Register names are different from those used in AVR32108. BUT: it seems pdca_reload_channel() sets the
		// -next- pointer, the one to be selected automatically after the current one is done. That may be why
		// we choose the same buffer number here as in the seq. code
		pdca_reload_channel(PDCA_CHANNEL_SSC_RX, (void *)audio_buffer_1, ADC_BUFFER_SIZE);
		ADC_buf_DMA_write = 1;
#ifdef USB_STATE_MACHINE_GPIO
#ifdef FEATURE_ADC_EXPERIMENTAL
    	gpio_set_gpio_pin(AVR32_PIN_PX55); 
#endif
#endif
	}
	else if (ADC_buf_DMA_write == 1) {
		pdca_reload_channel(PDCA_CHANNEL_SSC_RX, (void *)audio_buffer_0, ADC_BUFFER_SIZE);
		ADC_buf_DMA_write = 0;
#ifdef USB_STATE_MACHINE_GPIO
#ifdef FEATURE_ADC_EXPERIMENTAL
		gpio_clr_gpio_pin(AVR32_PIN_PX55);
#endif
#endif
	}
 
}

/*! \brief The PDCA interrupt handler for the DAC interface.
 *
 * The handler reload the PDCA settings with the correct address and size using the reload register.
 * The interrupt will happen when the reload counter reaches 0
 */
__attribute__((__interrupt__)) static void spk_pdca_int_handler(void) {
	if (DAC_buf_DMA_read == 0) {
		// Set PDCA channel reload values with address where data to load are stored, and size of the data block to load.
		pdca_reload_channel(PDCA_CHANNEL_SSC_TX, (void *)spk_buffer_1, DAC_BUFFER_SIZE);
		DAC_buf_DMA_read = 1;


#ifdef USB_STATE_MACHINE_GPIO
#ifdef PRODUCT_FEATURE_AMB
		gpio_set_gpio_pin(AVR32_PIN_PX56); // For AMB use PX56/GPIO_04
#else
		// RXMODFIX This particular debug is disabled for now
		// 20221013 re-enabled
		gpio_set_gpio_pin(AVR32_PIN_PX33); // BSB 20140820 debug on GPIO_09/TP70 (was PX56 / GPIO_04)
#endif
#endif
	}
	else if (DAC_buf_DMA_read == 1) {
		pdca_reload_channel(PDCA_CHANNEL_SSC_TX, (void *)spk_buffer_0, DAC_BUFFER_SIZE);
		DAC_buf_DMA_read = 0;

#ifdef USB_STATE_MACHINE_GPIO
#ifdef PRODUCT_FEATURE_AMB
		gpio_clr_gpio_pin(AVR32_PIN_PX56); // For AMB use PX56/GPIO_04
#else
		// RXMODFIX This particular debug is disabled for now
		// 20221013 re-enabled
		gpio_clr_gpio_pin(AVR32_PIN_PX33); // BSB 20140820 debug on GPIO_09/TP70 (was PX56 / GPIO_04)
#endif
#endif
	}

	// BSB 20131201 attempting improved playerstarted detection, FIX: move to seq. code!
	if (usb_buffer_toggle < USB_BUFFER_TOGGLE_LIM)
		usb_buffer_toggle++;

	// BSB 20140917 attempting to help uacX_device_audio_task.c synchronize to DMA
	if (!audio_OUT_alive)				// If no packet has been received since last DMA reset,
		audio_OUT_must_sync = 1;		// indicate that next arriving packet must enter mid-buffer
	audio_OUT_alive = 0;				// Start detecting packets on audio OUT endpoint at DMA reset
}

/*! \brief Init interrupt controller and register pdca_int_handler interrupt.
 */
static void pdca_set_irq(void) {
	// Disable all interrupt/exception.
	Disable_global_interrupt();

	// Register the compare interrupt handler to the interrupt controller
	// and enable the compare interrupt.
	// (__int_handler) &pdca_int_handler The handler function to register.
	// AVR32_PDCA_IRQ_0 The interrupt line to register to.
	// AVR32_INTC_INT2  The priority level to set for this interrupt line.  INT0 is lowest.
	// INTC_register_interrupt(__int_handler handler, int line, int priority);
	INTC_register_interrupt( (__int_handler) &pdca_int_handler, AVR32_PDCA_IRQ_0, AVR32_INTC_INT0); //2
	INTC_register_interrupt( (__int_handler) &spk_pdca_int_handler, AVR32_PDCA_IRQ_1, AVR32_INTC_INT0); //1
	// Enable all interrupt/exception.
	Enable_global_interrupt();
}

void AK5394A_pdca_disable(void) {
}

// The old pdca_enable() code which will remain.
void AK5394A_pdca_enable(void) {
	pdca_init_channel(PDCA_CHANNEL_SSC_RX, &PDCA_OPTIONS); // init PDCA channel with options.
	pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
}


// Turn on the RX pdca, run after ssc_i2s_init() This is the new, speculative version to try to prevent L/R swap
// FIX: Build some safety mechanism into the while loop to prevent lock-up!
void AK5394A_pdca_rx_enable(U32 frequency) {
	U16 countdown = 0xFFFF;

	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
	mobo_clear_adc_channel();

   	taskENTER_CRITICAL();

	if ( (frequency == FREQ_44) || (frequency == FREQ_48) ||
		 (frequency == FREQ_88) || (frequency == FREQ_96) ||
		 (frequency == FREQ_176) || (frequency == FREQ_192) ) {
		while ( (gpio_get_pin_value(AK5394_LRCK) == 0) && (countdown != 0) ) countdown--;
		while ( (gpio_get_pin_value(AK5394_LRCK) == 1) && (countdown != 0) ) countdown--;
		while ( (gpio_get_pin_value(AK5394_LRCK) == 0) && (countdown != 0) ) countdown--;
		while ( (gpio_get_pin_value(AK5394_LRCK) == 1) && (countdown != 0) ) countdown--;
	}
	
	// What is the optimal sequence? These two are simple write operations

	pdca_init_channel(PDCA_CHANNEL_SSC_RX, &PDCA_OPTIONS);
	ADC_buf_DMA_write = 0;
   	pdca_enable(PDCA_CHANNEL_SSC_RX);
	pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);

	taskEXIT_CRITICAL();

	// Debug L/R
	print_dbg_char('-');
	print_dbg_char_hex(countdown >> 8);
	print_dbg_char_hex(countdown);
	print_dbg_char('-');
	


/* debug
	print_dbg_char('.');
	if (i == 1)
		print_dbg_char('F'); // As in "Free running"
	else
		print_dbg_char('L'); // As in "Locked"
	print_dbg_char('.');
*/	

//	gpio_clr_gpio_pin(AVR32_PIN_PX43); // ch6 p88
}


// Turn on the TX pdca, run after ssc_i2s_init()
// FIX: Build some safety mechanism into the while loop to prevent lock-up!
void AK5394A_pdca_tx_enable(U32 frequency) {
	U16 countdown = 0xFFFF;

//	gpio_set_gpio_pin(AVR32_PIN_PX52); // ch5 p87

	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_TX);

	#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char_char('N'); // xperia
	#endif
	mobo_clear_dac_channel(); // To avoid odd spurs which some times occur

   	taskENTER_CRITICAL();

	if ( (frequency == FREQ_44) || (frequency == FREQ_48) ||
		 (frequency == FREQ_88) || (frequency == FREQ_96) ||
		 (frequency == FREQ_176) || (frequency == FREQ_192) ){
		while ( (gpio_get_pin_value(AVR32_PIN_PX27) == 0) && (countdown != 0) ) countdown--;
		while ( (gpio_get_pin_value(AVR32_PIN_PX27) == 1) && (countdown != 0) ) countdown--;
		while ( (gpio_get_pin_value(AVR32_PIN_PX27) == 0) && (countdown != 0) ) countdown--;
		while ( (gpio_get_pin_value(AVR32_PIN_PX27) == 1) && (countdown != 0) ) countdown--;
		pdca_init_channel(PDCA_CHANNEL_SSC_TX, &SPK_PDCA_OPTIONS);
		DAC_buf_DMA_read = 0;	// pdca_init_channel will force start from spk_buffer_0[] as NEXT buffer to use after int

		// RXMODFIX This particular debug is disabled for now
		// 2022-10-13 re-enabled
		gpio_clr_gpio_pin(AVR32_PIN_PX33);
	}
	else {	// No known frequency, don't halt system while polling for LRCK edge
		pdca_init_channel(PDCA_CHANNEL_SSC_TX, &SPK_PDCA_OPTIONS);
		DAC_buf_DMA_read = 0;

		// RXMODFIX This particular debug is disabled for now
		// 2022-10-13 re-enabled
		gpio_clr_gpio_pin(AVR32_PIN_PX33);
	}

	// What is the optimal sequence?
   	pdca_enable(PDCA_CHANNEL_SSC_TX);
	pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_TX);

	taskEXIT_CRITICAL();

//	gpio_clr_gpio_pin(AVR32_PIN_PX52); // ch5 p87
}



void AK5394A_task_init(const Bool uac1) {
	// Set up CS4344
	// Set up GLCK1 to provide master clock for CS4344
	gpio_enable_module_pin(GCLK1, GCLK1_FUNCTION);	// for DA_SCLK
	// LRCK is SCLK / 64 generated by TX_SSC
	// so SCLK of 6.144Mhz ===> 96khz

	mutexSpkUSB = xSemaphoreCreateMutex();

	pdca_channel = pdca_get_handler(PDCA_CHANNEL_SSC_RX);
	spk_pdca_channel = pdca_get_handler(PDCA_CHANNEL_SSC_TX);


	// from AK5394A Xtal Oscillator. This takes a while. Bit clock starts running after this.
	pm_enable_osc1_ext_clock(&AVR32_PM);	// OSC1 is clocked by 12.288Mhz Osc
	pm_enable_clk1(&AVR32_PM, OSC1_STARTUP);

	// Assign GPIO to SSC.
	gpio_enable_module(SSC_GPIO_MAP, sizeof(SSC_GPIO_MAP) / sizeof(SSC_GPIO_MAP[0]));
	gpio_enable_pin_glitch_filter(SSC_RX_CLOCK);
	gpio_enable_pin_glitch_filter(SSC_RX_DATA);
	gpio_enable_pin_glitch_filter(SSC_RX_FRAME_SYNC);
	gpio_enable_pin_glitch_filter(SSC_TX_CLOCK);
	gpio_enable_pin_glitch_filter(SSC_TX_DATA);
	gpio_enable_pin_glitch_filter(SSC_TX_FRAME_SYNC);

	// Disable PDCAs for good measure before messing with ssc_i2s configuration
	pdca_disable(PDCA_CHANNEL_SSC_TX);
	pdca_disable(PDCA_CHANNEL_SSC_RX);

	// set up SSC, it looks like frequency parameter (FPBA_HZ) is NOT in use
	// It doesn't start from 0 but from 1. 1st whole LRCK cycle is 2x its expected duration.
	if (uac1) {
		ssc_i2s_init(ssc, 48000, 24, 32, SSC_I2S_MODE_STEREO_OUT_STEREO_IN, FPBA_HZ);
	} else {
		ssc_i2s_init(ssc, 96000, 32, 32, SSC_I2S_MODE_STEREO_OUT_STEREO_IN, FPBA_HZ);
	}

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

// 	ADC_buf_DMA_write = 0; Now done in (global) variable declaration
//	DAC_buf_DMA_read = 0; Now done in (global) variable declaration
	// Register PDCA IRQ interruptS. // Plural those are!
	pdca_set_irq();

	// Init ADC channel for SPDIF buffering, HW_GEN_WFADC turns it on in separate state machine WFADC_site
	#if (defined HW_GEN_RXMOD) || (defined HW_GEN_WFADC)
	/*  Empty for now....
		pdca_init_channel(PDCA_CHANNEL_SSC_RX, &PDCA_OPTIONS); // init PDCA channel with options.
//		pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
		// pdca_enable() is called from WM8805 init functions
	 */
	#else
		// Init PDCA channel with the pdca_options.
		// REMOVE! The ADC should be designed out completely, this is taken over by SPDIF reception sub system ADC_site
		if (!FEATURE_ADC_NONE) {
			pdca_init_channel(PDCA_CHANNEL_SSC_RX, &PDCA_OPTIONS); // init PDCA channel with options.
			pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
		}
	#endif


	// Initial setup of clock and TX IO. This will cause LR inversion when called with FREQ_INVALID
	// Therefore, call it with proper frequency when playback starts.
	mobo_clock_division(FREQ_INVALID);


#ifdef HW_GEN_RXMOD
// No such power control yet
#endif



}
