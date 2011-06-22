/* spottone.h */
/*
This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2005 by Frank Brickle, AB2KT and Bob McGwier, N4HY

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

#ifndef _cwtone_h
#define _cwtone_h

#include <fromsys.h>
#include <banal.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>
#include <oscillator.h>
#include <math.h>

#define CWTone_IDLE (0)
#define CWTone_WAIT (1)
#define CWTone_RISE (2)
#define CWTone_STDY (3)
#define CWTone_FALL (4)
#define CWTone_HOLD (5)
#define CWSIN	sin
typedef struct _cw_tone_gen
{
  REAL curr, gain, mul, scl, sr;
  struct
  {
    REAL freq;
    OSC gen;
  } osc;
  double harmonic, amplitude, phase;
  struct
  {
    REAL dur, incr;
    int want, have;
  } rise, fall;
  int size, stage;
  CXB buf;
} CWToneGenDesc, *CWToneGen;

extern CWToneGen newCWToneGen (REAL gain,	// dB
			       REAL freq,	// Hz
			       REAL rise,	// msec
			       REAL fall,	// msec
			       int size,	// buflen
			       REAL samplerate);
extern void delCWToneGen (CWToneGen gen);
extern void setCWToneGenVals (CWToneGen gen,
			      REAL gain, REAL freq, REAL rise, REAL fall);
extern void CWToneOn (CWToneGen gen);
extern void CWToneOff (CWToneGen gen);
extern BOOLEAN CWTone (CWToneGen gen);

#endif
