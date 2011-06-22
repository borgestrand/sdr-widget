/* defs.h */
/*
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

#ifndef _defs_h
#define _defs_h

#define RINGMULT (4)
#define METERMULT (20)
#define SPECMULT (4)
#define DEFRATE (48000.0)
#define DEFSIZE (4096)
#define DEFMODE (USB)
#define DEFSPEC (4096)
#define DEFCOMP (512)

#define MAXRX (2)

#ifndef MAXPATHLEN
#define MAXPATHLEN (2048)
#endif

typedef enum _sdrmode
{
  LSB,				//  0
  USB,				//  1
  DSB,				//  2
  CWL,				//  3
  CWU,				//  4
  FMN,				//  5
  AM,				//  6
  DIGU,				//  7
  SPEC,				//  8
  DIGL,				//  9
  SAM,				// 10
  DRM				// 11
} SDRMODE;

typedef enum _swchstate {
  SWCH_FALL,
  SWCH_STDY,
  SWCH_RISE
} SWCHSTATE;

typedef enum _trxmode
{ RX, TX } TRXMODE;

#endif
