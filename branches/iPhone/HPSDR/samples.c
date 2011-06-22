/*
 *  samples.c
 *  SDR
 *
 *  Created by John Melton on 02/07/2009.
 *  Copyright 2009 G0ORX. All rights reserved.
 *
 */

#include "samples.h"

float samples[SAMPLES_PER_BUFFER];


long frequency;

int fps=10;

int specHigh=0;
int specLow=-160;
