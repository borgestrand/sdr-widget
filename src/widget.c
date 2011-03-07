/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * widget.c
 *
 *  Created on: 2011-03-01
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "wdt.h"
#include "flashc.h"
#include "rtc.h"

#include "features.h"
#include "widget.h"
#include "taskLCD.h"
#include "Mobo_config.h"

//
// lcd display
//
static uint8_t display_row = 0;
static char display_contents[4][21];

void widget_display_clear(void) {
	int i;
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
	lcd_q_clear();
	display_row = 0;
	for (i = 0; i < 4; i += 1)
		memset(&display_contents[i][0], ' ', 20);
	xSemaphoreGive( mutexQueLCD );
}

void widget_display_string_and_scroll(char *string) {
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
#if 1
	if (display_row == 4) {
		// scroll up
		int row;
		memmove(&display_contents[0][0], &display_contents[1][0], 3*21);
		for (row = 0; row < 3; row += 1) {
			lcd_q_goto(row,0);
			lcd_q_print(&display_contents[row][0]);
		}
		display_row = 3;
	}
#else
	display_row &= 3;
#endif
	sprintf(&display_contents[display_row][0], "%-20.20s", string);
	lcd_q_goto(display_row, 0);
	lcd_q_print(&display_contents[display_row][0]);
	display_row += 1;
	xSemaphoreGive( mutexQueLCD );
}

void widget_display_string_scroll_and_delay(char *string, uint16_t delay) {
	widget_display_string_and_scroll(string);
	vTaskDelay( delay );
}

//
// test if we're in supervisor mode
//
int widget_is_supervisor(void) {
	return (Get_system_register(AVR32_SR) & (7 << 22)) != 0;
}

//
// return true if we've begun running RTOS
//
int widget_is_tasking(void) {
	return xTaskGetCurrentTaskHandle() != NULL;
}

//
// decode the reset cause
//
char *widget_reset_cause(void) {
	// how did we get here?
	if (AVR32_PM.RCAUSE.wdt) {				/* watch dog reset*/
		return "watch dog";
	} else if (AVR32_PM.RCAUSE.por) {		/* power on reset */
		return "power on";
	} else if (AVR32_PM.RCAUSE.ext) {		/* external reset */
		return "external";
	} else if (AVR32_PM.RCAUSE.bod || AVR32_PM.RCAUSE.bod33) { /* brown out */
		return "brown out";
	} else if (AVR32_PM.RCAUSE.cpuerr) {		/* cpu error */
		return "cpu error";
	} else {					/* unknown */
		return "unknown";
	}
}

//
// provide a call which generates a system reset
//
void widget_reset(void) {
	if ( ! widget_is_supervisor() ) {
		widget_blink_morse("  reset not super  ");
	} else {
		widget_blink_morse("  reset  ");
	}
#if 0
	// Enable Watchdog with 100ms patience
	// This is what works for Loftur in demo_UAC1_v087_f_WinXP
	// Doesn't work in demo_UAC2_v005_DG8SAQ, or for any of my images
	// If it generates a privilege violation, what happens?
	wdt_enable(0);
	// Wait for it to fire, blinking
	while (1) {
		widget_blink(". ..  ...   ");
	}
#elif 0
	// This is provided in compiler.h with the advice that it doesn't
	// work in user application mode.  Maybe that's our problem, we're
	// getting a privilege violation?
	Reset_CPU();
#elif 1
	Long_call(0x80000000);
#endif
}

void widget_factory_reset(void) {
	// Force an EEPROM update in the mobo config
	flashc_memset8((void *)&nvram_cdata.EEPROM_init_check, 0xFF, sizeof(uint8_t), TRUE);
	// Force an EEPROM update in the features
	flashc_memset8((void *)&features_nvram, 0, 2, TRUE);
	// reset
	widget_reset();
}

//
// blink a dot-space code: dot is on, space is off
//
#define RTC_HZ	   115000						// 115kHz Real Time Counter
#define BLINKY_WPM 15							// words per minute to blink
#define PARIS_DPW  50							// dit clocks in PARIS
#define CODEX_DPW  60							// dit clocks in CODEX

