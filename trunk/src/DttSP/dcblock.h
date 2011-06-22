// dcblock.h
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

#ifndef _dcblock_h
#define _dcblock_h

#include <fromsys.h>
#include <defs.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>

#define BLKMEM (9)
#define DCB_LOW (0)
#define DCB_MED (1)
#define DCB_HIGH (2)
#define DCB_SUPER (3)
#define DCB_SINGLE_POLE (4)

typedef struct _dcblocker
{
  struct
  {
    REAL inp[BLKMEM], out[BLKMEM];
  } old;
  int lev;
  COMPLEX sigval;
  CXB buf;
  REAL gain;
  BOOLEAN flag;
} DCBlockerInfo, *DCBlocker;

extern void DCBlock (DCBlocker dcb);
extern void resetDCBlocker (DCBlocker dcb, int lev);
extern DCBlocker newDCBlocker (int lev, CXB buf);
extern void delDCBlocker (DCBlocker dcb);

#endif
