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
#include <getopt.h>

/* Needed to compile on older FC3 systems */
#if defined __linux
#include <sys/types.h>
#include <linux/limits.h>
#endif

#include <math.h>
#include <usb.h>    /* this is libusb, see http://libusb.sourceforge.net/ */

#include "operations.h"

extern int verbose;

extern int major;
extern int minor;

extern int i2cAddress;
extern double fXtall;
extern double startupFreq;
extern double multiplier;

int HS_DIV_MAP[] = {4,5,6,7,-1,9,-1,11};

#define USB_SUCCESS		    0
#define USB_ERROR_NOTFOUND  1
#define USB_ERROR_ACCESS    2
#define USB_ERROR_IO        3

char    serialNumberString[256];

int  usbGetStringAscii(usb_dev_handle *dev, int index, int langid, char *buf, int buflen)
{
  char    buffer[256];
  int     rval, i;

  if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, buffer, sizeof(buffer), 1000)) < 0)
    return rval;
  if(buffer[1] != USB_DT_STRING)
    return 0;
  if((unsigned char)buffer[0] < rval)
    rval = (unsigned char)buffer[0];
  rval /= 2;
  /* lossy conversion to ISO Latin1 */
  for(i=1;i<rval;i++){
    if(i > buflen)  /* destination buffer overflow */
      break;
    buf[i-1] = buffer[2 * i];
    if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
      buf[i-1] = '?';
  }
  buf[i-1] = 0;
  return i-1;
}

unsigned char usbOpenDevice(usb_dev_handle **device, int vendor, char *vendorName, int product, char *productName, char *usbSerialID)
{
  struct usb_bus      *bus;
  struct usb_device   *dev;
  usb_dev_handle      *handle = NULL;
  unsigned char       errorCode = USB_ERROR_NOTFOUND;
  static int          didUsbInit = 0;

  if(!didUsbInit){
    didUsbInit = 1;
    usb_init();
  }
  usb_find_busses();
  usb_find_devices();
  for(bus=usb_get_busses(); bus; bus=bus->next){
    for(dev=bus->devices; dev; dev=dev->next){
      if(dev->descriptor.idVendor == vendor && dev->descriptor.idProduct == product){
        char    string[256];
        int     len;
        handle = usb_open(dev); /* we need to open the device in order to query strings */
        if(!handle){
          errorCode = USB_ERROR_ACCESS;
          fprintf(stderr, "Warning: cannot open USB device: %s\n", usb_strerror());
          continue;
        }
        if(vendorName == NULL && productName == NULL){  /* name does not matter */
          break;
        }
        /* now check whether the names match: */
        len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
        if(len < 0){
          errorCode = USB_ERROR_IO;
          fprintf(stderr, "Warning: cannot query manufacturer for device: %s\n", usb_strerror());
        }else{
          errorCode = USB_ERROR_NOTFOUND;
           //fprintf(stderr, "seen device from vendor ->%s<-\n", string); 
          if(strcmp(string, vendorName) == 0){
            len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
            if(len < 0){
              errorCode = USB_ERROR_IO;
              fprintf(stderr, "Warning: cannot query product for device: %s\n", usb_strerror());
            }else{
              errorCode = USB_ERROR_NOTFOUND;
               //fprintf(stderr, "seen product ->%s<-\n", string); 
              if(strcmp(string, productName) == 0) {
		len = usbGetStringAscii(handle, dev->descriptor.iSerialNumber, 0x0409, serialNumberString, sizeof(serialNumberString));
		if (len < 0) {
		  errorCode = USB_ERROR_IO;
		  fprintf(stderr, "Warning: cannot query serial number for device: %s\n", usb_strerror());
		}else{
		  errorCode = USB_ERROR_NOTFOUND;
		  if ((usbSerialID == NULL) || (strcmp(serialNumberString, usbSerialID) == 0)) {
		    break;
		  }
		}
	      }
            }
          }
        }
        usb_close(handle);
        handle = NULL;
      }
    }
    if(handle)
      break;
  }
  if(handle != NULL){
    errorCode = USB_SUCCESS;
    *device = handle;
  }
  return errorCode;
}

