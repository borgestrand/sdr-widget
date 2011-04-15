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

#include "FreeRTOS.h"
#include "task.h"

#include "features.h"
#include "widget.h"
#include "taskLCD.h"
#include "Mobo_config.h"

//
// startup log
//
#if WIDGET_LOG_ENABLED
#define STARTUP_LOG_SIZE 512
#define STARTUP_LOG_LINES 32

static char startup_log[STARTUP_LOG_SIZE];
static char *startup_log_lines[STARTUP_LOG_LINES];
static char *startup_log_ptr = (char *)startup_log;
static char **startup_log_line_ptr = (char **)startup_log_lines;
static int startup_log_line_start = 1;
#endif

void widget_startup_log_char(char c) {
#if WIDGET_LOG_ENABLED
	if (startup_log_ptr == startup_log) {
		// we could initialize by erasing the entire buffer
	}
	if (startup_log_line_start) {
		if (startup_log_line_ptr < (char **)startup_log_lines + STARTUP_LOG_LINES) {
#if 0
			flashc_memset32((void *)startup_log_line_ptr, (uint32_t)startup_log_ptr, sizeof(char *), TRUE);
#else
			*startup_log_line_ptr = startup_log_ptr;
#endif
			startup_log_line_ptr += 1;
		}
		startup_log_line_start = 0;
	}
	if (startup_log_ptr < (char *)startup_log + STARTUP_LOG_SIZE) {
#if 0
		flashc_memset8((void *)startup_log_ptr, c, sizeof(char), TRUE);
#else
		*startup_log_ptr = c;
#endif
		startup_log_ptr += 1;
	}
	startup_log_line_start = (c == '\0');
#endif
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
#if WIDGET_LOG_ENABLED
	*buffer_lines = (char **)startup_log_lines;
	*lines = *buffer_lines - startup_log_line_ptr;
#else
	static char *message[] = {
		"no log enabled"
	};
	*buffer_lines = message;
	*lines = 1;
#endif
}

void widget_free_startup_buffer_lines(char **buffer_lines) {
}

//
// lcd display
// clear the display,
// display lines of text and scroll up after all four lines are filled
// ditto and delay after each line of text is displayed
//
static unsigned char display_grabbed = 0;
static unsigned char display_row = 0;
static char display_contents[4][21];

void widget_display_grab(void) {
	if ( ! display_grabbed)
		xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
	display_grabbed += 1;
}

void widget_display_drop(void) {
	if (display_grabbed) {
		display_grabbed -= 1;
		if ( ! display_grabbed )
			xSemaphoreGive( mutexQueLCD );
	}
}
	
void widget_display_clear(void) {
	int i;
	widget_display_grab();
	lcd_q_clear();
	display_row = 0;
	for (i = 0; i < 4; i += 1)
		memset(&display_contents[i][0], ' ', 20);
	widget_display_drop();
}

void widget_display_string_and_scroll(char *string) {
	widget_display_grab();
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
	sprintf(&display_contents[display_row][0], "%-20.20s", string);
	lcd_q_goto(display_row, 0);
	lcd_q_print(&display_contents[display_row][0]);
	display_row += 1;
	widget_display_drop();
}

void widget_display_string_scroll_and_delay(char *string, unsigned delay) {
	widget_display_string_and_scroll(string);
	widget_delay_task(delay);
}

void widget_display_string_scroll_and_log(char *string) {
	widget_display_string_and_scroll(string);
	widget_startup_log_line(string);
}
//
// provide a place for disasters to be reported
// only works when taskLCD is active
// shows a message for 30 seconds
//
void widget_oops(char *message) {
	if (widget_is_tasking()) {
		widget_display_grab();
		widget_display_clear();
		widget_display_string_and_scroll("widget_oops:");
		widget_display_string_scroll_and_delay(message, 30000000);
		widget_display_drop();
	}
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
	if (i >= WIDGET_FACTORY_RESET_HANDLERS)
		widget_oops("reset table is full"); /* keep it under 20 chars */
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
	widget_display_grab();
	widget_display_string_scroll_and_log("widget report:");
	widget_display_string_scroll_and_log("firmware = " FIRMWARE_VERSION);
	sprintf(buff, "reset = %s", widget_reset_cause());
	widget_display_string_scroll_and_log(buff);
	widget_display_drop();
}
