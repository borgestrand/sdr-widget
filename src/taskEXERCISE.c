#include <stdint.h>
#include <string.h>

#include "board.h"

#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "taskEXERCISE.h"
#include "PCF8574.h"
#include "taskAK5394A.h"

#if LCD_DISPLAY			// Multi-line LCD display
#include "taskLCD.h"
#endif

//#define GPIO_PIN_EXAMPLE_3    GPIO_PUSH_BUTTON_SW2

int i;
uint16_t y;
U8 c;
char lcd_prt1[10];
char lcd_prt2[10];
char lcd_prt3[10];
char lcd_prtdb[20];

//portBASE_TYPE xStatus;
//portBASE_TYPE ctr = 0;
//struct dataLCD lcdQUEDATA;

static void vtaskEXERCISE( void * pcParameters ) {

	/****************************
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
    lcdQUEDATA.CMD=lcdINIT;
    xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
    lcdQUEDATA.CMD=lcdPUTS;
    lcdQUEDATA.data.aString=(uint8_t *)"Display INIT OK.";
    xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
    xSemaphoreGive( mutexQueLCD );
    *****************************/

	// Wait for 9 seconds
	vTaskDelay(90000);
    for( ;; )
    {
    }
}



void vStartTaskEXERCISE( unsigned portBASE_TYPE uxPriority )
{
	xTaskCreate( vtaskEXERCISE,
               ( signed char * ) "taskEXERCISE",
                 configTSK_EXERCISE_STACK_SIZE,
                 NULL,
                 uxPriority,
                ( xTaskHandle * ) NULL );
}

