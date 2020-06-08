#include "gpio.h"
#include <avr32/io.h>
#include "compiler.h"
#include <stdint.h>

//! GPIO module instance.
#define GPIO  AVR32_GPIO


int32_t getcounter(void) {
	return Get_system_register(AVR32_COUNT);
}


int32_t BSBmult(int32_t A, int32_t V) {
	int32_t X;

	asm volatile(
		"muls.d		r10, %2, %1		\n\t"	// Double multiply, r9 holds 63:31, r8 holds 32:0
		"lsr     	r10, 28			\n\t"	// r9:r8 right-shifted arithmetically by 28 so that V=0x10000000 is unity gain
		"or			%0, r10, r11 << 4	\n\t"	// Merging stuff back together into X. (Try compiling "return A >> 28;" with S64 A !)
		
		:	"=r"(X)							// One output register X=%0
		:	"r"(A), "r"(V)					// No input registers  A=%1, V=%2
		:	"r10"							// Clobber registers, r10 is the the return of the muls.d, r11 follows. Declare it as clobber??
	);


	return X;
}
/* builds
	muls.d		r10, r11, r12
	lsr     	r10, 28
	or			r12, r10, r11 << 4
*/


S32 Cmult(int32_t A, int32_t V) {
//	int64_t X = (int64_t)( (int64_t)A * (int64_t)V ) >> 28;
//	return ( (int32_t)(X >> 0) );

	return (S32)( (int64_t)( (int64_t)A * (int64_t)V ) >> 28) ;

}
/* builds
	muls.d  	r10, r11, r12
	lsr     	r12, r10, 28
	or			r12, r12, r11 << 4
*/





/*
S64 sshhiifftt(S64 A) {

return A >> 28;
//	lsr     r10, 28
//	or		r10, r10, r11 << 4
//	asr     r11, 28

}
*/






int foo(void) {
	
	// Not immediately operational... Is IO pin really indexed correctly? Does it need to be init'ed?


	#define TIMEOUT_LIM 8000;
	int timeout = 8000;

/*
	// Code to determine GPIO constants, rewrite this (2 positions!) first, compile this section, then modify asm
	volatile avr32_gpio_port_t *gpio_port = &GPIO.port[AVR32_PIN_PA04 >> 5];
	while ( (timeout != 0) && ( ((gpio_port->pvr >> (AVR32_PIN_PA04 & 0x1F)) & 1) == 0) ) {
		timeout --;
	}
*/

	// Code to determine GPIO constants, rewrite this (2 positions!) first, compile this section, then modify asm
	// GEN_DIN20 / SP_DAC02 moved to PX09
	volatile avr32_gpio_port_t *gpio_port = &GPIO.port[AVR32_PIN_PX09 >> 5];
	while ( (timeout != 0) && ( ((gpio_port->pvr >> (AVR32_PIN_PX09 & 0x1F)) & 1) == 0) ) {
		timeout --;
	}



/*
	// Determining speed at TP16 / DAC_0P / PA04 for now.
	int timeout;
	
	asm volatile(
		"ssrf	16				\n\t"	// Disable global interrupt
		"mov	%0, 	8000	\n\t"	// Load timeout
		"mov	r9,		-61440	\n\t"	// Immediate load, set up pointer to PA04, (0xFFFF1000) recompile C for other IO pin, do once
		"mov	r9,		-61184	\n\t"	// Immediate load, set up pointer to PB11, (0XFFFF1100) recompile C for other IO pin, do once
		
		"L0:					\n\t"	// Loop while PA04 is 1
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	L0_done			\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L0				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L0_done:				\n\t"

		"L1:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	L1_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L1				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L1_done:				\n\t"

		"mov	%0, 	8000	\n\t"	// Restart countdon for actual timing of below half-cycles

		"L3:					\n\t"	// Loop while PBA04 is 1
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	L3_done			\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L3				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L3_done:				\n\t"

		"L4:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	L4_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L4				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L4_done:				\n\t"
		"rjmp	RETURN__		\n\t"
		
		"COUNTD:				\n\t"	// Countdown reached, %0 is 0

		"RETURN__:				\n\t"
		"csrf	16				\n\t"	// Enable global interrupt
		:	"=r" (timeout)				// One output register
		:								// No input registers
		:	"r8", "r9"					// Clobber registers, pushed/popped unless assigned by GCC as temps
	);
*/

	return timeout;
}



/* Drafting a sample rate detector. Compile with something like this:
 * http://www.delorie.com/djgpp/v2faq/faq8_20.html
 * gives:
 * avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -c -g -O2 -Wa,-a,-ad srd_test.c > srd_test.lst
 *
 * Alternatively:
 * avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -S -fverbose-asm -g -O2 srd_test.c
 * avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -S -g -O2 srd_test.c
 *
 * A good asm syntax list:
 * http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
 */
