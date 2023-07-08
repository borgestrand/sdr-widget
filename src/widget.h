/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * widget.h
 *
 *  Created on: 2011-03-01
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#ifndef WIDGET_H_
#define WIDGET_H_


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
// display a string on the lcd with scrolling
extern void widget_display_string_and_scroll(char *string);
// display a string on the lcd with scrolling and delay for microseconds
extern void widget_display_string_scroll_and_delay(char *string, unsigned delay);
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
// cause the widget to reset and reread image default values
extern void widget_factory_reset(void);
// initialize widget level stuff
extern void widget_init(void);
// mark widget ready to schedule
extern void widget_ready(char *msg);

#endif /* WIDGET_H_ */
