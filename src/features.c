#include "features.h"
#include "Mobo_config.h"

uint8_t *features_nvram() {
  return nvram_cdata.features;
}
uint8_t *features_live() {
  return cdata.features;
}



