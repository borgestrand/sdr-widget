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
  feature_adc_index,			// adc identifier, not implemented
  feature_dac_index,			// dac identifier, not implemented
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
	// board selection
	feature_board_none = 0,
	feature_board_widget,
	feature_board_dib,
	feature_board_ab1,
	feature_board_test,
	feature_end_board,
	// image selection
	feature_image_flashyblinky,
	feature_image_uac1_audio,
	feature_image_uac1_dg8saq,
	feature_image_uac2_audio,
	feature_image_uac2_dg8saq,
	feature_image_hpsdr,
	feature_image_test,
	feature_end_image,
	// input channel
	feature_in_normal,
	feature_in_swapped,
	feature_end_in,
	// output channel
	feature_out_normal,
	feature_out_swapped,
	feature_end_out,
	// adc
	feature_adc_none,
	feature_adc_ak5394a,
	feature_end_adc,
	// dac
	feature_dac_none,
	feature_dac_cs4344,
	feature_dac_es9022,
	feature_end_dac,
	// end
	feature_end_values
} feature_values_t;

#define FEATURE_VALUE_NAMES \
	"none",																\
		"widget",														\
		"dib",															\
		"ab1",															\
		"test",															\
		"end",															\
		"flashyblinky",													\
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
		"es9022",														\
		"end",															\
		"end"
	
typedef uint8_t features_t[feature_end_index];

extern features_t features_nvram, features;

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

#define FEATURE_BOARD_NONE				(features[feature_image_index] == (uint8_t)feature_board_none)
#define FEATURE_BOARD_WIDGET			(features[feature_image_index] == (uint8_t)feature_board_widget)
#define FEATURE_BOARD_DIB				(features[feature_image_index] == (uint8_t)feature_board_dib)
#define FEATURE_BOARD_AB1				(features[feature_image_index] == (uint8_t)feature_board_ab1)
#define FEATURE_BOARD_TEST				(features[feature_image_index] == (uint8_t)feature_board_test)

#define FEATURE_IMAGE_FLASHYBLINKY		(features[feature_image_index] == (uint8_t)feature_image_flashyblinky)
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
#define FEATURE_DAC_ES9022				(features[feature_dac_index] == (uint8_t)feature_dac_es9022)

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
// conditionally set the defaults for this build
//
#ifndef FEATURE_BOARD_DEFAULT
#define FEATURE_BOARD_DEFAULT			feature_board_widget
#endif
#ifndef FEATURE_IMAGE_DEFAULT
#if FEATURE_BOARD_DEFAULT == feature_board_widget
#define FEATURE_IMAGE_DEFAULT			feature_image_uac1_audio
#else
#define FEATURE_ADC_DEFAULT				feature_image_uac2_audio
#endif
#endif
#ifndef FEATURE_IN_DEFAULT
#define FEATURE_IN_DEFAULT				feature_in_normal
#endif
#ifndef FEATURE_OUT_DEFAULT
#define FEATURE_OUT_DEFAULT				feature_out_normal
#endif
#ifndef FEATURE_ADC_DEFAULT
#if FEATURE_BOARD_DEFAULT == feature_board_widget
#define FEATURE_ADC_DEFAULT				feature_adc_ak5394a
#else
#define FEATURE_ADC_DEFAULT				feature_adc_none
#endif
#endif
#ifndef FEATURE_DAC_DEFAULT
#define FEATURE_DAC_DEFAULT				feature_dac_cs4344
#endif


#define FEATURES_DEFAULT FEATURE_MAJOR_DEFAULT,		\
		FEATURE_MINOR_DEFAULT,						\
		FEATURE_BOARD_DEFAULT,						\
		FEATURE_IMAGE_DEFAULT,						\
		FEATURE_IN_DEFAULT,							\
		FEATURE_OUT_DEFAULT,						\
		FEATURE_ADC_DEFAULT,						\
		FEATURE_DAC_DEFAULT

extern void features_init();
extern void features_display(char *title, features_t fp, int delay);
extern void features_display_all();
extern uint8_t feature_set(uint8_t index, uint8_t value);
extern uint8_t feature_get(uint8_t index);
extern uint8_t feature_set_nvram(uint8_t index, uint8_t value);
extern uint8_t feature_get_nvram(uint8_t index);
extern void feature_factory_reset(void);

#define FEATURE_DG8SAQ_COMMAND			0x71
#define FEATURE_DG8SAQ_SET_NVRAM		3
#define FEATURE_DG8SAQ_GET_NVRAM		4
#define FEATURE_DG8SAQ_SET_RAM			5
#define FEATURE_DG8SAQ_GET_RAM			6
#define FEATURE_DG8SAQ_FACTORY_RESET	7

#endif /* FEATURES_H_ */
