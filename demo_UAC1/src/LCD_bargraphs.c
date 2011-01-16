/*
 * LCD_bargraphs.c
 *
 *! \brief Load the LCD with Bargraph symbols
 * and print bargraphs
 *
 * Created on: 2010-06-20
 *      Author: Loftur Jonasson, TF3LJ
 */

//**  Note: The lcdProgressBar() function is mostly swiped from the AVRLIB lcd.c/h.
//**
//**  Copy/Paste of copyright notice from AVRLIB lcd.h:
//*****************************************************************************
//
// File Name	: 'lcd.h'
// Title		: Character LCD driver for HD44780/SED1278 displays
//					(usable in mem-mapped, or I/O mode)
// Author		: Pascal Stang
// Created		: 11/22/2000
// Revised		: 4/30/2002
// Version		: 1.1
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
///	\ingroup driver_hw
/// \defgroup lcd Character LCD Driver for HD44780/SED1278-based displays (lcd.c)
/// \code #include "lcd.h" \endcode
/// \par Overview
///		This display driver provides an interface to the most common type of
///	character LCD, those based on the HD44780 or SED1278 controller chip
/// (about 90% of character LCDs use one of these chips).  The display driver
/// can interface to the display through the CPU memory bus, or directly via
/// I/O port pins.  When using the direct I/O port mode, no additional
/// interface hardware is needed except for a contrast potentiometer.
/// Supported functions include initialization, clearing, scrolling, cursor
/// positioning, text writing, and loading of custom characters or icons
/// (up to 8).  Although these displays are simple, clever use of the custom
/// characters can allow you to create animations or simple graphics.  The
/// "progress bar" function that is included in this driver is an example of
/// graphics using limited custom-chars.
///
/// \Note The driver now supports both 8-bit and 4-bit interface modes.
///
/// \Note For full text output functionality, you may wish to use the rprintf
/// functions along with this driver
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include "board.h"

#if LCD_DISPLAY			// Multi-line LCD display

#include <stdint.h>

#include "FreeRTOS.h"

#include "taskLCD.h"
#include "LCD_bargraphs.h"


/*! \brief Custom Character Definitions
 *
 */
const uint8_t LcdCustomChar[] =
{
	//
	// Five different alternatives, the fourth alternative is the original
	// bargraph alternative in the AVRLIB library.  TF3LJ - 2009-08-25
	//
	#if BARGRAPH_STYLE_1		// Used if LCD bargraph alternatives.  N8LP LP-100 look alike bargraph
	0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, // 0. 0/5 full progress block
	0x00, 0x10, 0x10, 0x15, 0x10, 0x10, 0x00, 0x00, // 1. 1/5 full progress block
	0x00, 0x18, 0x18, 0x1d, 0x18, 0x18, 0x00, 0x00, // 2. 2/5 full progress block
	0x00, 0x1c, 0x1c, 0x1d, 0x1c, 0x1c, 0x00, 0x00, // 3. 3/5 full progress block
	0x00, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x00, 0x00, // 4. 4/5 full progress block
	0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x00	// 5. 5/5 full progress block
	#elif BARGRAPH_STYLE_2		// Used if LCD bargraph alternatives.  Bargraph with level indicators
	0x01, 0x01, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00, // 0. 0/5 full progress block
	0x01, 0x01, 0x1f, 0x10, 0x10, 0x1f, 0x00, 0x00, // 1. 1/5 full progress block
	0x01, 0x01, 0x1f, 0x18, 0x18, 0x1f, 0x00, 0x00, // 2. 2/5 full progress block
	0x01, 0x01, 0x1f, 0x1C, 0x1C, 0x1f, 0x00, 0x00, // 3. 3/5 full progress block
	0x01, 0x01, 0x1f, 0x1E, 0x1E, 0x1f, 0x00, 0x00, // 4. 4/5 full progress block
	0x01, 0x01, 0x1f, 0x1F, 0x1F, 0x1f, 0x00, 0x00	// 5. 5/5 full progress block
	#elif BARGRAPH_STYLE_3		// Used if LCD bargraph alternatives.  Another bargraph with level indicators
	0x01, 0x01, 0x1f, 0x00, 0x00, 0x00, 0x1F, 0x00, // 0. 0/5 full progress block
	0x01, 0x01, 0x1f, 0x10, 0x10, 0x10, 0x1F, 0x00, // 1. 1/5 full progress block
	0x01, 0x01, 0x1f, 0x18, 0x18, 0x18, 0x1F, 0x00, // 2. 2/5 full progress block
	0x01, 0x01, 0x1f, 0x1C, 0x1C, 0x1C, 0x1F, 0x00, // 3. 3/5 full progress block
	0x01, 0x01, 0x1f, 0x1E, 0x1E, 0x1E, 0x1F, 0x00, // 4. 4/5 full progress block
	0x01, 0x01, 0x1f, 0x1F, 0x1F, 0x1F, 0x1F, 0x00	// 5. 5/5 full progress block
	#elif BARGRAPH_STYLE_4		// Used if LCD bargraph alternatives.  Original bargraph, Empty space enframed
	0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, // 0. 0/5 full progress block
	0x00, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x00, // 1. 1/5 full progress block
	0x00, 0x1F, 0x18, 0x18, 0x18, 0x18, 0x1F, 0x00, // 2. 2/5 full progress block
	0x00, 0x1F, 0x1C, 0x1C, 0x1C, 0x1C, 0x1F, 0x00, // 3. 3/5 full progress block
	0x00, 0x1F, 0x1E, 0x1E, 0x1E, 0x1E, 0x1F, 0x00, // 4. 4/5 full progress block
	0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00	// 5. 5/5 full progress block
	#elif BARGRAPH_STYLE_5		// Used if LCD bargraph alternatives.  True bargraph, Empty space is empty
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0. 0/5 full progress block
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, // 1. 1/5 full progress block
	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, // 2. 2/5 full progress block
	0x00, 0x1c, 0x1C, 0x1C, 0x1C, 0x1C, 0x1c, 0x00, // 3. 3/5 full progress block
	0x00, 0x1e, 0x1E, 0x1E, 0x1E, 0x1E, 0x1e, 0x00, // 4. 4/5 full progress block
	0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00	// 5. 5/5 full progress block
	#endif
};


