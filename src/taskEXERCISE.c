#include <stdint.h>
#include <string.h>

#include "board.h"

#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "device_audio_task.h"
#include "taskEXERCISE.h"
#include "PCF8574.h"
#include "taskAK5394A.h"

#if LCD_DISPLAY			// Multi-line LCD display
#include "taskLCD.h"
#endif

static void vtaskEXERCISE( void * pcParameters ) {
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	  while (TRUE){

		vTaskDelayUntil(&xLastWakeTime, configTSK_EXERCISE_PERIOD);

    }
}



void vStartTaskEXERCISE( unsigned portBASE_TYPE uxPriority )
{
	xTaskCreate( vtaskEXERCISE,
               ( signed char * ) "taskEXERCISE",
                 configTSK_EXERCISE_STACK_SIZE,
                 NULL,
                 configTSK_EXERCISE_PRIORITY,
                ( xTaskHandle * ) NULL );
}

