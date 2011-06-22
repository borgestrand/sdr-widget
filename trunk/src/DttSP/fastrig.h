/* fastrig.h
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

#ifndef _fastrig_h
#define _fastrig_h

#include <fromsys.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <math.h>

#ifdef notdef
#include <cxops.h>
#endif

#define SIN_TABLE_SIZE 4096
#define SIN_TABLE_SIZE_M1 4095

/* ********************************************** 
 * TRIG_SPEED: 
 * 0 = normal (slow); 
 * 1 = table look up with interpolation (medium);
 * 2 = table look up (fast) 
 * ***********************************************/

/* Interpolation is ALWAYS done on atan2.  The setting
   only applies to sin and cos */

#ifndef TRIG_SPEED
#define TRIG_SPEED 0
#endif

#if (TRIG_SPEED == 2)
#define SIN(x)     fast_sin(x)
#define COS(x)     fast_cos(x)
#define ATAN2(x,y) fast_atan2((x),(y))

#elif (TRIG_SPEED == 1)

#define SIN(x)     fast_sin(x)
#define COS(x)     fast_cos(x)
#define ATAN2(x,y) fast_atan2((x),(y))

#elif (TRIG_SPEED == 0)

#define SIN(x)     (REAL)sin((REAL)x)
#define COS(x)     (REAL)cos((REAL)x)
#define ATAN2(x,y) (REAL)atan2((REAL)(x),(REAL)(y))

#endif

#ifndef PI
#define PI M_PI
#endif /* PI */

#ifndef TWOPI
#define TWOPI    (2.0 * PI)
#endif

#ifndef ONE_OVER_TWOPI
#define ONE_OVER_TWOPI (0.159154943091895)
#endif


extern void InitSPEEDTRIG (void);
extern REAL fast_sin (REAL);
extern REAL fast_cos (REAL);
extern REAL fast_atan2 (REAL, REAL);


#endif