void widget_blink(char *dotspace) {
	// take the number of clocks per second, divide by the dits per second
	const int32_t clocks_per_dot = RTC_HZ / (BLINKY_WPM * PARIS_DPW / 60);
	// start off
	LED_Off(LED0); LED_Off(LED1);
	// until a nul terminator
	while (*dotspace != 0) {
		// on for dot, off for anything else
		if (*dotspace == '.') {
			LED_On(LED0); LED_On(LED1);
		} else if (*dotspace == ' ') {
			LED_Off(LED0); LED_Off(LED1);
		} else {
			break;
		}
		// increment dotspace code
		dotspace += 1;
		// count down the dot clock
		uint32_t start = rtc_get_value(&AVR32_RTC);
		while ( (rtc_get_value(&AVR32_RTC) - start) < clocks_per_dot );
	}
	LED_Off(LED0); LED_Off(LED1);
}

void widget_blink_morse(char *ascii) {
	while (*ascii != 0) {
		switch (*ascii++) {
		case ' ':           widget_blink("    "); continue;
		case 'e': case 'E': widget_blink(".   "); continue;
		case 'i': case 'I': widget_blink(". .   "); continue;
		case 't': case 'T': widget_blink("...   "); continue;
		case 'a': case 'A': widget_blink(". ...   "); continue;
		case 'n': case 'N': widget_blink("... .   "); continue;
		case 's': case 'S': widget_blink(". . .   "); continue;
		case 'd': case 'D': widget_blink("... . .   "); continue;
		case 'h': case 'H': widget_blink(". . . .   "); continue;
		case 'm': case 'M': widget_blink("... ...   "); continue;
		case 'r': case 'R': widget_blink(". ... .   "); continue;
		case 'u': case 'U': widget_blink(". . ...   "); continue;
		case 'b': case 'B': widget_blink("... . . .   "); continue;
		case 'f': case 'F': widget_blink(". . ... .   "); continue;
		case 'g': case 'G': widget_blink("... ... .   "); continue;
		case 'k': case 'K': widget_blink("... . ...   "); continue;
		case 'l': case 'L': widget_blink(". ... . .   "); continue;
		case 'v': case 'V': widget_blink(". . . ...   "); continue;
		case 'w': case 'W': widget_blink(". ... ...   "); continue;
		case 'c': case 'C': widget_blink("... . ... .   "); continue;
		case 'o': case 'O': widget_blink("... ... ...   "); continue;
		case 'p': case 'P': widget_blink(". ... ... .   "); continue;
		case 'x': case 'X': widget_blink("... . . ...   "); continue;
		case 'z': case 'Z': widget_blink("... ... . .   "); continue;
		case 'j': case 'J': widget_blink(". ... ... ...   "); continue;
		case 'q': case 'Q': widget_blink("... ... . ...   "); continue;
		case 'y': case 'Y': widget_blink("... . ... ...   "); continue;
		case '1':		    widget_blink(". ... ... ... ...   "); continue;
		case '2':		    widget_blink(". . ... ... ...   "); continue;
		case '3':		    widget_blink(". . . ... ...   "); continue;
		case '4':		    widget_blink(". . . . ...   "); continue;
		case '5':		    widget_blink(". . . . .   "); continue;
		case '6':		    widget_blink("... . . . .   "); continue;
		case '7':		    widget_blink("... ... . . .   "); continue;
		case '8':		    widget_blink("... ... ... . .   "); continue;
		case '9':		    widget_blink("... ... ... ... .   "); continue;
		case '0':		    widget_blink("... ... ... ... ...   "); continue;
		}
	}
}

void widget_init(void) {
	widget_blink_morse(" v ");
	// this returns not tasking during startup
	// if (widget_is_tasking()) widget_blink_morse(" tasking "); else widget_blink_morse(" not tasking ");
	//	widget_blink_morse("   ");
	//	widget_blink_morse(" reset from ");
	//	widget_blink_morse(widget_reset_cause());
	//	widget_blink_morse("   ");
	//	if ( ! widget_is_supervisor() )
	//		widget_blink_morse(" user");
	//	else
	//		widget_blink_morse(" super");
}

void widget_report(void) {
	char buff[32];
	widget_display_string_scroll_and_delay("widget report:", 10000);
	widget_display_string_scroll_and_delay("firmware = " FIRMWARE_VERSION, 10000);
	sprintf(buff, "reset = %s", widget_reset_cause());
	widget_display_string_scroll_and_delay(buff, 10000);
}
