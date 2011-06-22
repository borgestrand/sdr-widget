/* lmadf.h 

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

#ifndef _lmadf_h

#define _lmadf_h
#include <fftw3.h>
#include <fftw3_fix.h>
#include <fromsys.h>
#include <banal.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>


#define LMADF_INTERFERENCE 0
#define LMADF_NOISE 1
#define LMADF_NOSIG (-1)
#define LMADF_NOLINE (-2)
#define LMADF_NOFILT (-3)

extern int lmadf_err;

//#define REALLMS
typedef struct _BlockLMS
{
  CXB signal;
  COMPLEX *delay_line;
  COMPLEX *y;
  COMPLEX *Xhat;
  COMPLEX *What;
  COMPLEX *Y;
  COMPLEX *Yhat;
  COMPLEX *error;
  COMPLEX *Errhat;
  COMPLEX *Update;
  COMPLEX *update;
  REAL adaptation_rate;
  REAL leak_rate;
  int filter_type;
  fftwf_plan Xplan, Yplan, Wplan, Errhatplan, UPDplan;
} _blocklms, *BLMS;

#ifdef REALLMS

typedef struct _LMSR
{
  CXB signal;			/* Signal Buffer */
  int signal_size;		/* Number of samples in signal buffer */
  REAL *delay_line;		/* Delay Line circular buffer for holding samples */
  REAL *adaptive_filter;	/* Filter coefficients */
  REAL adaptation_rate;		/* Adaptation rate for the LMS stochastic gradient */
  REAL leakage;			/* Exponential decay constant for filter coefficients */
  int adaptive_filter_size;	/* number taps in adaptive filter */
  int filter_type;		/* Filter type */
  int delay;			/* Total delay between current sample and filter */
  int delay_line_ptr;		/* Pointer for next sample into the delay line */
  int size;			/* Delay line size */
  int mask;			/* Mask for circular buffer */
} *LMSR, _lmsstate;

#else

typedef struct _LMSR
{
  CXB signal;			/* Signal Buffer */
  int signal_size;		/* Number of samples in signal buffer */
  COMPLEX *delay_line;		/* Delay Line circular buffer for holding samples */
  COMPLEX *adaptive_filter;	/* Filter coefficients */
  REAL adaptation_rate;		/* Adaptation rate for the LMS stochastic gradient */
  REAL leakage;			/* Exponential decay constant for filter coefficients */
  int adaptive_filter_size;	/* number taps in adaptive filter */
  int filter_type;		/* Filter type */
  int delay;			/* Total delay between current sample and filter */
  int delay_line_ptr;		/* Pointer for next sample into the delay line */
  int size;			/* Delay line size */
  int mask;			/* Mask for circular buffer */
} *LMSR, _lmsstate;

#endif

extern LMSR new_lmsr (CXB signal,
		      int delay,
		      REAL adaptation_rate,
		      REAL leakage,
		      int adaptive_filter_size, int filter_type);


extern void del_lmsr (LMSR lms);

extern void lmsr_adapt (LMSR lms);

extern BLMS new_blms (CXB signal, REAL adaptation_rate, REAL leak_rate,
		      int filter_type, int pbits);

extern void blms_adapt (BLMS blms);

extern void del_blms (BLMS blms);

#endif
