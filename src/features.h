/*
 * features.h
 *
 *  Created on: 2011-02-11
 *      Author: Roger Critchlow, AD5DZ
 */

#ifndef FEATURES_H_
#define FEATURES_H_

#include <stdint.h>
#include "Mobo_config.h"

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

typedef uint8_t features_t[8];

static inline features_t features_nvram() { return nvram_cdata.features; }
static inline features_t features_live() {  return nvram_cdata.features; }

#define FEATURE_AUDIO_NONE		(features_live()[0] == feature_audio_none)
#define FEATURE_AUDIO_UAC1		(features_live()[0] == feature_audio_uac1)
#define FEATURE_AUDIO_UAC2		(features_live()[0] == feature_audio_uac2)
#define FEATURE_AUDIO_HPSDR		(features_live()[0] == feature_audio_hpsdr)
#define FEATURE_AUDIO_TEST		(features_live()[0] == feature_audio_test)

#define FEATURE_CONTROL_NONE		(features_live()[1] == feature_control_none)
#define FEATURE_CONTROL_MOBOKIT		(features_live()[1] == feature_control_mobokit)
#define FEATURE_CONTROL_RX_ENSEMBLE_II	(features_live()[1] == feature_control_rx_ensemble_ii)
#define FEATURE_CONTROL_RXTX_ENSEMBLE	(features_live()[1] == feature_control_rxtx_ensemble)
#define FEATURE_CONTROL_TEST		(features_live()[1] == feature_control_test)

#endif /* FEATURES_H_ */
