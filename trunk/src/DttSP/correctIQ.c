/* correctIQ.c

This routine restores quadrature between arms of an analytic signal
possibly distorted by ADC hardware.

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

#include <common.h>



IQ
newCorrectIQ (REAL phase, REAL gain, REAL mu)
{
	IQ iq = (IQ) safealloc (1, sizeof (iqstate), "IQ state");
	iq->phase = phase;
	iq->gain = gain;
	iq->mu = mu;
	iq->leakage = 0.000000f;
	iq->MASK=15;
	iq->index=0;
	iq->w = (COMPLEX *)safealloc(16,sizeof(COMPLEX),"correctIQ w");
	iq->y = (COMPLEX *)safealloc(16,sizeof(COMPLEX),"correctIQ y");
	iq->del = (COMPLEX *)safealloc(16,sizeof(COMPLEX),"correctIQ del");
	memset((void *)iq->w,0,16*sizeof(COMPLEX));
	iq->wbir_tuned = TRUE;
	iq->wbir_state = FastAdapt;
	return iq;
}

void
delCorrectIQ (IQ iq)
{
	safefree((char *)iq->w);
	safefree((char *)iq->y);
	safefree((char *)iq->del);
	safefree ((char *) iq);
}

int IQdoit = 1;

void
correctIQ (CXB sigbuf, IQ iq, BOOLEAN isTX, int subchan)
{
	int i;
	REAL doit;
	if (IQdoit == 0) return;
	if (subchan == 0) doit = iq->mu;
	else doit = 0;
	if(!isTX)
	{

		// if (subchan == 0) // removed so that sub rx's will get IQ correction
		switch (iq->wbir_state) {
			case FastAdapt:
				break;
			case SlowAdapt:
				break;
			case NoAdapt:
				break;
			default:
				break;
		}

		for (i = 0; i < CXBhave (sigbuf); i++)
		{
			iq->del[iq->index] = CXBdata(sigbuf, i);
			iq->y[iq->index] = Cadd(iq->del[iq->index], Cmul(iq->w[0], Conjg(iq->del[iq->index])));
			iq->y[iq->index] = Cadd(iq->y[iq->index], Cmul(iq->w[1], Conjg(iq->y[iq->index])));
			iq->w[1] = Csub(iq->w[1], Cscl(Cmul(iq->y[iq->index], iq->y[iq->index]), doit));  // this is where the adaption happens

			CXBdata(sigbuf, i) = iq->y[iq->index];
			iq->index = (iq->index + iq->MASK) & iq->MASK;
		}
		//fprintf(stderr, "w1 real: %g, w1 imag: %g\n", iq->w[1].re, iq->w[1].im); fflush(stderr); 
	}
	else
	{
		for (i = 0; i < CXBhave (sigbuf); i++)
		{
			CXBimag (sigbuf, i) += iq->phase * CXBreal (sigbuf, i);
			CXBreal (sigbuf, i) *= iq->gain;
		}
	}

}
