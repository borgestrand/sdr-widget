/**
* @file  hpsdr_usb.h
* @brief Define variables for Ozy initialization
* @author Dave McMcQuate, WA8YWQ
* @version 0.1
* @date 2010-06-24
*/

// hpsdr_usb.h

#define	USB_VID_FSF			0xfffe	  // Free Software Folks
#define USB_PID_FSF_HPSDR_HA    	0x0007    // High Performance Software Defined Radio (Host Assisted Boot)

#define	VRQ_FPGA_LOAD			0x02
#  define	FL_BEGIN			0	// wIndexL: begin fpga programming cycle.  stalls if trouble.
#  define	FL_XFER				1	// wIndexL: xfer up to 64 bytes of data
#  define	FL_END				2	// wIndexL: end programming cycle, check for success.
							//          stalls endpoint if trouble.
#define	VRT_VENDOR_IN			0xC0
#define	VRT_VENDOR_OUT			0x40
