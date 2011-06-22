//=================================================================
/// \file hw_reg.h
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

#ifndef _hw_reg_h
#define _hw_reg_h

#include <common.h>

/// Represents a hardware register up to 32 bits wide.
/** The HwReg class was designed to be an easy-to-extend base class for more
 *  specific classes to implement with specific functions to read and write
 *  to the hardware.  Two versions of the data are kept to represent the
 *  current software and hardware values.  Using the SetUpdateMode function,
 *  updates can be queued up and blasted to the hardware in one step to
 *  accelerate hardware writing.
 */
class HwReg
{
protected:
	/// name of the register (mainly for debugging)
	char* name;

	/// current hardware register value		
	uint32 hw_data;	

	/// current software register value	
	uint32 sw_data;	

	/// determines whether to immediately write changes to hardware	
	BOOLEAN update_hw;	
	
	/// Function that reads the contents of the hardware data (optional).
	/** Subclasses will implement this function with specifics for talking
	 *  to hardware to read the hardware register data.
	 *  \param read_data Pointer for read data.
	 *  \return 0 if successful.
	 */
	virtual int32 Read(uint32* read_data) {return 0;}

	/// Function that writes to the hardware data.
	/** Subclasses will implement this function with specifics for talking
	 *  to hardware to write the hardware register data.
	 *  \param new_data Data to be written to hardware.
	 *  \return 0 if successful.
	 */
	virtual int32 Write(uint32 new_data) {return 0;}

	/// Updates the hardware register if necessary using the Write function.
	int32 Update();

public:
	/// Returns the current software register data.
	uint32 GetData(); 	

	/// Sets the software register with new_data.
	/** Writes to the hardware if the current value differs and update_hw
	 *  is true.
	 *  \param new_data The new value to write to the software register.
	 *  \return 0 if successful.
	 */
	int32 SetData(uint32 new_data);	

	/// Gets the current hardware register data.
	/*  \warning Only available if Read is defined in Subclass.
	 *  \param get_data Pointer to integer in which to return the
	 *   hardware register data.
	 *  \return 0 if successful.
	 */
	int32 GetHWData(uint32* get_data);

	/// Gets a single bit from the software register.
	/** \param index Integer representing LSBs valid from 0 to 31.
	 *  \return 1 if true, 0 if false.
	 */
	uint8 GetBit(uint8 index);

	/// Sets a single bit in the software register.
	/** Writes to the hardware if the current value differs and update_hw
	 *  is true.
	 *  \param index Integer representing LSBs valid from 0 to 31.
	 *  \return 0 if successful.
	 */
	int32 SetBit(uint8 index);

	/// Clears a single bit in the software register.
	/** Writes to the hardware if the current value differs and update_hw
	 *  is true.
	 *  \param index Integer representing LSBs valid from 0 to 31.
	 *  \return 0 if successful.
	 */
	int32 ClearBit(uint8 index);

	/// Controls when updates are written to hardware.
	/** \param b If true, updates are written immediately.  If false,
	 *  updates will not be written to hardware until ForceUpdate is called
	 *  or SetUpdateMode(TRUE) is called.
	 *  \return 0 if successful.
	 */
	int32 SetUpdateMode(BOOLEAN b);

	/// Forces an immediate hardware write to synchronize.
	/** Updates the hardware even if update_hw is false.
	 *  \return 0 if successful.
	 */
	int32 ForceUpdate();

	/// Overrides the current software and hardware values.
	/** Forces the software and hardware representations to match.  This is
	 *  a good way to synchronize a powerup or reset condition where the
	 *  hardware state changes without explicitly changing the software
	 *  register.  The idea is to bring the software in line with whatever
	 *  the current state of the hardware.
	 *  \note that this function will NOT call Write.
	 *  \param new_data The new value for the registers.
	 */
	void OverrideData(uint32 new_data);
};

#endif // hw_reg_h
