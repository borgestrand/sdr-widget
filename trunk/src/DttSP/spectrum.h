/* spectrum.h */
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

#ifndef _spectrum_h
#define _spectrum_h

#include <fromsys.h>
#include <defs.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>
#include <ringb.h>
#include <lmadf.h>
#include <fftw3.h>
#include <fftw3_fix.h>
#include <ovsv.h>
#include <filter.h>
#include <oscillator.h>
#include <dttspagc.h>
#include <am_demod.h>
#include <fm_demod.h>
#include <noiseblanker.h>
#include <correctIQ.h>
#include <speechproc.h>
#include <spottone.h>
#include <update.h>
#include <window.h>

#define SPEC_MAG	(0)
#define SPEC_PWR	(1)

#define SPEC_SEMI_RAW	(0)
#define SPEC_PRE_FILT	(1)
#define SPEC_POST_FILT	(2)
#define SPEC_POST_AGC	(3)
#define SPEC_POST_DET	(4)
#define SPEC_PREMOD		(4)

typedef struct _spec_block
{
  BOOLEAN flag;
  int label;
  CXB accum, timebuf, freqbuf;
  int fill, buflen, rxk, scale, size, type, mask;
  Windowtype wintype;
  REAL *window;
  float *output, *oscope;
  COMPLEX *coutput;
  int planbits;
  fftwf_plan plan;
  BOOLEAN polyphase;
} SpecBlock;

extern void init_spectrum (SpecBlock * sb);
extern void reinit_spectrum (SpecBlock * sb);
extern void snap_spectrum (SpecBlock * sb, int label);
extern void snap_scope (SpecBlock * sb, int label);
extern void compute_spectrum (SpecBlock * sb);
extern void compute_complex_spectrum(SpecBlock * sb);
extern void finish_spectrum (SpecBlock * sb);

#endif
