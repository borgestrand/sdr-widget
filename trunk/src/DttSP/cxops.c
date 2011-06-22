/* cxops.c
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

#include <cxops.h>

// useful constants

COMPLEX cxzero = { 0.0, 0.0 };
COMPLEX cxone = { 1.0, 0.0 };
COMPLEX cxJ = { 0.0, 1.0 };
COMPLEX cxminusone = { -1.0, 0.0 };
COMPLEX cxminusJ = { 0.0, -1.0 };

#ifdef DONT_INCLUDE
void __forceinline vecSq(ssevec *a, ssevec *b, int l)
{
	int i;
	ssevec *ins_a,*ins_b;
	for (i=0;i<l;i++) 
	{
		ins_a = a+i;
		ins_b = b+i;
        _asm 
		{
			mov ebx, [ins_a]
			movups xmm0, [ebx]
			mulps xmm0,xmm0 
			mov ebx, [ins_b]
			movups [ebx], xmm1
		}
	}
}
#endif
