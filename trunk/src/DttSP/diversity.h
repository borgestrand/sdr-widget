/* diversity.h */

/*
This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2009 by Frank Brickle, AB2KT and Bob McGwier, N4HY

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
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

#ifndef _diversity_h
#define _diversity_h
#include <datatypes.h>
#include <pthread.h>

typedef struct _diversity_block
{
	BOOLEAN flag;
	COMPLEX scalar;
	REAL gain;
	pthread_mutex_t diversity_osc_mutex,diversity_sum_mutex, diversity_out_mutex, diversity_trx_mutex;
	pthread_cond_t dv_osc_cond,dv_sum_cond,dv_out_cond, dv_trx_cond;
} DiversityControl;


#if 0
extern void barrier_osc();
extern void barrier_sum();
extern void barrier_out();


#endif
#endif