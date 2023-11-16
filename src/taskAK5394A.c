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

#include "tc.h"

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
	.r_addr = NULL,                         // next memory address // What about using spk_buffer_1 here?
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
//    	gpio_set_gpio_pin(AVR32_PIN_PX31); 
#endif
	}
	else if (ADC_buf_DMA_write == 1) {
		pdca_reload_channel(PDCA_CHANNEL_SSC_RX, (void *)audio_buffer_0, ADC_BUFFER_SIZE);
		ADC_buf_DMA_write = 0;
#ifdef USB_STATE_MACHINE_GPIO
//		gpio_clr_gpio_pin(AVR32_PIN_PX31);
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


#ifdef PRODUCT_FEATURE_AMB
		gpio_set_gpio_pin(AVR32_PIN_PX56); // For AMB use PX56/GPIO_04
#else
		gpio_set_gpio_pin(AVR32_PIN_PX33);
#endif
	}
	else if (DAC_buf_DMA_read == 1) {
		pdca_reload_channel(PDCA_CHANNEL_SSC_TX, (void *)spk_buffer_0, DAC_BUFFER_SIZE);
		DAC_buf_DMA_read = 0;

#ifdef PRODUCT_FEATURE_AMB
		gpio_clr_gpio_pin(AVR32_PIN_PX56); // For AMB use PX56/GPIO_04
#else
		gpio_clr_gpio_pin(AVR32_PIN_PX33);
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



// TCFIX new code to set up spdif receive timer
// MCU has "Two Three-Channel 16-bit Timer/Counter (TC)" Each timer has three channels
#define spdif_tc_device		AVR32_TC1	// Using TC1 where we have CLK0 available on PA05
#define spdif_tc_channel	0			// Timer counter -channel-
#define TC1_CLK0_PIN		AVR32_TC1_CLK0_0_PIN
#define	TC1_CLK0_FUNCTION	AVR32_TC1_CLK0_0_FUNCTION

static const gpio_map_t TC1_CLK0_GPIO_MAP = {
	{TC1_CLK0_PIN, TC1_CLK0_FUNCTION}
};


__attribute__((__interrupt__)) static void spdif_packet_int_handler(void) {

	// Needed to restart timer?
	tc_read_sr(&spdif_tc_device, spdif_tc_channel);
	
	// Slow debug on console
	print_dbg_char('!');
	
	// Fast debug on scope
	static int test = 0;
	if (test == 0) {
		gpio_set_gpio_pin(AVR32_PIN_PX31);
		test = 1;
	}
	else {
		gpio_clr_gpio_pin(AVR32_PIN_PX31);
		test = 0;
	}
		
}


static void spdif_packet_SetupTimerInterrupt(void) {
	volatile avr32_tc_t *tc = &spdif_tc_device;	// TCFIX changed from &AVR32_TC to &AVR32_TC1
	

	// Options for waveform genration.
	tc_waveform_opt_t waveform_opt = {
		.channel  = spdif_tc_channel,                   /* Channel selection. */
		.bswtrg   = TC_EVT_EFFECT_NOOP,                /* Software trigger effect on TIOB. */
		.beevt    = TC_EVT_EFFECT_NOOP,                /* External event effect on TIOB. */
		.bcpc     = TC_EVT_EFFECT_NOOP,                /* RC compare effect on TIOB. */
		.bcpb     = TC_EVT_EFFECT_NOOP,                /* RB compare effect on TIOB. */
		.aswtrg   = TC_EVT_EFFECT_NOOP,                /* Software trigger effect on TIOA. */
		.aeevt    = TC_EVT_EFFECT_NOOP,                /* External event effect on TIOA. */
		.acpc     = TC_EVT_EFFECT_NOOP,                /* RC compare effect on TIOA: toggle. */
		.acpa     = TC_EVT_EFFECT_NOOP,                /* RA compare effect on TIOA: toggle (other possibilities are none, set and clear). */
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,/* Waveform selection: Up mode without automatic trigger on RC compare. */
		.enetrg   = FALSE,                             /* External event trigger enable. */
		.eevt     = 0,                                 /* External event selection. */
		.eevtedg  = TC_SEL_NO_EDGE,                    /* External event edge selection. */
		.cpcdis   = FALSE,                             /* Counter disable when RC compare. */
		.cpcstop  = FALSE,                             /* Counter clock stopped with RC compare. */
		.burst    = FALSE,                             /* Burst signal selection. */
		.clki     = FALSE,                             /* Clock inversion. */
		.tcclks   = TC_CLOCK_SOURCE_XC0                /* Presumably external source clock CLK0. */
	};

	tc_interrupt_t tc_interrupt = {
		.etrgs=0,
		.ldrbs=0,
		.ldras=0,
		.cpcs =1,
		.cpbs =0,
		.cpas =0,
		.lovrs=0,
		.covfs=0,
	};

	// Configure PA05 input pin as clock
	gpio_enable_module(TC1_CLK0_GPIO_MAP, sizeof(TC1_CLK0_GPIO_MAP) / sizeof(TC1_CLK0_GPIO_MAP[0]));

	// Register the compare interrupt handler to the interrupt controller and enable the compare interrupt
//	INTC_register_interrupt( (__int_handler) &spdif_packet_int_handler, AVR32_TC1_IRQ2, AVR32_INTC_INT2);

	// Should we do something like this???
//	tc_select_external_clock(tc, spdif_tc_channel, TC_CH0_EXT_CLK0_SRC_TCLK0);

	// Initialize the timer/counter
//	tc_init_waveform(tc, &waveform_opt);

	// For now aim for a division by 10 and monitor PX31
//	tc_write_rc(tc, spdif_tc_channel, 9);

//	tc_configure_interrupts(tc, spdif_tc_channel, &tc_interrupt );

	// Start the timer/counter, but only after we have reset the timer value to 0!
	// tc_software_trigger(tc, spdif_tc_channel);
//	tc_start(tc, spdif_tc_channel); // Implements SWTRG software trig and CLKEN clock enable
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
	
	// TCFIX new code	
	print_dbg_char('a');
	
	spdif_packet_SetupTimerInterrupt();

	print_dbg_char('c');	
	
	// Enable all interrupt/exception.
	Enable_global_interrupt();
}


// Turn on the RX pdca, run after ssc_i2s_init() This is the new, speculative version to try to prevent L/R swap
void AK5394A_pdca_rx_enable(U32 frequency) {
	pdca_disable(PDCA_CHANNEL_SSC_RX);	// Added, always disable pdca before enabling it 
	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
	mobo_clear_adc_channel();			// Fill it with zeros

	Disable_global_interrupt();			// Or taskENTER_CRITICAL();

	pdca_init_channel(PDCA_CHANNEL_SSC_RX, &PDCA_OPTIONS);
	ADC_buf_DMA_write = 0;

	if ( (frequency == FREQ_44) || (frequency == FREQ_48) ||
		 (frequency == FREQ_88) || (frequency == FREQ_96) ||
		 (frequency == FREQ_176) || (frequency == FREQ_192) ) {
		mobo_wait_LRCK_RX_asm(); // Wait for some well-defined action on LRCK pin, asm takes 572-778ns from LRCK fall to trigger fall. C code takes 478-992ns
	}
		
	pdca_enable(PDCA_CHANNEL_SSC_RX);	// Presumably the most timing critical ref. LRCK edge
	pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
	
	Enable_global_interrupt(); // Or taskEXIT_CRITICAL();
}


// Turn on the TX pdca, run after ssc_i2s_init()
void AK5394A_pdca_tx_enable(U32 frequency) {
	pdca_disable(PDCA_CHANNEL_SSC_TX);	// Added, always disable pdca before enabling it
	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_TX);
	mobo_clear_dac_channel();			// To avoid odd spurs which some times occur

	Disable_global_interrupt();			// Or taskENTER_CRITICAL();

	pdca_init_channel(PDCA_CHANNEL_SSC_TX, &SPK_PDCA_OPTIONS);
	DAC_buf_DMA_read = 0;				// pdca_init_channel will force start from spk_buffer_0[] as NEXT buffer to use after int

	if ( (frequency == FREQ_44) || (frequency == FREQ_48) ||
		 (frequency == FREQ_88) || (frequency == FREQ_96) ||
		 (frequency == FREQ_176) || (frequency == FREQ_192) ) {
		mobo_wait_LRCK_TX_asm(); // Wait for some well-defined action on LRCK pin
	}

	// What is the optimal sequence?
   	pdca_enable(PDCA_CHANNEL_SSC_TX);
	pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_TX);

	Enable_global_interrupt(); // Or taskEXIT_CRITICAL();
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

	// Init ADC channel for SPDIF buffering, HW_GEN_FMADC turns it on in separate state machine FMADC_site
	#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC)
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
