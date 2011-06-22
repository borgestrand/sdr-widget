/* spectrum.c */

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

#include <spectrum.h>
#include <bufvec.h>
#include <fftw3.h>
#include <fftw3_fix.h>
// snapshot of current signal
void
snap_spectrum (SpecBlock * sb, int label)
{
	int i, j;

	// where most recent signal started
	j = sb->fill;

	// copy starting from there in circular fashion,
	// applying window as we go
	if (!sb->polyphase)
	{
		for (i = 0; i < sb->size; i++)
		{
			CXBdata (sb->timebuf, i) =
				Cscl (CXBdata (sb->accum, j), sb->window[i]);
			j = (++j & sb->mask);
		}
	}
	else
	{
		int k;
		for (i = 0; i < sb->size; i++)
		{
			CXBreal (sb->timebuf, i) = CXBreal (sb->accum, j) * sb->window[i];
			CXBimag (sb->timebuf, i) = CXBimag (sb->accum, j) * sb->window[i];
			for (k = 1; k < 8; k++)
			{
				int accumidx = (j + k * sb->size) & sb->mask;
				int winidx = i + k * sb->size;
				CXBreal (sb->timebuf, i) +=
					CXBreal (sb->accum, accumidx) * sb->window[winidx];
				CXBimag (sb->timebuf, i) +=
					CXBimag (sb->accum, accumidx) * sb->window[winidx];
			}
			j = (++j & sb->mask);
		}

	}
	sb->label = label;
}

void
snap_scope (SpecBlock * sb, int label)
{
	int i, j;

	// where most recent signal started
	j = sb->fill;

	// copy starting from there in circular fashion
	for (i = 0; i < sb->size; i++)
	{
		CXBdata (sb->timebuf, i) = CXBdata (sb->accum, j);
		j = (++j & sb->mask);
	}

	sb->label = label;
}

void
compute_complex_spectrum(SpecBlock * sb)
{
	int i, j, half = sb->size / 2;

	// assume timebuf has windowed current snapshot

	fftwf_execute (sb->plan);

	for (i = 0, j = half; i < half; i++, j++) {
		sb->coutput[i] = CXBdata (sb->freqbuf, j);
		sb->coutput[j] = CXBdata (sb->freqbuf, i);
	}	
}

// snapshot -> frequency domain
void
compute_spectrum (SpecBlock * sb)
{
	int i, j, half = sb->size / 2;


	// assume timebuf has windowed current snapshot

	fftwf_execute (sb->plan);

	if (sb->scale == SPEC_MAG)
	{
		for (i = 0, j = half; i < half; i++, j++)
		{
			sb->output[i] = (float) Cmag (CXBdata (sb->freqbuf, j));
			sb->output[j] = (float) Cmag (CXBdata (sb->freqbuf, i));
		}
	}
	else
	{				// SPEC_PWR
		for (i = 0, j = half; i < half; i++, j++)
		{
			sb->output[i] =
				(float) (10.0 *
				log10 (Csqrmag (CXBdata (sb->freqbuf, j)) + 1e-60));
			sb->output[j] =
				(float) (10.0 *
				log10 (Csqrmag (CXBdata (sb->freqbuf, i)) + 1e-60));
		}
	}
}

void
init_spectrum (SpecBlock * sb)
{
	COMPLEX *p;
	sb->fill = 0;

	p = newvec_COMPLEX_fftw(sb->size*16,"spectrum accum");
	sb->accum = newCXB (sb->size * 16, p, "spectrum accum");
	p = newvec_COMPLEX_fftw(sb->size, "spectrum timebuf");
	sb->timebuf = newCXB (sb->size, p, "spectrum timebuf");
	p = newvec_COMPLEX_fftw(sb->size, "spectrum timebuf");
	sb->freqbuf = newCXB (sb->size, p, "spectrum freqbuf");
	sb->window = newvec_REAL (sb->size * 16, "spectrum window");
	makewindow (BLACKMANHARRIS_WINDOW, sb->size, sb->window);
	sb->mask = sb->size - 1;
	sb->polyphase = FALSE;
	sb->output =
		(float *) safealloc (sb->size, sizeof (float), "spectrum output");
	sb->coutput = (COMPLEX *)safealloc (sb->size, sizeof (COMPLEX), "spectrum output");;
	sb->plan =
		fftwf_plan_dft_1d (sb->size, (fftwf_complex *) CXBbase (sb->timebuf),
		(fftwf_complex *) CXBbase (sb->freqbuf),
		FFTW_FORWARD, sb->planbits);
}

void
reinit_spectrum (SpecBlock * sb)
{
	size_t polysize = 1;
	sb->fill = 0;
	if (sb->polyphase)
		polysize = 8;
	memset ((char *) CXBbase (sb->accum), 0,
		polysize * sb->size * sizeof (REAL));
	memset ((char *) sb->output, 0, sb->size * sizeof (float));
	memset ((char *) sb->coutput, 0, sb->size * sizeof(COMPLEX));
}

void
finish_spectrum (SpecBlock * sb)
{
	if (sb)
	{
		delvec_COMPLEX_fftw(sb->accum->data);
		delCXB (sb->accum);
		delvec_COMPLEX_fftw(sb->timebuf->data);
		delCXB (sb->timebuf);
		delvec_COMPLEX_fftw(sb->freqbuf->data);
		delCXB (sb->freqbuf);
		delvec_REAL (sb->window);
		safefree ((char *) sb->output);
		safefree ((char *) sb->coutput);
		fftwf_destroy_plan (sb->plan);
	}
}
