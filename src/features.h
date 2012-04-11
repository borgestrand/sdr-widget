/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * features.h
 *
 *  Created on: 2011-02-11
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#ifndef FEATURES_H_
#define FEATURES_H_

#include <stdint.h>

//
// each feature index defines a feature set
//
// to add a new feature set:
// 1) insert the feature index name into the feature_index_t enumeration;
// 2) insert the feature index name string into the FEATURE_INDEX_NAMES define;
// 3) insert the feature end marker value into the feature_values_t enumeration;
// 4) insert the feature end marker name into the FEATURE_VALUE_NAMES define.
//

typedef enum {
  feature_major_index = 0,		// major version number = feature_end_index
  feature_minor_index,			// minor version number = feature_end_values
  feature_board_index,			// board identifier
  feature_image_index,			// image to boot
  feature_in_index,				// keep or swap left/right channels on input
  feature_out_index,			// keep or swap left/right channels on output
  feature_adc_index,			// adc identifier
  feature_dac_index,			// dac identifier
  feature_lcd_index,			// lcd display type
  feature_log_index,			// startup log display timing
  feature_filter_index,			// setting of filter
  feature_end_index				// end marker, used to size arrays
} feature_index_t;

#define FEATURE_INDEX_NAMES "major",				\
		"minor",									\
		"board",									\
		"image",									\
		"in",										\
		"out",										\
		"adc",										\
		"dac",										\
		"lcd",										\
		"log",										\
		"filter",									\
		"end"

//
// the features are enumerated as single group
// which allows a single table of name strings
//
// some of these are probably wrong and will
// need to be adjusted.
//
// to add another value to an existing feature set:
// 1) add its name to the feature_values_t enumeration;
// 2) add its string name to the FEATURE_VALUE_NAMES define;
// 3) add a macro to test for the feature's value below
//
// so, for instance, if Alex wants to add a test image:
// 1) insert feature_image_alex_test before feature_end_image;
// 2) insert "alex_test" into the FEATURE_VALUE_NAMES;
// 2) #define FEATURE_IMAGE_ALEX_TEST 
//
typedef enum {
	feature_board_none = 0,		// board selection
	feature_board_widget,
	feature_board_usbi2s,
	feature_board_usbdac,
	feature_end_board,
	feature_image_uac1_audio,
	feature_image_uac1_dg8saq,
	feature_image_uac2_audio,
	feature_image_uac2_dg8saq,
	feature_image_hpsdr,
	feature_image_test,
	feature_end_image,
	feature_in_normal,			// input channel
	feature_in_swapped,
	feature_end_in,
	feature_out_normal,			// output channel
	feature_out_swapped,
	feature_end_out,
	feature_adc_none,			// adc
	feature_adc_ak5394a,
	feature_end_adc,
	feature_dac_none,			// dac
	feature_dac_cs4344,
	feature_dac_es9023,
	feature_dac_pcm5102,
	feature_end_dac,
	feature_lcd_none,			// lcd
	feature_lcd_hd44780,		/* normal hd44780 lcd controller */
	feature_lcd_ks0073,			/* ks0073 almost hd44780 compatible */
	feature_end_lcd,
	feature_log_none,			// log
	feature_log_500ms,
	feature_log_1sec,
	feature_log_2sec,
	feature_end_log,
	feature_filter_fir,			// filter select
	feature_filter_iir,
	feature_end_filter,
	feature_end_values			// end
} feature_values_t;

#define FEATURE_VALUE_NAMES \
		"none",															\
		"widget",														\
		"usbi2s",														\
		"usbdac",														\
		"end",															\
		"uac1_audio",													\
		"uac1_dg8saq",													\
		"uac2_audio",													\
		"uac2_dg8saq",													\
		"hpsdr",														\
		"test",															\
		"end",															\
		"normal",														\
		"swapped",														\
		"end",															\
		"normal",														\
		"swapped",														\
		"end",															\
		"none",															\
		"ak5394a",														\
		"end",															\
		"none",															\
		"cs4344",														\
		"es9023",														\
		"pcm5102",														\
		"end",															\
		"none",															\
		"hd44780",														\
		"ks0073",														\
		"end",															\
		"none",															\
		"500ms",														\
		"1sec",															\
		"2sec",															\
		"end",															\
		"fir",															\
		"iir",															\
		"end",															\
		"end"
	
