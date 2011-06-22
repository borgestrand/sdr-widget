/* speechproc.c
 
  This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY, Phil Harman, VK6APH
Based on Visual Basic code for SDR by Phil Harman

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

#include <speechproc.h>

SpeechProc
newSpeechProc (REAL K, REAL MaxCompression, COMPLEX * spdat, int size)
{
	SpeechProc sp = (SpeechProc) safealloc (1, sizeof (speech_proc),
		"new speech processor");
	sp->CG = newRLB (size + 1, NULL, "CG buffer in Speech Processor");
	sp->K = K;
	sp->MaxGain = (REAL) pow (10.0, MaxCompression * 0.05);
	sp->fac =
		(REAL) ((((0.0000401002 * MaxCompression) -
		0.0032093390) * MaxCompression +
		0.0612862687) * MaxCompression + 0.9759745718);
	sp->LastCG = 1.0;
	sp->SpeechProcessorBuffer = newCXB (size, spdat, "speech processor data");
	sp->size = size;
	return sp;
}

void
delSpeechProc (SpeechProc sp)
{
	if (sp)
	{
		delRLB (sp->CG);
		delCXB (sp->SpeechProcessorBuffer);
		safefree ((char *) sp);
	}
}

void
SpeechProcessor (SpeechProc sp)
{
	int i;
	REAL r = 0.0, Mag;

	if (sp->MaxGain == 1.0)
		return;
	// K was 0.4 in VB version, this value is better, perhaps due to filters that follow?
	for (i = 0; i < sp->size; i++)
		r = max (r, Cmag (CXBdata (sp->SpeechProcessorBuffer, i)));	// find the peak magnitude value in the sample buffer 

	RLBdata (sp->CG, 0) = sp->LastCG;	// restore from last time
	for (i = 1; i <= sp->size; i++)
	{
		Mag = Cmag (CXBdata (sp->SpeechProcessorBuffer, i - 1));
		if (Mag != 0.0)
		{
			RLBdata(sp->CG, i) = RLBdata(sp->CG, i - 1) * (1 - sp->K) + (sp->K * r / Mag);	// Frerking's formula
			if (RLBdata (sp->CG, i) > sp->MaxGain)
				RLBdata (sp->CG, i) = sp->MaxGain;
		}
		else
			RLBdata (sp->CG, i) = sp->MaxGain;
	}
	sp->LastCG = RLBdata (sp->CG, sp->size);	// save for next time 


	for (i = 0; i < sp->size; i++)	// multiply each sample by its gain constant 
		CXBdata (sp->SpeechProcessorBuffer, i) =
			Cscl (CXBdata (sp->SpeechProcessorBuffer, i),
				(REAL) (RLBdata (sp->CG, i) /
				(sp->fac * pow (sp->MaxGain, 0.25))));
}
