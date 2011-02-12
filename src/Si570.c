/*
 * Si570.c
 *
 *  Created on: 2010-02-27
 *      Author: TF3LJ
 */

#include "Si570.h"
#include "Mobo_config.h"

uint8_t	si570reg[6]; 					// Globally accessible var, Si570 registers

static double		rfreq, delta_rfreq, old_rfreq;// Used by Si570 freq calc functions below

/*! \brief Si570 Calculate the Registers, HSDIV and N1 dividers
 *
 * \retval bool indicating if valid dividers were found.
 */
uint8_t set_Regs_and_Dividers(double f)
{
	uint8_t		validDividers = TWI_INVALID_ARGUMENT;// Flag indicating if frequency is resolvable
	double	 	rfreq_fraction;
	uint32_t		rfreq_integer_part;
	uint32_t		rfreq_fraction_part;

	// Registers finding the lowest DCO frequenty - code from Fred
	unsigned char	xHS_DIV;				// HSDIV divider can be 4,5,6,7,9,11
	unsigned int	xN1;					// N1 divider can be 1,2,4,6,8...128
	unsigned int	xN;

	// Registers to save the found dividers
	unsigned char	sHS_DIV=0;
	unsigned char	sN1=0;
	unsigned int	sN=0;					// Total division
	unsigned int	N0;					// Total divider needed (N1 * HS_DIV)

	// Find the total division needed.
	// It is always one too low (not in the case reminder is zero, reminder not used here).

	N0 = (double) DCO_MIN / f;
	sN = 11*128;
	for(xHS_DIV = 11; xHS_DIV > 3; xHS_DIV--)
	{
		// Skip the unavailable divider's
		if (xHS_DIV == 8 || xHS_DIV == 10)
			continue;

		// Calculate the needed low speed divider
		xN1 = N0 / xHS_DIV + 1;

		if (xN1 > 128)
			continue;

		// Skip the unavailable divider's
		if (xN1 != 1 && (xN1 & 1) == 1)
			xN1 += 1;

		xN = xHS_DIV * xN1;
		if (sN > xN)
		{
			sN		= xN;
			sN1		= xN1;
			sHS_DIV	= xHS_DIV;
		}
	}

	if (sHS_DIV == 0) 				// no valid dividers found
	{
		return validDividers;
	}

	rfreq = f * (double) sN;			// DCO freq
 	if (rfreq > (double) DCO_MAX)			// calculated DCO freq > max
	{
		return validDividers;
	}

	validDividers = TRUE;

	// rfreq is a 38 bit number, MSB 10 bits integer portion, and LSB 28 fraction
	// in the Si570 registers, tempBuf[1] has 6 bits, and tempBuf[2] has 4 bits of the integer portion

	rfreq = rfreq / ((double) cdata.FreqXtal/_2(24));// DCO divided by fcryst
	rfreq_integer_part = rfreq;
	rfreq_fraction = rfreq - rfreq_integer_part;
	rfreq_fraction_part = rfreq_fraction * (1L << 28);

	sHS_DIV -= 4;
	sN1 -= 1;
	si570reg[0] = (sHS_DIV << 5) | (sN1 >> 2);
	si570reg[1] = (sN1 & 3) << 6;
	si570reg[1] |= ((rfreq_integer_part >> 4) & 0x3f);
	si570reg[2] = ((rfreq_integer_part & 0x0f) << 4) | (rfreq_fraction_part >> 24);
	si570reg[3] = rfreq_fraction_part >> 16;
	si570reg[4] = rfreq_fraction_part >> 8;
	si570reg[5] = rfreq_fraction_part;
	return validDividers;
}

/*! \brief Si570 Retrieve the running frequency based on the crystal frequency
 *
 * \retval none.
 */
double Freq_From_Register(double fcryst)			// side effects: rfreq and delta_rfreq are set
{
	double 	freq_double;
	uint8_t	n1;
	uint8_t	hsdiv;
	uint32_t	rfreq_integer_portion, rfreq_fraction_portion;

	// Now find out the current rfreq and freq
	hsdiv = ((si570reg[0] & 0xE0) >> 5) + 4;
	n1 = ((si570reg[0] & 0x1f ) << 2 ) + ((si570reg[1] & 0xc0 ) >> 6 );

	//	if(n1 == 0) n1 = 1;
	//	else if((n1 & 1) !=0) n1 += 1;
	n1 += 1;

	rfreq_integer_portion = ((uint32_t)(si570reg[1] & 0x3f)) << 4 |
							((uint32_t)(si570reg[2] & 0xf0)) >> 4;

	rfreq_fraction_portion = ((uint32_t) (si570reg[2] & 0x0f)) << 24;
	rfreq_fraction_portion += ((uint32_t)(si570reg[3])) << 16;
	rfreq_fraction_portion += ((uint32_t)(si570reg[4])) << 8;
	rfreq_fraction_portion += ((uint32_t)(si570reg[5]));

	rfreq = (double)rfreq_integer_portion + ((double)rfreq_fraction_portion / (1L << 28));

	if (rfreq >= old_rfreq) delta_rfreq = rfreq - old_rfreq;
	else delta_rfreq = old_rfreq - rfreq;

	freq_double = fcryst * rfreq / (double) hsdiv / (double) n1;
	return (freq_double);
}

