/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * widget.h
 *
 *  Created on: 2011-03-01
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#ifndef WIDGET_H_
#define WIDGET_H_

// clear the lcd
extern void widget_display_clear(void);
// display a string on the lcd with scrolling
extern void widget_display_string_and_scroll(char *string);
// display a string on the lcd with scrolling and a delay
extern void widget_display_string_scroll_and_delay(char *string, uint16_t delay);
// test if the widget is in supervisor mode
extern int widget_is_supervisor(void);
// return a string describing the cause of the last reset
extern char *widget_reset_cause(void);
// cause the widget to reset
extern void widget_reset(void);
// cause the widget to reset and reread image default values
extern void widget_factory_reset(void);
// blink dot space code on the led's using rtc for timing
extern void widget_blink(char *dotspace);
// blink morse code on the led's using rtc for timing
extern void widget_blink_morse(char *ascii);
// initialize widget level stuff
extern void widget_init(void);
// report the startup state of the widget
extern void widget_report(void);

#endif /* WIDGET_H_ */
