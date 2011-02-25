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

typedef enum {
  // image selection
  feature_image_flashyblinky = 0,
  feature_image_uac1_audio,
  feature_image_uac1_dg8saq,
  feature_image_uac2_audio,
  feature_image_uac2_dg8saq,
  feature_image_hpsdr,
  feature_image_test,
  // control variant
  feature_control_none,
  feature_control_mobokit,
  feature_control_rx_ensemble_ii,
  feature_control_rxtx_ensemble,
  feature_control_test,
  // input channel
  feature_iq_in_normal,
  feature_iq_in_swapped,
  // output channel
  feature_iq_out_normal,
  feature_iq_out_swapped,
  // hid
  feature_hid_off,
  feature_hid_on,
  // adc
  feature_adc_none,
  feature_adc_ak5394a,
  // dac
  feature_dac_none,
  feature_dac_cs4344,
  // end
  feature_end
} feature_values_t;

typedef enum {
  feature_version_index = 0,
  feature_image_index,
  feature_control_index,
  feature_iq_in_index,
  feature_iq_out_index,
  feature_hid_index,
  feature_adc_index,
  feature_dac_index,
  feature_last_index
} feature_index_t;

typedef uint8_t features_t[feature_last_index];

extern features_t features_nvram, features;

#define FEATURE_VERSION			(features[feature_version_index])
#define FEATURE_NVRAM_VERSION		(features_nvram[feature_version_index])

#define FEATURE_IMAGE_FLASHYBLINKY	(features[feature_image_index] == (U8)feature_image_flashyblinky)
#define FEATURE_IMAGE_UAC1_AUDIO	(features[feature_image_index] == (U8)feature_image_uac1_audio)
#define FEATURE_IMAGE_UAC1_DG8SAQ	(features[feature_image_index] == (U8)feature_image_uac1_dg8saq)
#define FEATURE_IMAGE_UAC2_AUDIO	(features[feature_image_index] == (U8)feature_image_uac2_audio)
#define FEATURE_IMAGE_UAC2_DG8SAQ	(features[feature_image_index] == (U8)feature_image_uac2_dg8saq)
#define FEATURE_IMAGE_HPSDR		(features[feature_image_index] == (U8)feature_image_hpsdr)
#define FEATURE_IMAGE_TEST		(features[feature_image_index] == (U8)feature_image_test)

#define FEATURE_CONTROL_NONE		(features[feature_control_index] == (U8)feature_control_none)
#define FEATURE_CONTROL_MOBOKIT		(features[feature_control_index] == (U8)feature_control_mobokit)
#define FEATURE_CONTROL_RX_ENSEMBLE_II	(features[feature_control_index] == (U8)feature_control_rx_ensemble_ii)
#define FEATURE_CONTROL_RXTX_ENSEMBLE	(features[feature_control_index] == (U8)feature_control_rxtx_ensemble)
#define FEATURE_CONTROL_TEST		(features[feature_control_index] == (U8)feature_control_test)

#define FEATURE_IQ_IN_NORMAL		(features[feature_iq_in_index] == (U8)feature_iq_in_normal)
#define FEATURE_IQ_IN_SWAPPED		(features[feature_iq_in_index] == (U8)feature_iq_in_swapped)

#define FEATURE_IQ_OUT_NORMAL		(features[feature_iq_out_index] == (U8)feature_iq_out_normal)
#define FEATURE_IQ_OUT_SWAPPED		(features[feature_iq_out_index] == (U8)feature_iq_out_swapped)

#define FEATURE_HID_OFF			(features[feature_hid_index] == (U8)feature_hid_off)
#define FEATURE_HID_ON			(features[feature_hid_index] == (U8)feature_hid_on)

#define FEATURE_ADC_NONE		(features[feature_adc_index] == (U8)feature_adc_none)
#define FEATURE_ADC_AK5394A		(features[feature_adc_index] == (U8)feature_adc_ak5394a)

#define FEATURE_DAC_NONE		(features[feature_dac_index] == (U8)feature_dac_none)
#define FEATURE_DAC_CS4344		(features[feature_dac_index] == (U8)feature_dac_cs4344)

// set the defaults for this build
//#define FEATURE_IMAGE_DEFAULT		feature_image_uac2_audio
#define FEATURE_IMAGE_DEFAULT		feature_image_uac2_dg8saq
#define FEATURE_CONTROL_DEFAULT		feature_control_none
#define FEATURE_IQ_IN_DEFAULT		feature_iq_in_normal
#define FEATURE_IQ_OUT_DEFAULT		feature_iq_out_normal
#define FEATURE_HID_DEFAULT		feature_hid_off
#define FEATURE_ADC_DEFAULT		feature_adc_ak5394a
#define FEATURE_DAC_DEFAULT		feature_dac_cs4344

// this checksum may fail if you change two default features at once
// so define it to be 0 on the command line
#ifndef FEATURE_VERSION_DEFAULT
#define FEATURE_VERSION_DEFAULT	( FEATURE_IMAGE_DEFAULT + FEATURE_CONTROL_DEFAULT + FEATURE_IQ_IN_DEFAULT + FEATURE_IQ_OUT_DEFAULT + FEATURE_HID_DEFAULT + \
				  FEATURE_ADC_DEFAULT + FEATURE_DAC_DEFAULT )
#endif

#define FEATURES_DEFAULT	FEATURE_VERSION_DEFAULT, FEATURE_IMAGE_DEFAULT, FEATURE_CONTROL_DEFAULT, FEATURE_IQ_IN_DEFAULT, FEATURE_IQ_OUT_DEFAULT, \
    FEATURE_HID_DEFAULT, FEATURE_ADC_DEFAULT, FEATURE_DAC_DEFAULT

extern void features_init();
extern void features_display();

#endif /* FEATURES_H_ */
