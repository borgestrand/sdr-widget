/** 
* @file filterbank.c
* @brief Functions to implement a filter bank 
* @author Frank Brickle, AB2KT and Bob McGwier, N4HY

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006, 2007, 2008 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Doxygen comments added by Dave Larsen, KV0S

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
rwmcgwier@gmail.com

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807
*/

#include <filterbank.h>


FIRDownsampler
newFIRPDownsampler(RealFIR filter, unsigned int downsample, unsigned int channel, COMPLEX *sigin, COMPLEX *sigout)
{
	FIRDownsampler tmp;
	unsigned int i,j;

	tmp = (FIRDownsampler) malloc(sizeof(firdownsampler));
	tmp->downsample = downsample;
	tmp->depth = (unsigned int)ceil((double)filter->size/(double)downsample);
	  
	// Set up numchans partitions
	tmp->filter_partition
		= (REAL    **)safealloc(downsample,sizeof(REAL    *),"fb partition pointers");
	tmp->filter_delayline
		= (COMPLEX **)safealloc(downsample,sizeof(COMPLEX *),"fb partition pointers");
	for (i=0;i<tmp->downsample;i++) { 
		tmp->filter_partition[i]
		= (REAL *)safealloc(tmp->depth,sizeof(REAL),"fb partition delay line");
		tmp->filter_delayline[i]
		= (COMPLEX *)safealloc(tmp->depth,sizeof(COMPLEX),"fb partition delay line");
	}
	for (i=0;i<downsample;i++) {
		memset(tmp->filter_partition[i],0,sizeof(REAL   ) * tmp->depth);
		memset(tmp->filter_delayline[i],0,sizeof(COMPLEX) * tmp->depth);
	}

	// Partition the filter

	for (i=0; i<tmp->downsample; i++)
		for (j=0; j<tmp->depth; j++) 
		tmp->filter_partition[i][j] = FIRtap(filter,i + j*(tmp->downsample));


	tmp->sigin  = sigin;
	tmp->sigout = sigout;
	tmp->rotator = (COMPLEX *)safealloc(tmp->downsample,sizeof(COMPLEX),"newFIRDownsampler: downsample rotator");
	for (i=0;i<tmp->downsample;i++) tmp->rotator[i] = Cmplx((REAL)cos(i*channel*2.0*M_PI/tmp->downsample),(IMAG)sin(i*channel*2.0*M_PI/tmp->downsample));

	return tmp;
}

FIRUpsampler
newFIRPUpsampler(RealFIR filter, unsigned int upsample, unsigned int channel, COMPLEX *sigin, COMPLEX *sigout)
{
	FIRUpsampler tmp;
	unsigned int i,j;

	tmp = (FIRUpsampler) malloc(sizeof(firpfb));
	tmp->upsample = upsample;
	tmp->depth = (unsigned int)ceil((double)filter->size/(double)upsample);
	  
	// Set up numchans partitions
	tmp->filter_partition
		= (REAL    **)safealloc(upsample,sizeof(REAL    *),"fb partition pointers");
	tmp->filter_delayline
		= (COMPLEX **)safealloc(upsample,sizeof(COMPLEX *),"fb partition pointers");
	for (i=0;i<tmp->upsample;i++) { 
		tmp->filter_partition[i]
		= (REAL *)safealloc(tmp->depth,sizeof(REAL),"fb partition delay line");
		tmp->filter_delayline[i]
		= (COMPLEX *)safealloc(tmp->depth,sizeof(COMPLEX),"fb partition delay line");
	}
	for (i=0;i<upsample;i++) {
		memset(tmp->filter_partition[i],0,sizeof(REAL   ) * tmp->depth);
		memset(tmp->filter_delayline[i],0,sizeof(COMPLEX) * tmp->depth);
	}

	// Partition the filter

	for (i=0; i<tmp->upsample; i++)
		for (j=0; j<tmp->depth; j++) 
		tmp->filter_partition[i][j] = FIRtap(filter,i + j*(tmp->upsample));


	tmp->sigin  = sigin;
	tmp->sigout = sigout;
	tmp->rotator = (COMPLEX *)safealloc(tmp->upsample,sizeof(COMPLEX),"newFIRUpsampler: upsample rotator");
	for (i=0;i<tmp->upsample;i++) tmp->rotator[i] = Cmplx((REAL)cos(i*channel*2.0*M_PI/tmp->upsample),(IMAG)sin(i*channel*2.0*M_PI/tmp->upsample));

	return tmp;
}

