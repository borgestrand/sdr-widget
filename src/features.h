/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
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
// the features are enumerated as a single set
// some of these are probably wrong and will
// need to be adjusted.
//
typedef enum {
  // image selection
  feature_image_flashyblinky = 0,
  feature_image_uac1_audio,
  feature_image_uac1_dg8saq,
  feature_image_uac2_audio,
  feature_image_uac2_dg8saq,
  feature_image_hpsdr,
  feature_image_test,
  feature_end_image,
  // input channel
  feature_iq_in_normal,
  feature_iq_in_swapped,
  feature_end_iq_in,
  // output channel
  feature_iq_out_normal,
  feature_iq_out_swapped,
  feature_end_iq_out,
  // adc
  feature_adc_none,
  feature_adc_ak5394a,
  feature_end_adc,
  // dac
  feature_dac_none,
  feature_dac_cs4344,
  feature_end_dac,
  // end
  feature_end_values
} feature_values_t;

typedef enum {
  feature_version_index = 0,
  feature_image_index,
  feature_iq_in_index,
  feature_iq_out_index,
  feature_adc_index,
  feature_dac_index,
  feature_end_index
} feature_index_t;

typedef uint8_t features_t[feature_end_index];

extern features_t features_nvram, features;

//
// test for the values of features with these macros
// if a feature is unconditionally off, then the macro
// will expand to a constant 0 and the dead code will
// be eliminated.
//

#define FEATURE_VERSION					(features[feature_version_index])
#define FEATURE_NVRAM_VERSION			(features_nvram[feature_version_index])

#define FEATURE_IMAGE_FLASHYBLINKY		(features[feature_image_index] == (uint8_t)feature_image_flashyblinky)
#define FEATURE_IMAGE_UAC1_AUDIO		(features[feature_image_index] == (uint8_t)feature_image_uac1_audio)
#define FEATURE_IMAGE_UAC1_DG8SAQ		(features[feature_image_index] == (uint8_t)feature_image_uac1_dg8saq)
#define FEATURE_IMAGE_UAC2_AUDIO		(features[feature_image_index] == (uint8_t)feature_image_uac2_audio)
#define FEATURE_IMAGE_UAC2_DG8SAQ		(features[feature_image_index] == (uint8_t)feature_image_uac2_dg8saq)
#define FEATURE_IMAGE_HPSDR				(features[feature_image_index] == (uint8_t)feature_image_hpsdr)
#define FEATURE_IMAGE_TEST				(features[feature_image_index] == (uint8_t)feature_image_test)

#define FEATURE_IQ_IN_NORMAL			(features[feature_iq_in_index] == (uint8_t)feature_iq_in_normal)
#define FEATURE_IQ_IN_SWAPPED			(features[feature_iq_in_index] == (uint8_t)feature_iq_in_swapped)

#define FEATURE_IQ_OUT_NORMAL			(features[feature_iq_out_index] == (uint8_t)feature_iq_out_normal)
#define FEATURE_IQ_OUT_SWAPPED			(features[feature_iq_out_index] == (uint8_t)feature_iq_out_swapped)

#define FEATURE_ADC_NONE				(features[feature_adc_index] == (uint8_t)feature_adc_none)
#define FEATURE_ADC_AK5394A				(features[feature_adc_index] == (uint8_t)feature_adc_ak5394a)

#define FEATURE_DAC_NONE				(features[feature_dac_index] == (uint8_t)feature_dac_none)
#define FEATURE_DAC_CS4344				(features[feature_dac_index] == (uint8_t)feature_dac_cs4344)

//
// conditionally set the defaults for this build
//
#ifndef FEATURE_VERSION_DEFAULT
#define FEATURE_VERSION_DEFAULT feature_end_index
#endif
#ifndef FEATURE_IMAGE_DEFAULT
//#define FEATURE_IMAGE_DEFAULT			feature_image_uac2_audio
//#define FEATURE_IMAGE_DEFAULT			feature_image_uac2_dg8saq
//#define FEATURE_IMAGE_DEFAULT			feature_image_uac1_audio
#define FEATURE_IMAGE_DEFAULT			feature_image_uac1_dg8saq
//#define FEATURE_IMAGE_DEFAULT			feature_image_hpsdr
#endif
#ifndef FEATURE_IQ_IN_DEFAULT
#define FEATURE_IQ_IN_DEFAULT			feature_iq_in_normal
#endif
#ifndef FEATURE_IQ_OUT_DEFAULT
#define FEATURE_IQ_OUT_DEFAULT			feature_iq_out_normal
#endif
#ifndef FEATURE_ADC_DEFAULT
#define FEATURE_ADC_DEFAULT				feature_adc_ak5394a
#endif
#ifndef FEATURE_DAC_DEFAULT
#define FEATURE_DAC_DEFAULT				feature_dac_cs4344
#endif


#define FEATURES_DEFAULT FEATURE_VERSION_DEFAULT,	\
		FEATURE_IMAGE_DEFAULT,						\
		FEATURE_IQ_IN_DEFAULT,						\
		FEATURE_IQ_OUT_DEFAULT,						\
		FEATURE_ADC_DEFAULT,						\
		FEATURE_DAC_DEFAULT

extern void features_init();
extern void features_display(char *title, features_t fp, int delay);
extern void features_display_all();

#endif /* FEATURES_H_ */
