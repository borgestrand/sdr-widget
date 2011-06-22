/* spottone.h */
/*
This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be reached by email at

ab2kt@arrl.net
or
rwmcgwier@comcast.net

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807
*/

#ifndef _spottone_h
#define _spottone_h

#include <fromsys.h>
#include <banal.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>
#include <oscillator.h>

#define SpotTone_IDLE (0)
#define SpotTone_WAIT (1)
#define SpotTone_RISE (2)
#define SpotTone_STDY (3)
#define SpotTone_FALL (4)
#define SpotTone_HOLD (5)

typedef struct _spot_tone_gen
{
  REAL curr, gain, mul, scl, sr;
  struct
  {
    REAL freq;
    OSC gen;
  } osc;
  struct
  {
    REAL dur, incr;
    int want, have;
  } rise, fall;
  int size, stage;
  CXB buf;
} SpotToneGenDesc, *SpotToneGen;

extern SpotToneGen newSpotToneGen (REAL gain,	// dB
				   REAL freq,	// Hz
				   REAL rise,	// msec
				   REAL fall,	// msec
				   int size,	// buflen
				   REAL samplerate);
extern void delSpotToneGen (SpotToneGen gen);
extern void setSpotToneGenVals (SpotToneGen gen,
				REAL gain, REAL freq, REAL rise, REAL fall);
extern void SpotToneOn (SpotToneGen gen);
extern void SpotToneOff (SpotToneGen gen);
extern BOOLEAN SpotTone (SpotToneGen gen);

#endif
