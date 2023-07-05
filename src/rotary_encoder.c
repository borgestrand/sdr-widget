/*
 * rotary_encoder.c
 *
 * \brief Rotary Encoder driven Dual Speed VFO and Menu selector, driven by
 * two Interrupt inputs, NMI and EXT_INT7.  Three encoder outputs provided
 * in the form of global (voaltile) values:
 *
 * (int32_t) freq_delta_from_enc is an incremental value to be added directly to
 * the uint32_t value representing the Si570 frequency.  One revolution of the
 * Encoder corresponds to an increment of 4kHz (Si570 frequency is divided by 4).
 * If the Encoder is turned for more than 1/5th of a turn at an angular speed
 * greater than defined in rotary_encoder.h, then the faster rate of the dual
 * speed function kicks in, and each revolution of the Encoder results in an
 * increment of 100 times the normal rate.  This function becomes inactive in
 * 1/3 of a second, or if briefly turned in the opposite direction, as defined
 * in rotary_encoder.h.
 * Associated with freq_delta_from_enc, there is also the (volatile) bool
 * FRQ_fromenc, indicating whether a new value is available as a result of turning
 * the encoder.
 *
 * (int8_t) menu_steps_from_enc is an incremental value which increases by 10 for
 * each turn of the Rotary Encoder.  This value is used by the Menu function in
 * taskPushButtonMenu.c.
 * Associated menu_steps_from_enc, there is also the (volatile) bool MENU_fromenc,
 * indicating whether a new value is available as a result of turning the encoder.
 *
 * (int16_t) val_steps_from_enc is an incremental value which increases by 100 for
 * each turn of the Rotary Encoder.  This value is used by the Menu function in
 * taskPushButtonMenu.c
 * Associated val_steps_from_enc, there is also the (volatile) bool VALUE_fromenc,
 * indicating whether a new value is available as a result of turning the encoder.
 *
 *  Created on: 2010-06-10
 *      Author: Loftur Jonasson, TF3LJ
 *
 * Note:  software_framework\services\freertos\source\include\portable\gcc\avr32_uc3\exception.x
 *        was modified as follows:
 *
 *        _handle_NMI:
 *
 *       // commented out: rjmp $
 *       // and replaced with the below
 *       lda.w   pc, eic_nmi_handler
 */

#include <stdint.h>
#include <avr32/io.h>
#include "compiler.h"
#include "gpio.h"
#include "pm.h"
#include "board.h"
#include "eic.h"
#include "intc.h"
#include "rtc.h"

#include "rotary_encoder.h"

#include "Mobo_config.h"	// cdata.Resolvable_States cdata.VFO_resolution

	//--------------------------------------------------------
	// RTC initialized as follows in a parental function
	// Initialize Real Time Counter
	//
	//rtc_init(&AVR32_RTC, RTC_OSC_RC, 0);	// RC clock at 115kHz
	//rtc_disable_interrupt(&AVR32_RTC);
	//rtc_set_top_value(&AVR32_RTC, 1150000);	// Counter reset once per 10 seconds
	//rtc_enable(&AVR32_RTC);
	//--------------------------------------------------------

// Add up frequency delta from Rotary Encoder, parental function adds the
// delta to the current running frequency
volatile int32_t 	freq_delta_from_enc = 0;

// Flag: New frequency from Encoder
volatile bool		FRQ_fromenc = FALSE;

// Add up delta from Rotary Encoder, 10 steps per revolution, for use by Menu function
volatile int8_t 	menu_steps_from_enc = 0;

// Flag: New Menu selection from Encoder
volatile bool		MENU_fromenc = FALSE;
volatile bool		prev_MENU_fromenc = FALSE;	// Internal to function

// Add up delta from Rotary Encoder, 100 steps per revolution, for use by Menu function
volatile int16_t	val_steps_from_enc = 0;

// Flag: New Value available from Encoder
volatile bool		VAL_fromenc = FALSE;
volatile bool		prev_VAL_fromenc = FALSE;	// Internal to function

// Used internally, by Rotary Encoder functions
volatile int16_t	steps = 0;

// Structure holding the configuration parameters of the EIC module.
eic_options_t 		eic_nmi_options, eic_inth1_options;


/*!
 * \brief Dual Speed function for the Rotary Encoder VFO
 */
