/* am_demod.c 

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004,2005,2006, 2007 by Frank Brickle, AB2KT and Bob McGwier, N4HY

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



#include <am_demod.h>
#include <cxops.h>


/*------------------------------------------------------------------------------*/
/* private to AM */
/*------------------------------------------------------------------------------*/

static void
init_pll (AMD am,
	  REAL samprate, REAL freq, REAL lofreq, REAL hifreq, REAL bandwidth)
{
	REAL fac = (REAL) (TWOPI / samprate);
	am->pll.freq.f = freq * fac;
	am->pll.freq.l = lofreq * fac;
	am->pll.freq.h = hifreq * fac;
	am->pll.phs = 0.0;
	am->pll.delay = cxJ;

	am->pll.iir.alpha = bandwidth * fac;	/* arm filter */
	am->pll.alpha = am->pll.iir.alpha * 0.3f;	/* pll bandwidth */
	am->pll.beta = am->pll.alpha * am->pll.alpha * 0.25f;	/* second order term */
	am->pll.fast_alpha = am->pll.alpha;
}

static void
pll (AMD am, COMPLEX sig)
{
	COMPLEX z = Cmplx ((REAL) cos (am->pll.phs), (REAL) sin (am->pll.phs));
	REAL diff;

	am->pll.delay.re = z.re * sig.re + z.im * sig.im;
	am->pll.delay.im = -z.im * sig.re + z.re * sig.im;
	diff = fast_atan2 (am->pll.delay.im, am->pll.delay.re);

	am->pll.freq.f += am->pll.beta * diff;

	if (am->pll.freq.f < am->pll.freq.l)
		am->pll.freq.f = am->pll.freq.l;
	if (am->pll.freq.f > am->pll.freq.h)
		am->pll.freq.f = am->pll.freq.h;

	am->pll.phs += am->pll.freq.f + am->pll.alpha * diff;

	while (am->pll.phs >= TWOPI)
		am->pll.phs -= (REAL) TWOPI;
	while (am->pll.phs < 0)
		am->pll.phs += (REAL) TWOPI;
}

static REAL
dem (AMD am)
{
	am->lock.curr =
		(REAL) (0.999 * am->lock.curr + 0.001 * fabs (am->pll.delay.im));

	/* env collapse? */
	/* if ((am->lock.curr < 0.05) && (am->lock.prev >= 0.05))
	{
		am->pll.alpha = 0.1 * am->pll.fast_alpha;
		am->pll.beta = am->pll.alpha * am->pll.alpha * 0.25;
    }
	else if ((am->pll.alpha > 0.05) && (am->lock.prev <= 0.05))
	{
		am->pll.alpha = am->pll.fast_alpha;
    } */
	am->lock.prev = am->lock.curr;
	am->dc = 0.99999f * am->dc + 0.00001f * am->pll.delay.re;
	return am->pll.delay.re - am->dc;
}

/*------------------------------------------------------------------------------*/
/* public */
/*------------------------------------------------------------------------------*/

void
AMDemod (AMD am)
{
	int i;
	REAL demout;
	switch (am->mode)
	{
		case SAMdet:
			for (i = 0; i < am->size; i++)
			{
				pll (am, CXBdata (am->ibuf, i));
				demout = dem (am);
				CXBdata (am->obuf, i) = Cmplx (demout, demout);
			}
			break;
		case AMdet:
			for (i = 0; i < am->size; i++)
			{
				am->lock.curr = Cmag (CXBdata (am->ibuf, i));
				am->dc = 0.9999f * am->dc + 0.0001f * am->lock.curr;
				am->smooth = 0.5f * am->smooth + 0.5f * (am->lock.curr - am->dc);
				/* demout = am->smooth; */
				CXBdata (am->obuf, i) = Cmplx (am->smooth, am->smooth);
			}
			break;
	}
}

AMD
newAMD (REAL samprate,
	REAL f_initial,
	REAL f_lobound,
	REAL f_hibound,
	REAL f_bandwid,
	int size, COMPLEX * ivec, COMPLEX * ovec, AMMode mode, char *tag)
{
	AMD am = (AMD) safealloc (1, sizeof (AMDDesc), tag);

	am->size = size;
	am->ibuf = newCXB (size, ivec, tag);
	am->obuf = newCXB (size, ovec, tag);
	am->mode = mode;
	init_pll (am, samprate, f_initial, f_lobound, f_hibound, f_bandwid);

	am->lock.curr = 0.5;
	am->lock.prev = 1.0;
	am->dc = 0.0;

	return am;
}

void
delAMD (AMD am)
{
	if (am)
	{
		delCXB (am->ibuf);
		delCXB (am->obuf);
		safefree ((char *) am);
	}
}
