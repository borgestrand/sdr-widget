//=================================================================
// hw_reg.cpp
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

#include <hw_reg.h>

int32 HwReg::Update()
{
	int32 ret_val = 0;
	if(update_hw)
	{
		if(hw_data != sw_data)
		{
			ret_val = this->Write(sw_data);
			hw_data = sw_data;
		}
	}
	return ret_val;
}

uint32 HwReg::GetData()
{
	return sw_data;
}

int32 HwReg::SetData(uint32 new_data)
{
	sw_data = new_data;
	return Update();
}

int32 HwReg::GetHWData(uint32* get_data)
{
	return this->Read(get_data);
}

uint8 HwReg::GetBit(uint8 index)
{
	return (sw_data>>index)&1;
}

int32 HwReg::SetBit(uint8 index)
{
	sw_data |= 1<<index;
	return Update();
}

int32 HwReg::ClearBit(uint8 index)
{
	sw_data &= ~(1<<index);
	return Update();
}

int32 HwReg::SetUpdateMode(BOOLEAN b)
{
	update_hw = b;
	return Update();
}

int32 HwReg::ForceUpdate()
{
	int32 ret_val = 0;
	ret_val = this->Write(sw_data);
	hw_data = sw_data;
	return ret_val;
}

void HwReg::OverrideData(uint32 new_data)
{
	sw_data = hw_data = new_data;
}
