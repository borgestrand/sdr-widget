/*	sdr1kusb.cpp - 20070111
 *	________________________________________________________________
 *	
 *	Sdr1kUsb - SDR-1000 USB hardware control
 *	Written by Sami Aintila (OH2BFO), Attocon Oy
 *	________________________________________________________________
 *	
 *	Copyright (c) 2006-2007 Attocon Oy.
 *	All rights reserved.
 *	
 *	Redistribution and use in source and binary forms, with or 
 *	without modification, are permitted provided that the following 
 *	conditions are met:
 *	1. Redistributions of source code must retain the above 
 *	copyright notice, this list of conditions and the following 
 *	disclaimer. 
 *	2. Redistributions in binary form must reproduce the above 
 *	copyright notice, this list of conditions and the following 
 *	disclaimer in the documentation and/or other materials provided 
 *	with the distribution. 
 *	3. Neither the name of the copyright holders nor the names of 
 *	their contributors may be used to endorse or promote products 
 *	derived from this software without specific prior written 
 *	permission. 
 *	
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
 *	CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 *	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 *	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 *	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR 
 *	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 *	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
 *	EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sdr1kusb.h"

#define SDR1KUSB_INFO

//	------------------------------------------------------------

int Sdr1kUsb::devcount = -1;
struct usb_device* Sdr1kUsb::device[USB_MAX_DEVICES];
Sdr1kUsbHardware* Sdr1kUsb::hw[USB_MAX_DEVICES];

//	------------------------------------------------------------

int Sdr1kUsb::GetNumDevs()
{
	if (devcount < 0) {
		usb_init();
		devcount = 0;
	}
	int changes = usb_find_busses();
	changes += usb_find_devices();
	if (changes == 0) return devcount;
	devcount = 0;
		
	struct usb_bus* bus = usb_get_busses();
	while (bus) {
		struct usb_device* dev = bus->devices;
		while (dev) {
			if (devcount == USB_MAX_DEVICES) break;
			int VID = dev->descriptor.idVendor;
			int PID = dev->descriptor.idProduct;
			for (int i = 0; i < hwcount; i++) {
				if (VID == hardware[i].VID && PID == hardware[i].PID) {
					device[devcount] = dev;
					hw[devcount] = &hardware[i];
					devcount++;
					break;
				}
			}
			dev = dev->next;
		}
		bus = bus->next;
	}
#ifdef SDR1KUSB_INFO
	printf("Sdr1kUsb::GetNumDevs = %d\n", devcount);
#endif
	return devcount;
}

//	------------------------------------------------------------

int Sdr1kUsb::Open(BOOL rfe, BOOL adc, int select)
{
	if (usbStatus >= 0) Close();
	GetNumDevs();
	int idx = select;
	if (select < 0) idx = 0;
	while (idx < devcount) {
		if (usbOpen(device[idx])) {
			if (usbSetConfiguration(1) == 0
				&& usbClaimInterface(0) == 0
				&& usbSetAltinterface(1) == 0)
			{
				usbStatus = 0;
				break;
			}
			usbClose();
		}
		if (select < 0) idx++;
		else return usbStatus;
	}
	if (usbStatus < 0) return usbStatus;

#ifdef SDR1KUSB_INFO
	printf("device %d: %s\n", idx, device[idx]->filename);
	printf("hardware: %s\n", hw[idx]->name);
#endif

	inEP = hw[idx]->inEP;
	outEP = hw[idx]->outEP;
	usbClearHalt(inEP);
	usbClearHalt(outEP);

	int* fwSize = hw[idx]->fwSize;
	int* fwAddr = hw[idx]->fwAddr;
	BYTE** fwBytes = hw[idx]->fwBytes;
	for (int i = 0; fwSize[i] > 0; i++) {
		ezusbFirmwareDownload(fwAddr[i], fwBytes[i], fwSize[i]);
	}

	int eepromSize = hw[idx]->eepromSize;
	if (eepromSize == 16) {
		BYTE eeprom[16];
		usbBulkRead(inEP, eeprom, 16);
#ifdef SDR1KUSB_INFO
		printf("EEPROM:");
		for (int i = 0; i < 16; i++) {
			printf(" %02X", eeprom[i]);
		}
		printf("\n");
#endif
	}

	inputEvent = 0;
	usbUpdatePeriod = USB_UPDATE_DEFAULT;

	outcount = 0;
	startUsbThread();

	Latch(0x0F, 0);
	DDSReset();

	rfeEnabled = rfe;
	if (rfeEnabled) {
		LatchBpf(0);	// bpfBits |= 0x24
		adcEnabled = adc;
		SRLoadIC11(0);
		SRLoadIC7(0);
		SRLoadIC10(0);
		SRLoadIC9(0);
	}
	else adcEnabled = 0;

	if (Commit() < 0) {
		Close();
		return usbStatus;
	}
	return idx;
}

//	------------------------------------------------------------

int Sdr1kUsb::usbCommand(BYTE cmd, BYTE data)
{
	lock();
	if (usbStatus >= 0) {
		outbuf[outcount++] = data;
		outbuf[outcount++] = cmd;
		if (outcount >= USB_BUFFULL) flush();
	}
	unlock();
	return usbStatus;
}

//	------------------------------------------------------------

void Sdr1kUsb::usbThread()
{
	lock();
	while (usbStatus >= 0) {
		waitUpdate();
		if (usbStatus >= 0) {
			usbStatus = usbBulkWrite(outEP, outbuf, outcount);
			outcount = 0;
			if (usbStatus >= 0) {
				usbStatus = usbBulkRead(inEP, inbuf, 3);
				if (inbuf[0] != pstatus) {
					pstatus = inbuf[0];
					signalInput();
				}
			}
		}
	}
	signalInput();
	unlock();
}

//	------------------------------------------------------------