void usbClose(usb_dev_handle * handle)
{
}
void printBuffer(char * buffer, int length)
{
	int i;
	for (i=0; i < length; i++) {
		printf("buffer[%d] = %X\n", i, (unsigned char) buffer[i]);
	}
}

double calculateFrequency(unsigned char * buffer) {
	int RFREQ_int = ((buffer[2] & 0xf0) >> 4) + ((buffer[1] & 0x3f) * 16);
	int RFREQ_frac = (256 * 256 * 256 * (buffer[2] & 0xf)) + (256 * 256 * buffer[3]) + (256 * buffer[4]) + (buffer[5]);
	double RFREQ = RFREQ_int + (RFREQ_frac / 268435456.0);
	int N1 = ((buffer[1] & 0xc0 ) >> 6) + ((buffer[0] & 0x1f) * 4);
	int HS_DIV = (buffer[0] & 0xE0) >> 5;
	double fout = fXtall * RFREQ / ((N1 + 1) * HS_DIV_MAP[HS_DIV]);
	
	if (verbose >= 2) {
	   printf("RFREQ = %f\n", RFREQ);
	   printf("N1 = %d\n", N1);
	   printf("HS_DIV = %d\n", HS_DIV);
	   printf("nHS_DIV = %d\n", HS_DIV_MAP[HS_DIV]);
	   printf("fout = %f\n", fout);
	}

	return fout;
}

unsigned short readVersion(usb_dev_handle *handle) {
	//unsigned char buffer[2];
	unsigned short version;
	int nBytes;

	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_VERSION, 0x0E00, 0, (char *) &version, sizeof(version), 500);

	if (nBytes == 2) {
		printf("Version     : %d.%d\n", (version & 0xFF00) >> 8, version & 0xFF);
		return version;
	} else {
		printf("Version     : UNKNOWN\n");
		return 0;
	}
}

double readFrequencyByValue(usb_dev_handle *handle) {
	unsigned int iFreq;
	int nBytes;
    
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_FREQUENCY, 0, 0, (char *) &iFreq, sizeof(iFreq), 500);

	if (nBytes == 4) {
		double dFreq;		
        	dFreq = (double)iFreq / (1UL<<21);
		return dFreq;
	}
	return 0.0;

}

void getRegisters(usb_dev_handle *handle) {
	unsigned char buffer[6];
	int nBytes;

	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_REGISTERS, SI570_I2C_ADDR, 0, (char *)buffer, sizeof(buffer), 5000);

	if (nBytes > 0) {
		int i;
		for (i=0; i < 6; i++) {
			printf("Register %d = %X (%d)\n", i + 7,  (unsigned char) buffer[i], (unsigned char) buffer[i]);
		}
	}
}

double getFrequency(usb_dev_handle *handle) {
	unsigned char buffer[6];
	int nBytes;

	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_READ_REGISTERS, SI570_I2C_ADDR, 0, (char *)buffer, sizeof(buffer), 5000);

	if (nBytes > 0) {
		int i;
		if (verbose >= 2) {
			for (i=0; i < 6; i++) {
				printf("Register %d = %X (%d)\n", i + 7,  (unsigned char) buffer[i], (unsigned char) buffer[i]);
			}
		}
		return  calculateFrequency(buffer);
		//printf("Frequency: %f MHz\n", calculateFrequency(buffer) / multiplier);
	}
	return 0.0;
}

void setPTT(usb_dev_handle *handle, int value) {
	char buffer[3];

	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	
	usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_SET_PTT, value, 0, (char *)buffer, sizeof(buffer), 5000);
}

