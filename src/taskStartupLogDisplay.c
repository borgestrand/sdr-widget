/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * taskStartupLogDisplay.c
 *
 * \brief This task displays the startup log line by line
 * with a specified delay after each line, and a 5x delay
 * at the end of the log.  This allows the startup code to
 * run without additional delays but permits the user to
 * see the sequence of startup event reports at a readable
 * rate.
 *
 *  Created on: 2011-04-26
 *      Author: Roger E Critchlow Jr, AD5DZ
 */


#include "board.h"

#include "FreeRTOS.h"
#include "task.h"

#include "widget.h"
#include "features.h"
#include "Mobo_config.h"

/*! \brief scroll through a line of startup and go to sleep
 *
 * \retval none
 */
static void vtaskStartupLogDisplay( void * pcParameters )
{

	widget_initialization_start();
	if ( ! FEATURE_LOG_NONE ) {
		int current_line = -1;
		unsigned startup_log_delay;

		if ( FEATURE_LOG_125MS )
			startup_log_delay = 125L * configTICK_RATE_HZ / 1000;
		if ( FEATURE_LOG_1SEC )
			startup_log_delay = 1 * configTICK_RATE_HZ;
		else if ( FEATURE_LOG_2SEC )
			startup_log_delay = 2 * configTICK_RATE_HZ;
		else if ( FEATURE_LOG_4SEC )
			startup_log_delay = 4 * configTICK_RATE_HZ;
		else
			startup_log_delay = 1 * configTICK_RATE_HZ;

		widget_display_clear();

		vTaskDelay( startup_log_delay );

		MENU_mode = TRUE;			// Grab LCD from Mobo tasks

		while( 1 ) {
			char **buffer_lines;
			int lines;
    
			current_line += 1;

			// refetch the startup log each time, since it may get
			// longer while we're scrolling through it
			widget_get_startup_buffer_lines(&buffer_lines, &lines);

			// if we're still working our way through the log,
			// display the next line, otherwise pause for 2 extra
			// delay periods at the end of the log
			if (current_line < lines) {
				widget_display_string_and_scroll(buffer_lines[current_line]);
			} else if (current_line < lines + 2) {
				;
			} else {
				TX_state = TRUE;	// Force Mobo LCD housekeeping
				MENU_mode = FALSE;	// return LCD to Mobo tasks
				break;
			}

			// Delay as specified by the feature above
			vTaskDelay( startup_log_delay );
		}
	}
	widget_initialization_finish();

	// delete ourself from the RTOS scheduler
	vTaskDelete(NULL);

	// just loop when we're done, we're not allowed to return
	// but it's not clear what we can do after deleting ourself
	while( 1 ) {
		vTaskDelay( 60 * configTICK_RATE_HZ );
	}
  
}


/*! \brief RTOS initialisation of the StartupLogDisplay task
 *
 * \retval none
 */
void vStartTaskStartupLogDisplay(void)
{
	xTaskCreate( vtaskStartupLogDisplay,
				 configTSK_LOGDISPLAY_NAME,
				 configTSK_LOGDISPLAY_STACK_SIZE,
				 NULL, 
				 configTSK_LOGDISPLAY_PRIORITY,
				 (xTaskHandle *)NULL );
}
