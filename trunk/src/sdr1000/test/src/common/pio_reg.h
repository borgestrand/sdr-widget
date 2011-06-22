//=================================================================
/// \file pio_reg.h
/// \author Eric Wachsmann KE5DTO
//=================================================================
// This file is part of a Software Defined Radio.
// Copyright (C) 2006, 2007  FlexRadio Systems
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// You may contact us via email at: sales@flex-radio.com.
// Paper mail may be sent to: 
//    FlexRadio Systems
//    12100 Technology Blvd.
//    Austin, TX 78727
//    USA
//=================================================================

#ifndef _pio_reg_h
#define _pio_reg_h

#include <common.h>
#include <hw_reg.h>

class SDR1000;

/// Represents a single 8-bit register on the SDR-1000 PIO board
class PIOReg : public HwReg
{
	/// Pointer to SDR1000 for access to configuration data
	SDR1000 *sdr;

	/// Address of particular register on PIO board
	uint8 addr;

protected:
	/// Writes to the PIO register.
	/** Uses the parallel port or USB Adapter depending on SDR1000 config.
	 */
	virtual int32 Write(uint32 new_data);

public:
	/// Basic constructor
	PIOReg(char* name, SDR1000* sdr, uint8 addr);
	/// Constructor with initial value
	PIOReg(char* name, SDR1000* sdr, uint8 addr, uint8 init_val);
};

#include <hw_sdr1000.h>

#endif // _pio_reg_h
