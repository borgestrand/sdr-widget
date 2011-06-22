/* isoband.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY

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

///////////////////////////////////////////////////////////////////////

#include <isoband.h>

typedef
struct _ISOband_t {
  REAL nominal, exact, low, high;
} ISOband_t;

static
ISOband_t _ISOband_info[] = {
  { /* 1 */      1.25,	  1.26f,     1.12f,    1.41f },
  { /* 2 */      1.6f,	  1.58f,     1.41f,    1.78f },
  { /* 3 */      2.0f,	  2.00f,     1.78f,    2.24f },
  { /* 4 */      2.5f,	  2.51f,     2.24f,    2.82f },
  { /* 5 */      3.15f,	  3.16f,     2.82f,    3.55f },
  { /* 6 */      4.0f,	  3.98f,     3.55f,    4.47f },
  { /* 7 */      5.0f,	  5.01f,     4.47f,    5.62f },
  { /* 8 */      6.3f,	  6.31f,     5.62f,    7.08f },
  { /* 9 */      8.0f,	  7.94f,     7.08f,    8.91f },
  { /* 10 */    10.0f,	 10.0f,      8.91f,   11.2f },
  { /* 11 */    12.5f,	 12.59f,    11.2f,    14.1f },
  { /* 12 */    16.0f,	 15.85f,    14.1f,    17.8f },
  { /* 13 */    20.0f,	 19.95f,    17.8f,    22.4f },
  { /* 14 */    25.0f,	 25.12f,    22.4f,    28.2f },
  { /* 15 */    31.5f,	 31.62f,    28.2f,    35.5f },
  { /* 16 */    40.0f,	 39.81f,    35.5f,    44.7f },
  { /* 17 */    50.0f, 	 50.12f,    44.7f,    56.2f },
  { /* 18 */    63.0f, 	 63.10f,    56.2f,    70.8f },
  { /* 19 */    80.0f,    79.43f,    70.8f,    89.1f },
  { /* 20 */   100.0f,   100.00f,    89.1f,   112.0f },
  { /* 21 */   125.0f,   125.89f,   112.0f,   141.0f },
  { /* 22 */   160.0f,   158.49f,   141.0f,   178.0f },
  { /* 23 */   200.0f,   199.53f,   178.0f,   224.0f },
  { /* 24 */   250.0f,   251.19f,   224.0f,   282.0f },
  { /* 25 */   315.0f,   316.23f,   282.0f,   355.0f },
  { /* 26 */   400.0f,   398.11f,   355.0f,   447.0f },
  { /* 27 */   500.0f,   501.19f,   447.0f,   562.0f },
  { /* 28 */   630.0f,   630.96f,   562.0f,   708.0f },
  { /* 29 */   800.0f,   794.33f,   708.0f,   891.0f },
  { /* 30 */  1000.0f,  1000.0f,    891.0f,  1120.0f },
  { /* 31 */  1250.0f,  1258.9f,   1120.0f,  1410.0f },
  { /* 32 */  1600.0f,  1584.9f,   1410.0f,  1780.0f },
  { /* 33 */  2000.0f,  1995.3f,   1780.0f,  2240.0f },
  { /* 34 */  2500.0f,  2511.9f,   2240.0f,  2820.0f },
  { /* 35 */  3150.0f,  3162.3f,   2820.0f,  3550.0f },
  { /* 36 */  4000.0f,  3981.1f,   3550.0f,  4470.0f },
  { /* 37 */  5000.0f,  5011.9f,   4470.0f,  5620.0f },
  { /* 38 */  6300.0f,  6309.6f,   5620.0f,  7080.0f },
  { /* 39 */  8000.0f,  7943.3f,   7080.0f,  8910.0f },
  { /* 40 */ 10000.0f, 10000.0f,   8910.0f, 11200.0f },
  { /* 41 */ 12500.0f, 12589.3f,  11200.0f, 14100.0f },
  { /* 42 */ 16000.0f, 15848.9f,  14100.0f, 17800.0f },
  { /* 43 */ 20000.0f, 19952.6f,  17800.0f, 22400.0f }
};

PRIVATE INLINE ISOband_t *
ISOband_get_info(int band) {
  if (band < 1 || band > 43) {
    fprintf(stderr, "ISO: band index out of range (%d)\n", band);
    exit(1);
  } else
    return &_ISOband_info[band - 1];
}

REAL
ISOband_get_nominal(int band) {
  return ISOband_get_info(band)->nominal;
}

REAL
ISOband_get_exact(int band) {
  return ISOband_get_info(band)->exact;
}

REAL
ISOband_get_low(int band) {
  return ISOband_get_info(band)->low;
}

REAL
ISOband_get_high(int band) {
  return ISOband_get_info(band)->high;
}
