/* ovsv.h

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

#ifndef _ovsv_h
#define _ovsv_h

#include <fromsys.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>
#include <update.h>
#include <lmadf.h>
#include <fftw3.h>
#include <fftw3_fix.h>

typedef struct _filt_ov_sav {
  int buflen, fftlen;
  COMPLEX *zfvec, *zivec, *zovec, *zrvec;
  fftwf_plan pfwd, pinv;
  REAL scale;
} filt_ov_sv, *FiltOvSv;

extern FiltOvSv newFiltOvSv(COMPLEX * coefs, int ncoef, int pbits);
extern void delFiltOvSv(FiltOvSv p);

extern COMPLEX *FiltOvSv_initpoint(FiltOvSv pflt);
extern int FiltOvSv_initsize(FiltOvSv pflt);

extern COMPLEX *FiltOvSv_fetchpoint(FiltOvSv pflt);
extern int FiltOvSv_fetchsize(FiltOvSv pflt);

extern COMPLEX *FiltOvSv_storepoint(FiltOvSv pflt);
extern int FiltOvSv_storesize(FiltOvSv pflt);

extern void filter_OvSv(FiltOvSv pflt);
extern void reset_OvSv(FiltOvSv pflt);

#endif