/*! \brief Print Bargraphs
 *
 * \retval Nothing returned.
 */
void lcdProgressBar(uint16_t progress, uint16_t maxprogress, uint8_t length, char *string)
{
	uint8_t i;
	uint16_t pixelprogress;
	uint8_t c;

	if (progress >= maxprogress) progress = maxprogress;	// Clamp the upper bound to prevent funky readings

	// draw a progress bar displaying (progress / maxprogress)
	// starting from the current cursor position
	// with a total length of "length" characters
	// ***note, LCD chars 1-6 must be programmed as the bar characters
	// char 1 = empty ... char 6 = full

	// total pixel length of bargraph equals length*PROGRESSPIXELS_PER_CHAR;
	// pixel length of bar itself is
	pixelprogress = ((uint32_t)(progress*(length*PROGRESSPIXELS_PER_CHAR))/maxprogress);

	// print exactly "length" characters
	for(i=0; i<length; i++)
	{
		// check if this is a full block, or partial or empty
		// (u16) cast is needed to avoid sign comparison warning
		if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)+5) > pixelprogress )
		{
			// this is a partial or empty block
			if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)) > pixelprogress )
			{
				// this is an empty block
				// use space character?
				c = 1;
			}
			else
			{
				// this is a partial block
				c = pixelprogress % PROGRESSPIXELS_PER_CHAR + 1;
			}
		}
		else
		{
			// this is a full block
			c = 6;
		}

		// write character to display
		//lcd_data(c);
		string[i] = c;
	}
}


/*! \brief Load the LCD with Bargraph symbols
 *
 * \retval Nothing returned.
 */
void lcd_bargraph_init(void)
{
	uint8_t i, j, lcdCharNum, romCharNum;
	// skip the first char (\0) and load the next 6 custom characters
	for (i=0; i<6; i++)
	{
		// multiply the character index by 8
		lcdCharNum = ( (i+1)<<3 );	// each character occupies 8 bytes
		romCharNum = ( i<<3 );		// each character occupies 8 bytes

		// copy the 8 bytes into CG (character generator) RAM
		for(j=0; j<8; j++)
		{
			xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
			lcd_q_set((1<<LCD_CGRAM) | (lcdCharNum + j));
			lcd_q_write(LcdCustomChar[ romCharNum + j ]);
		    xSemaphoreGive( mutexQueLCD );
		}
	}
}

#endif