int32_t encoder_dual_speed(int8_t direction)
{
	static bool		ENC_fast=FALSE;				// Encoder FAST mode enabled
	static bool		ENC_dir;					// Encoder Direction Change

	uint32_t		enc_time;					// Read and use time from Real Time Clock (RTC, 115kHz)
	static uint32_t	enc_last;					// Measure the time elapsed since the last encoder pulse

	static uint32_t	fast_counter=0;				// Number of rapid movements in succession
	static int8_t	last_direction;				// Direction of last encoder pulse

	uint32_t 		enc_fast_sense;				// Maximum time threshold (in steps of 1/RTC s) per click
												// to enable fast mode (1/115000 s)
	uint32_t 		enc_fast_trig;				// Number of fast clicks to enable fast Mode

	int32_t			speed;						// Speed Multiplier

	// Update these parameters if and when necessary by USB Command

	// Minimum angular speed for fast mode
	enc_fast_sense = ENC_FAST_REV/cdata.Resolvable_States;
	// Minimum travel angle for fast mode
	enc_fast_trig = cdata.Resolvable_States * ENC_FAST_ANGLE;

	// Measure the time since last encoder activity in units of appr 1/(RTC = 115kHz) seconds
	// Current RTC time
	enc_time = rtc_get_value(&AVR32_RTC);

	// Timer overrun, it is code efficient to do nothing
	if (enc_last > enc_time);
	// Inactivity for a long time, drop out of Fast mode
	else if ((enc_time-enc_last) > (ENC_FAST_PATIENCE*(ENC_RTC_RATE/10)))
	{
		ENC_fast = FALSE;
		fast_counter = 0;
	}
	// Fast movement detected
	else if (enc_fast_sense >= (enc_time-enc_last)) fast_counter++;

	// Store for next time measurement
	enc_last = enc_time;

	// We have the required number of fast movements, enable FAST mode
	if (fast_counter > enc_fast_trig) ENC_fast = TRUE;

	// If direction has changed, force a drop out of FAST mode
	if (last_direction != direction)
	{
		if (ENC_dir) ENC_dir = FALSE;	// Previous change was just a one shot event, clear flag
		else ENC_dir = TRUE;			// This is the first event in a new direction, set flag
	}
	else if (ENC_dir)					// Second shot with different direction,
	{									// now we take action and drop out of fast mode
		ENC_fast = FALSE;
		ENC_dir = FALSE;
		fast_counter = 0;
	}
	last_direction = direction;			// Save encoder direction for next time

	speed = 1;		// When fast mode, multiply speed by the set MULTIPLY factor
	if (ENC_fast) 	speed = speed * ENC_FAST_MULTIPLY;

	return speed;
}


/*!
 * \brief Interrupt handler, ROTQ input from Rotary Encoder
 * Uses the NMI (pin PA20)
 *
 * \note This function is not static because it is referenced outside
 * (in exception.S for GCC and in exception.s82 for IAR).
 */
