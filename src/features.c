/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * features.h
 *
 *  Created on: 2011-02-11
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "flashc.h"

#include "features.h"
#include "taskLCD.h"

// Set up NVRAM (EEPROM) storage
#if defined (__GNUC__)
__attribute__((__section__(".userpage")))
#endif
features_t features_nvram;

features_t features = { FEATURES_DEFAULT };

//
// these arrays of names need to be kept in sync
// with the enumerations defined in features.h
//
const char *feature_value_names[] = { FEATURE_VALUE_NAMES };

const char *feature_index_names[] = { FEATURE_INDEX_NAMES };

//
static uint8_t display_row = 0;
static char display_contents[4][21];

static void display_clear() {
	int i;
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
	lcd_q_clear();
	display_row = 0;
	for (i = 0; i < 4; i += 1)
		memset(&display_contents[i][0], ' ', 20);
	xSemaphoreGive( mutexQueLCD );
}

static void display_string_and_scroll(char *string) {
	xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
#if 1
	if (display_row == 4) {
		// scroll up
		int row;
		memmove(&display_contents[0][0], &display_contents[1][0], 3*21);
		for (row = 0; row < 3; row += 1) {
			lcd_q_goto(row,0);
			lcd_q_print(&display_contents[row][0]);
		}
		display_row = 3;
	}
#else
	display_row &= 3;
#endif
	sprintf(&display_contents[display_row][0], "%-20.20s", string);
	lcd_q_goto(display_row, 0);
	lcd_q_print(&display_contents[display_row][0]);
	display_row += 1;
	xSemaphoreGive( mutexQueLCD );
}

static void display_string_scroll_and_delay(char *string, uint16_t delay) {
	display_string_and_scroll(string);
	vTaskDelay( delay );
}

//
void features_init() {
  // Enforce "Factory default settings" when a mismatch is detected between the
  // checksum in the memory copy and the matching number in the NVRAM storage.
  // This can be the result of either a fresh firmware upload, or cmd 0x41 with data 0xff
  if( FEATURE_MAJOR != FEATURE_MAJOR_NVRAM || FEATURE_MINOR != FEATURE_MINOR_NVRAM ) {
	  flashc_memcpy((void *)&features_nvram, &features, sizeof(features), TRUE);
  } else {
	  memcpy(&features, &features_nvram, sizeof(features));
  }
}

void features_display(char *title, features_t fp, int delay) {
	int i;
	char buff[32];
	display_string_scroll_and_delay(title, delay);
	sprintf(buff, "%s = %u.%u", "version", fp[feature_major_index], fp[feature_minor_index]);
	display_string_scroll_and_delay(buff, delay);
	for (i = feature_image_index; i < feature_end_index; i += 1) {
		strcpy(buff, feature_index_names[i]);
		strcat(buff, " = ");
		if (features[i] < feature_end_values)
			strcat(buff, (char *)feature_value_names[fp[i]]);
		else
			strcat(buff, "invalid!");
		display_string_scroll_and_delay(buff, delay);
	}
}

void features_display_all() {
	display_clear();
	features_display("features ram:", features, 10000);
	// features_display("features nvram:", features_nvram, 10000);
}

uint8_t feature_set(uint8_t index, uint8_t value) {
	return index > feature_minor_index && index < feature_end_index && value < feature_end_values ?
		features[index] = value :
		0xFF;
}

uint8_t feature_get(uint8_t index) {
	return index < feature_end_index ? features[index] : 0xFF;
}

uint8_t feature_set_nvram(uint8_t index, uint8_t value)  {
	if ( index > feature_minor_index && index < feature_end_index && value < feature_end_values ) {
		flashc_memset8((void *)&features_nvram[index], value, sizeof(uint8_t), TRUE);
		return features_nvram[index];
	} else
		return 0xFF;
}

uint8_t feature_get_nvram(uint8_t index)  {
	return index < feature_end_index ? features_nvram[index] : 0xFF;
}

