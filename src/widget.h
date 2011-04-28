/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * widget.h
 *
 *  Created on: 2011-03-01
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#ifndef WIDGET_H_
#define WIDGET_H_

// the type of a factory reset handler,
// a pointer to a function which takes no arguments and returns void
typedef void (*widget_factory_reset_handler_t)(void);

// the number of factory reset handlers allowed
// there are 2 known at present: features and moboConfig
#define WIDGET_FACTORY_RESET_HANDLERS	4

// widget is in initialization
extern int widget_is_initializing(void);
// widget module is starting initialization
extern void widget_initialization_start(void);
// widget module is finishing initialization
extern void widget_initialization_finish(void);
// append a character to the startup log
extern void widget_startup_log_char(char c);
// append a string to the startup log
extern void widget_startup_log_string(char *string);
// append a string and a newline to the startup log
extern void widget_startup_log_line(char *string);
// parse the startup log into a lines array
extern void widget_get_startup_buffer_lines(char ***buffer_lines, int *lines);
// seize control of the LCD
extern void widget_display_grab(void);
// release control of the LCD
extern void widget_display_drop(void);
// clear the lcd
extern void widget_display_clear(void);
// display a string on the lcd with scrolling
extern void widget_display_string_and_scroll(char *string);
// display a string on the lcd with scrolling and delay for microseconds
extern void widget_display_string_scroll_and_delay(char *string, unsigned delay);
// display a string on the lcd with scrolling and log as a line
extern void widget_display_string_scroll_and_log(char *string);
// report a serious problem, somehow
extern void widget_oops(char *message);
// test if the widget is in supervisor mode
extern int widget_is_supervisor(void);
// return true if widget appears to be in tasking mode (can be wrong)
extern int widget_is_tasking(void);
// dealy by vTaskDelay for microseconds
extern void widget_delay_task(unsigned delay);
// delay by rtc for microseconds
extern void widget_delay_rtc(unsigned delay);
// return a string describing the cause of the last reset
extern char *widget_reset_cause(void);
// cause the widget to reset
extern void widget_reset(void);
// register a factory reset handler
extern void widget_factory_reset_handler_register(widget_factory_reset_handler_t handler);
// cause the widget to reset and reread image default values
extern void widget_factory_reset(void);
// blink dot space code on the led's using rtc for timing
extern void widget_blink(char *dotspace);
// blink morse code on the led's using rtc for timing
extern void widget_blink_morse(char *ascii);
// initialize widget level stuff
extern void widget_init(void);
// mark widget ready to schedule
extern void widget_ready(char *msg);
// report the startup state of the widget
extern void widget_report(void);

#endif /* WIDGET_H_ */
