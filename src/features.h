/*
 * features.h
 *
 *  Created on: 2011-02-11
 *      Author: Roger Critchlow, AD5DZ
 */

#ifndef FEATURES_H_
#define FEATURES_H_

#include <stdint.h>

#define FEATURE_COUNT 8
typedef uint8_t features_t[FEATURE_COUNT];

extern uint8_t *features_nvram();
extern uint8_t *features_live();

typedef enum {
  feature_audio_none,
  feature_audio_uac1,
  feature_audio_uac2,
  feature_audio_hpsdr,
  feature_audio_test
} feature_audio_t;

typedef enum {
  feature_control_none,
  feature_control_mobokit,
  feature_control_rx_ensemble_ii,
  feature_control_rxtx_ensemble,
  feature_control_test
} feature_control_t;

typedef enum {
  feature_adc_none,
  feature_adc_AK5394A,
  feature_adc_test
} feature_adc_t;

#define FEATURE_AUDIO_INDEX		0
#define FEATURE_AUDIO_NONE		(features_live()[FEATURE_AUDIO_INDEX] == feature_audio_none)
#define FEATURE_AUDIO_UAC1		(features_live()[FEATURE_AUDIO_INDEX] == feature_audio_uac1)
#define FEATURE_AUDIO_UAC2		(features_live()[FEATURE_AUDIO_INDEX] == feature_audio_uac2)
#define FEATURE_AUDIO_HPSDR		(features_live()[FEATURE_AUDIO_INDEX] == feature_audio_hpsdr)
#define FEATURE_AUDIO_TEST		(features_live()[FEATURE_AUDIO_INDEX] == feature_audio_test)

#define FEATURE_CONTROL_INDEX		1
#define FEATURE_CONTROL_NONE		(features_live()[FEATURE_CONTROL_INDEX] == feature_control_none)
#define FEATURE_CONTROL_MOBOKIT		(features_live()[FEATURE_CONTROL_INDEX] == feature_control_mobokit)
#define FEATURE_CONTROL_RX_ENSEMBLE_II	(features_live()[FEATURE_CONTROL_INDEX] == feature_control_rx_ensemble_ii)
#define FEATURE_CONTROL_RXTX_ENSEMBLE	(features_live()[FEATURE_CONTROL_INDEX] == feature_control_rxtx_ensemble)
#define FEATURE_CONTROL_TEST		(features_live()[FEATURE_CONTROL_INDEX] == feature_control_test)

#define FEATURE_ADC_INDEX		2
#define FEATURE_ADC_NONE		(features_live()[FEATURE_ADC_INDEX] == feature_adc_none)
#define FEATURE_ADC_AK5394A		(features_live()[FEATURE_ADC_INDEX] == feature_adc_AK5394A)
#define FEATURE_ADC_TEST		(features_live()[FEATURE_ADC_INDEX] == feature_adc_test)

#define FEATURES_DEFAULT { feature_audio_uac1, feature_control_none, feature_adc_AK5394A }

#endif /* FEATURES_H_ */
