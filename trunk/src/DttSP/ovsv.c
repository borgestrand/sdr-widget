/* ovsv.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006, 2007 by Frank Brickle, AB2KT and Bob McGwier, N4HY

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
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

#include <ovsv.h>

/*------------------------------------------------------------*/
/* run the filter */

void
filter_OvSv(FiltOvSv pflt) {
  int i, m = pflt->fftlen, n = pflt->buflen;
  COMPLEX *zfvec = pflt->zfvec,
          *zivec = pflt->zivec,
          *zovec = pflt->zovec,
          *zrvec = pflt->zrvec;
  REAL scl = pflt->scale;

  /* input sig -> z */

  fftwf_execute(pflt->pfwd);

  /* convolve in z */
  for (i = 0; i < m; i++)
    zivec[i] = Cmul(zivec[i], zfvec[i]);

  /* z convolved sig -> time output sig */
 
  fftwf_execute(pflt->pinv);


  /* prepare input sig vec for next fill */
 
  memcpy((void *)zrvec,(const void *)&zrvec[n],n*sizeof(COMPLEX));
}

void
reset_OvSv(FiltOvSv pflt) 
{
  memset((char *) pflt->zrvec, 0, pflt->fftlen * sizeof(COMPLEX));
  memset((char *) pflt->zovec, 0, pflt->fftlen * sizeof(COMPLEX));
}

/*------------------------------------------------------------*/
/* info: */
/* NB strategy. This is the address we pass to newCXB as
   the place to read samples into. It's the right half of
   the true buffer. Old samples get slid from here to
   left half after each go-around. */
COMPLEX *
FiltOvSv_initpoint(FiltOvSv pflt) {
  return &(pflt->zrvec[pflt->buflen]);
}

/* how many to put there */
int
FiltOvSv_initsize(FiltOvSv pflt) {
  return (pflt->fftlen - pflt->buflen);
}

/* where to put next batch of samples to filter */
COMPLEX *
FiltOvSv_fetchpoint(FiltOvSv pflt) {
  return &(pflt->zrvec[pflt->buflen]);
}

/* how many samples to put there */

int
FiltOvSv_fetchsize(FiltOvSv pflt) {
  return (pflt->fftlen - pflt->buflen);
}

/* where samples should be taken from after filtering */
#ifdef LHS
COMPLEX *
FiltOvSv_storepoint(FiltOvSv pflt) {
  return ((pflt->zovec) + pflt->buflen);
}
#else
COMPLEX *
FiltOvSv_storepoint(FiltOvSv pflt) {
  return ((pflt->zovec));
}
#endif

/* how many samples to take */
/* NB strategy. This is the number of good samples in the
   left half of the true buffer. Samples in right half
   are circular artifacts and are ignored. */
int
FiltOvSv_storesize(FiltOvSv pflt) {
  return (pflt->fftlen - pflt->buflen);
}

/*------------------------------------------------------------*/
/* create a new overlap/save filter from complex coefficients */

FiltOvSv
newFiltOvSv(COMPLEX * coefs, int ncoef, int pbits) {
  int buflen, fftlen;
  FiltOvSv p;
  fftwf_plan pfwd, pinv;
  COMPLEX *zrvec, *zfvec, *zivec, *zovec;
  p = (FiltOvSv) safealloc(1, sizeof(filt_ov_sv), "new overlap/save filter");
  buflen = nblock2(ncoef - 1), fftlen = 2 * buflen;
  zrvec = newvec_COMPLEX_fftw(fftlen, "raw signal vec in newFiltOvSv");
  zfvec = newvec_COMPLEX_fftw(fftlen, "filter z vec in newFiltOvSv");
  zivec = newvec_COMPLEX_fftw(fftlen, "signal in z vec in newFiltOvSv");
  zovec = newvec_COMPLEX_fftw(fftlen, "signal out z vec in newFiltOvSv");

  /* prepare frequency response from filter coefs */
  {
    int i;
    COMPLEX *zcvec;
    fftwf_plan ptmp;

    zcvec = newvec_COMPLEX(fftlen, "temp filter z vec in newFiltOvSv");
    //ptmp = fftw_create_plan(fftlen, FFTW_FORWARD, pbits);
    ptmp =
      fftwf_plan_dft_1d(fftlen, (fftwf_complex *) zcvec,
			(fftwf_complex *) zfvec, FFTW_FORWARD, pbits);

#ifdef LHS
    for (i = 0; i < ncoef; i++)
      zcvec[i] = coefs[i];
#else
    for (i = 0; i < ncoef; i++)
      zcvec[fftlen - ncoef + i] = coefs[i];
#endif

    //fftw_one(ptmp, (fftw_complex *) zcvec, (fftw_complex *) zfvec);
    fftwf_execute(ptmp);
    fftwf_destroy_plan(ptmp);
    delvec_COMPLEX(zcvec);
  }

  /* prepare transforms for signal */
  //pfwd = fftw_create_plan(fftlen, FFTW_FORWARD, pbits);
  //pinv = fftw_create_plan(fftlen, FFTW_BACKWARD, pbits);
  pfwd = fftwf_plan_dft_1d(fftlen, (fftwf_complex *) zrvec,
			   (fftwf_complex *) zivec,
			   FFTW_FORWARD,
			   pbits);
  pinv = fftwf_plan_dft_1d(fftlen, (fftwf_complex *) zivec,
			   (fftwf_complex *) zovec,
			   FFTW_BACKWARD,
			   pbits);
  /* stuff values */
  p->buflen = buflen;
  p->fftlen = fftlen;
  p->zfvec = zfvec;
  p->zivec = zivec;
  p->zovec = zovec;
  p->zrvec = zrvec;
  p->pfwd = pfwd;
  p->pinv = pinv;
  p->scale = 1.0f / (REAL) fftlen;
  normalize_vec_COMPLEX (p->zfvec, p->fftlen, p->scale);
  return p;
}

/* deep-six the filter */
void
delFiltOvSv(FiltOvSv p) {
  if (p) {
    delvec_COMPLEX_fftw(p->zfvec);
    delvec_COMPLEX_fftw(p->zivec);
    delvec_COMPLEX_fftw(p->zovec);
    delvec_COMPLEX_fftw(p->zrvec);
    fftwf_destroy_plan(p->pfwd);
    fftwf_destroy_plan(p->pinv);
    safefree((char *) p);
  }
}

/*------------------------------------------------------------*/
