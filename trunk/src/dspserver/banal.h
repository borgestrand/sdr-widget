/* banal.h
   stuff we're too embarrassed to declare otherwise
   
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

#ifndef _banal_h

#define _banal_h

#include <fromsys.h>
#include <defs.h>
#include <datatypes.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#define abs(a) ((a) >= 0 ? (a) : -(a))

#define MONDO 1e15
#define BITSY 1e-15

#define TRUE 1
#define FALSE 0

extern void nilfunc (void);
extern INLINE REAL sqr (REAL);
extern int popcnt (int);
extern int npoof2 (int);
extern int nblock2 (int);

extern int in_blocks (int count, int block_size);

extern FILE *efopen (char *path, char *mode);
extern FILE *efreopen (char *path, char *mode, FILE * strm);
extern size_t filesize (char *path);
extern size_t fdsize (int fd);

extern struct timeval now_tv (void);
extern struct timeval diff_tv (struct timeval *, struct timeval *);
extern struct timeval sum_tv (struct timeval *, struct timeval *);
extern char *fmt_tv (struct timeval *);
extern char *since (struct timeval *);
extern struct timeval now_tv (void);

extern int hinterp_vec (REAL *, int, REAL *, int);

extern void status_message (char *msg);

extern FILE *find_rcfile (char *base);

extern unsigned long hash (unsigned char *str);
extern int gcd (int m, int n);
extern int least_common_mul (int i, int j);

#endif