FIRPFB
newFIRHBPFBChannelizer(RealFIR filter, COMPLEX *sigin, COMPLEX *sigout)
{
	FIRPFB tmp;
	unsigned int i,j;

	tmp = (FIRPFB) malloc(sizeof(firpfb));
	tmp->numchans = 2;
	tmp->depth = (unsigned int)ceil(((double)filter->size-1)/2.0);
	  
	// Set up numchans partitions
	tmp->filter_partition
		= (REAL    **)safealloc(2,sizeof(REAL    *),"fb partition pointers");
	tmp->filter_delayline
		= (COMPLEX **)safealloc(2,sizeof(COMPLEX *),"fb partition pointers");
	for (i=0;i<tmp->numchans;i++) { 
		tmp->filter_partition[i]
		= (REAL *)safealloc(tmp->depth,sizeof(REAL),"fb partition delay line");
		tmp->filter_delayline[i]
		= (COMPLEX *)safealloc(tmp->depth,sizeof(COMPLEX),"fb partition delay line");
	}
	for (i=0;i<tmp->numchans;i++) {
		memset(tmp->filter_partition[i],0,sizeof(REAL   ) * tmp->depth);
		memset(tmp->filter_delayline[i],0,sizeof(COMPLEX) * tmp->depth);
	}

	// Partition the filter

	for (i=0; i<tmp->numchans; i++)
		for (j=0; j<tmp->depth; j++) 
		tmp->filter_partition[i][j] = FIRtap(filter,i + j*(tmp->numchans));


	tmp->sigin  = sigin;
	tmp->sigout = sigout;

	return tmp;
}
FIRPFB
newFIRPFBChannelizer(RealFIR filter, unsigned int numchans, COMPLEX *sigin, COMPLEX *sigout, int pbits)
{
	FIRPFB tmp;
	unsigned int i,j;

	tmp = (FIRPFB) malloc(sizeof(firpfb));
	tmp->numchans = numchans;
	tmp->depth = (unsigned int)ceil((double)filter->size/(double)numchans);
	  
	// Set up numchans partitions
	tmp->filter_partition
		= (REAL    **)safealloc(numchans,sizeof(REAL    *),"fb partition pointers");
	tmp->filter_delayline
		= (COMPLEX **)safealloc(numchans,sizeof(COMPLEX *),"fb partition pointers");
	for (i=0;i<tmp->numchans;i++) { 
		tmp->filter_partition[i]
		= (REAL *)safealloc(tmp->depth,sizeof(REAL),"fb partition delay line");
		tmp->filter_delayline[i]
		= (COMPLEX *)safealloc(tmp->depth,sizeof(COMPLEX),"fb partition delay line");
	}
	for (i=0;i<numchans;i++) {
		memset(tmp->filter_partition[i],0,sizeof(REAL   ) * tmp->depth);
		memset(tmp->filter_delayline[i],0,sizeof(COMPLEX) * tmp->depth);
	}

	// Partition the filter

	for (i=0; i<tmp->numchans; i++)
		for (j=0; j<tmp->depth; j++) 
		tmp->filter_partition[i][j] = FIRtap(filter,i + j*(tmp->numchans));


	tmp->sigin  = sigin;
	tmp->sigout = sigout;
	tmp->fftbuffer = (COMPLEX *)safealloc(tmp->numchans,sizeof(COMPLEX),"fft input buffer");

	tmp->pchan = fftwf_plan_dft_1d(tmp->numchans,
					(fftwf_complex *)tmp->fftbuffer,
					(fftwf_complex *)tmp->sigout,
					FFTW_FORWARD,
					pbits);

	return tmp;
}

