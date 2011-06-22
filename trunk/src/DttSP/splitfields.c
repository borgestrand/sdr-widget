/* splitfields.c
   
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

#include <splitfields.h>

static char *_white = " \t\n";

SPLIT
newSPLIT (void)
{
	return (SPLIT) safealloc (1, sizeof (splitfld), "splitfield");
}

void
delSPLIT (SPLIT s)
{
	if (s)
		free ((char *) s);
}

char *
F (SPLIT s, int i)
{
	return s->f[i];
}

char **
Fptr (SPLIT s, int i)
{
	return &(s->f[i]);
}

int
NF (SPLIT s)
{
	return s->n;
}

int
splitonto (SPLIT s, char *str, char *delim, char **fld, int fmx)
{
	int i = 0;
	char *p = strtok (str, delim);
	while (p)
    {
		fld[i] = p;
		if (++i >= fmx)
			break;
		p = strtok (0, delim);
    }
	return i;
}

int
spliton (SPLIT s, char *str, char *delim)
{
	return (s->n = splitonto (s, str, delim, s->f, MAXFLD));
}

void
split (SPLIT s, char *str)
{
	spliton (s, str, _white);
}
