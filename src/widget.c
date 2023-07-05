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
#include "gpio.h"


#include "FreeRTOS.h"
#include "task.h"

#include "features.h"
#include "widget.h"
#include "Mobo_config.h"

//
// initialization flag
//
static int initialization_count = 0;

int widget_is_initializing(void) { return initialization_count != 0; }
void widget_initialization_start(void) { initialization_count += 1; }
void widget_initialization_finish(void) { initialization_count -= 1; }

//
// startup log
//
#define STARTUP_LOG_SIZE 1024
#define STARTUP_LOG_LINES 64

static char startup_log[STARTUP_LOG_SIZE];
static char *startup_log_lines[STARTUP_LOG_LINES+2];
static char *startup_log_ptr = (char *)startup_log;
static char **startup_log_line_ptr = (char **)startup_log_lines;
static int startup_log_line_start = 1;

void widget_startup_log_char(char c) {
	// if we have room to remamber another character
	if (startup_log_ptr < (char *)startup_log + STARTUP_LOG_SIZE) {

		// if we are at the beginning of a line, remember where it starts
		if (startup_log_line_start) {
			// but only if we have room to remember another line start
			if (startup_log_line_ptr < (char **)startup_log_lines + STARTUP_LOG_LINES) {
				*startup_log_line_ptr = startup_log_ptr;
				startup_log_line_ptr += 1;
			}
			// and clear the beginning of a line flag
			startup_log_line_start = 0;
		}

		// remember the character
		*startup_log_ptr = c;
		startup_log_ptr += 1;
		// we're at the beginning of a line if this character was a NUL
		startup_log_line_start = (c == '\0');
	} else if (startup_log[STARTUP_LOG_SIZE-1] != '\0') {
		// make sure that the last line recorded has a NUL on it
		startup_log[STARTUP_LOG_SIZE-1] = '\0';
	}
}

void widget_startup_log_string(char *string) {
	while (*string)
		widget_startup_log_char(*string++);
}

void widget_startup_log_line(char *string) {
	widget_startup_log_string(string);
	widget_startup_log_char('\0');
}

void widget_get_startup_buffer_lines(char ***buffer_lines, int *lines) {
	// append a usage summary to any startup log listing
	static char log_usage[2][20];
	sprintf(log_usage[0], "log %d/%d lines", (int)(startup_log_line_ptr - (char **)startup_log_lines), STARTUP_LOG_LINES);
	sprintf(log_usage[1], "log %d/%d chars", (int)(startup_log_ptr-startup_log), STARTUP_LOG_SIZE);
	startup_log_line_ptr[0] = log_usage[0];
	startup_log_line_ptr[1] = log_usage[1];
	*buffer_lines = (char **)startup_log_lines;
	*lines = (startup_log_line_ptr - (char **)startup_log_lines) + 2;
}

//
// lcd display
// clear the display,
// display lines of text and scroll up after all four lines are filled
// ditto and delay after each line of text is displayed
//
static unsigned char display_grabbed = 0;

void widget_display_grab(void) {
	if ( ! display_grabbed) {
	}
	display_grabbed += 1;
}

void widget_display_drop(void) {
	if (display_grabbed) {
		display_grabbed -= 1;
		if ( ! display_grabbed ) {
		}
	}
}
	
void widget_display_clear(void) {
}

void widget_display_string_and_scroll(char *string) {
}

void widget_display_string_scroll_and_delay(char *string, unsigned delay) {
}


//
// test if we're in supervisor mode
// more specifically if we're not in user mode
//
int widget_is_supervisor(void) {
	return (Get_system_register(AVR32_SR) & (7 << 22)) != 0;
}

//
// return true if we've begun running FreeRTOS
//
// this depends on the fact that xTaskGetCurrentTaskHandle
// returns NULL before the scheduler starts running.
// it allows widget_delay() to switch between busy waiting
// the real time clock and calling vTaskDelay.
//
// ah, but it returns non-NULL after task creation and before
// the scheduler starts.
//
int widget_is_tasking(void) {
	return xTaskGetCurrentTaskHandle() != NULL;
}

//
// delay the current execution by us microseconds
// using vTaskDelay and the configured FreeRTOS tick rate
//
void widget_delay_task(unsigned us) {
	unsigned tick = (unsigned long long)us * configTICK_RATE_HZ / 1000000;
	vTaskDelay(tick);
}

//
// delay the current execution by us microseconds
// using the real time clock running at 115000 Hz.
//
void widget_delay_rtc(unsigned us) {
#define RTC_HZ	   115000						// 115kHz Real Time Counter
	unsigned tick = (unsigned long long)us * RTC_HZ / 1000000;
	unsigned start = rtc_get_value(&AVR32_RTC);
	while ( (rtc_get_value(&AVR32_RTC) - start) < tick );
}