void delFIRPB(FIRPFB p)
{
	unsigned int i;
	fftwf_destroy_plan(p->pchan);
	safefree((char *)p->fftbuffer);
	for(i=0;i<p->numchans;i++)
		safefree((char *)p->filter_partition[i]),
		safefree((char *)p->filter_delayline[i]);
	safefree((char *)p->filter_partition);
	safefree((char *)p->filter_delayline);
	safefree((char *)p);
}

void doFIRPFBChannelizer(FIRPFB p)
{
	unsigned int i,j;
	unsigned k = p->numchans - 1;
	for(i=0;i<p->numchans;i++) {
		p->filter_delayline[i][0] = p->sigin[k-i]; // Put new signal into partition delay lines

		p->fftbuffer[i]= Cscl(p->sigin[i],p->filter_partition[i][0]); // initialize calculation partition filters;

		for (j=1;j<p->depth;j++) // calculate rest of partition filter (convolution)
			p->fftbuffer[i] = Cadd(Cscl(p->filter_delayline[i][j],p->filter_partition[i][j]),p->fftbuffer[i]);

		for (j=p->depth-1;j>0;j--) // Move data along delay line 
		p->filter_delayline[i][j] = p->filter_delayline[i][j-1];
	}
	fftwf_execute(p->pchan); // Do Channelizer
}

void doFIRPFBUpsampler(FIRUpsampler p)
{
	unsigned int i,j;

	for(i=0;i<p->upsample;i++) {
		p->filter_delayline[i][0] = Cmul(p->sigin[i],p->rotator[i]); // Put new signal into partition delay lines and apply channel rotator before filter.
		p->sigout[i]=cxzero;

		for (j=0;j<p->depth;j++) // calculate rest of partition filter (convolution)
			p->sigout[i] = Cadd(Cscl(p->filter_delayline[i][j],p->filter_partition[i][j]),p->sigout[i]);

		for (j=p->depth-1;j>0;j--) // Move data along delay line 
		p->filter_delayline[i][j] = p->filter_delayline[i][j-1];
	}

}

void doFIRPFBdownsampler(FIRDownsampler p)
{
	unsigned int i,j;
	COMPLEX accum = cxzero;
	unsigned k = p->downsample - 1;

	*p->sigout=cxzero; // Initialize polyphase sum

	for(i=0;i<p->downsample;i++) {
		p->filter_delayline[i][0] = p->sigin[k-i]; // Put new signal into partition delay lines

		for (j=0;j<p->depth;j++) // calculate rest of partition filter (convolution)
			accum = Cadd(Cscl(p->filter_delayline[i][j],p->filter_partition[i][j]),accum);
		*p->sigout = Cadd(Cmul(accum,p->rotator[i]),*p->sigout);


		for (j=p->depth-1;j>0;j--) // Move data along delay line 
		p->filter_delayline[i][j] = p->filter_delayline[i][j-1];
	}

}

void delFIRDownsampler(FIRDownsampler p)
{
  unsigned int i;
  
  for (i=0;i<p->downsample;i++){
    safefree((char *)p->filter_delayline[i]);
    safefree((char *)p->filter_partition[i]);
  }
  safefree((char *)p->filter_delayline);
  safefree((char *)p->filter_partition);
  safefree((char *)p);
}

void delFIRUpsample(FIRUpsampler p)
{
  unsigned int i;
  
  for (i=0;i<p->upsample;i++){
    safefree((char *)p->filter_delayline[i]);
    safefree((char *)p->filter_partition[i]);
  }
  safefree((char *)p->filter_delayline);
  safefree((char *)p->filter_partition);
  safefree((char *)p);
}
