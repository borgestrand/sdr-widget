#include <stdio.h>
#include <string.h>

#include "board.h"

#include "gpio.h"
//#include "delay.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "taskEXERCISE.h"
//#include "libLCD.h"
//#include "delay.h"
#include "taskLCD.h"
#include "PCF8574.h"
#include "taskAK5394A.h"


#define GPIO_PIN_EXAMPLE_3    GPIO_PUSH_BUTTON_SW2

portBASE_TYPE xStatus;
portBASE_TYPE ctr = 0;
struct dataLCD lcdQUEDATA;
int i;
uint16_t y;
U8 c;
char lcd_prt1[10];
char lcd_prt2[10];

static void vtaskEXERCISE( void * pcParameters ) {

 	int32_t x;

	/****************************
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
    lcdQUEDATA.CMD=lcdINIT;
    xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
    lcdQUEDATA.CMD=lcdPUTS;
    lcdQUEDATA.data.aString=(uint8_t *)"Display INIT OK.";
    xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
    xSemaphoreGive( mutexQueLCD );
    *****************************/
	lcd_q_init();
	lcd_q_print("Display INIT OK.");

    for( ;; )
    {
    	/******************************
    	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
		lcdQUEDATA.CMD = lcdPOSW;
	    xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );

	    lcdQUEDATA.CMD=lcdGOTO;
		lcdQUEDATA.data.scrnPOS.row = 2;
		lcdQUEDATA.data.scrnPOS.col = 1;
		xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        lcdQUEDATA.CMD=lcdPUTH;
        c = audio_buffer_1[0] >> 24;
        lcdQUEDATA.data.aChar=c;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        lcdQUEDATA.CMD=lcdPUTH;
        c = audio_buffer_1[0] >> 16;
        lcdQUEDATA.data.aChar=c;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        c = audio_buffer_1[0] >> 8;
        lcdQUEDATA.data.aChar=c;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        lcdQUEDATA.CMD=lcdPUTH;
        c = audio_buffer_1[0];
        lcdQUEDATA.data.aChar=c;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );

	    lcdQUEDATA.CMD=lcdGOTO;
		lcdQUEDATA.data.scrnPOS.row = 2;
		lcdQUEDATA.data.scrnPOS.col = 12;
		xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        lcdQUEDATA.CMD=lcdPUTH;
        c = audio_buffer_1[1] >> 24;
        lcdQUEDATA.data.aChar=c;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        lcdQUEDATA.CMD=lcdPUTH;
        c = audio_buffer_1[1] >> 16;
        lcdQUEDATA.data.aChar=c;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        c = audio_buffer_1[1] >> 8;
        lcdQUEDATA.data.aChar=c;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        lcdQUEDATA.CMD=lcdPUTH;
        c = audio_buffer_1[1];
        lcdQUEDATA.data.aChar=c;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );

        lcdQUEDATA.CMD = lcdPOSR;
        xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
        xSemaphoreGive( mutexQueLCD );
        *******************/

    	lcd_q_goto(2,0);
    	lcd_q_print("2ch:");

     	// Print a snapshot of the first channel
    	// Contains a 24bit signed integer
    	lcd_q_goto(2,5);
     	x = audio_buffer_0[0];
     	if(x>0x7FFFFF)
     	{
     		lcd_q_print("-");
     		x = 0xFFFFFF - x;
     	}
     	else  lcd_q_print(" ");
     	sprintf(lcd_prt1,"%06lx", (uint32_t)x);
     	lcd_q_print(lcd_prt1);

     	// Print a snapshot of the second channel
    	// Contains a 24bit signed integer
     	lcd_q_goto(2,13);
     	x = audio_buffer_0[1];
     	if(x>0x7FFFFF)
     	{
     		lcd_q_print("-");
     		x = 0xFFFFFF - x;
     	}
     	else  lcd_q_print(" ");
     	sprintf(lcd_prt2,"%06lx", (uint32_t)x);
     	lcd_q_print(lcd_prt2);


        vTaskDelay(1000/portTICK_RATE_MS );
    }
}



void vStartTaskEXERCISE( unsigned portBASE_TYPE uxPriority )
{
	xStatus = xTaskCreate( vtaskEXERCISE, 
                         ( signed char * ) "taskEXERCISE", 
                           configTSK_EXERCISE_STACK_SIZE,
                           NULL, 
                           uxPriority, 
                         ( xTaskHandle * ) NULL );
}

