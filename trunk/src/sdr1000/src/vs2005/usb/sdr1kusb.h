/**	\file sdr1kusb.h
 *	\author Sami Aintila OH2BFO
 */
/*	sdr1kusb.h - 20070111
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

#ifndef _sdr1kusb_h
#define _sdr1kusb_h

#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <stdio.h>

#include "usb.h"

//	these declared in windows.h:
//	typedef int BOOL;
//	typedef unsigned char BYTE;

//	------------------------------------------------------------

#define USB_TIMEOUT 100
#define USB_UPDATE_DEFAULT 40
#define USB_BUFSIZE 64
#define USB_BUFFULL 48
#define USB_MAX_DEVICES 8

//	------------------------------------------------------------

struct Sdr1kUsbHardware
{
	char* name;
	int VID, PID;
	int inEP, outEP, eepromSize;
	int* fwSize;
	int* fwAddr;
	BYTE** fwBytes;
};

//	------------------------------------------------------------

class Sdr1kUsb
{
protected:
	int usbStatus;

	BOOL rfeEnabled;
	BOOL adcEnabled;

	int outcount;
	BYTE outbuf[USB_BUFSIZE];
	BYTE inbuf[3];
	BYTE pstatus;

	BYTE extBits, bpfBits;
	BYTE ic11Bits, ic7Bits, ic10Bits, ic9Bits;

	int inEP, outEP;
	int usbCommand(BYTE cmd, BYTE data);

//	------------------------------------------------------------

	static int hwcount;
	static Sdr1kUsbHardware* hardware;

	static int devcount;
	static struct usb_device* device[USB_MAX_DEVICES];
	static Sdr1kUsbHardware* hw[USB_MAX_DEVICES];

//	------------------------------------------------------------

	usb_dev_handle* udev;

	usb_dev_handle* usbOpen(struct usb_device* dev) {
		if (udev) usb_close(udev);
		udev = usb_open(dev);
		return udev;
	}
	int usbClose() {
		int result = 0;
		if (udev) {
			result = usb_close(udev);
			udev = 0;
		}
		return result;
	}
	int usbSetConfiguration(int configuration) {
		return usb_set_configuration(udev, configuration);
	}
	int usbClaimInterface(int interface) {
		return usb_claim_interface(udev, interface);
	}
	int usbSetAltinterface(int alternate) {
		return usb_set_altinterface(udev, alternate);
	}
	int usbClearHalt(int ep) {
		return usb_clear_halt(udev, ep);
	}
	int usbBulkWrite(int ep, BYTE* bytes, int size) {
		return usb_bulk_write(udev, ep, (char*) bytes, size, USB_TIMEOUT);
	}
	int usbBulkRead(int ep, BYTE* bytes, int size) {
		return usb_bulk_read(udev, ep, (char*) bytes, size, USB_TIMEOUT);
	}
	int ezusbFirmwareDownload(int addr, BYTE* bytes, int size) {
		return usb_control_msg(udev, 0x40, 0xA0, addr, 0,
			(char*) bytes, size, USB_TIMEOUT);
	}

//	------------------------------------------------------------

	void usbThread();

	static void _run(void* ptr) {
		Sdr1kUsb* _this = (Sdr1kUsb*) ptr;
		_this->usbThread();
	}
	void startUsbThread() {
		_beginthread(_run, 0, this);
	}

	int usbUpdatePeriod;

	CRITICAL_SECTION critical;
	HANDLE inputEvent;
	HANDLE flushEvent;

	void lock() {
		EnterCriticalSection(&critical);
	}
	void unlock() {
		LeaveCriticalSection(&critical);
	}
	void waitUpdate() {		// call only within lock
		unlock();
		WaitForSingleObject(flushEvent, usbUpdatePeriod);
		lock();
	}
	void flush() {			// call only within lock
		SetEvent(flushEvent);
		while (outcount > 0 && usbStatus >= 0) {
			unlock();
			Sleep(1);
			lock();
		}
	}
	void signalInput() {
		if (inputEvent) SetEvent(inputEvent);
	}

//	------------------------------------------------------------

public:
	int Commit() {
		lock();
		if (usbStatus >= 0) flush();
		unlock();
		return usbStatus;
	}
	int SetUpdatePeriod(int msec) {
		lock();
		if (usbStatus >= 0) {
			int before = usbUpdatePeriod;
			usbUpdatePeriod = msec;
			if (usbUpdatePeriod < before) flush();
		}
		unlock();
		return usbStatus;
	}
	int SetNotify(HANDLE hEvent) {
		lock();
		if (usbStatus >= 0) {
			inputEvent = hEvent;
			if (inputEvent) {
				pstatus = 0xFF;
				flush();
			}
		}
		unlock();
		return usbStatus;
	}

//	------------------------------------------------------------

	int DDSWrite(BYTE addr, BYTE data) {
		return usbCommand(addr & 0x3F, data);
	}
	int DDSReset() {
		return usbCommand(0x40, 0);
	}

//	------------------------------------------------------------

	int LatchExt(BYTE data) {
		extBits = data;
		return usbCommand(0x81, data);
	}
	int LatchBpf(BYTE data) {
		if (rfeEnabled) data |= 0x24;
		bpfBits = data;
		return usbCommand(0x82, data);
	}

	enum { EXT = 0x01, BPF = 0x02 };
	int Latch(BYTE latch, BYTE data) {
		if (latch & EXT) LatchExt(data);
		if (latch & BPF) LatchBpf(data);
		latch &= 0x0C;
		if (latch) usbCommand(0x80 | latch, data);
		return usbStatus;
	}

//	------------------------------------------------------------

	int GetStatusPort() {
		lock();
		int result = (usbStatus >= 0) ? inbuf[0] : usbStatus;
		unlock();
		return result;
	}

//	------------------------------------------------------------

	int SRLoadIC11(BYTE data) {
		if (adcEnabled) data |= 0x20;
		ic11Bits = data;
		return usbCommand(0xC0, data);
	}
	int SRLoadIC7(BYTE data) {
		ic7Bits = data;
		return usbCommand(0xC8, data);
	}
	int SRLoadIC10(BYTE data) {
		ic10Bits = data;
		return usbCommand(0xD0, data);
	}
	int SRLoadIC9(BYTE data) {
		ic9Bits = data;
		return usbCommand(0xD8, data);
	}

	enum { IC11 = 0x00, IC7 = 0x08, IC10 = 0x10, IC9 = 0x18 };
	int SRLoad(BYTE reg, BYTE data) {
		if (reg == IC11) return SRLoadIC11(data);
		if (reg == IC7) return SRLoadIC7(data);
		if (reg == IC10) return SRLoadIC10(data);
		if (reg == IC9) return SRLoadIC9(data);
		return -1;
	}

//	------------------------------------------------------------

	int GetADC() {
		usbCommand(0xC7, ic11Bits);
		if (Commit() < 0) return usbStatus;
		return inbuf[1] | (inbuf[2] << 8);
	}

//	------------------------------------------------------------

	static int GetNumDevs();

	int Open(BOOL rfe, BOOL adc, int select = -1);

	int Close() {
		lock();
		usbStatus = -1;
		flush();
		unlock();
		Sleep(USB_TIMEOUT);
		return usbClose();
	}

	Sdr1kUsb() {
		udev = 0;
		usbStatus = -1;
		InitializeCriticalSection(&critical);
		flushEvent = CreateEvent(0, 0, 0, 0);
	}
	~Sdr1kUsb() {
		Close();
		DeleteCriticalSection(&critical);
		CloseHandle(flushEvent);
	}
};

//	------------------------------------------------------------

#endif //_sdr1kusb_h
