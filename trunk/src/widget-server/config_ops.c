/***
 * SoftRock USB I2C host control program
 * Copyright (C) 2009 Andrew Nilsson (andrew.nilsson@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Based on powerSwitch.c by Christian Starkjohann,
 * and usbtemp.c by Mathias Dalheimer
 * of Objective Development Software GmbH (2005)
 * (see http://www.obdev.at/avrusb)
 */

/*
   General Description:
   This program queries and controls the AVR Si570 hardware.
   It must be linked with libusb, a library for accessing the USB bus from
   Linux, FreeBSD, Mac OS X and other Unix operating systems. Libusb can be
   obtained from http://libusb.sourceforge.net/.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

/* Needed to compile on older FC3 systems */
#if defined __linux
#include <sys/types.h>
#include <linux/limits.h>
#endif

#include <usb.h>    /* this is libusb, see http://libusb.sourceforge.net/ */

#include "operations.h"
#include "config_ops.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

extern int verbose;

extern int major;
extern int minor;

extern int i2cAddress;
extern double fXtall;
extern double startupFreq;
extern double multiplier;
extern double subHarmonicStart;

void displayBPFTable(usb_dev_handle *handle, unsigned short *FilterCrossOver, int nFilters);
void displayLPFtable(usb_dev_handle *handle, unsigned short *FilterCrossOver, int length);


void calibrate(usb_dev_handle *handle)
{
	char buffer[6];
	int request = REQUEST_READ_REGISTERS;
	int value = SI570_I2C_ADDR;
	int index = 0;
	int retVal;
        
	
	// Si570 RECALL function
	char i2cError;
    	retVal = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 0x20, SI570_I2C_ADDR | (135<<8), 0x01, &i2cError, 1, 500);
	if (retVal != 1 || i2cError != 0) {
		fprintf(stderr, "Failed reseting to factory frequency\n");
	}

	// send message to USB device
	if (usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, request, value, index, buffer, 6, 500)) 
	{	

		int RFREQ_int = ((buffer[2] & 0xf0) >> 4) + ((buffer[1] & 0x3f) * 16);
		int RFREQ_frac = (256 * 256 * 256 * (buffer[2] & 0xf)) + (256 * 256 * buffer[3]) + (256 * buffer[4]) + (buffer[5]);
		double RFREQ = RFREQ_int + (RFREQ_frac / 268435456.0);
		
		int N1 = ((buffer[1] & 0xc0 ) >> 6) + ((buffer[0] & 0x1f) * 4);
		
		int HS_DIV = (buffer[0] & 0xE0) >> 5;
		int HS_DIV_MAP[] = {4,5,6,7,-1,9,-1,11}; 
		
		if (verbose){
			printf("RFREQ = %f\n", RFREQ);
			printf("N1 = %d\n", N1);
			printf("HS_DIV = %d, HS_DIV_MAP[%d] = %d\n", HS_DIV, HS_DIV, HS_DIV_MAP[HS_DIV]);
		}
		// Validate the calibration result before saving it
		// DEFAULT_XTALL
		double newXtallFreq = (startupFreq * (N1 +1) * HS_DIV_MAP[HS_DIV]) / RFREQ;
		
		if ((1000000.0 * fabs(newXtallFreq - SI570_NOMINAL_XTALL_FREQ) / SI570_NOMINAL_XTALL_FREQ) <= SI570_XTALL_DEVIATION_PPM) {
			printf("fXTALL = %f\n", newXtallFreq);
		} else {
			fprintf(stderr, "Calibration Failed: The calculated crystal reference is outside of the spec for the Si570 device. The most likely possibility is that since power own, the Si570 has been shifted from the factory default frequency.\n");
		}
	} else {
		fprintf(stderr, "Error communicating with USB device\n");
	}
}

void readStartupFreq(usb_dev_handle *handle) {
	unsigned int iFreq;
	int nBytes;
	
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_STARTUP, 0, 0, (char *) &iFreq, sizeof(iFreq), 500);

	if (nBytes == 4) {
		double dFreq;		
        	dFreq = (double)iFreq / (1UL<<21);
		printf("Startup Freq: %f (x %.2f)\n", dFreq / multiplier, multiplier);
	}
}

void readXtallFreq(usb_dev_handle *handle) {
	unsigned int iFreq;
	int nBytes;
	
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_XTALL, 0, 0, (char *) &iFreq, sizeof(iFreq), 500);

	if (nBytes == 4) {
		double dFreq;		
        	dFreq = (double)iFreq / (1UL<<24);
		printf("Xtall Freq  : %f\n", dFreq);
	}
}