typedef uint8_t features_t[feature_end_index];

extern features_t features_nvram, features;
extern const features_t features_default;

//
// test for the values of features with these macros
// if a feature is unconditionally off, then the macro
// will expand to a constant 0 and the dead code will
// be eliminated.
//

#define FEATURE_MAJOR					(features[feature_major_index])
#define FEATURE_MAJOR_NVRAM				(features_nvram[feature_major_index])

#define FEATURE_MINOR					(features[feature_minor_index])
#define FEATURE_MINOR_NVRAM				(features_nvram[feature_minor_index])

#define FEATURE_BOARD_NONE				(features[feature_board_index] == (uint8_t)feature_board_none)
#define FEATURE_BOARD_WIDGET			(features[feature_board_index] == (uint8_t)feature_board_widget)
#define FEATURE_BOARD_USBI2S			(features[feature_board_index] == (uint8_t)feature_board_usbi2s)
#define FEATURE_BOARD_USBDAC			(features[feature_board_index] == (uint8_t)feature_board_usbdac)

#define FEATURE_IMAGE_UAC1_AUDIO		(features[feature_image_index] == (uint8_t)feature_image_uac1_audio)
#define FEATURE_IMAGE_UAC1_DG8SAQ		(features[feature_image_index] == (uint8_t)feature_image_uac1_dg8saq)
#define FEATURE_IMAGE_UAC2_AUDIO		(features[feature_image_index] == (uint8_t)feature_image_uac2_audio)
#define FEATURE_IMAGE_UAC2_DG8SAQ		(features[feature_image_index] == (uint8_t)feature_image_uac2_dg8saq)
#define FEATURE_IMAGE_HPSDR				(features[feature_image_index] == (uint8_t)feature_image_hpsdr)
#define FEATURE_IMAGE_TEST				(features[feature_image_index] == (uint8_t)feature_image_test)

#define FEATURE_IN_NORMAL				(features[feature_in_index] == (uint8_t)feature_in_normal)
#define FEATURE_IN_SWAPPED				(features[feature_in_index] == (uint8_t)feature_in_swapped)

#define FEATURE_OUT_NORMAL				(features[feature_out_index] == (uint8_t)feature_out_normal)
#define FEATURE_OUT_SWAPPED				(features[feature_out_index] == (uint8_t)feature_out_swapped)

#define FEATURE_ADC_NONE				(features[feature_adc_index] == (uint8_t)feature_adc_none)
#define FEATURE_ADC_AK5394A				(features[feature_adc_index] == (uint8_t)feature_adc_ak5394a)

#define FEATURE_DAC_NONE				(features[feature_dac_index] == (uint8_t)feature_dac_none)
#define FEATURE_DAC_CS4344				(features[feature_dac_index] == (uint8_t)feature_dac_cs4344)
#define FEATURE_DAC_ES9023				(features[feature_dac_index] == (uint8_t)feature_dac_es9023)
#define FEATURE_DAC_PCM5102				(features[feature_dac_index] == (uint8_t)feature_dac_pcm5102)

#define FEATURE_LCD_NONE				(features[feature_lcd_index] == (uint8_t)feature_lcd_none)
#define FEATURE_LCD_HD44780				(features[feature_lcd_index] == (uint8_t)feature_lcd_hd44780)
#define FEATURE_LCD_KS0073				(features[feature_lcd_index] == (uint8_t)feature_lcd_ks0073)

