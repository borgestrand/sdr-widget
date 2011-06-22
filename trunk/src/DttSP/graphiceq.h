/* graphiceq.h
 * 
 *   PCM time-domain equalizer
 *
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

This is derived from equ.xmms:

 *   Copyright (C) 2002  Felipe Rivera <liebremx at users sourceforge net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   $Id: iir.c,v 1.10 2004/05/16 02:24:31 liebremx Exp $ */

#ifndef _graphiceq_h
#define _graphiceq_h

#include <datatypes.h>
#include <bufvec.h>
#include <ovsv.h>
#include <filter.h>
#include <fftw3.h>
#include <fftw3_fix.h>

typedef struct _eq {
  CXB data;
  FiltOvSv p;
  CXB in, out;
  COMPLEX num[9], den[6];
  BOOLEAN notchflag;
} eq, *EQ;

extern EQ new_EQ (CXB d, REAL samplerate, int pbits);
extern void graphiceq (EQ a);

#endif /* #define GRPHEQ_H */
