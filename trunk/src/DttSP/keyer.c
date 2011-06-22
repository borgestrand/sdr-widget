/* keyer.c */
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

// see below for places where serial port is read
// and needs replacement for parallel port hookup

#include <keyer.h>

//========================================================================
// nothing affected by physical port connection here

BOOLEAN
klogic (KeyerLogic kl,
	BOOLEAN dit,
	BOOLEAN dah,
	REAL wpm,
	int iambicmode,
	BOOLEAN midelementmodeB,
	BOOLEAN ditmemory,
	BOOLEAN dahmemory,
	BOOLEAN autocharspacing,
	BOOLEAN autowordspacing, int weight, REAL ticklen)
{
	REAL ditlen = 1200 / wpm;
	int set_element_timeouts = NO_TIMEOUTS_SCHED;

	/* Do we need to initialize the keyer? */
	if (!kl->flag.init)
	{
		kl->flag.prev.dit = dit;
		kl->flag.prev.dah = dah;
		kl->element.last = kl->element.curr = NO_ELEMENT;
		kl->element.iamb = NO_PADDLE_SQUEEZE;
		kl->element.psqam = 0;
		kl->element.invtd = 0;
		kl->timeout.midl = kl->timeout.beep = kl->timeout.elem = 0;
		kl->timeout.dlay = 0;
		kl->dlay_type = NO_DELAY;
		kl->flag.init = 1;
	}

	/* Decrement the timeouts */
	kl->timeout.dlay -= (kl->timeout.dlay > 0 ? ticklen : 0);
	if (kl->timeout.dlay <= 0)
	{
		/* If nothing is scheduled to play, and we just did a character
		spacing delay, and we do auto word spacing, wait for a word
		spacing delay, otherwise resume the normal element timeout
		countdowns */
		if (kl->timeout.elem <= 0 &&
			kl->dlay_type == CHAR_SPACING_DELAY && autowordspacing)
		{
			kl->timeout.dlay = ditlen * 4;
			kl->dlay_type = WORD_SPACING_DELAY;
		}
		else
		{
			kl->dlay_type = NO_DELAY;
			kl->timeout.midl -= kl->timeout.midl > 0 ? ticklen : 0;
			kl->timeout.beep -= kl->timeout.beep > 0 ? ticklen : 0;
			kl->timeout.elem -= kl->timeout.elem > 0 ? ticklen : 0;
		}
	}

	/* Are both paddles squeezed? */
	if (dit && dah)
	{
		kl->element.iamb = PADDLES_SQUEEZED;

		/* Are the paddles squeezed past the middle of the element? */
		if (kl->timeout.midl <= 0)
			kl->element.psqam = 1;
	}
	else
		/* Are both paddles released and we had gotten a squeeze in this element? */
		if (!dit && !dah && kl->element.iamb == PADDLES_SQUEEZED)
			kl->element.iamb = PADDLES_RELEASED;

	/* Is the current element finished? */
	if (kl->timeout.elem <= 0 && kl->element.curr != NO_ELEMENT)
	{
		kl->element.last = kl->element.curr;

		/* Should we insert an inverted element? */
		if ((dit && dah) ||
			(kl->element.invtd && kl->element.iamb != PADDLES_RELEASED) ||
			(kl->element.iamb == PADDLES_RELEASED && iambicmode == MODE_B && (!midelementmodeB || kl->element.psqam)))
		{
			if (kl->element.last == DAH)
				set_element_timeouts = kl->element.curr = DIT;
			else
				set_element_timeouts = kl->element.curr = DAH;
		}
		else
		{
			/* No more element */
			kl->element.curr = NO_ELEMENT;

			/* Do we do automatic character spacing? */
			if (autocharspacing && !dit && !dah)
			{
				kl->timeout.dlay = ditlen * 2; 
				kl->dlay_type = CHAR_SPACING_DELAY;
			}
		}

		kl->element.invtd = 0;
		kl->element.iamb = NO_PADDLE_SQUEEZE;
		kl->element.psqam = 0;
	}

	/* Is no element currently being played? */
	if (kl->element.curr == NO_ELEMENT)
	{
		if (dah)			/* Dah paddle down ? */
			set_element_timeouts = kl->element.curr = DAH;
		else if (dit)		/* Dit paddle down ? */
			set_element_timeouts = kl->element.curr = DIT;
	}

	/* Do the dah memory */
	if (kl->element.curr == DIT && !kl->flag.prev.dah && dah && dahmemory)
		kl->element.invtd = 1;

	/* Do the dit memory */
	if (kl->element.curr == DAH && !kl->flag.prev.dit && dit && ditmemory)
		kl->element.invtd = 1;

	/* If we had a dit (or dah) scheduled to be played after a delay,
	and the operator lifted both paddles before the end of the delay,
	and we have no dit (or dah) memory, forget it */

	if (kl->timeout.dlay > 0 &&
		!dit &&
		!dah &&
		((kl->element.curr == DIT &&
		!ditmemory) || (kl->element.curr == DAH && !dahmemory)))
		set_element_timeouts = kl->element.curr = NO_ELEMENT;

	/* Do we need to set the playing timeouts of an element? */
	switch (set_element_timeouts)
	{
		case NO_ELEMENT:	/* Cancel any dit or dah */
			kl->timeout.beep = 0;
			kl->timeout.midl = 0;
			kl->timeout.elem = 0;
			break;

		case DIT:			/* Schedule a dit? */
			kl->timeout.beep = (ditlen * (REAL) weight) / 50;
			kl->timeout.midl = kl->timeout.beep / 2;
			kl->timeout.elem = ditlen * 2;
			break;

		case DAH:			/* Schedule a dah? */
			kl->timeout.beep = (ditlen * (REAL) weight) / 50 + ditlen * 2;
			kl->timeout.midl = kl->timeout.beep / 2;
			kl->timeout.elem = ditlen * 4;
			break;
	}

	kl->flag.prev.dit = dit;
	kl->flag.prev.dah = dah;

	return kl->timeout.beep > 0 && kl->timeout.dlay <= 0;
}

KeyerState
newKeyerState (void)
{
	return (KeyerState) safealloc (1, sizeof (KeyerStateInfo), "newKeyerState");
}

void
delKeyerState (KeyerState ks)
{
	safefree ((char *) ks);
}

KeyerLogic
newKeyerLogic (void)
{
	return (KeyerLogic) safealloc (1, sizeof (KeyerLogicInfo), "newKeyerLogic");
}

void
delKeyerLogic (KeyerLogic kl)
{
	safefree ((char *) kl);
}

//========================================================================
