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
#include "widget.h"

// Set up NVRAM (EEPROM) storage
#if defined (__GNUC__)
__attribute__((__section__(".userpage")))
#endif
features_t features_nvram;

features_t features = { FEATURES_DEFAULT };
const features_t features_default = { FEATURES_DEFAULT };

//
// these arrays of names need to be kept in sync
// with the enumerations defined in features.h
//
const char * const feature_value_names[] = { FEATURE_VALUE_NAMES };

const char * const feature_index_names[] = { FEATURE_INDEX_NAMES };

// factory reset handler for features
static void feature_factory_reset_handler(void) {
	// Force an EEPROM update in the features
	// set the major and minor version numbers to 0 in the nvram
	// this value cannot happen naturally, so the image initialized
	// version will be copied to nvram on the next reset
	flashc_memset8((void *)&features_nvram, 0, 2, TRUE);
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
  // Register a factory reset handler
  widget_factory_reset_handler_register(feature_factory_reset_handler);
}

void features_display(char *title, features_t fp) {
	int i;
	char buff[32];
	widget_startup_log_line(title);
	sprintf(buff, "%s = %u.%u", "version", fp[feature_major_index], fp[feature_minor_index]);
	widget_startup_log_line(buff);
	for (i = feature_board_index; i < feature_end_index; i += 1) {
		strcpy(buff, feature_index_names[i]);
		strcat(buff, " = ");
		if (features[i] < feature_end_values)
			strcat(buff, (char *)feature_value_names[fp[i]]);
		else
			strcat(buff, "invalid!");
		widget_startup_log_line(buff);
	}
}

void features_display_all() {
	widget_display_clear();
	widget_report();
	features_display("features ram:", features);
	// features_display("features nvram:", features_nvram, 500000);
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

uint8_t feature_get_default(uint8_t index) {
	return index < feature_end_index ? features_default[index] : 0xFF;
}

void feature_factory_reset(void) {
	feature_factory_reset_handler();
}

static int find_end(int start) {
	while (TRUE) {
		if (start+1 == feature_end_values)
			return 0xFF;
		if (strcmp(feature_value_names[start+1], "end") == 0)
			return start;
		start += 1;
	}
}

void feature_find_first_and_last_value(uint8_t index, uint8_t *firstp, uint8_t *lastp) {
	uint8_t this_index, first, last;
	if (index <= feature_minor_index || index >= feature_end_index) {
		first = 0xFF;
		last = 0xFF;
	} else {
		this_index = feature_minor_index+1;
		first = 0;
		last = find_end(first);
		while (this_index < index) {
			this_index += 1;
			last = find_end(first = last+2);
		}
	}
	*firstp = first;
	*lastp = last;
}
