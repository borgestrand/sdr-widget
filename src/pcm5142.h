/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */

/* Written by Borge Strand-Bergesen 20211116
 * Interaction with PCM5142
 *
 * Copyright (C) Borge Strand-Bergesen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PCM5142_H_
#define PCM5142_H_

#include <stdint.h>

#define PCM5142_DEV_ADR		0x4C 				// 0x4C with various address selectors grounded

// Select input built-in interpolation filter
void pcm5142_filter(uint8_t filter_sel);

// Read a single byte from PCM5142
uint8_t pcm5142_read_byte(uint8_t int_adr);

// Write a single byte to PCM5142
uint8_t pcm5142_write_byte(uint8_t int_adr, uint8_t int_data);

#endif /* PCM5142_H_ */
