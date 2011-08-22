/* bufvec.h

defs for vector and buffer data structures and utilities
   
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

#ifndef _bufvec_h

#define _bufvec_h

#include <fromsys.h>
#include <datatypes.h>
#include <complex.h>
#include <cxops.h>

typedef struct _complex_buffer_desc {
  COMPLEX *data;
  int size, ovlp, want, have, done, mine;
} CXBuffer, *CXB;

/* all these should be OK rhs or lhs */

#define CXBbase(p) ((p)->data)
#define CXBdata(p, i) (CXBbase(p)[(i)])
#define CXBreal(p, i) (CXBbase(p)[(i)].re)
#define CXBimag(p, i) (CXBbase(p)[(i)].im)
#define CXBsize(p) ((p)->size)
#define CXBovlp(p) ((p)->ovlp)
#define CXBwant(p) ((p)->want)
#define CXBhave(p) ((p)->have)
#define CXBdone(p) ((p)->done)
#define CXBmine(p) ((p)->mine)

typedef struct _real_buffer_desc {
  REAL *data;
  int size, ovlp, want, have, done, mine;
} RLBuffer, *RLB;

#define RLBbase(p) ((p)->data)
#define RLBdata(p, i) (RLBbase(p)[(i)])
#define RLBsize(p) ((p)->size)
#define RLBovlp(p) ((p)->ovlp)
#define RLBwant(p) ((p)->want)
#define RLBhave(p) ((p)->have)
#define RLBdone(p) ((p)->done)
#define RLBmine(p) ((p)->mine)

extern char *safealloc(int count, int nbytes, char *tag);
extern void safefree(char *p);
extern size_t safememcurrcount(void);
extern void safememreset(void);

extern REAL *newvec_REAL(int size, char *tag);
extern void delvec_REAL(REAL *vec);
extern IMAG *newvec_IMAG(int size, char *tag);
extern void delvec_IMAG(IMAG *vec);
extern COMPLEX *newvec_COMPLEX(int size, char *tag);
extern void delvec_COMPLEX(COMPLEX * buf);
extern COMPLEX *newvec_COMPLEX_fftw(int size, char *tag);
extern void delvec_COMPLEX_fftw(COMPLEX * buf);
extern void dump_REAL(FILE *fp, char *head, REAL *ptr, int beg, int fin);
extern void dump_IMAG(FILE *fp, char *head, IMAG *ptr, int beg, int fin);
extern void dump_CX(FILE *fp, char *head, COMPLEX *ptr, int beg, int fin);

extern CXB newCXB(int size, COMPLEX *base, char *tag);
extern void delCXB(CXB p);

extern RLB newRLB(int size, REAL *base, char *tag);
extern void delRLB(RLB p);

extern REAL normalize_vec_REAL(REAL *, int, REAL);
extern REAL normalize_vec_COMPLEX(COMPLEX *, int, REAL);

#endif
