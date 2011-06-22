/* window.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Implemented from code by Bill Schottstaedt of Snd Editor at CCRMA

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

#ifndef _window_h
#define _window_h

#include <stdlib.h>
#include <stdio.h>
#include <datatypes.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

/* #define RECTANGULAR_WINDOW 	 1
#define HANNING_WINDOW 		 2
#define WELCH_WINDOW 		 3
#define PARZEN_WINDOW 		 4
#define BARTLETT_WINDOW 	 5
#define HAMMING_WINDOW 		 6
#define BLACKMAN2_WINDOW	 7
#define BLACKMAN3_WINDOW 	 8
#define BLACKMAN4_WINDOW 	 9
#define EXPONENTIAL_WINDOW 	10
#define RIEMANN_WINDOW 		11 */

typedef enum _windowtype
{
  RECTANGULAR_WINDOW,
  HANNING_WINDOW,
  WELCH_WINDOW,
  PARZEN_WINDOW,
  BARTLETT_WINDOW,
  HAMMING_WINDOW,
  BLACKMAN2_WINDOW,
  BLACKMAN3_WINDOW,
  BLACKMAN4_WINDOW,
  EXPONENTIAL_WINDOW,
  RIEMANN_WINDOW,
  BLACKMANHARRIS_WINDOW,
  NUTTALL_WINDOW,
} Windowtype;

extern REAL *makewindow (Windowtype type, int size, REAL * window);
//extern char *window_name(int n);
extern REAL sqr (REAL x);

#endif
