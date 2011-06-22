//=================================================================
/// \file hw_sdr1000.h
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

#ifndef _hw_sdr1000_h
#define _hw_sdr1000_h

#include <common.h>
#include <pio_reg.h>
#include <rfe_reg.h>

#include <unistd.h>
#include <sys/io.h>
#define outp(a, b) outb(b, a)
#define inp(a) inb(a)
#define Sleep sleep

#include <sdr1kusb.h> 

/// Number of open instances
static int init_count = 0;

// PIO Register Address
#define PIO_IC01 0x9
#define PIO_IC03 0xA
#define PIO_IC08 0x3
#define PIO_IC11 0xF
#define PIO_NONE 0xB

// RFE 1:4 Decoder (74HC139) values to drive shift register RCK lines
#define RFE_IC07 0x08
#define RFE_IC09 0x18
#define RFE_IC10 0x10
#define RFE_IC11 0x00

// Control and data line pins for RFE serial decoders
#define SER	0x01
#define SCK 0x02
#define SCLR_NOT 0x04
#define DCDR_NE	0x20

// DDS Variables
#define DDSWRB      0x40
#define DDSRESET    0x80
#define COMP_PD     0x10		// DDS Comparator power down
#define BYPASS_PLL  0x20		// Bypass DDS PLL
#define BYPASS_SINC 0x40		// Bypass Inverse Sinc Filter

class AD9854;

/// Represents one SDR-1000 set of hardware
class SDR1000
{
	/// text name of SDR1000 (for debugging)
	char* name;
	
	/// TRUE if RFE board is present
	bool rfe;
	
	/// TRUE if 100W PA is present
	bool pa;

	/// Parallel port address
	unsigned short lpt_addr;

	/// Register representing IC1 on the PIO board
	PIOReg* pio_ic01;

	/// Register representing IC3 on the PIO board
	PIOReg* pio_ic03;

	/// Register representing IC9 on the RFE board
	RFEReg* rfe_ic09;

	/// Register representing IC10 on the RFE board
	RFEReg* rfe_ic10;

	/// Register representing IC7 on the RFE board
	RFEReg* rfe_ic07;

	/// Register representing IC11 on the RFE board
	RFEReg* rfe_ic11;

	/// Object that represents the AD9854 DDS
	AD9854* dds;

	/// used to calibrate the DDS when the 200MHz Osc is off.
	double freq_cal_offset;

	/// optional reference input frequency
	double clock_ref_freq;

	/// spur reduction mask to use only high order bits in the ftw
	unsigned long long spur_reduction_mask;

	/// determines whether to use the spur reduction mask when sending frequencies
	bool spur_reduction_enabled;

	/// Sets the amplifier ADC Chip Select Not line
	void SetPA_ADC_CS_NOT(bool b);

	/// Sets the amplifier ADC Data In line
	void SetPA_ADC_DI(bool b);

	/// Sets the amplifier ADC Clock line
	void SetPA_ADC_CLK(bool b);

	/// Returns the current frequency tuning word (last sent)
	/** \return 48-bit frequency tuning word
	 */
	unsigned long long CurrentFTW();

	/// Changes a tuning word into a frequency in MHz
	/** \param ftw Frequency tuning word to convert
	 *  \return Converted frequency in MHz
	 */
	double FTW2Freq(unsigned long long ftw);

	/// Changes a frequency in MHz to a tuning word
	/** \param freq Frequency in MHz to convert
	 *  \return 48-bit frequency tuning word
	 */
	unsigned long long Freq2FTW(double freq);

public:
	/// Constructor
	SDR1000(char* name, bool rfe, bool pa, bool usb, unsigned short lpt_addr);
	
	/// Destructor
	~SDR1000();

	/// Basic hardware writing for PIO
	/** \param addr PIO board address to write
	 *  \param data Data to be written
	 */
	void Latch(unsigned char addr, unsigned char data);	

	/// Basic hardware writing for RFE
	/** \param addr RFE board address to write
	 *  \param new_data Data to be written
	 */
	void SRLoad(unsigned char addr, unsigned char new_data);

	/// Basic hardware writing for AD9854
	/** \param reg_index Register index to write
	 *  \param new_data Data to write
	 */
	void WriteDDS(unsigned char reg_index, unsigned char new_data);

