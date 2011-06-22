/* keyer.h */
/*
This file is part of a program that implements a Software-Defined Radio.

The code in this file is derived from routines originally written by
Pierre-Philippe Coupard for his CWirc X-chat program. That program
is issued under the GPL and is
Copyright (C) Pierre-Philippe Coupard - 18/06/2003

This derived version is
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

#ifndef _keyer_h
#define _keyer_h

#include <fromsys.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>

//========================================================================

#define DSR_LINE_CLOSED_KEY	 (1)
#define CTS_LINE_CLOSED_KEY	 (1)
#define DTR_LINE_SET		 (0)
#define RTS_LINE_SET		 (0)

#define NO_TIMEOUTS_SCHED	(-2)
#define NO_ELEMENT		(-1)
#define DIT			 (0)
#define DAH			 (1)
#define MODE_A			 (0)
#define MODE_B			 (1)
#define NO_PADDLE_SQUEEZE	 (0)
#define PADDLES_SQUEEZED	 (1)
#define PADDLES_RELEASED	 (2)
#define NO_DELAY		 (0)
#define CHAR_SPACING_DELAY	 (1)
#define WORD_SPACING_DELAY	 (2)
#define DEBOUNCE_BUF_MAX_SIZE	(30)

//========================================================================

typedef struct _keyer_state
{
	struct
	{
		BOOLEAN iambic,		// iambic or straight
		mdlmdB, revpdl;		// paddles reversed

		struct
		{
			BOOLEAN dit, dah;
		} memory;

		struct
		{
			BOOLEAN khar, word;
		} autospace;

	} flag;

	int debounce,			// # seconds to read paddles
		mode,			// 0 = mode A, 1 = mode B
		weight;			// 15 -> 85%

	REAL wpm;			// for iambic keyer

} KeyerStateInfo, *KeyerState;

extern KeyerState newKeyerState (void);
extern void delKeyerState (KeyerState ks);

//------------------------------------------------------------------------

typedef struct _keyer_logic
{
	struct
	{
		BOOLEAN init;

		struct
		{
			BOOLEAN dit, dah;
		} prev;

	} flag;

	struct
	{
		BOOLEAN invtd,		// insert inverted element
			psqam;			// paddles squeezed after mid-element
		int curr,			// -1 = nothing, 0 = dit, 1 = dah
			iamb,			//  0 = none, 1 = squeezed, 2 = released
			last;			// -1 = nothing, 0 = dit, 1 = dah
	} element;

	struct
	{
		REAL beep, dlay, elem, midl;
	} timeout;

	int dlay_type;		// 0 = none, 1 = interchar, 2 = interword

} KeyerLogicInfo, *KeyerLogic;

extern KeyerLogic newKeyerLogic (void);
extern void delKeyerLogic (KeyerLogic kl);

//========================================================================

extern BOOLEAN klogic (KeyerLogic kl,
		       BOOLEAN dit,
		       BOOLEAN dah,
		       REAL wpm,
		       int iambicmode,
		       BOOLEAN midelementmodeB,
		       BOOLEAN ditmemory,
		       BOOLEAN dahmemory,
		       BOOLEAN autocharspacing,
		       BOOLEAN autowordspacing, int weight, REAL ticklen);

extern BOOLEAN read_straight_key_serial (KeyerState ks, int fd);
extern BOOLEAN read_iambic_key_serial (KeyerState ks, int fd, KeyerLogic kl,
				       REAL ticklen);

//========================================================================

#endif