int readMultiplyLO(usb_dev_handle *handle, int index, double * mul, double *sub) {
	
//	double sub, mul;
	int nBytes;
	unsigned int iSM[2];
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_MULTIPLY_LO, 0, index, (char *) iSM, sizeof(iSM), 500);

	if (nBytes == sizeof(iSM)) {
		*sub = (double)(int) iSM[0] / (1UL << 21); // Signed value
		*mul = (double)      iSM[1] / (1UL << 21);
			return 0;
	} else {
			return -1;
	}
}


void readSmoothTunePPM(usb_dev_handle *handle) {
	unsigned short smooth;
	int nBytes;
	
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_SMOOTH_TUNE_PPM, 0, 0, (char *) &smooth, sizeof(smooth), 500);

	if (nBytes == 2) {
		printf("Smooth Tune : %d PPM\n", smooth);
	}
}

int readFilters(usb_dev_handle *handle, int isLPF, unsigned short * FilterCrossOver, int length) {
//	unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
	int nBytes;
	int index = isLPF == 0 ? 255 : 255 + 256;
	
	// first find out how may cross over points there are for the 1st bank, use 255 for index
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, 0, index, (char *) FilterCrossOver, length, 500);
  
	return nBytes / 2;  
}

int readBPFAddresses(usb_dev_handle *handle, unsigned char * addresses) {
	int nBytes;
	
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_BPF_ADDRESS, 0, 0, (char *) addresses, 16, 500);
	
	return nBytes;
}

void setBPFAddress(usb_dev_handle *handle, int index, int value) {
	unsigned char * addresses[16]; 
	
	usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_SET_BPF_ADDRESS, value, index, (char *) addresses, sizeof(addresses), 500);

}

void setBPFCrossOver(usb_dev_handle *handle, int index, float newFreq) {
  unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
  int nFilters;

	
  // first find out how may cross over points there are for the 1st bank, use 255 for index
  //nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, 0, 256+255, (char *) FilterCrossOver, sizeof(FilterCrossOver), 500);
  nFilters = readFilters(handle, FALSE, FilterCrossOver, sizeof(FilterCrossOver));

  if (nFilters > 0) {
    FilterCrossOver[index] = newFreq * (1<<5);
    int i;
    // even if we just set one point, we have to set all
    for (i = 0; i < nFilters - 1; i++) 
	    usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, FilterCrossOver[i], i, NULL, 0, 500);
    // read out the values when setting the flag.
    usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, FilterCrossOver[i], i, (char *) FilterCrossOver, sizeof(FilterCrossOver), 500);
    displayBPFTable(handle, FilterCrossOver, nFilters);
  }

}

int readLPFAddresses(usb_dev_handle *handle, unsigned char * addresses) {
	int nBytes;
	
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_LPF_ADDRESS, 0, 0, (char *) addresses, 16, 500);
	
	return nBytes;
}

void setLPFAddress(usb_dev_handle *handle, int index, int value) {
	unsigned char * addresses[16]; 
	
	usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_SET_LPF_ADDRESS, value, index, (char *) addresses, sizeof(addresses), 500);

}

void setLPFCrossOver(usb_dev_handle *handle, int index, float newFreq) {
	unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
	int nFilters;

	// first find out how may cross over points there are for the 2nd bank, use 256+255 for index
	//nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, 0, 256+255, (char *) FilterCrossOver, sizeof(FilterCrossOver), 500);
  nFilters = readFilters(handle, TRUE, FilterCrossOver, sizeof(FilterCrossOver));

	if (nFilters > 0) {
		FilterCrossOver[index] = newFreq * (1<<5);
		int i;
		// even if we just set one point, we have to set all
		for (i = 0; i < nFilters - 1; i++) 
			usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, FilterCrossOver[i], 256+i, NULL, 0, 500);
		// read out the values when setting the flag.
		usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, FilterCrossOver[i], 256+i, (char *) FilterCrossOver, sizeof(FilterCrossOver), 500);
		displayLPFtable(handle, FilterCrossOver, nFilters);
	}
}

