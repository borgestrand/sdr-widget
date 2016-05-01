/*
 * LCD_bargraphs.h
 *
 *  Created on: 2010-06-20
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef LCD_BARGRAPHS_H_
#define LCD_BARGRAPHS_H_

// Select which bargraph style to use in the Custom Character table, see LCD_bargraphs.c
#define BARGRAPH_STYLE_1		1

// DEFS for the LCD Bargrap Symbols
#define LCDCHAR_PROGRESS05		0	// 0/5 full progress block
#define LCDCHAR_PROGRESS15		1	// 1/5 full progress block
#define LCDCHAR_PROGRESS25		2	// 2/5 full progress block
#define LCDCHAR_PROGRESS35		3	// 3/5 full progress block
#define LCDCHAR_PROGRESS45		4	// 4/5 full progress block
#define LCDCHAR_PROGRESS55		5	// 5/5 full progress block

// progress bar defines
#define PROGRESSPIXELS_PER_CHAR	6

extern void lcdProgressBar(uint16_t progress, uint16_t maxprogress, uint8_t length, char *string);
extern void lcd_bargraph_init(void);

#endif /* LCD_BARGRAPHS_H_ */