int calcDividers(double f, struct solution* solution)
{
	struct solution sols[8]; 
	int i;
	double y;
	int imin;
	double fmin;
	
	// Count down through the dividers
	for (i=7;i >= 0;i--) {
		
		if (HS_DIV_MAP[i] > 0) {
			sols[i].HS_DIV = i;
			y = (SI570_DCO_HIGH + SI570_DCO_LOW) / (2 * f);
			y = y / HS_DIV_MAP[i];
			if (y < 1.5) {
				y = 1.0;
			} else {
				y = 2 * round ( y / 2.0);
			}
			if (y > 128) {
				y = 128;
			}
			sols[i].N1 = trunc(y) - 1;
			sols[i].f0 = f * y * HS_DIV_MAP[i];
		} else {
			sols[i].f0 = 10000000000000000.0;
		}
	}
	imin = -1;
	fmin = 10000000000000000.0;
		
	for (i=0; i < 8; i++) {
		if ((sols[i].f0 >= SI570_DCO_LOW) && (sols[i].f0 <= SI570_DCO_HIGH)) {
			if (sols[i].f0 < fmin) {
				fmin = sols[i].f0;
				imin = i;
			}
		}
	}
		
	if (imin >=0) {
		solution->HS_DIV = sols[imin].HS_DIV;
		solution->N1 = sols[imin].N1;
		solution->f0 = sols[imin].f0;
		solution->RFREQ = sols[imin].f0 / fXtall;

		if (verbose >= 2) {
			printf("solution->HS_DIV = %d\n", solution->HS_DIV);
			printf("solution->N1 = %d\n", solution->N1);
			printf("solution->f0 = %f\n", solution->f0);
			printf("solution->RFREQ = %f\n", solution->RFREQ);
		}	

		return 1;
	} else {
		return 0;
	}
}

void setLongWord( int value, char * bytes)
{
	bytes[0] = value & 0xff;
	bytes[1] = ((value & 0xff00) >> 8) & 0xff;
	bytes[2] = ((value & 0xff0000) >> 16) & 0xff;
	bytes[3] = ((value & 0xff000000) >> 24) & 0xff;
} 

void setFrequency(usb_dev_handle * handle, double frequency)
{
	
	char buffer[6];
	int request = REQUEST_SET_FREQ;
	int value = 0x700 + i2cAddress;
	int index = 0;
	double f = frequency * multiplier;
	if (verbose)
		printf("Setting Si570 Frequency by registers to: %f\n", f);
	
	struct solution theSolution;
	calcDividers(f, &theSolution); 
	
	int RFREQ_int = trunc(theSolution.RFREQ);
	int RFREQ_frac = round((theSolution.RFREQ - RFREQ_int)*268435456);
	unsigned char fracBuffer[4];
	unsigned char intBuffer[4];
	setLongWord(RFREQ_int, (char *) intBuffer);
	setLongWord(RFREQ_frac, (char *) fracBuffer);
	
	buffer[5] = fracBuffer[0];
	buffer[4] = fracBuffer[1];
	buffer[3] = fracBuffer[2];
	buffer[2] = fracBuffer[3];
	buffer[2] = buffer[2] | ((intBuffer[0] & 0xf) << 4);
	buffer[1] = RFREQ_int / 16;
	buffer[1] = buffer[1] + ((theSolution.N1 & 3) << 6);
	buffer[0] = theSolution.N1 / 4;
	buffer[0] = buffer[0] + (theSolution.HS_DIV << 5);
	
	if (usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, request, value, index, buffer, sizeof(buffer), 5000)) {
		if (verbose >= 2) printBuffer(buffer, 2);
		
	} else {
		fprintf(stderr, "Failed writing frequency to device\n");
	}
}

void setFreqByValue(usb_dev_handle * handle, double frequency)
{	
	char buffer[4];
	int request = REQUEST_SET_FREQ_BY_VALUE;
	int value = 0x700 + i2cAddress;
	int index = 0;
	
	double f = multiplier * frequency;
    
	setLongWord(round(f * 2097152.0), buffer);
	if (verbose) {
		printf("Setting Si570 Frequency by value to: %f\n", f);
        if (verbose >= 2)
		  printBuffer(buffer,4);
	}

	if (usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, request, value, index, buffer, sizeof(buffer), 5000)) {
		if (verbose >= 2) printBuffer(buffer, 2);
		
	} else {
		fprintf(stderr, "Failed setting frequency");
	}
}
