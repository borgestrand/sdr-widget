/* oscillator.c 

This routine implements a common fixed-frequency oscillator

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

#define HUGE_PHASE 1256637061.43593

void
ComplexOSC (OSC p)
{
  int i;
  COMPLEX z, delta_z;

  if (OSCphase (p) > TWOPI)
    OSCphase (p) -= TWOPI;
  else if (OSCphase (p) < -TWOPI)
    OSCphase (p) += TWOPI;

  z = Cmplx ((REAL) cos (OSCphase (p)), (IMAG) sin (OSCphase (p))),
    delta_z = Cmplx ((REAL) cos (OSCfreq (p)), (IMAG) sin (OSCfreq (p)));

  for (i = 0; i < OSCsize (p); i++)
    /*z = CXBdata ((CXB) OSCbase (p), i)
      = Cmul (z, delta_z), OSCphase (p) += OSCfreq (p);*/
	  CXBdata((CXB)OSCbase(p), i) = Cmplx ((REAL) cos (OSCphase (p)), (IMAG) sin (OSCphase (p))),
		OSCphase (p) += OSCfreq (p);

}

#ifdef notdef
void
ComplexOSC (OSC p)
{
  int i;
/*  if (OSCphase (p) > 1256637061.43593)
    OSCphase (p) -= 1256637061.43593; */
  for (i = 0; i < OSCsize (p); i++)
    {
      OSCreal (p, i) = cos (OSCphase (p));
      OSCimag (p, i) = sin (OSCphase (p));
      OSCphase (p) += OSCfreq (p);
    }
}
#endif

PRIVATE double
_phasemod (double angle)
{
  while (angle >= TWOPI)
    angle -= TWOPI;
  while (angle < 0.0)
    angle += TWOPI;
  return angle;
}

void
RealOSC (OSC p)
{
  int i;
  for (i = 0; i < OSCsize (p); i++)
    {
      OSCRdata (p, i) = (REAL) sin (OSCphase (p));
      OSCphase (p) = _phasemod (OSCfreq (p) + OSCphase (p));
    }
}

OSC
newOSC (int size,
	OscType TypeOsc,
	double Frequency, double Phase, REAL SampleRate, char *tag)
{
  OSC p = (OSC) safealloc (1, sizeof (oscillator), tag);
  if ((OSCtype (p) = TypeOsc) == ComplexTone)
    OSCbase (p) = (void *) newCXB (size,
				   NULL,
				   "complex buffer for oscillator output");
  else
    OSCbase (p) = (void *) newRLB (size,
				   NULL, "real buffer for oscillator output");
  OSCsize (p) = size;
  OSCfreq (p) = 2.0 * M_PI * Frequency / SampleRate;
  OSCphase (p) = Phase;
  return p;
}

void
delOSC (OSC p)
{
  if (p->OscillatorType == ComplexTone)
    delCXB ((CXB) p->signalpoints);
  else
    delRLB ((RLB) p->signalpoints);
  if (p)
    safefree ((char *) p);
}
