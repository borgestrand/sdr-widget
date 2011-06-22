/* window.c
This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Implemented from code by Bill Schottstaedt of Snd Editor at CCRMA

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

#include <window.h>

/* shamelessly stolen from Bill Schottstaedt's clm.c */
/* made worse in the process, but enough for our purposes here */

//static REAL
//sqr(REAL x) { return (x * x); }

/* mostly taken from
 *    Fredric J. Harris, "On the Use of Windows for Harmonic Analysis with the
 *    Discrete Fourier Transform," Proceedings of the IEEE, Vol. 66, No. 1,
 *    January 1978.
 *    Albert H. Nuttall, "Some Windows with Very Good Sidelobe Behaviour", 
 *    IEEE Transactions of Acoustics, Speech, and Signal Processing, Vol. ASSP-29,
 *    No. 1, February 1981, pp 84-91
 *
 * JOS had slightly different numbers for the Blackman-Harris windows.
 */

REAL *
makewindow (Windowtype type, int size, REAL * window)
{
  int i, j, midn, midp1, midm1;
  REAL freq, rate, sr1, angle, expn, expsum, cx, two_pi;

  midn = size >> 1;
  midp1 = (size + 1) / 2;
  midm1 = (size - 1) / 2;
  two_pi = (REAL) (8.0 * atan (1.0));
  freq = two_pi / (REAL) size;
  rate = (REAL) (1.0 / (REAL) midn);
  angle = 0.0;
  expn = (REAL) (log (2.0) / (REAL) midn + 1.0);
  expsum = 1.0;

  switch (type)
    {
    case RECTANGULAR_WINDOW:
      for (i = 0; i < size; i++)
	window[i] = 1.0;
      break;
    case HANNING_WINDOW:	/* Hann would be more accurate */
      for (i = 0, j = size - 1, angle = 0.0; i <= midn;
	   i++, j--, angle += freq)
	window[j] = (window[i] = (REAL) (0.5 - 0.5 * cos (angle)));
      break;
    case WELCH_WINDOW:
      for (i = 0, j = size - 1; i <= midn; i++, j--)
	window[j] =
	  (window[i] =
	   (REAL) (1.0 - sqr ((REAL) (i - midm1) / (REAL) midp1)));
      break;
    case PARZEN_WINDOW:
      for (i = 0, j = size - 1; i <= midn; i++, j--)
	window[j] =
	  (window[i] =
	   (REAL) (1.0 - fabs ((REAL) (i - midm1) / (REAL) midp1)));
      break;
    case BARTLETT_WINDOW:
      for (i = 0, j = size - 1, angle = 0.0; i <= midn;
	   i++, j--, angle += rate)
	window[j] = (window[i] = angle);
      break;
    case HAMMING_WINDOW:
      for (i = 0, j = size - 1, angle = 0.0; i <= midn;
	   i++, j--, angle += freq)
	window[j] = (window[i] = (REAL) (0.54 - 0.46 * cos (angle)));
      break;
    case BLACKMAN2_WINDOW:	/* using Chebyshev polynomial equivalents here */
      for (i = 0, j = size - 1, angle = 0.0; i <= midn;
	   i++, j--, angle += freq)
	{
	  cx = (REAL) cos (angle);
	  window[j] = (window[i] =
		       (REAL) (.34401 + (cx * (-.49755 + (cx * .15844)))));
	}
      break;
    case BLACKMAN3_WINDOW:
      for (i = 0, j = size - 1, angle = 0.0; i <= midn;
	   i++, j--, angle += freq)
	{
	  cx = (REAL) cos (angle);
	  window[j] =
	    (window[i] =
	     (REAL) (.21747 +
		     (cx * (-.45325 + (cx * (.28256 - (cx * .04672)))))));
	}
      break;
    case BLACKMAN4_WINDOW:
      for (i = 0, j = size - 1, angle = 0.0; i <= midn;
	   i++, j--, angle += freq)
	{
	  cx = (REAL) cos (angle);
	  window[j] = (window[i] = (REAL)
		       (.084037 +
			(cx *
			 (-.29145 +
			  (cx *
			   (.375696 + (cx * (-.20762 + (cx * .041194)))))))));
	}
      break;
    case EXPONENTIAL_WINDOW:
      for (i = 0, j = size - 1; i <= midn; i++, j--)
	{
	  window[j] = (window[i] = (REAL) (expsum - 1.0));
	  expsum *= expn;
	}
      break;
    case RIEMANN_WINDOW:
      sr1 = two_pi / (REAL) size;
      for (i = 0, j = size - 1; i <= midn; i++, j--)
	{
	  if (i == midn)
	    window[j] = (window[i] = 1.0);
	  else
	    {
	      /* split out because NeXT C compiler can't handle the full expression */
	      cx = sr1 * (midn - i);
	      window[i] = (REAL) (sin (cx) / cx);
	      window[j] = window[i];
	    }
	}
      break;
    case BLACKMANHARRIS_WINDOW:
      {
	REAL a0 = 0.35875f, a1 = 0.48829f, a2 = 0.14128f, a3 = 0.01168f;


	for (i = 0; i < size; i++)
	  {
	    window[i] =
	      (REAL) (a0 -
		      a1 * cos (two_pi * (REAL) (i) /
				(REAL) (size - 1)) +
		      a2 * cos (2.0 * two_pi * (REAL) (i) /
				(REAL) (size - 1)) -
		      a3 * cos (3.0 * two_pi * (REAL) (i) /
				(REAL) (size - 1)));
	  }
      }
      break;
    case NUTTALL_WINDOW:
      {
	REAL a0 = 0.3635819f, a1 = 0.4891775f, a2 = 0.1365995f, a3 =
	  0.0106411f;


	for (i = 0; i < size; i++)
	  {
	    window[i] =
	      (REAL) (a0 -
		      a1 * cos (two_pi * (REAL) (i) /
				(REAL) (size - 1)) +
		      a2 * cos (2.0 * two_pi * (REAL) (i) /
				(REAL) (size - 1)) -
		      a3 * cos (3.0 * two_pi * (REAL) (i) /
				(REAL) (size - 1)));
	  }
      }
      break;
    default:
      return 0;
      break;
    }

  return window;
}
