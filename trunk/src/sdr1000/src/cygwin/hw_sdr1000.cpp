//=================================================================
// hw_sdr1000.cpp
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

#include <hw_sdr1000.h>

SDR1000::SDR1000(char* name, BOOLEAN rfe, BOOLEAN pa, BOOLEAN usb, uint16 lpt_addr)
{
	this->name = name;
	this->rfe = rfe;
	this->pa = pa;
	this->usb = usb;
	this->lpt_addr = lpt_addr;

	if(usb)
	{
		sdr1kusb = new Sdr1kUsb();
		int ret_val = sdr1kusb->Open(rfe, pa, -1);
		if(ret_val != 0) 
			usb = FALSE;
	}
	else
	{
		if(init_count == 0) OpenPortTalk();
		init_count++;
	}
	
	pio_ic01 = new PIOReg("pio_ic01", this, PIO_IC01, 0xA0);
	pio_ic03 = new PIOReg("pio_ic03", this, PIO_IC03, 0x80);
	if(rfe)
	{
		rfe_ic07 = new RFEReg("rfe_ic07", this, RFE_IC07, 0x40);
		rfe_ic09 = new RFEReg("rfe_ic09", this, RFE_IC09);
		rfe_ic10 = new RFEReg("rfe_ic10", this, RFE_IC10);
		rfe_ic11 = new RFEReg("rfe_ic11", this, RFE_IC11);
	}
	dds = new AD9854("dds", this);
	freq_cal_offset = 0.0;
	clock_ref_freq = 10.0;
	spur_reduction_enabled = false;
	spur_reduction_mask = 0xFFFF00000000llu;
	StandBy();
}

SDR1000::~SDR1000()
{
	delete pio_ic01;
	delete pio_ic03;
	if(rfe)
	{
		delete rfe_ic07;
		delete rfe_ic09;
		delete rfe_ic10;
		delete rfe_ic11;
	}
	delete dds;
	if(!usb)
	{
		init_count--;
		if(init_count == 0) ClosePortTalk();
	}
}

void SDR1000::UpdateHW(BOOLEAN b)
{
	pio_ic01->SetUpdateMode(b);
	pio_ic03->SetUpdateMode(b);
	if(rfe)
	{
		rfe_ic07->SetUpdateMode(b);
		rfe_ic09->SetUpdateMode(b);
		rfe_ic10->SetUpdateMode(b);
		rfe_ic11->SetUpdateMode(b);
	}
}

void SDR1000::StandBy()
{
	//dds->Reset(); // causes DDS to go into never never land -- why?
	dds->PowerDown(TRUE);
	UpdateHW(FALSE);

	uint8 temp;
	temp = pio_ic01->GetData();
	pio_ic01->SetData(0xA0);
	pio_ic01->ForceUpdate();
	pio_ic01->SetData(temp);

	temp = pio_ic03->GetData();
	pio_ic03->SetData(0x80);
	pio_ic03->ForceUpdate();
	pio_ic03->SetData(temp);

	if(rfe)
	{
		temp = rfe_ic07->GetData();
		rfe_ic07->SetData(0x40);
		rfe_ic07->ForceUpdate();
		rfe_ic07->SetData(temp);

		temp = rfe_ic09->GetData();
		rfe_ic09->SetData(0);
		rfe_ic09->ForceUpdate();
		rfe_ic09->SetData(temp);

		temp = rfe_ic10->GetData();
		rfe_ic10->SetData(0);
		rfe_ic10->ForceUpdate();
		rfe_ic10->SetData(temp);

		temp = rfe_ic11->GetData();
		rfe_ic11->SetData(0);
		rfe_ic11->ForceUpdate();
		rfe_ic11->SetData(temp);
	}
}

void SDR1000::PowerOn()
{
	dds->PowerDown(FALSE);

	pio_ic01->ForceUpdate();
	pio_ic03->ForceUpdate();
	if(rfe)
	{
		rfe_ic07->ForceUpdate();
		rfe_ic09->ForceUpdate();
		rfe_ic10->ForceUpdate();
		rfe_ic11->ForceUpdate();
	}
	UpdateHW(TRUE);
}