void setBPF(usb_dev_handle *handle, int enable) {
	unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
	int nBytes;

	// first find out how may cross over points there are for the 1st bank, use 255 for index
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, 0, 255, (char *) FilterCrossOver, sizeof(FilterCrossOver), 500);
  
	if (nBytes > 2) {

		nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, enable, (nBytes / 2) - 1, (char *) FilterCrossOver, sizeof(FilterCrossOver), 500);

		printf("Filter Bank 1:\n");
		int i;
		for (i = 0; i < (nBytes / 2) - 1; i++) {
			printf("  CrossOver[%d] = %f\n", i, (double) FilterCrossOver[i] / (1UL << 5));
		}
		printf("  BPF Enabled: %d\n", FilterCrossOver[(nBytes / 2) - 1]); 

	}	
}

void setLPF(usb_dev_handle *handle, int enable) {
	unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
	int nBytes;

	// first find out how may cross over points there are for the 2nd bank, use 256+255 for index
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, 0, 256+255, (char *) FilterCrossOver, sizeof(FilterCrossOver), 500);
  
	if (nBytes > 2) {

		nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_FILTERS, enable, 256 + (nBytes / 2) - 1, NULL, 0, 500);

	}	
}

void setXtallFrequency(usb_dev_handle *handle, double xtallFreq) {
	unsigned int iXtallFreq;
	int nBytes;

	iXtallFreq = (unsigned int)( xtallFreq * (1UL<<24) );
	if (verbose)
		printf("Crystal Freq = %f (%d)\n", xtallFreq, iXtallFreq);

	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, REQUEST_SET_XTALL_FREQ, 0, 0, (char *) &iXtallFreq, sizeof(iXtallFreq), 500);
  
	if (verbose >= 2)
		printf("Return = %d\n", nBytes);

	if (nBytes < 0)
		fprintf(stderr, "Failed writing Xtall Frequency to device\n");
}

void setStartupFrequency(usb_dev_handle *handle, double startupFreq) {
	unsigned int iStartupFreq;
	int nBytes;

	iStartupFreq = (unsigned int)( startupFreq * multiplier * (1UL<<21) );

	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, REQUEST_SET_STARTUP_FREQ, 0, 0, (char *) &iStartupFreq, sizeof(iStartupFreq), 500);
  
	if (nBytes < 0)
		fprintf(stderr, "Failed writing startup Frequency to device\n");
}

void setSi570Address(usb_dev_handle *handle, unsigned char newI2cAddress) {
  unsigned char i2cAddress;
  int nBytes;

  nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_SET_SI570_ADDR, 0, newI2cAddress, (char *) &i2cAddress, sizeof(i2cAddress), 500);
  
  if (nBytes < 0)
    fprintf(stderr, "Failed writing si570 i2c addresss to device\n");

}

short readSi570Address(usb_dev_handle *handle) {
  unsigned char i2cAddress;
  int nBytes;
  
  nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_SET_SI570_ADDR, 0, 0, (char *) &i2cAddress, sizeof(i2cAddress), 500);
  
  if (nBytes = 1) {
    printf("Si570 I2C   : %x Hex\n", i2cAddress);
    return i2cAddress;
  } else {
    fprintf(stderr, "Failed writing si570 i2c addresss to device\n");
    return -1;
  }
}

void setMultiplyLo(usb_dev_handle * handle, int index, double mul, double sub) 
{
	int nBytes;
    unsigned int iSM[2];

	iSM[0] = (unsigned int)( sub * (1UL << 21) );
	iSM[1] = (unsigned int)( mul * (1UL << 21) );

	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, REQUEST_SET_MULTIPLY_LO, 0, index, (char *) &iSM, sizeof(iSM), 500);
  
	if (nBytes != sizeof(iSM))
		fprintf(stderr, "Failed writing multiply/lo values to device\n");

}

void displayBands(usb_dev_handle *handle) {
  unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
  int nFilters;
  
  nFilters = readFilters(handle, FALSE, FilterCrossOver, sizeof(FilterCrossOver));
  
  if (nFilters > 0) {
    unsigned char addresses[16];
    int nBytes;
    if ((nBytes = readBPFAddresses(handle, addresses)) < nFilters) {
      // failed to read anything - populate with default
      for (int i = 0; i < nFilters; i++) {
				addresses[i] = i;
      }
    }
    printf("  BPF Enabled: %d\n", FilterCrossOver[nFilters - 1]);
    if (FilterCrossOver[nFilters - 1] == 0)
      return;
    printf("    Band    BPF      Si570\n");
    printf("----------  ---  -----------------\n");
    int i;
    double mul, sub;
    double bandStart = 0.0;
    double bandStop;
    for (i = 0; i < nFilters - 1; i++) {
      readMultiplyLO(handle, i, &mul, &sub);
      bandStop =  (double) FilterCrossOver[i] / (1UL << 5);
      printf("%4.1f..%4.1f  %2d   (F - %3.2f) * %3.5f\n", bandStart, bandStop, addresses[i], sub, mul);
      bandStart = bandStop;
    }
    readMultiplyLO(handle, i, &mul, &sub);
    printf("%4.1f..      %2d   (F - %3.2f) * %3.5f\n", bandStart, addresses[i], sub, mul);
  }
}

