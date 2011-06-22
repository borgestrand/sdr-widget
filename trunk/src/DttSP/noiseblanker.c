/* noiseblanker.c

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

NB
new_noiseblanker (CXB sigbuf, REAL threshold)
{
  NB nb = (NB) safealloc (1, sizeof (nbstate), "new nbstate");
  nb->sigbuf = sigbuf;
  nb->threshold = threshold;
  nb->average_mag = 1.0;
  nb->hangtime = 0;
  nb->sigindex = 0;
  nb->delayindex = 2;
  memset (nb->delay, 0, 8 * sizeof (COMPLEX));
  return nb;
}

void
del_nb (NB nb)
{
  if (nb)
    {
      safefree ((char *) nb);
    }
}

void
noiseblanker (NB nb)
{
  int i;
  for (i = 0; i < CXBsize (nb->sigbuf); i++)
    {
      REAL cmag = Cmag (CXBdata (nb->sigbuf, i));
      nb->delay[nb->sigindex] = CXBdata (nb->sigbuf, i);
      nb->average_mag = (REAL) (0.999 * (nb->average_mag) + 0.001 * cmag);
      if ((nb->hangtime == 0) && (cmag > (nb->threshold * nb->average_mag)))
	nb->hangtime = 7;
      if (nb->hangtime > 0)
	{
	  CXBdata (nb->sigbuf, i) = Cmplx (0.0, 0.0);
	  nb->hangtime--;
	}
      else
	CXBdata (nb->sigbuf, i) = nb->delay[nb->delayindex];
      nb->sigindex = (nb->sigindex + 7) & 7;
      nb->delayindex = (nb->delayindex + 7) & 7;
    }
}

void
SDROMnoiseblanker (NB nb)
{
  int i;
  for (i = 0; i < CXBsize (nb->sigbuf); i++)
    {
      REAL cmag = Cmag (CXBdata (nb->sigbuf, i));
      nb->average_sig = Cadd (Cscl (nb->average_sig, 0.75),
			      Cscl (CXBdata (nb->sigbuf, i), 0.25));
      nb->average_mag = (REAL) (0.999 * (nb->average_mag) + 0.001 * cmag);
      if (cmag > (nb->threshold * nb->average_mag))
	CXBdata (nb->sigbuf, i) = nb->average_sig;
    }
}