void SDR1000::Latch(uint8 addr, uint8 data)
{
	outp(lpt_addr, data);
	outp(lpt_addr+2, addr);
	outp(lpt_addr+2, 0xB); // 0xB selects none of the boards
}

void SDR1000::SRLoad(uint8 addr, uint8 new_data) // parallel only
{
	uint8 tmp_data;
	uint8 pio_data;
	pio_data = pio_ic01->GetData();
	pio_data &= 0xC0; // clear unused bits
	// Shift 8 bits into the 4 RFE Registers
	for(int i=0x80; i>0; i >>= 1)
	{
		if((i & new_data) == 0) // current bit is low
		{
			tmp_data = pio_data | SCLR_NOT; tmp_data |= DCDR_NE;
			Latch(PIO_IC01, tmp_data);		// Output 0 bit
			tmp_data |= SCK;
			Latch(PIO_IC01, tmp_data);		// Clock 0 into shift register
		}
		else // current bit is high
		{
			tmp_data = pio_data | SCLR_NOT; tmp_data |= DCDR_NE; tmp_data |= SER;
			Latch(PIO_IC01, tmp_data);							// Output 1 bit
			tmp_data |= SCK;
			Latch(PIO_IC01, tmp_data);							// Clock 1 into shift register
		}

		tmp_data = (pio_data | SCLR_NOT); tmp_data |= DCDR_NE;
		Latch(PIO_IC01, tmp_data);				// Return SCK low
	}

	// Strobe the RFE 1:4 decoder output to transfer contents
	// of shift register to output latches
	tmp_data = pio_data | SCLR_NOT; tmp_data |= addr; tmp_data |= DCDR_NE; 
	Latch(PIO_IC01, tmp_data);		// Latch 2:4 decoder outputs
	tmp_data = pio_data | SCLR_NOT; tmp_data |= addr;
	Latch(PIO_IC01, tmp_data);		// Take 2:4 decoder enable low
	tmp_data = pio_data | SCLR_NOT; tmp_data |= addr; tmp_data |= DCDR_NE; 
	Latch(PIO_IC01, tmp_data);		// Take 2:4 decoder enable high
}

void SDR1000::WriteDDS(uint8 reg_index, uint8 new_data) // parallel only
{
	Latch(PIO_IC11, new_data); // setup data bits
	Latch(PIO_IC08, reg_index | DDSWRB); // setup address bits with WRB high
	Latch(PIO_IC08, reg_index); // Send write command with WRB low
	Latch(PIO_IC08, reg_index | DDSWRB); // take WRB back high
}

void SDR1000::ResetDDS() // parallel only
{
	Latch(PIO_IC08, DDSRESET | DDSWRB); // reset the chip
	Sleep(1);
	Latch(PIO_IC08, DDSWRB); // leave DDSWRB high
}

void SDR1000::SetBPF(double freq) // valid from [-infinity, 65.0]  negative indicates no filter
{
	// set BPF
	if(rfe)
	{
		if(freq < 0.0) // select no filter
			rfe_ic10->SetData(rfe_ic10->GetData() & 0x03);
		else if(freq < 2.5) // 160m
			rfe_ic10->SetData((rfe_ic10->GetData() & 0x03) | (1<<7));
		else if(freq < 6.0) // 60m
			rfe_ic10->SetData((rfe_ic10->GetData() & 0x03) | (1<<6));
		else if(freq < 12.0) // 40m
			rfe_ic10->SetData((rfe_ic10->GetData() & 0x03) | (1<<4));
		else if(freq < 24.0) // 20m
			rfe_ic10->SetData((rfe_ic10->GetData() & 0x03) | (1<<5));
		else if(freq < 36.0) // 10m
			rfe_ic10->SetData((rfe_ic10->GetData() & 0x03) | (1<<3));
		else if(freq <= 65.0)
			rfe_ic10->SetData((rfe_ic10->GetData() & 0x03) | (1<<2));
	}
	else
	{
		if(freq < 0.0) // select no filter
			pio_ic01->SetData(pio_ic01->GetData() & 0xC0);
		else if(freq < 2.5) // 160m
			pio_ic01->SetData((pio_ic01->GetData() & 0xC0) | (1<<0));
		else if(freq < 6.0) // 60m
			pio_ic01->SetData((pio_ic01->GetData() & 0xC0) | (1<<1));
		else if(freq < 12.0) // 40m
			pio_ic01->SetData((pio_ic01->GetData() & 0xC0) | (1<<3));
		else if(freq < 24.0) // 20m
			pio_ic01->SetData((pio_ic01->GetData() & 0xC0) | (1<<2));
		else if(freq < 36.0) // 10m
			pio_ic01->SetData((pio_ic01->GetData() & 0xC0) | (1<<4));
		else if(freq <= 65.0)
			pio_ic01->SetData((pio_ic01->GetData() & 0xC0) | (1<<5));
	}
}

