/* filter.c
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

#include <filter.h>

//static REAL onepi = 3.141592653589793;
#define onepi M_PI
#define twopi (2*onepi)

RealFIR
newFIR_REAL (int size, char *tag)
{
	RealFIR p = (RealFIRDesc *) safealloc (1, sizeof (RealFIRDesc), tag);
	FIRcoef (p) = (REAL *) safealloc (size, sizeof (REAL), tag);
	FIRsize (p) = size;
	FIRtype (p) = FIR_Undef;
	FIRiscomplex (p) = FALSE;
	FIRfqlo (p) = FIRfqhi (p) = -1.0;
	return p;
}

ComplexFIR
newFIR_COMPLEX (int size, char *tag)
{
	ComplexFIR p =
		(ComplexFIRDesc *) safealloc (1, sizeof (ComplexFIRDesc), tag);
	FIRcoef (p) = (COMPLEX *) safealloc (size, sizeof (COMPLEX), tag);
	FIRsize (p) = size;
	FIRtype (p) = FIR_Undef;
	FIRiscomplex (p) = TRUE;
	FIRfqlo (p) = FIRfqhi (p) = -1.0;
	return p;
}

void
delFIR_REAL (RealFIR p)
{
	if (p)
	{
		delvec_REAL (FIRcoef (p));
		free ((void *) p);
	}
}

void
delFIR_COMPLEX (ComplexFIR p)
{
	if (p)
	{
		delvec_COMPLEX (FIRcoef (p));
		free ((void *) p);
	}
}

RealFIR
newFIR_Lowpass_REAL (REAL cutoff, REAL sr, int size)
{
	if ((cutoff < 0.0) || (cutoff > (sr / 2.0)))
		return 0;
	else if (size < 1)
		return 0;
	else {
		int msize = size-1;
		RealFIR p;
		REAL *h, *w, fc = cutoff / sr;
		REAL corrector = 0.0, midpoint;
		int i;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_REAL (size, "newFIR_Lowpass_REAL");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Lowpass_REAL window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);
		for (i = 0; i < size; i++)
		{
            if ((REAL)i != midpoint)
				h[i] =
					(REAL) ((sin (twopi * ((REAL)i - midpoint) * fc) /
					(onepi * ((REAL)i - midpoint))) * w[i]);
				else
					h[i] = 2.0f * fc;

		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Lowpass;
		return p;
    }
}

ComplexFIR
newFIR_Lowpass_COMPLEX (REAL cutoff, REAL sr, int size)
{
	if ((cutoff < 0.0) || (cutoff > (sr / 2.0)))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		int msize=size-1;
		BOOLEAN even=FALSE;
		ComplexFIR p;
		COMPLEX *h;
		REAL *w, fc = cutoff / sr, midpoint;
		int i;

		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_COMPLEX (size, "newFIR_Lowpass_COMPLEX");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Lowpass_REAL window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		for (i = 0; i < size; i++)
		{
            if ((REAL)i != midpoint)
				h[i].re =
					(REAL) ((sin (twopi * ((REAL)i - midpoint) * fc) /
					(onepi * ((REAL)i - midpoint))) * w[i]);
				else
					h[i].re = 2.0f * fc;
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Lowpass;
		return p;
	}
}

RealFIR
newFIR_Bandpass_REAL (REAL lo, REAL hi, REAL sr, int size)
{
	if ((lo < 0.0) || (hi > (sr / 2.0)) || (hi <= lo))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		RealFIR p;
		REAL *h, *w, fc, ff, midpoint;
		int i, msize;

		msize = size-1;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_REAL (size, "newFIR_Bandpass_REAL");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Bandpass_REAL window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		lo /= sr, hi /= sr;
		fc = (hi - lo) / 2.0f;
		ff = (REAL) ((lo + hi) * onepi);

		for (i = 0; i < size; i++)
		{
			if ((REAL)i != midpoint)
				h[i] =
				(REAL) ((sin (twopi * ((REAL)i - midpoint) * fc) /
				(onepi * ((REAL)i - midpoint))) * w[i]);
			else
				h[i] = 2.0f * fc;
			h[i] *= (REAL) (2.0 * cos (ff * ((REAL)i - midpoint)));
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Bandpass;
		return p;
    }
}

ComplexFIR
newFIR_Bandpass_COMPLEX (REAL lo, REAL hi, REAL sr, int size)
{
	if ((lo < -(sr / 2.0)) || (hi > (sr / 2.0)) || (hi <= lo))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		ComplexFIR p;
		COMPLEX *h;
		REAL *w, fc, ff, midpoint;
		int i,msize;

		msize = size-1;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_COMPLEX (size, "newFIR_Bandpass_COMPLEX");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Bandpass_COMPLEX window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		lo /= sr, hi /= sr;
		fc = (REAL) ((hi - lo) / 2.0);
		ff = (REAL) ((lo + hi) * onepi);

		for (i = 0; i < size; i++)
		{
			REAL k = (REAL)i - midpoint;
			REAL tmp, phs = ff * k;
			if ((REAL)i != midpoint)
				tmp = (REAL) ((sin (twopi * k * fc) / (onepi * k)) * w[i]);
			else
				tmp = (REAL) (2.0 * fc);
			tmp *= 2.0;
			h[i].re = (REAL) (tmp * cos (phs));
			h[i].im = (IMAG) (tmp * sin (phs));
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Bandpass;
		return p;
    }
}

RealFIR
newFIR_Highpass_REAL (REAL cutoff, REAL sr, int size)
{
	if ((cutoff < 0.0) || (cutoff > (sr / 2.0)))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		RealFIR p;
		REAL *h, *w, fc = cutoff / sr, midpoint;
		int i, msize;


		msize = size-1;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_REAL (size, "newFIR_Highpass_REAL");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Highpass_REAL window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		for (i = 0; i < size; i++)
		{
			if ((REAL)i != midpoint)
				h[i] =
				(REAL) ((sin (twopi * ((REAL)i - midpoint) * fc) /
				(onepi * ((REAL)i - midpoint))) * w[i]);
			else
				h[i] = (REAL) (2.0 * fc);
		}

		for (i = 0; i < size; i++)
		{
			if ((REAL)i != midpoint)
				h[i] = -h[i];
			else
				h[i] = 1.0f - h[i];
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Highpass;
		return p;
    }
}

ComplexFIR
newFIR_Highpass_COMPLEX (REAL cutoff, REAL sr, int size)
{
	if ((cutoff < 0.0) || (cutoff > (sr / 2.0)))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		ComplexFIR p;
		COMPLEX *h;
		REAL *w, fc = cutoff / sr, midpoint;
		int i,msize;

		msize = size-1;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_COMPLEX (size, "newFIR_Highpass_REAL");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Highpass_REAL window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		for (i = 0; i < size; i++)
		{
			if ((REAL)i != midpoint)
				h[i].re =
				(REAL) ((sin (twopi * ((REAL)i - midpoint) * fc) /
				(onepi * ((REAL)i - midpoint))) * w[i]);
			else
				h[i].re = (REAL) (2.0 * fc);
		}

		for (i = 0; i < size; i++)
		{
			if ((REAL)i != midpoint)
				h[i].re = -h[i].re;
			else
				h[i].re = (REAL) (1.0 - h[i].re);
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Highpass;
		return p;
    }
}

RealFIR
newFIR_Hilbert_REAL (REAL lo, REAL hi, REAL sr, int size)
{
	if ((lo < 0.0) || (hi > (sr / 2.0)) || (hi <= lo))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		RealFIR p;
		REAL *h, *w, fc, ff, midpoint;
		int i,msize;

		msize = size-1;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_REAL (size, "newFIR_Hilbert_REAL");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Hilbert_REAL window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		lo /= sr, hi /= sr;
		fc = (REAL) ((hi - lo) / 2.0);
		ff = (REAL) ((lo + hi) * onepi);

		for (i = 0; i < size; i++)
		{

			if ((REAL)i != midpoint)
				h[i] =
				(REAL) ((sin (twopi * ((REAL)i - midpoint) * fc) /
				(onepi * ((REAL)i - midpoint))) * w[i]);
			else
				h[i] = (REAL) (2.0 * fc);
			h[i] *= (REAL) (2.0 * sin (ff * ((REAL)i - midpoint)));
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Hilbert;
		return p;
    }
}

ComplexFIR
newFIR_Hilbert_COMPLEX (REAL lo, REAL hi, REAL sr, int size)
{
	if ((lo < 0.0) || (hi > (sr / 2.0)) || (hi <= lo))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		ComplexFIR p;
		COMPLEX *h;
		REAL *w, fc, ff, midpoint;
		int i, msize;

		msize = size-1;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_COMPLEX (size, "newFIR_Hilbert_COMPLEX");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Hilbert_COMPLEX window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		lo /= sr, hi /= sr;
		fc = (REAL) ((hi - lo) / 2.0);
		ff = (REAL) ((lo + hi) * onepi);

		for (i = 0; i < size; i++)
		{
			REAL tmp, phs = ff * ((REAL)i - midpoint),tmpang = ((REAL)i - midpoint);
			if (i != midpoint)
				tmp =
				(REAL) ((sin (twopi * tmpang * fc) /
				(onepi * tmpang)) * w[i]);
			else
				tmp = (REAL) (2.0 * fc);
			tmp *= 2.0f;
			/* h[j].re *= tmp * cos(phs); */
			h[i].im *= (REAL) (tmp * sin (phs));
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Hilbert;
		return p;
    }
}

