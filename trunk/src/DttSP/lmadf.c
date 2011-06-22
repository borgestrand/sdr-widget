/* lmadf.c 

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

#include <lmadf.h>

// just to make the algorithm itself a little clearer,
// get the admin stuff out of the way

#define ssiz (lms->signal_size)
#define asiz (lms->adaptive_filter_size)
#define dptr (lms->delay_line_ptr)
#define rate (lms->adaptation_rate)
#define leak (lms->leakage)

#define ssig(n)   (CXBreal(lms->signal,(n)))
#define ssig_i(n) (CXBimag(lms->signal,(n)))
#define cssig(n)  (CXBdata(lms->signal,(n)))

#define dlay(n) (lms->delay_line[(n)])

#define afil(n) (lms->adaptive_filter[(n)])
#define wrap(n) (((n) + (lms->delay) + (lms->delay_line_ptr)) & (lms->mask))
#define bump(n) (((n) + (lms->mask)) & (lms->mask))

#ifdef REALLMS
LMSR
new_lmsr (CXB signal,
	  int delay,
	  REAL adaptation_rate,
	  REAL leakage, int adaptive_filter_size, int filter_type)
{
  LMSR lms = (LMSR) safealloc (1, sizeof (_lmsstate), "new_lmsr state");

  lms->signal = signal;
  lms->signal_size = CXBsize (lms->signal);
  lms->delay = delay;
  lms->size = 512;
  lms->mask = lms->size - 1;
  lms->delay_line = newvec_REAL (lms->size, "lmsr delay");
  lms->adaptation_rate = adaptation_rate;
  lms->leakage = leakage;
  lms->adaptive_filter_size = adaptive_filter_size;
  lms->adaptive_filter = newvec_REAL (128, "lmsr filter");
  lms->filter_type = filter_type;
  lms->delay_line_ptr = 0;

  return lms;
}

void
del_lmsr (LMSR lms)
{
  if (lms)
    {
      delvec_REAL (lms->delay_line);
      delvec_REAL (lms->adaptive_filter);
      safefree ((char *) lms);
    }
}



static void
lmsr_adapt_i (LMSR lms)
{
  int i, j, k;
  REAL sum_sq, scl1, scl2;
  REAL accum, error;

  scl1 = (REAL) (1.0 - rate * leak);

  for (i = 0; i < ssiz; i++)
    {

      dlay (dptr) = ssig (i);
      accum = 0.0;
      sum_sq = 0.0;

      for (j = 0; j < asiz; j++)
		{
			k = wrap (j);
			sum_sq += sqr (dlay (k));
			accum += afil (j) * dlay (k);
		}

      error = ssig (i) - accum;
      ssig_i (i) = ssig (i) = error;

      scl2 = (REAL) (rate / (sum_sq + 1.19e-6));
      error *= scl2;
      for (j = 0; j < asiz; j++)
		{
			k = wrap (j);
			afil (j) = afil (j) * scl1 + error * dlay (k);
		}

      dptr = bump (dptr);
    }
}

static void
lmsr_adapt_n (LMSR lms)
{
  int i, j, k;
  REAL sum_sq, scl1, scl2;
  REAL accum, error;

  scl1 = (REAL) (1.0 - rate * leak);

  for (i = 0; i < ssiz; i++)
    {

      dlay (dptr) = ssig (i);
      accum = 0.0;
      sum_sq = 0.0;

      for (j = 0; j < asiz; j++)
		{
			k = wrap (j);
			sum_sq += sqr (dlay (k));
			accum += afil (j) * dlay (k);
		}

      error = ssig (i) - accum;
      ssig_i (i) = ssig (i) = accum;

      scl2 = (REAL) (rate / (sum_sq + 1.19e-6));
      error *= scl2;
      for (j = 0; j < asiz; j++)
		{
			k = wrap (j);
			afil (j) = afil (j) * scl1 + error * dlay (k);
		}

      dptr = bump (dptr);
    }
}

#else
LMSR
new_lmsr (CXB signal,
	  int delay,
	  REAL adaptation_rate,
	  REAL leakage, int adaptive_filter_size, int filter_type)
{
  LMSR lms = (LMSR) safealloc (1, sizeof (_lmsstate), "new_lmsr state");

  lms->signal = newCXB(signal->size,CXBbase(signal),"lmadf CXB");
  lms->signal_size = CXBsize (lms->signal);
  lms->delay = delay;
  lms->size = 4096;
  lms->mask = lms->size - 1;
  lms->delay_line = newvec_COMPLEX (lms->size, "lmsr delay");
  lms->adaptation_rate = adaptation_rate;
  lms->leakage = leakage;
  lms->adaptive_filter_size = adaptive_filter_size;
  lms->adaptive_filter = newvec_COMPLEX (128, "lmsr filter");
  lms->filter_type = filter_type;
  lms->delay_line_ptr = 0;

  return lms;
}

void
del_lmsr (LMSR lms)
{
  if (lms)
    {
	  delCXB(lms->signal);
      delvec_COMPLEX (lms->delay_line);
      delvec_COMPLEX (lms->adaptive_filter);
      safefree ((char *) lms);
    }
}



static void
lmsr_adapt_i (LMSR lms)
{
  int i, j, k;
  REAL sum_sq, scl1, scl2;
  COMPLEX accum, error;

  scl1 = (REAL) (1.0 - rate * leak);

  for (i = 0; i < ssiz; i++)
    {

      dlay (dptr) = CXBdata (lms->signal, i);
      accum = cxzero;
      sum_sq = 0;

      for (j = 0; j < asiz; j++)
		{
			k = wrap (j);
			sum_sq += Csqrmag (dlay (k));
			accum.re += afil (j).re * dlay (k).re;
			accum.im += afil (j).im * dlay (k).im;
		}

      error = Csub(cssig(i),accum);
	  cssig(i) = error;
//     ssig_i (i) = error.im;
//	  ssig (i) = error.re;

      scl2 = (REAL) (rate / (sum_sq + 1.19e-7));
      error = Cscl(error,scl2);
      for (j = 0; j < asiz; j++)
		{
			k = wrap (j);
			afil (j).re = afil (j).re * scl1 + error.re * dlay (k).re;
			afil (j).im = afil (j).im * scl1 + error.im * dlay (k).im;
		}

      dptr = bump (dptr);
    }
}

static void
lmsr_adapt_n (LMSR lms)
{
  int i, j, k;
  REAL sum_sq, scl1, scl2;
  COMPLEX accum, error;

  scl1 = (REAL) (1.0 - rate * leak);

  for (i = 0; i < ssiz; i++)
    {

      dlay (dptr) = cssig (i);
      accum = cxzero;
      sum_sq = 0.0;

      for (j = 0; j < asiz; j++)
		{
			k = wrap (j);
			sum_sq += Csqrmag (dlay (k));
			accum.re += afil (j).re * dlay (k).re;
			accum.im += afil (j).im * dlay (k).im;
		}

      error = Csub(cssig (i),accum);
      cssig(i)= accum;

      scl2 = (REAL) (rate / (sum_sq + 1.19e-7));
      error = Cscl(error,scl2);
      for (j = 0; j < asiz; j++)
		{
			k = wrap (j);
			afil (j).re = afil (j).re * scl1 + error.re * dlay (k).re;
			afil (j).im = afil (j).im * scl1 + error.im * dlay (k).im;
		}

      dptr = bump (dptr);
    }
}

#endif
extern void
lmsr_adapt (LMSR lms)
{
  switch (lms->filter_type)
    {
    case LMADF_NOISE:
      lmsr_adapt_n (lms);
      break;
    case LMADF_INTERFERENCE:
      lmsr_adapt_i (lms);
      break;
    }
}

void
del_blms (BLMS blms)
{
  if (blms)
    {
      fftwf_destroy_plan (blms->Xplan);
      fftwf_destroy_plan (blms->Yplan);
      fftwf_destroy_plan (blms->Errhatplan);
      fftwf_destroy_plan (blms->UPDplan);
      fftwf_destroy_plan (blms->Wplan);
      delvec_COMPLEX (blms->update);
      delvec_COMPLEX (blms->Update);
      delvec_COMPLEX (blms->What);
      delvec_COMPLEX (blms->Xhat);
      delvec_COMPLEX (blms->error);
      delvec_COMPLEX (blms->Errhat);
      delvec_COMPLEX (blms->Yhat);
      delvec_COMPLEX (blms->y);
      delvec_COMPLEX (blms->delay_line);
      safefree ((char *) blms);
    }
}

BLMS
new_blms (CXB signal, REAL adaptation_rate, REAL leak_rate, int filter_type,
	  int pbits)
{
  BLMS tmp;
  tmp = (BLMS) safealloc (1, sizeof (_blocklms), "block lms");
  tmp->delay_line = newvec_COMPLEX (256, "block lms delay line");
  tmp->y = newvec_COMPLEX (256, "block lms output signal");
  tmp->Yhat = newvec_COMPLEX (256, "block lms output transform");
  tmp->Errhat = newvec_COMPLEX (256, "block lms Error transform");
  tmp->error = newvec_COMPLEX (256, "block lms Error signal");
  tmp->Xhat = newvec_COMPLEX (256, "block lms signal transform");
  tmp->What = newvec_COMPLEX (256, "block lms filter transform");
  tmp->Update = newvec_COMPLEX (256, "block lms update transform");
  tmp->update = newvec_COMPLEX (256, "block lms update signal");
  tmp->adaptation_rate = adaptation_rate;
  tmp->leak_rate = 1.0f - leak_rate;
  tmp->signal = signal;
  tmp->filter_type = filter_type;
  tmp->Xplan = fftwf_plan_dft_1d (256,
				  (fftwf_complex *) tmp->delay_line,
				  (fftwf_complex *) tmp->Xhat,
				  FFTW_FORWARD, pbits);

  tmp->Yplan = fftwf_plan_dft_1d (256,
				  (fftwf_complex *) tmp->Yhat,
				  (fftwf_complex *) tmp->y,
				  FFTW_BACKWARD, pbits);

  tmp->Errhatplan = fftwf_plan_dft_1d (256,
				       (fftwf_complex *) tmp->error,
				       (fftwf_complex *) tmp->Errhat,
				       FFTW_FORWARD, pbits);
  tmp->UPDplan = fftwf_plan_dft_1d (256,
				    (fftwf_complex *) tmp->Errhat,
				    (fftwf_complex *) tmp->update,
				    FFTW_BACKWARD, pbits);
  tmp->Wplan = fftwf_plan_dft_1d (256,
				  (fftwf_complex *) tmp->update,
				  (fftwf_complex *) tmp->Update,
				  FFTW_FORWARD, pbits);
  return tmp;
}

#define BLKSCL (REAL)(3.90625e-3)
void
blms_adapt (BLMS blms)
{
  int sigsize = CXBhave (blms->signal);
  int sigidx = 0;

 // fputs("Inside\n",stderr),fflush(stderr);
  do {
      int j;
      memcpy (blms->delay_line, &blms->delay_line[128], sizeof (COMPLEX) * 128);	// do overlap move
      memcpy (&blms->delay_line[128], &CXBdata (blms->signal, sigidx), sizeof (COMPLEX) * 128);	// copy in new data
      fftwf_execute (blms->Xplan);	// compute transform of input data
      for (j = 0; j < 256; j++) {
          blms->Yhat[j] = Cmul (blms->What[j], blms->Xhat[j]);	// Filter new signal in freq. domain
		  blms->Xhat[j] = Conjg (blms->Xhat[j]);	// take input data's complex conjugate
	  }
      fftwf_execute (blms->Yplan);	//compute output signal transform
      for (j = 128; j < 256; j++)
		  blms->y[j] = Cscl (blms->y[j], BLKSCL);
      memset (blms->y, 0, 128 * sizeof (COMPLEX));
      for (j = 128; j < 256; j++)
		  blms->error[j] = Csub (blms->delay_line[j], blms->y[j]);	// compute error signal

      if (blms->filter_type)
		  memcpy (&CXBdata (blms->signal, sigidx), &blms->y[128], 128 * sizeof (COMPLEX));	// if noise filter, output y
      else
		  memcpy (&CXBdata (blms->signal, sigidx), &blms->error[128], 128 * sizeof (COMPLEX));	// if notch filter, output error

      fftwf_execute (blms->Errhatplan);	// compute transform of the error signal
      for (j = 0; j < 256; j++)
		  blms->Errhat[j] = Cmul (blms->Errhat[j], blms->Xhat[j]);	// compute cross correlation transform
      fftwf_execute (blms->UPDplan);	// compute inverse transform of cross correlation transform
      for (j = 0; j < 128; j++)
		  blms->update[j] = Cscl (blms->update[j], BLKSCL);
      memset (&blms->update[128], 0, sizeof (COMPLEX) * 128);	// zero the last block of the update, so we get
      // filter coefficients only at front of buffer
      fftwf_execute (blms->Wplan);
      for (j = 0; j < 256; j++)
	  {
		blms->What[j] = Cadd (Cscl (blms->What[j], blms->leak_rate),	// leak the W away
				Cscl (blms->Update[j], blms->adaptation_rate));	// update at adaptation rate
	  }
      sigidx += 128;		// move to next block in the signal buffer
  }	while (sigidx < sigsize);	// done?
}
