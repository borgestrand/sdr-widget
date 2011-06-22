/* graphiceq.c
 * 
 *   PCM frequency domain equalizer
 *
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

#include <graphiceq.h>


static REAL EQ_Num[3];
static REAL EQ_Den[3];

static REAL EQ_Num_48000[] =  {0.99220706371f, -1.98392450292f, 0.99220706371f};
static REAL EQ_Den_48000[] =  {1.98392450292f, -0.98441412742f};
static REAL EQ_Num_96000[] =  {0.99608835009f, -1.99205381333f, 0.99608835009f};
static REAL EQ_Den_96000[] =  {1.99205381333f, -0.99217670018f};
static REAL EQ_Num_192000[] = {0.99804034984f, -1.99604991764f, 0.99804034984f};
static REAL EQ_Den_192000[] = {1.99604991764f, -0.99608069967f};

EQ
new_EQ (CXB d, REAL samplerate, int pbits) {
  ComplexFIR BP;
  EQ a = (EQ) safealloc(1, sizeof (eq), "new eq state");

  BP = newFIR_Bandpass_COMPLEX(-6000.0, 6000.0, samplerate, 257);
  a->p = newFiltOvSv(BP->coef, 257, pbits);
  normalize_vec_COMPLEX(a->p->zfvec,a->p->fftlen,a->p->scale);
  a->in = newCXB(256, FiltOvSv_fetchpoint(a->p), "EQ input CXB");
  a->out = newCXB(256, FiltOvSv_storepoint(a->p), "EQ output CXB");
  a->data = d;
  delFIR_Bandpass_COMPLEX(BP);
  memset(a->num, 0, 9 * sizeof (COMPLEX));
  memset(a->den, 0, 6 * sizeof (COMPLEX));
  a->notchflag = FALSE;
  if (samplerate == 48000.0) {
    EQ_Num[0]=EQ_Num_48000[0];
    EQ_Num[1]=EQ_Num_48000[1];
    EQ_Num[2]=EQ_Num_48000[2];
    EQ_Den[0]=EQ_Den_48000[0];
    EQ_Den[1]=EQ_Den_48000[1];
  }
  if (samplerate == 96000.0) {
    EQ_Num[0]=EQ_Num_96000[0];
    EQ_Num[1]=EQ_Num_96000[1];
    EQ_Num[2]=EQ_Num_96000[2];
    EQ_Den[0]=EQ_Den_96000[0];
    EQ_Den[1]=EQ_Den_96000[1];
  }
  if (samplerate == 192000.0) {
    EQ_Num[0]=EQ_Num_192000[0];
    EQ_Num[1]=EQ_Num_192000[1];
    EQ_Num[2]=EQ_Num_192000[2];
    EQ_Den[0]=EQ_Den_192000[0];
    EQ_Den[1]=EQ_Den_192000[1];
  }
  return a;
}

void
delEQ (EQ a) {
  if (a) {
    delCXB(a->in);
    delCXB(a->out);
    delFiltOvSv (a->p);
    safefree ((char *) a);
  }
}

void
graphiceq (EQ a) {
  int sigsize = CXBhave (a->data), sigidx = 0;

  do {
    memcpy(CXBbase(a->in),
	   &CXBdata(a->data, sigidx),
	   256 * sizeof (COMPLEX));
    filter_OvSv(a->p);
    memcpy(&CXBdata (a->data, sigidx),
	   CXBbase (a->out),
	   256 * sizeof (COMPLEX));
    sigidx += 256;
  } while (sigidx < sigsize);
  if (a->notchflag) {
    int i;
    for (i = 0; i < sigsize; i++) {
      int j;
      for(j = 0; j < 3; j++) {
	COMPLEX numfilt;
	int k = 3 * j,
	    l = 2 * j;
	a->num[k] = CXBdata(a->data, i);
	numfilt.re =
	  a->num[k].re     * EQ_Num[0] +
	  a->num[k + 1].re * EQ_Num[1] +
	  a->num[k + 2].re * EQ_Num[2];
	numfilt.im =
	  a->num[k].im     * EQ_Num[0] +
	  a->num[k + 1].im * EQ_Num[1] +
	  a->num[k + 2].im * EQ_Num[2];
	a->num[k + 2] = a->num[k + 1];
	a->num[k + 1] = a->num[k];
	CXBdata(a->data, i) =
	  Cmplx(numfilt.re + a->den[l].re * EQ_Den[0] + a->den[l + 1].re * EQ_Den[1],
		numfilt.im + a->den[l].im * EQ_Den[0] + a->den[l + 1].im * EQ_Den[1]);
	a->den[l + 1] = a->den[l];
	a->den[l] = CXBdata(a->data, i);
      }
    }
  }
}
