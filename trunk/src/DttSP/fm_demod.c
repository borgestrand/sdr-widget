/* fm_demod.c */

/* This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006, 2007 by Frank Brickle, AB2KT and Bob McGwier, N4HY

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

#include <fm_demod.h>

/*------------------------------------------------------------------------------*/
/* private to FM */
/*------------------------------------------------------------------------------*/

PRIVATE void
init_pll (FMD fm,
	  REAL samprate, REAL freq, REAL lofreq, REAL hifreq, REAL bandwidth)
{
	REAL fac = (REAL) (TWOPI / samprate);

	fm->pll.freq.f = freq * fac;
	fm->pll.freq.l = lofreq * fac;
	fm->pll.freq.h = hifreq * fac;
	fm->pll.phs = 0.0;
	fm->pll.delay = cxJ;

	fm->pll.iir.alpha = bandwidth * fac;	/* arm filter */
	fm->pll.alpha = fm->pll.iir.alpha * 0.3f;	/* pll bandwidth */
	fm->pll.beta = fm->pll.alpha * fm->pll.alpha * 0.25f;	/* second order term */
}

PRIVATE void
pll (FMD fm, COMPLEX sig)
{
	COMPLEX z = Cmplx ((REAL) cos (fm->pll.phs), (IMAG) sin (fm->pll.phs));
	REAL diff;

	fm->pll.delay.re = z.re * sig.re + z.im * sig.im;
	fm->pll.delay.im = -z.im * sig.re + z.re * sig.im;
	diff = fast_atan2 (fm->pll.delay.im, fm->pll.delay.re);

	fm->pll.freq.f += fm->pll.beta * diff;

	if (fm->pll.freq.f < fm->pll.freq.l)
		fm->pll.freq.f = fm->pll.freq.l;
	if (fm->pll.freq.f > fm->pll.freq.h)
		fm->pll.freq.f = fm->pll.freq.h;

	fm->pll.phs += fm->pll.freq.f + fm->pll.alpha * diff;

	while (fm->pll.phs >= TWOPI)
		fm->pll.phs -= (REAL) TWOPI;
	while (fm->pll.phs < 0)
		fm->pll.phs += (REAL) TWOPI;
}

/*------------------------------------------------------------------------------*/
/* public */
/*------------------------------------------------------------------------------*/

void
FMDemod (FMD fm)
{
	int i;
	for (i = 0; i < CXBsize (fm->ibuf); i++)
	{
		pll (fm, CXBdata (fm->ibuf, i));
		fm->afc = (REAL) (0.9999 * fm->afc + 0.0001 * fm->pll.freq.f);
		CXBreal (fm->obuf, i) =
			CXBimag (fm->obuf, i) = (fm->pll.freq.f - fm->afc) * fm->cvt;
	}
}

FMD
newFMD (REAL samprate,
		REAL f_initial,
		REAL f_lobound,
		REAL f_hibound,
		REAL f_bandwid, int size, COMPLEX * ivec, COMPLEX * ovec, char *tag)
{
	FMD fm = (FMD) safealloc (1, sizeof (FMDDesc), tag);

	fm->size = size;
	fm->ibuf = newCXB (size, ivec, tag);
	fm->obuf = newCXB (size, ovec, tag);

	init_pll (fm, samprate, f_initial, f_lobound, f_hibound, f_bandwid);

	fm->lock = 0.5;
	fm->afc = 0.0;
	fm->cvt = (REAL) (0.45 * samprate / (M_PI * f_bandwid));

	return fm;
}

void
delFMD (FMD fm)
{
	if (fm)
	{
		delCXB (fm->ibuf);
		delCXB (fm->obuf);
		safefree ((char *) fm);
	}
}
