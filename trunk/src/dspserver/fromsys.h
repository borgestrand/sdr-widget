/* fromsys.h
   stuff we need to import everywhere 
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

#ifndef _fromsys_h
#define _fromsys_h

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <winsock2.h>
//#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358928
#endif
#include <assert.h>
//#define DttSP_EXP __declspec(dllexport)
//#define DttSP_IMP __declspec(dllimport)
#define DttSP_EXP
#define DttSP_IMP
#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN (2048)
#endif

#include <iosdr.h>
struct timezone
{
  int tz_minuteswest;		/* minutes W of Greenwich */
  int tz_dsttime;		/* type of dst correction */
};

#ifdef DONT_INCLUDE
__inline int
gettimeofday (struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  LARGE_INTEGER li;
  __int64 t;
  static int tzflag;

  if (tv)
    {
      GetSystemTimeAsFileTime (&ft);
      li.LowPart = ft.dwLowDateTime;
      li.HighPart = ft.dwHighDateTime;
      t = li.QuadPart;		/* In 100-nanosecond intervals */
      t -= EPOCHFILETIME;	/* Offset to the Epoch time */
      t /= 10;			/* In microseconds */
      tv->tv_sec = (long) (t / 1000000);
      tv->tv_usec = (long) (t % 1000000);
    }

  if (tz)
    {
      if (!tzflag)
	{
	 _tzset ();
	 tzflag++;
	}
      tz->tz_minuteswest = _timezone / 60;
      tz->tz_dsttime = _daylight;
    }

  return 0;
}
#endif

//#define snprintf _snprintf
#endif // _fromsys_h
