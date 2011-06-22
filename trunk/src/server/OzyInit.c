/**
* @file OzyInit.c
* @brief Manage the Ozy initialization on Windows
* @author Dave McQuate, WA8YWQ; most code from USRP HPSDR
* @version 0.1
* @date 2010-06-24
*/

/* Copyright (C)
* 2010 - Dave McQuate, WA8YWQ
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <stdio.h>
#include <string.h>

#ifndef __linux__
#include "usb.h"
#include "hpsdr_usb.h"

#define bool int
#define true 1
#define false 0
#endif

static struct usb_dev_handle *udh;
static struct usb_device *dev;

#define OZY_IO_TIMEOUT 2000

/**
 * Write a block to USB
 * \param ep The USB endpoint
 * \param buffer Pointer to bytes to be written
 * \param buffer_size Integer number of bytes to be written
 */
int ozy_write(int ep,unsigned char* buffer,int buffer_size) 
{
    int rc;

    rc = usb_bulk_write(udh,(unsigned char)ep, (char *)buffer,buffer_size,OZY_IO_TIMEOUT);
    return rc;
}

/**
 * Read a block from USB
 * \param ep The USB endpoint
 * \param buffer Pointer to read buffer
 * \param buffer_size Integer size of buffer in bytes
 * \return negative if error; positive indicates number of bytes read
 */

int ozy_read(int ep,unsigned char* buffer,int buffer_size) 
{
    int rc;

    rc = usb_bulk_read(udh,(unsigned char)ep, (char *)buffer,buffer_size,OZY_IO_TIMEOUT);
    return rc;
}

static const char *default_firmware_filename = "ozyfw-sdr1k.hex";
static const char *default_fpga_filename     = "Ozy_Janus.rbf";

void
hpsdr_one_time_init ()
{
  static bool first = true;

  if (first){
    first = false;
    usb_init ();			// usb library init
    usb_find_busses ();
    usb_find_devices ();
  }
}


struct usb_device *
hpsdr_find_device ()
{
  struct usb_bus *p;
  struct usb_device *q;
  hpsdr_one_time_init ();
  
  
  p = usb_get_busses();
  while (p != NULL){
    q = p->devices;
    while (q != NULL){
      if ( (q->descriptor.idVendor == USB_VID_FSF) 
	&&   (q->descriptor.idProduct == USB_PID_FSF_HPSDR_HA) )
	{
	  return q;
      }
      q = q->next;
    }
    p = p->next;
  }
  return NULL;	// not found
}

// ----------------------------------------------------------------
// Danger, big, fragile KLUDGE.  The problem is that we want to be
// able to get from a usb_dev_handle back to a usb_device, and the
// right way to do this is buried in a non-installed include file.
// --Not used in hpsdr server

static struct usb_device *
dev_handle_to_dev (usb_dev_handle *udh)
{
  struct usb_dev_handle_kludge {
    int			 fd;
    struct usb_bus	*bus;
    struct usb_device	*device;
  };

  return ((struct usb_dev_handle_kludge *) udh)->device;
}

static struct usb_dev_handle *
hpsdr_open_interface (struct usb_device *dev, int interface, int altinterface)
{	int rc;
  struct usb_dev_handle *udh;
  
  for  ( ; (udh = usb_open(dev)) == NULL; Sleep(50) )		
  {	printf("%s\n", "Can't open USB");
  }
  if (udh == 0)
    return NULL;


// Take a simple approach, without checking--

	rc = usb_set_configuration(udh, 1);
	rc = usb_claim_interface(udh, 0);
	rc = usb_set_altinterface(udh, 0);
	// the following will fail before Ozy firmware has been loaded
	rc = usb_clear_halt(udh, 0x86);		// #### clear halt only if EP6 is present
//	rc = usb_clear_halt(udh, 0x84);		// #### clear halt only if EP4 is present
	rc = usb_clear_halt(udh, 2);

  return udh;
}


bool
hpsdr_close_interface (struct usb_dev_handle *udh)
{
  // we're assuming that closing an interface automatically releases it.
  return usb_close (udh) == 0;
}

// write internal ram using Cypress vendor extension

#define MAX_EP0_PKTSIZE 64

static bool
write_internal_ram (struct usb_dev_handle *udh, unsigned char *buf,
		    int start_addr, size_t len)
{
  int addr;
  int n;
  int a;
  int quanta = MAX_EP0_PKTSIZE;

  for (addr = start_addr; addr < start_addr + (int) len; addr += quanta){
    n = len + start_addr - addr;
    if (n > quanta)
      n = quanta;

    a = usb_control_msg (udh, 0x40, 0xA0,
			 addr, 0, (char *)(buf + (addr - start_addr)), n, 1000);

    if (a < 0){
      fprintf(stderr,"write_internal_ram failed: %s\n", usb_strerror());
      return false;
    }
  }
  return true;
}

static bool
reset_cpu (struct usb_dev_handle *udh, bool reset_p)
{
  unsigned char v;

  if (reset_p)
    v = 1;		// hold processor in reset
  else
    v = 0;	        // release reset

  return write_internal_ram (udh, &v, 0xE600, 1);
}

// ----------------------------------------------------------------
// Load intel format file into cypress FX2 (8051)

