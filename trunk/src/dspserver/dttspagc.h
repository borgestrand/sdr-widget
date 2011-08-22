/* n4hyagc.h

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
#ifndef _dttspagc_h
#define _dttspagc_h
typedef enum _agcmode
{ agcOFF, agcLONG, agcSLOW, agcMED, agcFAST } AGCMODE;
typedef struct _dttspagc
{
  int mode;
  struct _gain
  {
    REAL top, now, fastnow, bottom, old, target, raw, fix;
  } gain, fastgain;
  REAL attack;
  REAL one_m_attack;
  REAL decay;
  REAL one_m_decay;
  REAL slope;
  REAL fastattack;
  REAL one_m_fastattack;
  REAL fastdecay;
  REAL one_m_fastdecay;
  REAL hangtime;
  REAL hangthresh;
  REAL fasthangtime;		
  COMPLEX *circ;
  REAL *magvec;
  CXB buff;
  int mask;
  int slowindx;
  int out_indx;
  int hangindex;
  int fastindx;
  int fasthang;			
  char tag[4];
} dttspagc, *DTTSPAGC;

extern void DttSPAgc (DTTSPAGC a, int tick);
extern DTTSPAGC newDttSPAgc (AGCMODE mode, COMPLEX * Vec, int BufSize,
			     REAL Limit, REAL attack, REAL decay,
			     REAL slope, REAL hangtime, REAL samprate,
			     REAL MaxGain, REAL MinGain, REAL Curgain,
			     char *tag);
extern void delDttSPAgc (DTTSPAGC a);
extern void DttSPAgc_flushbuf(DTTSPAGC a);
#define FASTLEAD 200
#endif
