/* filter.h
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

#ifndef _filter_h

#define _filter_h


#include <fromsys.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>
#include <fastrig.h>
#include <update.h>
#include <lmadf.h>
#include <fftw3.h>
#include <fftw3_fix.h>
#include <window.h>
#include <math.h>

typedef enum
{
  FIR_Undef, FIR_Lowpass, FIR_Bandpass, FIR_Highpass, FIR_Hilbert,
  FIR_Bandstop
} FIR_response_type;

typedef enum
{ FIR_Even, FIR_Odd } FIR_parity_type;

typedef struct _real_FIR
{
  REAL *coef;
  int size;
  FIR_response_type type;
  BOOLEAN cplx;
  struct
  {
    REAL lo, hi;
  } freq;
} RealFIRDesc, *RealFIR;

typedef struct _complex_FIR
{
  COMPLEX *coef;
  int size;
  FIR_response_type type;
  BOOLEAN cplx;
  struct
  {
    REAL lo, hi;
  } freq;
} ComplexFIRDesc, *ComplexFIR;

#define FIRcoef(p) ((p)->coef)
#define FIRtap(p, i) (FIRcoef(p)[(i)])
#define FIRsize(p) ((p)->size)
#define FIRtype(p) ((p)->type)
#define FIRiscomplex(p) ((p)->cplx)
#define FIRisreal(p) (!FIRiscomplex(p))
#define FIRfqlo(p) ((p)->freq.lo)
#define FIRfqhi(p) ((p)->freq.hi)

#define delFIR_Lowpass_REAL(p) delFIR_REAL(p)
#define delFIR_Lowpass_COMPLEX(p) delFIR_COMPLEX(p)
#define delFIR_Bandpass_REAL(p) delFIR_REAL(p)
#define delFIR_Bandpass_COMPLEX(p) delFIR_COMPLEX(p)
#define delFIR_Highpass_REAL(p) delFIR_REAL(p)
#define delFIR_Highpass_COMPLEX(p) delFIR_COMPLEX(p)
#define delFIR_Hilbert_REAL(p) delFIR_REAL(p)
#define delFIR_Hilbert_COMPLEX(p) delFIR_COMPLEX(p)
#define delFIR_Bandstop_REAL(p) delFIR_REAL(p)
#define delFIR_Bandstop_COMPLEX(p) delFIR_COMPLEX(p)

extern RealFIR newFIR_REAL (int size, char *tag);
extern ComplexFIR newFIR_COMPLEX (int size, char *tag);
extern void delFIR_REAL (RealFIR p);
extern void delFIR_COMPLEX (ComplexFIR p);
extern RealFIR newFIR_Lowpass_REAL (REAL cutoff, REAL sr, int size);
extern ComplexFIR newFIR_Lowpass_COMPLEX (REAL cutoff, REAL sr, int size);
extern RealFIR newFIR_Bandpass_REAL (REAL lo, REAL hi, REAL sr, int size);
extern ComplexFIR newFIR_Bandpass_COMPLEX (REAL lo, REAL hi, REAL sr,
					   int size);
extern RealFIR newFIR_Highpass_REAL (REAL cutoff, REAL sr, int size);
extern ComplexFIR newFIR_Highpass_COMPLEX (REAL cutoff, REAL sr, int size);
extern RealFIR newFIR_Hilbert_REAL (REAL lo, REAL hi, REAL sr, int size);
extern ComplexFIR newFIR_Hilbert_COMPLEX (REAL lo, REAL hi, REAL sr,
					  int size);
extern RealFIR newFIR_Bandstop_REAL (REAL lo, REAL hi, REAL sr, int size);
extern ComplexFIR newFIR_Bandstop_COMPLEX (REAL lo, REAL hi, REAL sr,
					   int size);

#endif
