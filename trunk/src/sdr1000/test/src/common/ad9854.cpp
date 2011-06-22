//=================================================================
// ad9854.cpp
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

#include <ad9854.h>

AD9854::AD9854(char* name, SDR1000* sdr)
{
	this->name = name;
	this->sdr = sdr;

	for(int i=0; i<0x28; i++)
	{
		uint8 def_val = 0;
		switch(i)
		{
			case 0x19: def_val = 0x40; break;
			case 0x1D: def_val = 0x10; break;
			case 0x1E: def_val = 0x64; break;
			case 0x1F: def_val = 0x01; break;
			case 0x20: def_val = 0x20; break;
			case 0x25: def_val = 0x80; break;
			default:   def_val = 0; break;
		}
		reg[i] = new AD9854Reg("AD9854Reg", sdr, i);
	}
	Update(TRUE);
	reg[0x1D]->SetData(0x17);
	reg[0x1E]->SetData(0x20);
	reg[0x20]->SetData(0x40);
}

AD9854::~AD9854()
{
	for(int i=0; i<0x28; i++) 
		delete reg[i];
}

void AD9854::Update(BOOLEAN b)
{
	for(int i=0; i<0x28; i++)
		reg[i]->SetUpdateMode(b);
}

void AD9854::Write(uint8 reg_index, uint8 new_data)
{
	reg[reg_index]->SetData(new_data);	
}

uint8 AD9854::Read(uint8 reg_index)
{
	return reg[reg_index]->GetData();
}

void AD9854::SetFreq(uint64 ftw)
{
	for(int i=0; i<6; i++)
		reg[i+4]->SetData((uint8)(ftw>>(40-i*8)));
}

void AD9854::Reset()
{
	if(sdr->usb)
		sdr->sdr1kusb->DDSReset();
	else
		sdr->ResetDDS();

	for(int i=0; i<0x28; i++)
	{
		uint8 def_val = 0;
		switch(i)
		{
			case 0x19: def_val = 0x40; break;
			case 0x1D: def_val = 0x10; break;
			case 0x1E: def_val = 0x64; break;
			case 0x1F: def_val = 0x01; break;
			case 0x20: def_val = 0x20; break;
			case 0x25: def_val = 0x80; break;
			default:   def_val = 0; break;
		}

		uint8 temp = reg[i]->GetData();
		reg[i]->OverrideData(def_val);
		reg[i]->SetData(temp);
	}
}

void AD9854::PowerDown(BOOLEAN b)
{
	if(b)
		reg[0x1D]->SetData(0x17);
	else
		reg[0x1D]->SetData(0x10);
}
