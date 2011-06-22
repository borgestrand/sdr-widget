/*  dttspagc.c

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


DTTSPAGC
newDttSPAgc (AGCMODE mode,
			 COMPLEX *Vec,
			 int BufSize,   // Size of the input buffer
			 REAL target,   // The target voltage
			 REAL attack,   // Attack time constant in msec
			 REAL decay,    // Decay time constant in msec
			 REAL slope,    // Rate of change of gain after agc threshold is exceeded
			 REAL hangtime, // Hang Time in msec where NO decay is allowed in the agc
			 REAL samprate, // Sample Rate so time can be turned into sample number
			 REAL MaxGain,  // Maximum allowable gain
			 REAL MinGain,  // Minimum gain (can be -dB / less than 1.0 as in ALC or strong signals on receive
			 REAL CurGain,  // The initial gain setting
			 char *tag)     // String to emit when error conditions occur
{
	DTTSPAGC a;

	a = (DTTSPAGC) safealloc (1, sizeof (dttspagc), tag);
	a->mode = mode;

	// Attack: decrease gain when input signal will produce an output signal above the target voltage
	// Decay:  increase gain when input signal will produce an output signal below the target voltage
	
	// Compute exponential attack rate per sample from attack time and sampler rate.
	a->attack = (REAL) (1.0 - exp (-1000.0 / (attack * samprate))); 
	// Compute 1.0 - attack time.
	a->one_m_attack = (REAL) (1.0 - a->attack);

	// Compute exponential decay rate per sample from attack time and sampler rate.
	a->decay = (REAL) (1.0 - exp (-1000.0 / (decay * samprate)));
	// Compute 1.0 - decay time.
	a->one_m_decay = (REAL) (1.0 - a->decay);

	// Our system has a two track agc response.  The normal one found in most receive
	// systems is the determined from the calculations we just made.
	// Ours has a fast track agc which is designed to prevent large pulse signals from pushing
	// us into serious attack and wait for a long decay when the signal is short duration < 3 time constants for attack

	// Compute exponential fast attack rate per sample from fast attack time and sampler rate.
	a->fastattack = (REAL) (1.0 - exp (-1000.0 / (0.2 * samprate)));
	// Compute 1.0 - fast attack time.
	a->one_m_fastattack = (REAL) (1.0 - a->fastattack);

	// Compute exponential fast decay rate per sample from fast attack time and sampler rate.
	a->fastdecay = (REAL) (1.0 - exp (-1000.0 / (3.0 * samprate)));
	// Compute 1.0 - fast decay time.
	a->one_m_fastdecay = (REAL) (1.0 - a->fastdecay);

	// Save the error identification message
	strcpy (a->tag, tag);

	// This computes the index max for our circular buffer
	a->mask = 2 * BufSize - 1;
	
	// Hangtime:  This is a period of NO allowed decay even though we need to increase gain to stay at target level
	// hangindex is used to compute how deep into the hang interval we are.
	a->hangindex = a->slowindx = 0;
	// Change hang time to hang length in samples using time and sample rate.
	a->hangtime = hangtime * 0.001f;
	//a->hangthresh = 0.0; // not neccessary?
	// We BEGIN applying decreasing gain 3 time constants ahead of the signals arrival to narrow the modulation
	// spread arising from modulating the signal with the agc gain.  The index to application of computed gain
	// sndx and it is computed as number of samples in 3 attack time constants
	a->out_indx = (int) (samprate * attack * 0.003f);

	// We do the same anticipation in the fast channel but we only do two time constants.
	a->fastindx = (int)(0.0027f*samprate);
	a->gain.fix = 10.0;

	// Slope:  The rate at which gain is decreased as signal strength increases after it exceeds the agc threshold
	// that is, after the required gain to reach the desired output level is LESS THAN the maximum allowable gain.
	a->slope = slope;

	// gain.top = Maxgain which is the maximum allowable gain.  This determines the agc threshold
	a->gain.top = MaxGain;
	// gain.bottom = MinGain is the minimum allowable gain.  If < 0dB or 1.0, then this is an attenuation.
	a->hangthresh = a->gain.bottom = MinGain;
	// Initialize the gain state to an initial value
	a->gain.fastnow = a->gain.old = a->gain.now = CurGain;

	// Set the target voltage.
	a->gain.target = target;

	// Given the vector of input samples,  make a local complex buffer (struct to handle metadata associated with
	// the vector or array of samples
	a->buff = newCXB (BufSize, Vec, "agc in buffer");
	// Use a circular buffer for efficiency in dealing with the anticipatory part of the agc algorithm
	a->circ = newvec_COMPLEX (2 * BufSize, "circular agc buffer");

	// intialize fast hang index to 0
	a->fasthang = 0;

	// Whatever the hangtime is for the slow channel, make the fast agc channel hangtime 10% of it
	a->fasthangtime = 0.1f*a->hangtime;

	return a;
}

void
DttSPAgc_flushbuf(DTTSPAGC a)
{
	memset((void *)a->circ,0,sizeof(COMPLEX)*(a->mask+1));
}

void
DttSPAgc (DTTSPAGC a, int tick)
{
	int i;
	int hangtime = (int) (uni[0].samplerate * a->hangtime); // hangtime in samples
	int fasthangtime = (int) (uni[0].samplerate * a->fasthangtime); // fast track hangtime in samples

	REAL hangthresh; // gate for whether to hang or not

//	if (a->hangthresh > 0)
		hangthresh =
			a->gain.top * a->hangthresh + a->gain.bottom * (REAL) (1.0 -
			a->hangthresh);
//	else
//		hangthresh = 0.;

	if (a->mode == 0)
	{
		for (i = 0; i < CXBsize (a->buff); i++)
			CXBdata (a->buff, i) = Cscl (CXBdata (a->buff, i), a->gain.fix);
		return;
	}

	for (i = 0; i < CXBsize (a->buff); i++)
	{
		REAL tmp;
		a->circ[a->slowindx] = CXBdata (a->buff, i);	/* Drop sample into circular buffer */

		// first, calculate slow gain
		tmp = Cmag (a->circ[a->slowindx]);
		if (tmp > 0.0f)
			tmp = a->gain.target / tmp;	// if mag(sample) not too small, calculate putative gain
										// all the rest of the code which follows is running this
										// signal through the control laws.
		else
			tmp = a->gain.now;	// sample too small, just use old gain
		if (tmp < hangthresh)
			a->hangindex = hangtime;  // If the gain is less than the current hang threshold, then stop hanging.

		if (tmp >= a->gain.now)     // If the putative gain is greater than the current gain then we are in decay.
									// That is, we need to "decay the ALC voltage", or increase the gain.
		{
			//a->gain.raw = a->one_m_decay * a->gain.now + a->decay * tmp;
			if (a->hangindex++ > hangtime)  // Have we HUNG around long enough?
			{
				a->gain.now =  // Compute the applicable slow channel gain through the decay control law.
					a->one_m_decay * a->gain.now +
					a->decay * min (a->gain.top, tmp);
			}
		}
		else // if the putative gain is greater than the current gain,  we are in attack mode and need to decrease gain 
		{
			a->hangindex = 0;  // We don't need to hang, we need to attack so we reset the hang index to zero
			//a->gain.raw = a->one_m_attack * a->gain.now + a->attack * tmp;
			a->gain.now =      // Compute the applicable slow channel gain through the attack control law.
				a->one_m_attack * a->gain.now + a->attack * max (tmp,
				a->gain.bottom); 
		}

		// then, calculate fast gain
		// Fast channel to handle short duration events and not capture the slow channel decay
		tmp = Cmag (a->circ[a->fastindx]);
		if (tmp > 0.0f)
			tmp = a->gain.target / tmp; // if mag(sample) not too small, calculate putative gain
										// all the rest of the code which follows is running this
										// signal through the control laws.
		else
			tmp = a->gain.fastnow;      // too small, just use old gain
		if (tmp > a->gain.fastnow)		// If the putative gain is greater than the current gain then we are in decay.
										// That is, we need to "decay the ALC voltage", or increase the gain.
		{
			if (a->fasthang++ > fasthangtime) // Have we HUNG around long enough?
			{
				a->gain.fastnow =  // Compute the applicable fast channel gain through the decay control law.
					a->one_m_fastdecay * a->gain.fastnow +
					a->fastdecay * min (a->gain.top, tmp);
			}
		}
		else
		{
			a->fasthang = 0; // We don't need to hang, we need to attack so we reset the hang index to zero
			a->gain.fastnow = // Compute the applicable fast channel gain through the fast attack control law.
				a->one_m_fastattack * a->gain.fastnow +
				a->fastattack * max (tmp, a->gain.bottom);
		}
		// Are these two lines necessary? I don't think so.  Let's test that.  tmp is bounded in the statements
		// above to be inside the gain limits
//		a->gain.fastnow =
//			max (min (a->gain.fastnow, a->gain.top), a->gain.bottom);
//		a->gain.now = max (min (a->gain.now, a->gain.top), a->gain.bottom);
		// Always apply the lower gain.
		CXBdata (a->buff, i) =
			Cscl (a->circ[a->out_indx],
			min (a->gain.fastnow, (a->slope * a->gain.now)));

		// Move the indices to prepare for the next sample to be processed
		a->slowindx = (a->slowindx + a->mask) & a->mask; 
		a->out_indx = (a->out_indx + a->mask) & a->mask;
		a->fastindx = (a->fastindx + a->mask) & a->mask;
	}
}

void
delDttSPAgc (DTTSPAGC a)
{
	delCXB (a->buff);
	delvec_COMPLEX (a->circ);
	if (a)
		safefree ((char *) a);
}