void displayBPFTable(usb_dev_handle *handle, unsigned short *FilterCrossOver, int nFilters) { 
//  if (nFilters > 0) {
    unsigned char addresses[16];
    int nBytes;
    if ((nBytes = readBPFAddresses(handle, addresses)) < nFilters) {
      // failed to read anything - populate with default
      for (int i = 0; i < nFilters; i++) {
				addresses[i] = i;
      }
    }
    printf("  BPF Enabled: %d\n", FilterCrossOver[nFilters - 1]);
    if (FilterCrossOver[nFilters - 1] == 0)
      return;
    printf("    Band    BPF\n");
    printf("----------  ---\n");
    int i;
    double bandStart = 0.0;
    double bandStop;
    for (i = 0; i < nFilters - 1; i++) {
      bandStop =  (double) FilterCrossOver[i] / (1UL << 5);
      printf("%4.1f..%4.1f  %2d\n", bandStart, bandStop, addresses[i]);
      bandStart = bandStop;
    }
    printf("%4.1f..      %2d\n", bandStart, addresses[i]);
//  }
}

void displayLPFs(usb_dev_handle *handle) {
	unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
	int nFilters;
  
  nFilters = readFilters(handle, TRUE, FilterCrossOver, sizeof(FilterCrossOver));
	if (nFilters > 0) {
		displayLPFtable(handle, FilterCrossOver, nFilters);
	}
}

void displayLPFtable(usb_dev_handle *handle, unsigned short *FilterCrossOver, int nFilters) {
  //unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
  //int nFilters;
  
  //nFilters = readFilters(handle, TRUE, FilterCrossOver, length);
  
  //if (nFilters > 0) {
    unsigned char addresses[16];
    int nBytes;
    if ((nBytes = readLPFAddresses(handle, addresses)) < nFilters) {
      // failed to read anything - populate with default
      for (int i = 0; i < nFilters; i++) {
				addresses[i] = i;
      }
    }

    printf("  LPF Enabled: %d\n", FilterCrossOver[nFilters - 1]); 
    if (FilterCrossOver[nFilters - 1] == 0)
      return;
    printf("    Band    LPF\n");
    printf("----------  ---\n");
    int i;
    double mul, sub;
    double bandStart = 0.0;
    double bandStop;
    for (i = 0; i < nFilters - 1; i++) {
      bandStop =  (double) FilterCrossOver[i] / (1UL << 5);
      printf("%4.1f..%4.1f  %2d\n", bandStart, bandStop, addresses[i]);
      bandStart = bandStop;
    }
    printf("%4.1f..      %2d\n", bandStart, addresses[i]);
    
  //}
}

/*void displayBPFCrossovers(usb_dev_handle *handle) {
  unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
  int nFilters;
  
  nFilters = readFilters(handle, FALSE, FilterCrossOver, sizeof(FilterCrossOver));
  
  if (nFilters > 0) {
    printf("BPF Filter Bank:\n");
    int i;
    for (i = 0; i < nFilters - 1; i++) {
	    printf("  CrossOver[%d] = %f\n", i, (double) FilterCrossOver[i] / (1UL << 5));
    }
    printf("  BPF Enabled: %d\n", FilterCrossOver[nFilters - 1]); 

  }	
}

void displayLPFCrossovers(usb_dev_handle *handle) {
  
  unsigned short FilterCrossOver[16];        // allocate enough space for up to 16 filters
  int nFilters;
  
  nFilters = readFilters(handle, TRUE, FilterCrossOver, sizeof(FilterCrossOver));
  
  if (nFilters > 0) {
    printf("LPF Filter Bank:\n");
    int i;
    for (i = 0; i < nFilters - 1; i++) {
	    printf("  CrossOver[%d] = %f\n", i, (double) FilterCrossOver[i] / (1UL << 5));
    }
    printf("  LPF Enabled: %d\n", FilterCrossOver[nFilters - 1]); 

  }	
}*/