RealFIR
newFIR_Bandstop_REAL (REAL lo, REAL hi, REAL sr, int size)
{
	if ((lo < 0.0) || (hi > (sr / 2.0)) || (hi <= lo))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		RealFIR p;
		REAL *h, *w, fc, ff, midpoint;
		int i,msize;

		msize = size-1;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_REAL (size, "newFIR_Bandstop_REAL");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Bandstop_REAL window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		lo /= sr, hi /= sr;
		fc = (REAL) ((hi - lo) / 2.0);
		ff = (REAL) ((lo + hi) * onepi);

		for (i = 0; i < size; i++)
		{
			if ((REAL)i != midpoint)
				h[i] =
				(REAL) ((sin (twopi * ((REAL)i - midpoint) * fc) /
				(onepi * ((REAL)i - midpoint))) * w[i]);
			else
				h[i] = (REAL) (2.0 * fc);
			h[i] *= (REAL) (2.0 * cos (ff * ((REAL)i - midpoint)));
		}

		for (i = 0; i < size; i++)
		{
			if ((REAL)i != midpoint)
				h[i] = -h[i];
			else
				h[i] = (REAL) (1.0 - h[i]);
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Bandstop;
		return p;
    }
}

ComplexFIR
newFIR_Bandstop_COMPLEX (REAL lo, REAL hi, REAL sr, int size)
{
	if ((lo < 0.0) || (hi > (sr / 2.0)) || (hi <= lo))
		return 0;
	else if (size < 1)
		return 0;
	else
	{
		ComplexFIR p;
		COMPLEX *h;
		REAL *w, fc, ff, midpoint;
		int i, msize;

		msize = size -1;
		midpoint = (REAL)(0.5*(REAL)msize);
		p = newFIR_COMPLEX (size, "newFIR_Bandstop_REAL");
		h = FIRcoef (p);
		w = newvec_REAL (size, "newFIR_Bandstop_REAL window");
		(void) makewindow (BLACKMANHARRIS_WINDOW, size, w);

		lo /= sr, hi /= sr;
		fc = (REAL) ((hi - lo) / 2.0);
		ff = (REAL) ((lo + hi) * onepi);
        
		for (i = 0; i < size; i++)
		{
			REAL tmp, phs = ff * ((REAL)i - midpoint);
			if ((REAL)i != midpoint)
				tmp =
				(REAL) ((sin (twopi * ((REAL)i - midpoint) * fc) /
				(onepi * ((REAL)i - midpoint))) * w[i]);
			else
				tmp = (REAL) (2.0 * fc);
			tmp *= 2.0;
			h[i].re *= (REAL) (tmp * cos (phs));
			h[i].im *= (REAL) (tmp * sin (phs));
		}

		for (i = 0; i < size; i++)
		{
			if ((REAL)i != midpoint)
				h[i] = Cmul (h[i], cxminusone);
			else
				h[i] = Csub (cxone, h[i]);
		}

		delvec_REAL (w);
		FIRtype (p) = FIR_Bandstop;
		return p;
    }
}

