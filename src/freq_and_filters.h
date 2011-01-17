/*
 * freq_and_filters.h
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef FREQ_AND_FILTERS_H_
#define FREQ_AND_FILTERS_H_

#include <stdint.h>

typedef union {								// Union of unsigned and signed 16 bit
	uint16_t		w;						// and 8bit[2]
	//int16_t		i;
	struct {
		uint8_t		b0;
		uint8_t		b1;
	};
} sint16_t;

typedef union {								// Union of an unsigned 32bit and the
	uint32_t		dw;						// above defined sint16_t[2]
	struct {
		uint16_t	w0;
		uint16_t	w1;
	};
} sint32_t;

extern uint8_t SetFreq_Handler(uint32_t freq);
extern void freq_and_filter_control(void);

#endif /* FREQ_AND_FILTERS_H_ */
