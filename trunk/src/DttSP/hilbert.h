/* hilbert.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006, 2007, 2008 by Frank Brickle, AB2KT and Bob McGwier, N4HY

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
rwmcgwier@gmail.com

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807
*/

#ifndef _hilbert_h
#define _hilbert_h

#include <fromsys.h>
#include <defs.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>

typedef
struct _hilbert {
  int size;
  REAL *c, *x1, *y1;
  struct {
    CXB i, o;
  } buf;
  BOOLEAN invert;
} HilbertInfo, *Hilbert;

typedef
struct _hilsim {
  int size;
  struct {
    CXB i, o;
  } buf;
  REAL x[4], y[6], d[6];
  BOOLEAN invert;
} HilsimInfo, *Hilsim;

extern Hilbert newHilbert(CXB ibuf, CXB obuf, REAL rate);
extern Hilsim newHilbertsim(CXB ibuf, CXB obuf);
extern void delHilbert(Hilbert h);
extern void delHilsim(Hilsim h);
extern void hilbert_transform(Hilbert h);



void hilsim_transform(Hilsim h);

#endif