/*! \brief Si570 Set the running frequency
 *
 * \retval none.
 */
uint8_t SetFrequency(double f)
{
	uint8_t	status = TWI_INVALID_ARGUMENT;
	double freq_double, delta_freq;
	static double old_freq_double;

	status = (set_Regs_and_Dividers(f));

	if(status == TRUE)
	{
		// check for smooth tune range
		freq_double = Freq_From_Register((double)cdata.FreqXtal/_2(24));

		if (freq_double >= old_freq_double) delta_freq = freq_double - old_freq_double;
		else delta_freq = old_freq_double - freq_double;

		// If outside of smoothtune range, update everything
		if (((delta_rfreq / old_rfreq ) > ((double) cdata.SmoothTunePPM / 1000000L)) || (delta_freq > 0.5))
		{
			// Write all, including dividers.  This will cause a pause in the Si570 output
			status = Si570FreezeNCO(cdata.Si570_I2C_addr);
			if (status == TWI_SUCCESS)
			{
				WriteRegToSi570(cdata.Si570_I2C_addr);
				Si570UnFreezeNCO(cdata.Si570_I2C_addr);
				Si570NewFreq(cdata.Si570_I2C_addr);
			}
			old_rfreq = rfreq;
			old_freq_double = freq_double;
		}
		// Inside smoothtune range, just update the registers
		else status = WriteRegToSi570(cdata.Si570_I2C_addr);	// Write only the updated rFreq (smoothtune)

		// set filters, using set freq without offset and multiplier
		//if (abpf_flag) Set_BPF((float) set_frequency);
		//Set_LPF((float)set_frequency);

	}
	return status;	// Return TWI_SUCCESS / TWI errors / or TWI_INVALID_ARGUMENT if no valid dividers
}

//-----------------------------------------------------------------------------------------------------
/*! \brief Si570 I2C functions
 *
 * \retval I2C status.
 */

uint8_t Si570Init(uint8_t i2c_address)
{
	uint8_t status;
	status = WriteCmdToSi570(i2c_address, 135, 0x00);

	return status;
}

uint8_t Si570NewFreq(uint8_t i2c_address)
{
	uint8_t status;
	status = WriteCmdToSi570(i2c_address, 135, 0x40);

	return status;
}

uint8_t Si570FreezeNCO(uint8_t i2c_address)
{
	uint8_t status;
	status = WriteCmdToSi570(i2c_address, 137, 0x10);

	return status;
}

uint8_t Si570UnFreezeNCO(uint8_t i2c_address)
{
	uint8_t status;
	status = WriteCmdToSi570(i2c_address, 137, 0x00);

	return status;
}

uint8_t WriteCmdToSi570(uint8_t i2c_address, uint8_t reg, uint8_t d)
{
	uint8_t	status;
	twi_package_t packet;

	uint8_t data[2];

	data[0]= reg;
	data[1]= d;

	// Wait for I2C port to become free
	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0x00;
	packet.addr_length = 0;
	packet.buffer = data;
	packet.length = 2;

	status = twi_master_write(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	return status;
}

uint8_t WriteRegToSi570(uint8_t i2c_address)
{
	twi_package_t packet;
	uint8_t	status, i;
	uint8_t reg_write[7];

	reg_write[0]= 7;	// Set to Register 7
	for (i=0;i<6;i++)	// Copy the registers into write buffer
	{
		reg_write[i+1] = si570reg[i];
	}

	// Wait for I2C port to become free
	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0;
	packet.addr_length = 0;
	packet.buffer = reg_write;
	packet.length = 7;

	status = twi_master_write(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	return status;
}

// This function is not really necessary, as we always know our registers.
uint8_t GetRegFromSi570(uint8_t i2c_address)
{
	uint8_t status;
	twi_package_t packet;
	uint8_t data[1];

	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0;
	packet.addr_length = 0;

	// Set Register offset to 7 and write out
	*data = 7;
	packet.buffer = data;
	packet.length = 1;
	status = twi_master_write(MOBO_TWI, &packet);

	// Read registers from Si570
	packet.buffer = si570reg;
	packet.length = 6;
	status = twi_master_read(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	// Todo... revise:
	return status ? 0 : 1;
}
