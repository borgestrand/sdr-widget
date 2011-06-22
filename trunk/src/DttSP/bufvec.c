/* bufvec.c
   creation, deletion, management for vectors and buffers 
   
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

//------------------------------------------------------------------------

#include <bufvec.h>
#include <fftw3.h>
#include <fftw3_fix.h>
//#include <malloc16.h>
//------------------------------------------------------------------------
// wrapper around calloc

PRIVATE size_t _safemem_currcount = 0;

char *
safealloc(int count, int nbytes, char *tag)
{
	char *p = calloc(count, nbytes);
	if (!p)
	{
		if (tag && *tag)
			fprintf(stderr, "safealloc: %s\n", tag);
		else
			perror("safealloc");
		exit(1);
	}

	_safemem_currcount += count * nbytes;

	return p;
}

void
safefree(char *p) 
{
	if (p)
		free((void *) p);
}

size_t
safememcurrcount(void)
{
	return _safemem_currcount;
}

void
safememreset(void)
{
	_safemem_currcount = 0;
}

//------------------------------------------------------------------------
// allocate/free just vectors

REAL *
newvec_REAL(int size, char *tag)
{
	return (REAL *) safealloc(size, sizeof(REAL), tag);
}

void
delvec_REAL(REAL *vec)
{
	safefree((char *) vec);
}

IMAG *
newvec_IMAG(int size, char *tag)
{
	return (IMAG *) safealloc(size, sizeof(IMAG), tag);
}

void
delvec_IMAG(IMAG *vec)
{
	safefree((char *) vec);
}

COMPLEX *
newvec_COMPLEX(int size, char *tag)
{
	return (COMPLEX *) safealloc(size, sizeof(COMPLEX), tag);
}

void
delvec_COMPLEX(COMPLEX *vec)
{
	safefree((char *) vec);
}

COMPLEX *
newvec_COMPLEX_fftw(int size, char *tag)
{
	COMPLEX *p = fftwf_malloc(size*sizeof(COMPLEX));
	if (!p)
	{
		if (tag && *tag)
			fprintf(stderr, "fftwf_mlloc: %s\n", tag);
		else
			perror("fftw malloc");
		exit(1);
	}
    
	return p;
}

void
delvec_COMPLEX_fftw(COMPLEX *vec)
{
	fftwf_free((char *) vec);
}

//------------------------------------------------------------------------
// buffers (mainly i/o)
//------------------------------------------------------------------------
// complex

CXB
newCXB(int size, COMPLEX *base, char *tag) 
{
	CXB p = (CXB) safealloc(1, sizeof(CXBuffer), tag);
	if (base) 
	{
		CXBbase(p) = base;
		CXBmine(p) = FALSE;
	}
	else
	{
		CXBbase(p) = newvec_COMPLEX(size, "newCXB");
		CXBmine(p) = TRUE;
	}
	CXBsize(p) = CXBwant(p) = size;
	CXBovlp(p) = CXBhave(p) = CXBdone(p) = 0;
	return p;
}

void
delCXB(CXB p)
{
	if (p)
	{
		if (CXBmine(p))
			delvec_COMPLEX(CXBbase(p));
		safefree((char *) p);
	}
}

//------------------------------------------------------------------------
// real

RLB
newRLB(int size, REAL *base, char *tag)
{
	RLB p = (RLB) safealloc(1, sizeof(RLBuffer), tag);
	if (base)
	{
		RLBbase(p) = base;
		RLBmine(p) = FALSE;
	}
	else
	{
		RLBbase(p) = newvec_REAL(size, "newRLB");
		RLBmine(p) = TRUE;
	}
	RLBsize(p) = RLBwant(p) = size;
	RLBovlp(p) = RLBhave(p) = RLBdone(p) = 0;
	return p;
}

void
delRLB(RLB p)
{
	if (p)
	{
		if (p->mine)
			delvec_REAL(RLBbase(p));
		safefree((char *) p);
	}
}

//========================================================================
// return normalization constant

REAL
normalize_vec_REAL(REAL *v, int n, REAL scl)
{
	if (v && (n > 0))
	{
		int i;
		REAL big = -(REAL) MONDO;
		for (i = 0; i < n; i++) 
		{
			REAL a = abs(v[i]);
			big = max(big, a);
		}
		if (big > 0.0)
		{
			REAL scl = (REAL) (1.0 / big);
			for (i = 0; i < n; i++)
				v[i] *= scl;
			return scl;
		}
		else return 0.0;
	}
	else return 0.0;
}

REAL
normalize_vec_COMPLEX(COMPLEX *z, int n, REAL scale)
{
	if (z && (n > 0)) 
	{
		int i;
		REAL big = -(REAL) MONDO;
		for (i = 0; i < n; i++) 
		{
			REAL a = Cabs(z[i]);
			big = max(big, a);
		}
		if (big > 0.0) 
		{
			REAL scl = (REAL) (scale / big);
			for (i = 0; i < n; i++)
				z[i] = Cscl(z[i], scl);
			return scl;
		}
		else return 0.0;
	}
	else return 0.0;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// mostly for debugging when necessary

void
dump_REAL(FILE * fp, char *head, REAL *ptr, int beg, int fin) 
{
	int i;
	FILE *iop = fp ? fp : stderr;
	if (head && *head)
		fprintf(iop, "dump_REAL: %s\n", head);
	for (i = beg; i < fin; i++)
		fprintf(iop, "%5d %g\n", i, ptr[i]);
}

void
dump_IMAG(FILE * fp, char *head, IMAG *ptr, int beg, int fin) 
{
	int i;
	FILE *iop = fp ? fp : stderr;
	if (head && *head)
		fprintf(iop, "dump_REAL: %s\n", head);
	for (i = beg; i < fin; i++)
		fprintf(iop, "%5d %g\n", i, ptr[i]);
}

void
dump_CX(FILE * fp, char *head, COMPLEX *ptr, int beg, int fin) 
{
	int i;
	FILE *iop = fp ? fp : stderr;
	if (head && *head)
		fprintf(iop, "dump_CX: %s\n", head);
	for (i = beg; i < fin; i++)
		fprintf(iop, "%5d %g %g\n", i, ptr[i].re, ptr[i].im);
}