//
// decode the reset cause to a string
//
char *widget_reset_cause(void) {
	if (AVR32_PM.RCAUSE.wdt)			/* watch dog timer reset*/
		return "watch dog";
	else if (AVR32_PM.RCAUSE.por)		/* power on reset */
		return "power on";
	else if (AVR32_PM.RCAUSE.ext)		/* external reset */
		return "external";
	else if (AVR32_PM.RCAUSE.bod)		/* brown out reset */
		return "brown out";
	else if (AVR32_PM.RCAUSE.cpuerr)	/* cpu error */
		return "cpu error";
	else								/* unknown */
		return "unknown";
}

//
// provide a call which generates a system reset
//
void widget_reset(void) {
	wdt_enable(5000000);	// Enable Watchdog with 500ms patience
	while (1);				// Wait for it to fire
}

//
// widget factory reset handler table
//
static widget_factory_reset_handler_t handlers[WIDGET_FACTORY_RESET_HANDLERS];

//
// register a widget factory reset handler
//
void widget_factory_reset_handler_register(widget_factory_reset_handler_t handler) {
	int i;
	for (i = 0; i < WIDGET_FACTORY_RESET_HANDLERS; i += 1)
		if (handlers[i] == NULL) {
			handlers[i] = handler;
			break;
		}
}

//
// force a factory reset, which reinitializes all nvram data to values from the
// image on next reset.
// each module which has data which can be factory reset should have registered
// a factory reset handler for us to call.
// this way we don't need to know where every module was storing its nvram and
// how to force reset it.
//
void widget_factory_reset(void) {
	// call the registered handlers
	int i;
	for (i = 0; i < WIDGET_FACTORY_RESET_HANDLERS; i += 1)
		if (handlers[i] != NULL)
			handlers[i]();
	widget_reset();				// reset
}

//
// blink a dot-space code: dot is on, space is off
//
#define BLINKY_WPM 15							// words per minute to blink
#define PARIS_DPW  50							// dit clocks in PARIS
#define CODEX_DPW  60							// dit clocks in CODEX

// LED0_GPIO - mounted led0, contended for by uac
// LED1_GPIO - mounted led1, contended for by uac
// PTT_1 - one of these three gets set eventually
// PTT_2
// PTT_3
void widget_blink(char *dotspace) {
	// take the number of clocks per second, divide by the dits per second
	const int32_t us_per_dot = 1000000 / (BLINKY_WPM * PARIS_DPW / 60);
	// start off
	LED_Off(LED0); LED_Off(LED1);
	// until a nul terminator
	while (*dotspace != 0) {
		// on for dot, off for anything else
		if (*dotspace == '.') {
			gpio_clr_gpio_pin(PTT_1); gpio_clr_gpio_pin(PTT_2); gpio_clr_gpio_pin(PTT_3);
		} else if (*dotspace == ' ') {
			gpio_set_gpio_pin(PTT_1); gpio_set_gpio_pin(PTT_2); gpio_set_gpio_pin(PTT_3);
		} else {
			break;
		}
		// increment dotspace code
		dotspace += 1;
		// count down the dot clock
		widget_delay_rtc(us_per_dot);
	}
	gpio_set_gpio_pin(PTT_1); gpio_set_gpio_pin(PTT_2); gpio_set_gpio_pin(PTT_3);
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
		case '"':			widget_blink(". ... . ... .   "); continue;
		case '\'':			widget_blink(". ... ... ... ... .   "); continue;
		case '$':			widget_blink(". . . ... . . ...   "); continue;
		case '(':		    widget_blink("... . ... ... .   "); continue;
		case ')':		    widget_blink("... . ... ... . ...   "); continue;
		case '+':		    widget_blink(". ... . ... .   "); continue;
		case ',':		    widget_blink("... ... . . ... ...   "); continue;
		case '-':		    widget_blink("... . . . . ...   "); continue;
		case '.':		    widget_blink(". ... . ... . ...   "); continue;
		case '/':			widget_blink("... . . ... .   "); continue;
		case ':':			widget_blink("... ... ... . . .   "); continue;
		case ';':			widget_blink("... . ... . ... .   "); continue;
		case '=':			widget_blink("... . . . ...   "); continue;
		case '?':			widget_blink(". . ... ... . .   "); continue;
		case '_':			widget_blink(". . ... ... . ...   "); continue;
		case '@':			widget_blink(". ... ... . ... .   "); continue;
		}
	}
}

void widget_init(void) {
	// widget_blink_morse(" v ");
	// char buffer[64];
	// strncpy(buffer,widget_reset_cause(),64);
	// buffer[3] = 0;
	// widget_blink_morse(buffer);
	// widget_blink_morse(" v ");
	
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

void widget_ready(char *msg) {
	// widget_blink_morse(msg);
}

void widget_report(void) {
	char buff[32];
	widget_startup_log_line("sdr-widget");
	widget_startup_log_line(FIRMWARE_VERSION);
	sprintf(buff, "reset = %s", widget_reset_cause());
	widget_startup_log_line(buff);
}
