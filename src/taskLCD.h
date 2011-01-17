#ifndef __TASKLCD_H__
#define __TASKLCD_H__

#include <stdint.h>

#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define lcd_e_high()    gpio_set_gpio_pin(LCD_E)
#define lcd_e_low()     gpio_clr_gpio_pin(LCD_E)
#define lcd_e_toggle()  ___toggle_e()

#define lcd_rw_high()   gpio_set_gpio_pin(LCD_RW)
#define lcd_rw_low()    gpio_clr_gpio_pin(LCD_RW)

#define lcd_rs_high()   gpio_set_gpio_pin(LCD_RS)
#define lcd_rs_low()    gpio_clr_gpio_pin(LCD_RS)


enum cmdLCD { lcdINIT=1, lcdHOME, lcdCLEAR, lcdWRITE, lcdGOTO, lcdPUTC,
	lcdCRLF, lcdPUTH, lcdPUTS, lcdSET, lcdPOSW, lcdPOSR };

struct dataLCD {
    uint8_t CMD;             // Commmand to be processed by task
    xQueueHandle replyQUEUE; // optional: Queue for returning info.
    union u_tag {
        uint8_t aChar;
        uint8_t rawBYTE;
        uint8_t * aString;    // Null terminated string
        struct {
            uint8_t row;
            uint8_t col;
        } scrnPOS;
    } data;
};


//volatile uint8_t position;
//uint8_t position_saved;

extern portBASE_TYPE xStatus;
extern xQueueHandle lcdCMDQUE;
extern struct dataLCD lcdQUEDATA;
extern xQueueHandle lcdCMDQUE;
extern xSemaphoreHandle mutexQueLCD;
extern void vStartTaskLCD( void );

extern void lcd_q_init(void);
extern void lcd_q_clear(void);
extern void lcd_q_crlf(void);
extern void lcd_q_goto(uint8_t row, uint8_t col);
extern void lcd_q_write(char ch);
extern void lcd_q_putc(char ch);
extern void lcd_q_puth(uint8_t hex);
extern void lcd_q_print(char *string);
extern void lcd_q_set(uint8_t cmd);

#define DATA_REGISTER       0
#define COMMAND_REGISTER    1

/**
 *  @name Definitions for LCD command instructions
 *  The constants define the various LCD controller instructions which can be passed to the 
 *  function lcd_command(), see HD44780 data sheet for a complete description.
 */

/* instruction register bit positions, see HD44780U data sheet */
#define LCD_CLR               0      /* DB0: clear display                  */
#define LCD_HOME              1      /* DB1: return to home position        */
#define LCD_ENTRY_MODE        2      /* DB2: set entry mode                 */
#define LCD_ENTRY_INC         1      /*   DB1: 1=increment, 0=decrement     */
#define LCD_ENTRY_SHIFT       0      /*   DB2: 1=display shift on           */
#define LCD_ON                3      /* DB3: turn lcd/cursor on             */
#define LCD_ON_DISPLAY        2      /*   DB2: turn display on              */
#define LCD_ON_CURSOR         1      /*   DB1: turn cursor on               */
#define LCD_ON_BLINK          0      /*     DB0: blinking cursor ?          */
#define LCD_MOVE              4      /* DB4: move cursor/display            */
#define LCD_MOVE_DISP         3      /*   DB3: move display (0-> cursor) ?  */
#define LCD_MOVE_RIGHT        2      /*   DB2: move right (0-> left) ?      */
#define LCD_FUNCTION          5      /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     4      /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_FUNCTION_2LINES   3      /*   DB3: two lines (0->one line)      */
#define LCD_FUNCTION_10DOTS   2      /*   DB2: 5x10 font (0->5x7 font)      */
#define LCD_CGRAM             6      /* DB6: set CG RAM address             */
#define LCD_DDRAM             7      /* DB7: set DD RAM address             */
#define LCD_BUSY              7      /* DB7: LCD is busy                    */

/* set entry mode: display shift on/off, dec/inc cursor move direction */
#define LCD_ENTRY_DEC            0x04   /* display shift off, dec cursor move dir */
#define LCD_ENTRY_DEC_SHIFT      0x05   /* display shift on,  dec cursor move dir */
#define LCD_ENTRY_INC_           0x06   /* display shift off, inc cursor move dir */
#define LCD_ENTRY_INC_SHIFT      0x07   /* display shift on,  inc cursor move dir */

/* display on/off, cursor on/off, blinking char at cursor position */
#define LCD_DISP_OFF             0x08   /* display off                            */
#define LCD_DISP_ON              0x0C   /* display on, cursor off                 */
#define LCD_DISP_ON_BLINK        0x0D   /* display on, cursor off, blink char     */
#define LCD_DISP_ON_CURSOR       0x0E   /* display on, cursor on                  */
#define LCD_DISP_ON_CURSOR_BLINK 0x0F   /* display on, cursor on, blink char      */

/* move cursor/shift display */
#define LCD_MOVE_CURSOR_LEFT     0x10   /* move cursor left  (decrement)          */
#define LCD_MOVE_CURSOR_RIGHT    0x14   /* move cursor right (increment)          */
#define LCD_MOVE_DISP_LEFT       0x18   /* shift display left                     */
#define LCD_MOVE_DISP_RIGHT      0x1C   /* shift display right                    */


#endif