#ifdef notdef
int
main (int argc, char **argv)
{
	int i, size = 101;
	RealFIR filt;
#ifdef notdef
	filt = newFIR_Lowpass_REAL (1000.0, 48000.0, size);
	for (i = 0; i < FIRsize (filt); i++)
		printf ("%d %f\n", i, FIRtap (filt, i));
	delFIR_REAL (filt);
#endif

#ifdef notdef
	filt = newFIR_Bandstop_REAL (1000.0, 2000.0, 48000.0, size);
	for (i = 0; i < FIRsize (filt); i++)
		printf ("%d %f\n", i, FIRtap (filt, i));
	delFIR_REAL (filt);
#endif

	filt = newFIR_Bandpass_REAL (1000.0, 2000.0, 48000.0, size);
	for (i = 0; i < FIRsize (filt); i++)
		printf ("%d %f\n", i, FIRtap (filt, i));
	delFIR_REAL (filt);

#ifdef notdef
	filt = newFIR_Highpass_REAL (1000.0, 48000.0, size);
	for (i = 0; i < FIRsize (filt); i++)
		printf ("%d %f\n", i, FIRtap (filt, i));
	delFIR_REAL (filt);
#endif

#ifdef notdef
	filt = newFIR_Hilbert_REAL (1000.0, 2000.0, 48000.0, size);
	for (i = 0; i < FIRsize (filt); i++)
		printf ("%d %f\n", i, FIRtap (filt, i));
	delFIR_REAL (filt);
#endif

#ifdef notdef
	{
		COMPLEX *z;
		REAL *ttbl;
		int fftlen;
		fftlen = nblock2 (size) * 2;
		z = newvec_COMPLEX (fftlen, "z");
		ttbl = newvec_REAL (fftlen, "ttbl");
		cfftm (FFT_INIT, fftlen, (float *) z, (float *) z, ttbl);
		for (i = 0; i < FIRsize (filt); i++)
			z[i].re = FIRtap (filt, i);
		cfftm (FFT_FORWARD, fftlen, (float *) z, (float *) z, ttbl);
		for (i = 0; i < size; i++)
		{
			printf ("%d %f\n", i, Cabs (z[i]));
			delvec_COMPLEX (z);
			delvec_REAL (ttbl);
		}
	}
#endif
	exit (0);
}
#endif