	/// Reset AD9854 using RSET pin
	void ResetDDS();

	Sdr1kUsb* sdr1kusb;
	bool usb;

	/// put the SDR-1000 into a StandBy (low power) state
	void StandBy();

	/// Put the SDR-1000 into a Run state
	void PowerOn();

	/// Returns the current value of the status port
	unsigned char StatusPort();

	/// Sets the update mode of all registers
	void UpdateHW(bool b);
	
	/// Sets filters, DDS appropriately
	/** \param freq Frequency in MHz
	 */
	void SetFreq(double freq);

	/// Sets the Band Pass Filter on the BPF board
	/** \param freq Frequency in MHz
	 */
	void SetBPF(double freq);

	/// Sets the Low Pass Filter on the RFE board
	/** \param freq Frequency in MHz
	 */
	void SetLPF(double freq);

	/// Sets the Low Pass Filter on the Amplifier
	/** \param freq Frequency in MHz
	 */
	void SetPALPF(double freq);

	/// Sets the Mute Relay on the TRX board
	/** \param b If true, mutes the SPKR output
	 */
	void SetMute(bool b);

	/// Sets the INA Gain
	/** \param b If true, turns on the INA gain
	 */
	void SetINAOn(bool b);

	/// Sets the Attenuator state
	/** \param b If true, turns on the 10dB Attenuator
	 */
	void SetATTOn(bool b);

	/// Sets the transmit relay on the TRX board
	/** \param b If true, puts the relay in transmit mode
	 */
	void SetTRX_TR(bool b);

	/// Sets the transmit relay on the RFE board
	/** \param b If true, puts the relay in transmit mode
	 */
	void SetRFE_TR(bool b);

	/// Sets the transmit relay on the amplifier
	/** \param b If true, puts the relay in transmit mode
	 */
	void SetPA_TR(bool b);

	/// Sets the XVTR transmit relay on the RFE board
	/** \param b If true, puts the relay in transmit mode
	 */
	void SetXVTR_TR(bool b);

	/// Controls whether the RF path to go through the XVTR
	/** \param b If true, the RF goes through the XVTR.  Otherwise it is 
	 *  bypassed.
	 */
	void SetXVTR_RF(bool b);

	/// Sets the lower order 6 bits of the X2 port
	/** \param val Hex value to put on the port.
	 */
	void SetX2(unsigned char val);

	/// Controls whether the impulse circuitry is in the RF path
	/** \param b If true, puts the impulse in circuit.  Otherwise it is
	 *  bypassed.
	 */
	void SetImpOn(bool b);

	/// Controls whether the amplifier bias is on
	/** \param b If true, turns on the amplifier bias.  Otherwise it is
	 *  turned off.
	 */
	void SetPA_Bias(bool b);

	/// Sets the reference clock frequency in MHz
	/** \param freq Frequency in MHz (only integer divisors of 200MHz accepted)
	 */
	void SetClockRefFreq(char freq);

	/// Sets the calibration offset in MHz
	/** \param freq Frequency calibration correction in MHz at 200MHz
	 */
	void SetFreqCalOffset(double freq);

	/// Sets the Spur Reduction Mask
	/** \param mask The mask used to prevent phase truncation spurs on the DDS
	 */
	void SetSpurReductionMask(unsigned long long mask);

	/// Fires the impulse (requires impulse to be in circuit to work)
	void DoImpulse();

	/// Reads the power ADC on the amplifier
	/** \param c If 0, selects forward power.  If 1, selects reverse power.
	 *  \return ADC value from 0 to 255
	 */
	unsigned char PA_ReadADC(unsigned char c);

	/// Runs the ATU tune sequence
	/** \param mode 0 selects bypass, 1 selects memory, 2 selects full.
	 *  \return 0 if successful, 1 if failed on start, 2 if failed on
	 *  detecting finish.
	 */
	unsigned char ATU_Tune(unsigned char mode);

	/// Read DDS register
	/** \param reg_index Register to read
	 *  \return Value read from register
	 */
	unsigned char ReadDDSReg(unsigned char reg_index);
	
	/// Write DDS register
	/** \param reg_index Register to write
	 *  \param new_data Data to write to the register
	 */
	void WriteDDSReg(unsigned char reg_index, unsigned char new_data);
};

#include <ad9854.h>

#endif // _hw_sdr1000_h