#define FEATURE_LOG_NONE				(features[feature_log_index] == (uint8_t)feature_log_none)
#define FEATURE_LOG_500MS				(features[feature_log_index] == (uint8_t)feature_log_500ms)
#define FEATURE_LOG_1SEC				(features[feature_log_index] == (uint8_t)feature_log_1sec)
#define FEATURE_LOG_2SEC				(features[feature_log_index] == (uint8_t)feature_log_2sec)

#define FEATURE_FILTER_FIR				(features[feature_filter_index] == (uint8_t)feature_filter_fir)
#define FEATURE_FILTER_IIR				(features[feature_filter_index] == (uint8_t)feature_filter_iir)
//
// the version in the features specifies
// the number of feature indexes and the number of feature values
// which should change anytime someone alters the list
// if there is a feature_major or feature_minor mismatch, then
// the initial values in the build are copied into nvram
//
#define FEATURE_MAJOR_DEFAULT			feature_end_index
#define FEATURE_MINOR_DEFAULT			feature_end_values

//
// Check the defaults for this build
//
// BSB 20120409 edited after Roger's mail
#ifndef FEATURE_BOARD_DEFAULT
#error "FEATURE_BOARD_DEFAULT must be defined by the Makefile"
#endif
#ifndef FEATURE_IMAGE_DEFAULT
#error "FEATURE_IMAGE_DEFAULT must be defined by the Makefile"
#endif
#ifndef FEATURE_IN_DEFAULT
#error "FEATURE_IN_DEFAULT must be defined by the Makefile"
#endif
#ifndef FEATURE_OUT_DEFAULT
#error "FEATURE_OUT_DEFAULT must be defined by the Makefile"
#endif
#ifndef FEATURE_DAC_DEFAULT
#error "FEATURE_DAC_DEFAULT must be defined by the Makefile"
#endif
#ifndef FEATURE_LCD_DEFAULT
#error "FEATURE_LCD_DEFAULT must be defined by the Makefile"
#endif
#ifndef FEATURE_LOG_DEFAULT
#error "FEATURE_LOG_DEFAULT must be defined by the Makefile"
#endif
#ifndef FEATURE_FILTER_DEFAULT
#error "FEATURE_FILTER_DEFAULT must be defined by the Makefile"
#endif

#define FEATURES_DEFAULT FEATURE_MAJOR_DEFAULT,		\
		FEATURE_MINOR_DEFAULT,						\
		FEATURE_BOARD_DEFAULT,						\
		FEATURE_IMAGE_DEFAULT,						\
		FEATURE_IN_DEFAULT,							\
		FEATURE_OUT_DEFAULT,						\
		FEATURE_ADC_DEFAULT,						\
		FEATURE_DAC_DEFAULT,						\
		FEATURE_LCD_DEFAULT,						\
		FEATURE_LOG_DEFAULT,						\
		FEATURE_FILTER_DEFAULT

extern const char * const feature_value_names[];
extern const char * const feature_index_names[];
extern void features_init();
extern void features_display(char *title, features_t fp);
extern void features_display_all();
extern uint8_t feature_set(uint8_t index, uint8_t value);
extern uint8_t feature_get(uint8_t index);
extern uint8_t feature_set_nvram(uint8_t index, uint8_t value);
extern uint8_t feature_get_nvram(uint8_t index);
extern uint8_t feature_get_default(uint8_t index);
extern void feature_factory_reset(void);
extern void feature_find_first_and_last_value(uint8_t index, uint8_t *first, uint8_t *last);

#define FEATURE_DG8SAQ_COMMAND			0x71
#define FEATURE_DG8SAQ_SET_NVRAM		3
#define FEATURE_DG8SAQ_GET_NVRAM		4
#define FEATURE_DG8SAQ_SET_RAM			5
#define FEATURE_DG8SAQ_GET_RAM			6
#define FEATURE_DG8SAQ_GET_INDEX_NAME	7
#define FEATURE_DG8SAQ_GET_VALUE_NAME	8
#define FEATURE_DG8SAQ_GET_DEFAULT		9

#endif /* FEATURES_H_ */