void SDR1000::SetLPF(double freq) // valid from [-infinity, +infinity]  negative indicates no filter, >65.0 indicates custom filter
{
	if(rfe)
	{
		if(freq < 0.0) // select no filter
		{
			rfe_ic09->SetData(0);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
		else if(freq < 2.5) // 160m
		{
			rfe_ic09->SetData(0);
			rfe_ic10->SetData((rfe_ic10->GetData() & 0xFC) | (1<<1));
		}
		else if(freq < 4.0) // 80m
		{
			rfe_ic09->SetData(1<<7);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
		else if(freq < 6.0) // 60m
		{
			rfe_ic09->SetData(1<<2);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
		else if(freq < 7.3) // 40m
		{
			rfe_ic09->SetData(1<<5);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
		else if(freq < 12.0) // 30m
		{
			rfe_ic09->SetData(1<<4);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
		else if(freq < 14.5) // 20m
		{
			rfe_ic09->SetData(1<<3);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
		else if(freq < 21.5) // 17/15m
		{
			rfe_ic09->SetData(0);
			rfe_ic10->SetData((rfe_ic10->GetData() & 0xFC) | (1<<0));
		}
		else if(freq < 30.0) // 12/10m
		{
			rfe_ic09->SetData(1<<6);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
		else if(freq < 65.0) // 6m
		{
			rfe_ic09->SetData(1<<1);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
		else // Aux
		{
			rfe_ic09->SetData(1<<0);
			rfe_ic10->SetData(rfe_ic10->GetData() & 0xFC);
		}
	}
}

void SDR1000::SetPALPF(double freq) // valid from [-infinity, 30.0] negative indicates no filter
{
	if(rfe & pa)
	{
		if(freq < 0.0 || freq > 30.0) // select no filter
			rfe_ic11->SetData(rfe_ic11->GetData() & 0xF8);
		else if(freq < 2.5) // 160m
			rfe_ic11->SetData((rfe_ic11->GetData() & 0xF8) | 0x06);
		else if(freq < 4.0) // 80m
			rfe_ic11->SetData((rfe_ic11->GetData() & 0xF8) | 0x05);
		else if(freq < 7.3) // 60/40m
			rfe_ic11->SetData((rfe_ic11->GetData() & 0xF8) | 0x04);
		else if(freq < 14.5) // 30/20m
			rfe_ic11->SetData((rfe_ic11->GetData() & 0xF8) | 0x03);
		else if(freq < 21.5) // 17/15m
			rfe_ic11->SetData((rfe_ic11->GetData() & 0xF8) | 0x02);
		else if(freq <= 30.0) // 12/10m
			rfe_ic11->SetData((rfe_ic11->GetData() & 0xF8) | 0x01);
	}
}

uint8 SDR1000::StatusPort() // returns status port value
{
	if(usb)
		return this->sdr1kusb->GetStatusPort();
	else
		return inp(lpt_addr+1);
}

void SDR1000::SetMute(BOOLEAN b) // sets Mute relay
{
	if(b) pio_ic01->ClearBit(7);
	else pio_ic01->SetBit(7);
}

void SDR1000::SetINAOn(BOOLEAN b) // turns INA gain post mixer on
{
	if(b) pio_ic03->ClearBit(7);
	else pio_ic03->SetBit(7);
}

void SDR1000::SetATTOn(BOOLEAN b) // turns on 10dB Attenuator
{
	if(rfe)
	{
		if(b) rfe_ic07->SetBit(4);
		else rfe_ic07->ClearBit(4);
	}
}

void SDR1000::SetTRX_TR(BOOLEAN b) // sets original TR relay
{
	if(b) pio_ic01->SetBit(6);
	else pio_ic01->ClearBit(6);
}

void SDR1000::SetRFE_TR(BOOLEAN b) // sets RFE TR switches
{
	if(rfe)
	{
		if(b) rfe_ic07->SetData(rfe_ic07->GetData() | 0x03);
		else rfe_ic07->SetData(rfe_ic07->GetData() & 0xFC);
	}
}

void SDR1000::SetPA_TR(BOOLEAN b) // sets PA TR switches
{
	if(rfe & pa)
	{
		if(b) rfe_ic11->SetBit(6);
		else rfe_ic11->ClearBit(6);
	}
}

void SDR1000::SetXVTR_TR(BOOLEAN b) // sets XVTR TR switch
{
	if(rfe)
	{
		if(b) rfe_ic07->SetBit(2);
		else rfe_ic07->ClearBit(2);
	}
}

void SDR1000::SetXVTR_RF(BOOLEAN b) // sets XVTR RF relay
{
	if(rfe)
	{
		if(b) rfe_ic07->SetBit(3);
		else rfe_ic07->ClearBit(3);
	}
}

void SDR1000::SetX2(uint8 val) // valid int values [0, 0x7F] (7 bits)
{
	pio_ic03->SetData((pio_ic03->GetData() & 80) | (val&0x7F));
}

void SDR1000::SetImpOn(BOOLEAN b) // switches in the impulse response to the RF line
{
	if(rfe)
	{
		if(b) rfe_ic07->SetBit(5);
		else rfe_ic07->ClearBit(5);
	}
}

void SDR1000::DoImpulse() // does the impulse.  Requires the SetImpOn be True
{
	if(rfe)
	{
		rfe_ic07->SetBit(7);
		rfe_ic07->ClearBit(7);
	}
}

void SDR1000::SetPA_Bias(BOOLEAN b)
{
	if(rfe)
	{
		if(b) rfe_ic07->SetBit(6);
		else rfe_ic07->ClearBit(6);
	}
}

void SDR1000::SetPA_ADC_CS_NOT(BOOLEAN b)
{
	if(rfe)
	{
		if(b) rfe_ic11->SetBit(5);
		else rfe_ic11->ClearBit(5);
	}
}

void SDR1000::SetPA_ADC_DI(BOOLEAN b)
{
	if(rfe)
	{
		if(b) rfe_ic11->SetBit(4);
		else rfe_ic11->ClearBit(4);
	}
}

void SDR1000::SetPA_ADC_CLK(BOOLEAN b)
{
	if(rfe)
	{
		if(b) rfe_ic11->SetBit(3);
		else rfe_ic11->ClearBit(3);
	}
}

void SDR1000::SetClockRefFreq(int8 freq) // acceptable values are selected integeral divisors of 200MHz
{
	switch(freq)
	{
		case 100: clock_ref_freq = 100.0; dds->Write(0x1E, 2); break;
		case 50: clock_ref_freq = 50.0; dds->Write(0x1E, 4); break;
		case 40: clock_ref_freq = 40.0; dds->Write(0x1E, 5); break;
		case 25: clock_ref_freq = 25.0; dds->Write(0x1E, 8); break;
		case 20: clock_ref_freq = 20.0; dds->Write(0x1E, 10); break;
		case 10: clock_ref_freq = 10.0; dds->Write(0x1E, 20); break;
		default: clock_ref_freq = 10.0; dds->Write(0x1E, 0x20); break; // ref input disabled
	}
}

uint8 SDR1000::PA_ReadADC(uint8 chan) // chan 0 = FWD, chan 1 = REV -- returns an 8 bit value
{
	if(rfe)
	{
		if(usb)
		{
			int tmp = this->sdr1kusb->GetADC();
			if(chan == 0) return (uint8)(tmp&0xFF);
			else if(chan == 1) return (uint8)((tmp>>8)&0xFF);
		}
		else
		{
			SetPA_ADC_CS_NOT(FALSE);			// CS not goes low
			SetPA_ADC_DI(TRUE);			// set DI bit high for start bit
			SetPA_ADC_CLK(TRUE);			// clock it into shift register
			SetPA_ADC_CLK(FALSE);

			// set DI bit high for single ended -- done since DI is already high
			SetPA_ADC_CLK(TRUE);			// clock it into shift register
			SetPA_ADC_CLK(FALSE);

			if(chan == 0)	// Forward Power
			{
				SetPA_ADC_DI(FALSE);		// set DI bit low for Channel 0
			}	
			else	// Reverse Power
			{
				// set DI bit high for Channel 1 -- done since DI is already high
			}
			
			SetPA_ADC_CLK(TRUE);			// clock it into shift register
			SetPA_ADC_CLK(FALSE);

			uint8 num = 0;

			for(int i=0; i<8; i++)			// read 15 bits out of DO
			{
				SetPA_ADC_CLK(TRUE);			// clock high
				SetPA_ADC_CLK(FALSE);			// clock low

				if(StatusPort() & 0x40)	// read DO 
					num++;	// add bit		
				
				if(i != 7) num <<= 1;
			}

			SetPA_ADC_CS_NOT(TRUE);		// CS not goes high

			return num;
		}
	}
	return 0;
}

uint8 SDR1000::ATU_Tune(uint8 mode) // mode 0=bypass, 1=memory, 2=full -- returns 0 if successful
{
	rfe_ic11->ClearBit(7);

	int delay = 0;
	switch(mode)
	{
		case 0: delay = 250; break;
		case 1: delay = 2000; break;
		case 2: delay = 3250; break;
	}

	Sleep(delay);
	rfe_ic11->SetBit(7);

	int count = 0;
	switch(mode)
	{
		case 1:
		case 2:
			while(StatusPort() & 0x40)	// wait for low output from ATU
			{
				Sleep(50);
				if(count++ > 240)	// 12 seconds
					return 1;
			}
			count = 0;
			while(StatusPort() & 0x40)	// wait for high output from ATU
			{
				Sleep(50);
				if(count++ > 240)	// 12 seconds
					return 2;
			}
			Sleep(250);
	}
	return 0;
}

uint8 SDR1000::ReadDDSReg(uint8 reg_index)
{
	return dds->Read(reg_index);
}

void SDR1000::WriteDDSReg(uint8 reg_index, uint8 new_data)
{
	dds->Write(reg_index, new_data);
}

uint64 SDR1000::CurrentFTW()
{
	uint64 ftw = 0;
	for(int i=0; i<6; i++)
	{
		ftw += (uint8)dds->Read(4+i);
		if(i != 5) ftw <<= 8;
	}
	return ftw;
}

double SDR1000::FTW2Freq(uint64 ftw)
{
	return (double)(ftw*(200.0+freq_cal_offset)/0xFFFFFFFFFFFFllu);
}

uint64 SDR1000::Freq2FTW(double freq)
{
	uint64 ftw = (uint64)(freq*0xFFFFFFFFFFFFllu/(200.0+freq_cal_offset));
	if(spur_reduction_enabled) ftw &= spur_reduction_mask;
	return ftw;
}

void SDR1000::SetFreq(double freq) // sets DDS frequency
{
	// set filters
	SetBPF(freq);
	if(rfe) SetLPF(freq);
	if(rfe & pa) SetPALPF(freq);
	dds->SetFreq(Freq2FTW(freq));
}

void SDR1000::SetFreqCalOffset(double freq)
{
	double current_freq = FTW2Freq(CurrentFTW());
	freq_cal_offset = freq;
	dds->SetFreq(Freq2FTW(current_freq));
}

void SDR1000::SetSpurReductionMask(uint64 mask)
{
	double current_freq;
	if(spur_reduction_enabled)
		current_freq = FTW2Freq(CurrentFTW());
	spur_reduction_mask = mask;
	if(spur_reduction_enabled)
		dds->SetFreq(Freq2FTW(current_freq));
}
