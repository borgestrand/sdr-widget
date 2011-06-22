//=================================================================
/// \file ad9854.h
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

#ifndef _ad9854_h
#define _ad9854_h

#include <common.h>
#include <ad9854_reg.h>

class SDR1000;

/// Represents a single AD9854 DDS.
class AD9854
{
	/// text name (mainly for debugging)
	char* name;

	/// Pointer to SDR1000 object for config data
	SDR1000* sdr;

	/// Array of 8-bit registers
	AD9854Reg* reg[0x28];

public:
	/// Basic constructor -- allocates registers dynamically
	AD9854(char* name, SDR1000* sdr);
	/// Destructor -- deallocates registers
	~AD9854();	
	/// Sets the UpdateMode for all registers
	void Update(BOOLEAN b);
	/// Writes selected data to selected register.
	/** \param reg_index Selects which register to write. Valid from 0 to 0x27.
	 *  \param new_data The new value for the register.
	 */  
	void Write(uint8 reg_index, uint8 new_data);

	/// Read the data from the selected register.
	/** \param reg_index Selects which register to read. Valid from 0 to 0x27.
	 */
	uint8 Read(uint8 reg_index);

	/// Sets the 48 bit frequency tuning word.
	/** The frequency can be calculated as ftw/(2^48-1)*clock.
	 *  /param ftw The frequency tuning word to write.
	 */
	void SetFreq(uint64 ftw);

	/// Resets the AD9854 using the reset pin.
	void Reset();

	/// Sets whether to put the AD9854 into run or sleep (low power) mode.
	/** \param b If true, puts the AD9854 into a low power state. If false,
	 *  puts it into run mode.
	 */
	void PowerDown(BOOLEAN b);
};

#include <hw_sdr1000.h>

#endif // _ad9854_h
