/* local.h

Some manifest constants for the particular implementation
   
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

/* #include <fromsys.h> */
/* #include <datatypes.h> */
/* #include <banal.h> */
/* #include <fftw3.h> */
/* #include <sdrexport.h> */

#ifndef _local_h
#define _local_h

#include <common.h>
//\\.\pipe\pipename
#define RCBASE ".DttSPrc"
#define PARMPATH  "\\\\.\\pipe\\SDRcommands"
#define METERPATH "\\\\.\\pipe\\SDRmeter"
#define SPECPATH  "\\\\.\\pipe\\SDRspectrum"
#define WISDOMPATH ".\\wisdom"


extern struct _loc
{
  char name[MAXPATHLEN];
  struct
  {
    char rcfile[MAXPATHLEN],
      parm[MAXPATHLEN],
      meter[MAXPATHLEN], spec[MAXPATHLEN], wisdom[MAXPATHLEN];
  } path;
  struct
  {
    REAL rate;
    int size, nrx, spec, comp;
    SDRMODE mode;
  } def;
  struct
  {
    int ring;
  } mult;
} loc[3];


#endif
