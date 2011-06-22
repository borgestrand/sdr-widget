/*
 *  samples.h
 *  SDR
 *
 *  Created by John Melton on 30/06/2009.
 *  Copyright 2009 G0ORX. All rights reserved.
 *
 */

#define SAMPLES_PER_BUFFER 480

float samples_frequency;
float samples_rate;

float samples[SAMPLES_PER_BUFFER];
long frequency;
int filterLow;
int filterHigh;
int mode;
int sampleRate;
int meter;

int fps;

int specHigh;
int specLow;