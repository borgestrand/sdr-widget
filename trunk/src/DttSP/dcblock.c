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

#include <dcblock.h>

// NB may have to ramify this a little
// for other sampling rates; maybe not  

PRIVATE REAL
butterworth_hipass_100_2 (REAL xin, REAL * xv, REAL * yv),
butterworth_hipass_100_4 (REAL xin, REAL * xv, REAL * yv),
butterworth_hipass_100_6 (REAL xin, REAL * xv, REAL * yv),
butterworth_hipass_100_8 (REAL xin, REAL * xv, REAL * yv);

void
DCBlock (DCBlocker dcb)
{
	int i;
	REAL x, y;

	switch (dcb->lev)
	{
		case DCB_LOW:
			for (i = 0; i < CXBsize (dcb->buf); i++)
			{
				x = CXBreal (dcb->buf, i);
				y = butterworth_hipass_100_2 (x, dcb->old.inp, dcb->old.out);
				CXBdata (dcb->buf, i) = Cmplx (y, 0.0);
			}
			break;

		case DCB_MED:
			for (i = 0; i < CXBsize (dcb->buf); i++)
			{
				x = CXBreal (dcb->buf, i);
				y = butterworth_hipass_100_4 (x, dcb->old.inp, dcb->old.out);
				CXBdata (dcb->buf, i) = Cmplx (y, 0.0);
			}
			break;

		case DCB_HIGH:
			for (i = 0; i < CXBsize (dcb->buf); i++)
			{
				x = CXBreal (dcb->buf, i);
				y = butterworth_hipass_100_6 (x, dcb->old.inp, dcb->old.out);
				CXBdata (dcb->buf, i) = Cmplx (y, 0.0);
			}
			break;

		case DCB_SUPER:
			for (i = 0; i < CXBsize (dcb->buf); i++)
			{
				x = CXBreal (dcb->buf, i);
				y = butterworth_hipass_100_8 (x, dcb->old.inp, dcb->old.out);
				CXBdata (dcb->buf, i) = Cmplx (y, 0.0);
			}
			break;
		case DCB_SINGLE_POLE:
			for (i = 0; i < CXBsize (dcb->buf); i++)
			{
				COMPLEX x=CXBdata(dcb->buf,i);
				dcb->sigval = Cadd(Cscl(x,.00005f), Cscl(dcb->sigval,0.99995f));
				CXBdata (dcb->buf, i) = Csub(x,dcb->sigval);
			}
			break;
		default:
			break;
    }
}

void
resetDCBlocker (DCBlocker dcb, int lev)
{
	memset ((char *) dcb->old.inp, 0, BLKMEM * sizeof (REAL));
	memset ((char *) dcb->old.out, 0, BLKMEM * sizeof (REAL));
	dcb->lev = lev;
}

DCBlocker
newDCBlocker (int lev, CXB buf)
{
	DCBlocker dcb =
		(DCBlocker) safealloc (1, sizeof (DCBlockerInfo), "DCBlocker");
	dcb->buf = newCXB (CXBsize (buf), CXBbase (buf), "DCBlocker");
	dcb->lev = lev;
	dcb->sigval = cxzero;
	return dcb;
}

void
delDCBlocker (DCBlocker dcb)
{
	if (dcb)
	{
		delCXB (dcb->buf);
		safefree ((char *) dcb);
	}
}

// f == 0.002083 == 100 Hz at 48k

PRIVATE REAL
butterworth_hipass_100_2 (REAL xin, REAL * xv, REAL * yv)
{
	int i;

	for (i = 1; i < 2; i++)
		xv[i - 1] = xv[i], yv[i - 1] = yv[i];

	xv[2] = (REAL) (xin / 1.009297482);

	yv[2] = (REAL) ((xv[0] + xv[2])
		+ -2.0 * xv[1]
		+ -0.9816611902 * yv[0] 
		+ 1.9814914708 * yv[1]);

	return yv[2];
}

PRIVATE REAL
butterworth_hipass_100_4 (REAL xin, REAL * xv, REAL * yv)
{
	int i;

	for (i = 1; i < 4; i++)
		xv[i - 1] = xv[i], yv[i - 1] = yv[i];

	xv[4] = (REAL) (xin / 1.012);

	yv[4] = (REAL) ((xv[0] + xv[4])
		+ -4.0 * (xv[1] + xv[3])
		+ 6.0 * xv[2]
		+ -0.976340271 * yv[0]
		+ 3.928738552 * yv[1]
		+ -5.928454312 * yv[2] 
		+ 3.976056024 * yv[3]);

	return yv[4];
}

PRIVATE REAL
butterworth_hipass_100_6 (REAL xin, REAL * xv, REAL * yv)
{
	int i;

	for (i = 1; i < 6; i++)
		xv[i - 1] = xv[i], yv[i - 1] = yv[i];

	xv[6] = (REAL) (xin / 1.025606415);

	yv[6] = (REAL) ((xv[0] + xv[6])
		+ -6.0 * (xv[1] + xv[5])
		+ 15.0 * (xv[2] + xv[4])
		+ -20.0 * xv[3]
		+ -0.9506891622 * yv[0]
		+ 5.7522090378 * yv[1]
		+ -14.5019247580 * yv[2]
		+ 19.4994114580 * yv[3]
		+ -14.7484389800 * yv[4]
		+ 5.9494324049 * yv[5]);

	return yv[6];
}

PRIVATE REAL
butterworth_hipass_100_8 (REAL xin, REAL * xv, REAL * yv)
{
	int i;

	for (i = 1; i < 8; i++)
		xv[i - 1] = xv[i], yv[i - 1] = yv[i];

	xv[8] = (REAL) (xin / 1.034112352);

	yv[8] = (REAL) ((xv[0] + xv[8])
		+ -8.0 * (xv[1] + xv[7])
		+ 28.0 * (xv[2] + xv[6])
		+ -56.0 * (xv[3] + xv[5])
		+ 70.0 * xv[4]
		+ -0.9351139781 * yv[0]
		+ 7.5436450525 * yv[1]
		+ -26.6244301320 * yv[2]
		+ 53.6964633920 * yv[3]
		+ -67.6854640540 * yv[4]
		+ 54.6046308830 * yv[5]
		+ -27.5326449810 * yv[6]
		+ 7.9329138172 * yv[7]);

	return yv[8];
}
