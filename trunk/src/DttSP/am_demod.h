/* am_demod.h
   
   This file is part of a program that implements a Software-Defined Radio.
   
   Copyright (C) 2004, 2005, 2006, 2007 by Frank Brickle, AB2KT and Bob McGwier, N4HY
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
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

#ifndef _am_demod_h
#define _am_demod_h

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
#include <ovsv.h>
#include <filter.h>
#include <oscillator.h>

typedef enum _ammode
{ AMdet, SAMdet } AMMode;
typedef struct _am_demod
{
  int size;
  CXB ibuf, obuf;

  struct
  {
    REAL alpha, beta, fast_alpha;
    struct
    {
      REAL f, l, h;
    } freq;
    REAL phs;
    struct
    {
      REAL alpha;
    } iir;
    COMPLEX delay;
  } pll;

  struct
  {
    REAL curr, prev;
  } lock;

  REAL dc;
  REAL smooth;
  AMMode mode;
} AMDDesc, *AMD;

extern void AMDemod (AMD am);
extern AMD newAMD (REAL samprate,
		   REAL f_initial,
		   REAL f_lobound,
		   REAL f_hibound,
		   REAL f_bandwid,
		   int size,
		   COMPLEX * ivec, COMPLEX * ovec, AMMode mode, char *tag);
extern void delAMD (AMD am);

#ifndef TWOPI
#define TWOPI (2.0*M_PI)
#endif

#endif