static bool
hpsdr_load_firmware (struct usb_dev_handle *udh, const char *filename)
{
  char s[1024];
  int length;
  int addr;
  int type;
  unsigned char data[256];
  unsigned char checksum, a;
  unsigned int b;
  int i;

  FILE	*f = fopen (filename, "r");		// #### changed mode from "ra" to "r"
  if (f == 0){
    perror (filename);
    return false;
  }

  if (!reset_cpu (udh, true))	// hold CPU in reset while loading firmware
    goto fail;

  while (!feof(f)){
    fgets(s, sizeof (s), f); /* we should not use more than 263 bytes normally */
    if(s[0]!=':'){
      fprintf(stderr,"%s: invalid line: \"%s\"\n", filename, s);
      goto fail;
    }
    sscanf(s+1, "%02x", &length);
    sscanf(s+3, "%04x", &addr);
    sscanf(s+7, "%02x", &type);

    if(type==0){

      a=length+(addr &0xff)+(addr>>8)+type;
      for(i=0;i<length;i++){
	sscanf (s+9+i*2,"%02x", &b);
	data[i]=b;
	a += data[i];
      }

      sscanf (s+9+length*2,"%02x", &b);
      checksum=b;
      if (((a+checksum)&0xff)!=0x00){
	fprintf (stderr, "  ** Checksum failed: got 0x%02x versus 0x%02x\n", (-a)&0xff, checksum);
	goto fail;
      }
      if (!write_internal_ram (udh, data, addr, length))
	goto fail;
    }
    else if (type == 0x01){      // EOF
      break;
    }
    else if (type == 0x02){
      fprintf(stderr, "Extended address: whatever I do with it?\n");
      fprintf (stderr, "%s: invalid line: \"%s\"\n", filename, s);
      goto fail;
    }
  }

  if (!reset_cpu (udh, false))		// take CPU out of reset
    goto fail;

  fclose (f);
  return true;

 fail:
  fclose (f);
  return false;
}

static int
write_cmd (struct usb_dev_handle *udh,
	   int request, int value, int index,
	   unsigned char *bytes, int len)
{
  int	requesttype = (request & 0x80) ? VRT_VENDOR_IN : VRT_VENDOR_OUT;

  int r = usb_control_msg (udh, requesttype, request, value, index,
			   (char *) bytes, len, 1000);
#if 0
  if (r < 0){
    // we get EPIPE if the firmware stalls the endpoint.
    if (errno != EPIPE)
      fprintf (stderr, "usb_control_msg failed: %s\n", usb_strerror ());
  }
#endif

  return r;
}

#if 0
// These are not needed for the server
bool 
hpsdr_set_fpga_reset (struct usb_dev_handle *udh, bool on)
{
  return hpsdr_set_switch (udh, VRQ_FPGA_SET_RESET, on);
}

static bool
hpsdr_set_switch (struct usb_dev_handle *udh, int cmd_byte, bool on)
{
  return write_cmd (udh, cmd_byte, on, 0, 0, 0) == 0;
}
#endif

// ----------------------------------------------------------------
// load fpga

static bool
hpsdr_load_fpga (struct usb_dev_handle *udh, const char *filename)
{
  bool ok = true;


  unsigned char buf[MAX_EP0_PKTSIZE];	// 64 is max size of EP0 packet on FX2
  int n;

  FILE	*fp = fopen (filename, "rb");
  if (fp == 0){
    perror (filename);
    return false;
  }

    if (write_cmd (udh, VRQ_FPGA_LOAD, 0, FL_BEGIN, 0, 0) != 0)
    goto fail;
  
  while ((n = fread (buf, 1, sizeof (buf), fp)) > 0){
    if (write_cmd (udh, VRQ_FPGA_LOAD, 0, FL_XFER, buf, n) != n)
      goto fail;
//	  Sleep(1);
  }

  if (write_cmd (udh, VRQ_FPGA_LOAD, 0, FL_END, 0, 0) != 0)
    goto fail;
  
  fclose (fp);

  return true;

 fail:
  fclose (fp);
  return false;
}



#define	VRQ_SDR1K_CTL 0x0d
#define	SDR1KCTRL_READ_VERSION 0x7
#define	VRT_VENDOR_IN 0xC0

#define BUF_LEN 100
char firmware_string_buf[BUF_LEN];

char *get_ozy_firmware_string(struct usb_dev_handle *udh)
{
	int rc = usb_control_msg(udh, VRT_VENDOR_IN, VRQ_SDR1K_CTL, SDR1KCTRL_READ_VERSION, 0, firmware_string_buf, BUF_LEN, 1000);

	if (rc == 8)
	{	firmware_string_buf[8] = '\0';
		return firmware_string_buf;
	}
	return NULL;
}

/** 
 * Open USB, find hpsdr, open the interface, load Ozy FX2 code and FPGA contents
 */
bool init_hpsdr()
{	bool rc;
	char * fw_string;

	dev = hpsdr_find_device();
	if (dev == NULL) return false;

	udh = hpsdr_open_interface (dev, 0, 0);
	if (udh == NULL) return false;

	fw_string = get_ozy_firmware_string(udh);
	if (fw_string == NULL)
	{	rc = hpsdr_load_firmware (udh, default_firmware_filename);
		if (rc == false) return false;

		hpsdr_close_interface (udh);	// allow for  re-numeration
		Sleep(2000);

		dev = hpsdr_find_device();
		if (dev == NULL) return false;
		udh = hpsdr_open_interface (dev, 0, 0);
		if (udh == NULL) return false;

		rc = hpsdr_load_fpga (udh, default_fpga_filename);
		if (rc == false) return false;

		hpsdr_close_interface (udh);
		Sleep(1000);

		dev = hpsdr_find_device();
		if (dev == NULL) return false;

		udh = hpsdr_open_interface (dev, 0, 0);
		if (udh == NULL) return false;

	}
	return true;
}