#if __GNUC__
__attribute__((__naked__))
#elif __ICCAVR32__
#pragma shadow_registers = full
#endif
void eic_nmi_handler( void )
{
	//--------------------------------------------------------
	// Push registers and make ready for NMI handling
	__asm__ __volatile__
	(
		"pushm   r0-r12, lr\n\t"
	);
	//--------------------------------------------------------

	int8_t	increment = 0;				// Encoder direction
	bool 	rotq, roti;					// Encoder input states

	// Get current state of Encoder Pins
	rotq = gpio_get_pin_value(ENCODER_ROTQ_PIN);
	roti = gpio_get_pin_value(ENCODER_ROTI_PIN);

	//-------------------------------------------------------
	// Ugly kludge to double the interrupt resolution,
	// Flip the Rising/Falling Edge sense, depending on state
	if (rotq)
	{
		eic_nmi_options.eic_edge  = EIC_EDGE_FALLING_EDGE;
		// gpio_clr_gpio_pin(LED0_GPIO);
	}
	else
	{
		eic_nmi_options.eic_edge  = EIC_EDGE_RISING_EDGE;
		// gpio_set_gpio_pin(LED0_GPIO);
	}
	eic_clear_interrupt_line(&AVR32_EIC, EXT_NMI);
	// Flip the Interrupt Edge
	eic_init(&AVR32_EIC, &eic_nmi_options,1);
	//--------------------------------------------------------

	// Encoder has generated a pulse
	// check the relative phase of the input channels
	// and update position accordingly
	if (rotq == roti)
	{
		increment++;							// Increment
	}
	else
	{
		increment--;							// Decrement
	}

	// Add or subtract Menu selection, hardcoded 10 and 100 steps per revolution
	if(prev_MENU_fromenc && !MENU_fromenc)		// Values have been read, clear
	{
		steps = 0;
		prev_MENU_fromenc = FALSE;
	}
	else if(prev_VAL_fromenc && !VAL_fromenc)	// Values have been read, clear
	{
		steps = 0;
		prev_VAL_fromenc = FALSE;
	}
	#if	ENCODER_DIR_REVERSE
	steps = steps - increment
	#else
	steps = steps + increment;
	#endif
	menu_steps_from_enc = steps / (ENC_PULSES/10);
	val_steps_from_enc = steps / (ENC_PULSES/100);
	if (menu_steps_from_enc)
	{
		MENU_fromenc = TRUE;
		prev_MENU_fromenc = TRUE;
	}
	if (val_steps_from_enc)
	{
		VAL_fromenc = TRUE;
		prev_VAL_fromenc = TRUE;
	}

	// Dual Speed function based on measured angular speed and direction of VFO
	increment = increment * encoder_dual_speed(increment);

	// Add or subtract VFO frequency
	#if	ENCODER_DIR_REVERSE
	freq_delta_from_enc -= increment*(ENC_RESOLUTION/cdata.Resolvable_States)*cdata.VFO_resolution;
	#else
	freq_delta_from_enc += increment*(ENC_RESOLUTION/cdata.Resolvable_States)*cdata.VFO_resolution;
	#endif

	FRQ_fromenc = TRUE;						// Frequency was modified

	//--------------------------------------------------------
	// 	Restore registers and return from NMI
	__asm__ __volatile__
	(
		"popm   r0-r12, lr\n\t"
		"rete"
	);
	//--------------------------------------------------------
}


/*!
 * \brief Interrupt handler, ROTI input from Rotary Encoder
 * Uses EXT_INT7 (pin PA13)
 */
#if __GNUC__
__attribute__((__interrupt__))
#elif __ICCAVR32__
__interrupt
#endif
static void eic_int_handler1(void)
{
	int8_t	increment = 0;				// Encoder direction
	bool rotq, roti;					// Encoder input states

	// Get current state of Encoder Pins
	rotq = gpio_get_pin_value(ENCODER_ROTQ_PIN);
	roti = gpio_get_pin_value(ENCODER_ROTI_PIN);

	//-------------------------------------------------------
	// Ugly kludge to double the interrupt resolution,
	// Flip the Rising/Falling Edge sense, depending on state
	if (roti)
	{
		eic_inth1_options.eic_edge  = EIC_EDGE_FALLING_EDGE;
		// gpio_clr_gpio_pin(LED1_GPIO);
	}
	else
	{
		eic_inth1_options.eic_edge  = EIC_EDGE_RISING_EDGE;
		// gpio_set_gpio_pin(LED1_GPIO);
	}
	eic_clear_interrupt_line(&AVR32_EIC, EXT_INT7);
	// Flip the Interrupt Edge
	eic_init(&AVR32_EIC, &eic_inth1_options,1);
	//-------------------------------------------------------

	// Encoder has generated a pulse
	// check the relative phase of the input channels
	// and update position accordingly
	if (rotq == roti)
	{
		increment--;							// Decrement
	}
	else
	{
		increment++;							// Increment
	}

	// Add or subtract Menu selection, hardcoded 10 and 100 steps per revolution
	if(prev_MENU_fromenc && !MENU_fromenc)		// Values have been read, clear
	{
		steps = 0;
		prev_MENU_fromenc = FALSE;
	}
	else if(prev_VAL_fromenc && !VAL_fromenc)	// Values have been read, clear
	{
		steps = 0;
		prev_VAL_fromenc = FALSE;
	}
	#if	ENCODER_DIR_REVERSE
	steps = steps - increment
	#else
	steps = steps + increment;
	#endif
	menu_steps_from_enc = steps / (ENC_PULSES/10);
	val_steps_from_enc = steps / (ENC_PULSES/100);
	if (menu_steps_from_enc)
	{
		MENU_fromenc = TRUE;
		prev_MENU_fromenc = TRUE;
	}
	if (val_steps_from_enc)
	{
		VAL_fromenc = TRUE;
		prev_VAL_fromenc = TRUE;
	}

	// Dual Speed function based on measured angular speed and direction of VFO
	increment = increment * encoder_dual_speed(increment);

	// Add or subtract VFO frequency
	#if	ENCODER_DIR_REVERSE
	freq_delta_from_enc -= increment*(ENC_RESOLUTION/cdata.Resolvable_States)*cdata.VFO_resolution;

	#else
	freq_delta_from_enc += increment*(ENC_RESOLUTION/cdata.Resolvable_States)*cdata.VFO_resolution;
	#endif

	FRQ_fromenc = TRUE;						// Frequency was modified
}


