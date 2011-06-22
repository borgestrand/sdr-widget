/* banal.c

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

#include <fromsys.h>
#include <banal.h>

void
nilfunc (void)
{
}

REAL
sqr (REAL x)
{
	return x * x;
}

int
popcnt (int k)
{
	int c, i;
	c = k & 01;
	for (i = 1; i < 32; i++)
		c += (k >> i) & 01;
	return c;
}

int
npoof2 (int n)
{
	int i = 0;
	--n;
	while (n > 0)
		n >>= 1, i++;
	return i;
}

int
nblock2 (int n)
{
	return 1 << npoof2 (n);
}

int
in_blocks (int count, int block_size)
{
	if (block_size < 1)
	{
		fprintf (stderr, "block_size zero in in_blocks\n");
		exit (1);
	}
	return (1 + ((count - 1) / block_size));
}

FILE *
efopen (char *path, char *mode)
{
	FILE *iop = fopen (path, mode);
	if (!iop)
	{
		fprintf (stderr, "can't open \"%s\" in mode \"%s\"\n", path, mode);
		exit (1);
	}
	return iop;
}

FILE *
efreopen (char *path, char *mode, FILE * strm)
{
	FILE *iop = freopen (path, mode, strm);
	if (!iop)
	{
		fprintf (stderr, "can't reopen \"%s\" in mode \"%s\"\n", path, mode);
		exit (1);
	}
	return iop;
}

size_t
filesize (char *path)
{
	struct stat sbuf;
	if (stat (path, &sbuf) == -1)
		return -1;
	return sbuf.st_size;
}

size_t
fdsize (int fd)
{
	struct stat sbuf;
	if (fstat (fd, &sbuf) == -1)
		return -1;
	return sbuf.st_size;
}

#define MILLION (1000000)

// return current tv
struct timeval
now_tv (void)
{
	struct timeval tv;
	gettimeofday (&tv, 0);
	return tv;
}

// return ta - tb
struct timeval
diff_tv (struct timeval *ta, struct timeval *tb)
{
	struct timeval tv;
	if (tb->tv_usec > ta->tv_usec)
	{
		ta->tv_sec--;
		ta->tv_usec += MILLION;
	}
	tv.tv_sec = ta->tv_sec - tb->tv_sec;
	if ((tv.tv_usec = ta->tv_usec - tb->tv_usec) >= MILLION)
	{
		tv.tv_usec -= MILLION;
		tv.tv_sec++;
	}
	return tv;
}

// return ta + tb
struct timeval
sum_tv (struct timeval *ta, struct timeval *tb)
{
	struct timeval tv;
	tv.tv_sec = ta->tv_sec + tb->tv_sec;
	if ((tv.tv_usec = ta->tv_usec + tb->tv_usec) >= MILLION)
	{
		tv.tv_usec -= MILLION;
		tv.tv_sec++;
	}
	return tv;
}

char *
fmt_tv (struct timeval *tv)
{
	static char buff[256];
	snprintf (buff, sizeof (buff), "%ds%du", tv->tv_sec, tv->tv_usec);
	return buff;
}

char *
since (struct timeval *tv)
{
	struct timeval nt = now_tv (), dt = diff_tv (&nt, tv);
	return fmt_tv (&dt);
}

// linear integer interpolation:
// real vector v, n long, -> real vector u, m long
// *** n must divide m
// returns actual number of valid points in u
// (== n - m/n since v[n] is undefined)

int
hinterp_vec (REAL * u, int m, REAL * v, int n)
{
	if (!u || !v || (n < 2) || (m < n) || (m % n))
		return 0;
	else
	{
		int div = m / n, i, j = 0;
		for (i = 1; i < n; i++)
		{
			int k;
			REAL vl = v[i - 1], del = (v[i] - vl) / div;
			u[j++] = vl;
			for (k = 1; k < div; k++)
				u[j++] = vl + k * del;
		}
		u[j++] = v[n - 1];
		return j;
	}
}

void
status_message (char *msg)
{
	write (2, msg, strlen (msg));
}

FILE *
find_rcfile (char *base)
{
	char path[MAXPATHLEN];
	FILE *fp;
	sprintf (path, "./%s", base);
	if ((fp = fopen (path, "r")))
		return fp;
	else
	{
		char *home = getenv ("HOME");
		if (!home)
			fprintf (stderr, "can't get HOME!\n"), exit (1);
		sprintf (path, "%s/%s", home, base);
		if ((fp = fopen (path, "r")))
			return fp;
	}
	return 0;
}

//------------------------------------------------------------------------

unsigned long
hash (unsigned char *str)
{
	unsigned long hash = 5381;
	int c;
	while (c = *str++)
		hash = ((hash << 5) + hash) + c;	// (hash * 33 + c) better
	return hash;
}

int
gcd (int m, int n)
{
	return (m % n == 0 ? n : gcd (n, m % n));
}

int
least_common_mul (int i, int j)
{
	return (abs ((i * j) / gcd (i, j)));
}
