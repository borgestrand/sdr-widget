// wscompand.c
// waveshaping compander, mostly for speech
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

#include <wscompand.h>

PRIVATE INLINE REAL
WSCLookup (WSCompander wsc, REAL x)
{
	if (x > 0.0)
	{
		/*REAL d = x - (int) x, y, *tbl = wsc->tbl;
		int i = (int) (x * wsc->npts), end = wsc->nend;*/
		int i = (int) (x * wsc->npts), end = wsc->nend;
		REAL d = x * wsc->npts - i, y, *tbl = wsc->tbl;

		if (i < end)
			y = tbl[i] + d * (tbl[i + 1] - tbl[i]);
		else
			y = tbl[end];
		return y / x;
	}
	else
	{
		return 0.0;
	}
}

void
WSCompand (WSCompander wsc)
{
	int i, n = CXBsize (wsc->buff);

	if (wsc->fac != 0.0) 
	{
		for (i = 0; i < n ; i++)
		{
			COMPLEX val = CXBdata (wsc->buff, i);
			REAL mag = Cmag (val), scl = WSCLookup (wsc, mag);
			CXBdata (wsc->buff, i) = Cscl (val,scl);
		}
	}
}

void
WSCReset (WSCompander wsc, REAL fac)
{
	int i;
	REAL *tbl = wsc->tbl;

	if (fac == 0.0)		// just linear
	{
		for (i = 0; i < wsc->npts; i++)
			tbl[i] = i / (REAL) wsc->nend;
	}
	else
	{				// exponential
		REAL del = fac / wsc->nend, scl = (REAL) (1.0 - exp (fac));
		for (i = 0; i < wsc->npts; i++)
			tbl[i] = (REAL) ((1.0 - exp (i * del)) / scl);
	}
	wsc->fac = fac;
}

// fac < 0: compression
// fac > 0: expansion

WSCompander
newWSCompander (int npts, REAL fac, CXB buff)
{
	WSCompander wsc;

	wsc = (WSCompander) safealloc (1,
		sizeof (WSCompanderInfo),
		"WSCompander struct");
	wsc->npts = npts;
	wsc->nend = npts - 1;
	wsc->tbl = newvec_REAL (npts, "WSCompander table");
	wsc->buff = newCXB (CXBsize (buff), CXBbase (buff), "WSCompander buff");
	WSCReset (wsc, fac);
	return wsc;
}

void
delWSCompander (WSCompander wsc)
{
	if (wsc)
	{
		delvec_REAL (wsc->tbl);
		delCXB (wsc->buff);
		safefree ((char *) wsc);
	}
}