/*!
 * \brief Interrupt handler initialize for the Rotary Encoder
 * Initialises NMI (PA20) and EXT_INT7 (PA13)
 */
void encoder_init(void)
{
	// Disable all interrupts.
	Disable_global_interrupt();
	// Initialize interrupt vectors.
	//** Breaks the code, unnecessary - done elsewhere: INTC_init_interrupts();

	// Set up the NMI vector for ROTQ
	// Enable Pin Pullup
	gpio_enable_pin_pull_up(ENCODER_ROTQ_PIN);
	// Enable edge-triggered interrupt.
	eic_nmi_options.eic_mode   = EIC_MODE_EDGE_TRIGGERED;
	// Interrupt will trigger on falling edge.
	eic_nmi_options.eic_edge   = EIC_EDGE_FALLING_EDGE;
	// Initialize in synchronous mode : interrupt is synchronized to the clock
	eic_nmi_options.eic_async  = EIC_SYNCH_MODE;
	// Set the interrupt line number.
	eic_nmi_options.eic_line   = EXT_NMI;
	eic_nmi_options.eic_filter = EIC_FILTER_ENABLED;
	// Map the interrupt line to the GPIO pin with the appropriate peripheral function.
	gpio_enable_module_pin(ENCODER_ROTQ_PIN,ENCODER_ROTQ_FUNCTION);
	// Since the NMI is not an interrupt but an exception managed by the CPU, we have
	// to make sure that the NMI handler calls our handler: this is done in the
	// files exception.S(for GCC) & exception.s82(for IAR); look for the _handle_NMI
	// assembly label. See "Note" at top of this file
	// Init the EIC controller with the NMI options
	eic_init(&AVR32_EIC, &eic_nmi_options,1);
	// Enable the EXT_NMI line and its corresponding interrupt feature.
	eic_enable_line(&AVR32_EIC, eic_nmi_options.eic_line);
	eic_enable_interrupt_line(&AVR32_EIC, eic_nmi_options.eic_line);

	// Set up the EXT_INT7 vector for ROTI
	// Enable Pin Pullup
	gpio_enable_pin_pull_up(ENCODER_ROTI_PIN);
	// Enable edge-triggered interrupt.
	eic_inth1_options.eic_mode  = EIC_MODE_EDGE_TRIGGERED;
	// Interrupt will trigger on falling edge.
	eic_inth1_options.eic_edge  = EIC_EDGE_FALLING_EDGE;
	// Initialize in synchronous mode : interrupt is synchronized to the clock
	eic_inth1_options.eic_async = EIC_SYNCH_MODE;
	// Set the interrupt line number.
	eic_inth1_options.eic_line  = EXT_INT7;
	eic_inth1_options.eic_filter= EIC_FILTER_ENABLED;
	// Map the interrupt line to the GPIO pin with the appropriate peripheral function.
	gpio_enable_module_pin(ENCODER_ROTI_PIN,ENCODER_ROTI_FUNCTION);
	// Register the interrupt handler for ROTI (EXT_INT7)
	INTC_register_interrupt(&eic_int_handler1, ENCODER_ROTI_IRQ, AVR32_INTC_INT0);
	// Init the EIC controller with the EXT_INT7 options
	eic_init(&AVR32_EIC, &eic_inth1_options,1);
	// Enable the EXT_INT7 line and its corresponding interrupt feature.
	eic_enable_line(&AVR32_EIC, eic_inth1_options.eic_line);
	eic_enable_interrupt_line(&AVR32_EIC, eic_inth1_options.eic_line);

	// Enable all interrupts.
	Enable_global_interrupt();
}
