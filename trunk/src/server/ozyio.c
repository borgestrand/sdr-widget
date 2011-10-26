/**
* @file ozyio.c
* @brief USB I/O with Ozy
* @author John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/


/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
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
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

#include "ozyio.h"

#define OZY_PID (0x0007)
#define OZY_VID (0xfffe)

#define VRQ_SDR1K_CTL 0x0d
#define SDR1KCTRL_READ_VERSION  0x7
#define VRT_VENDOR_IN 0xC0

#define VENDOR_REQ_TYPE_IN 0xc0
#define VENDOR_REQ_TYPE_OUT 0x40

#define VENDOR_REQ_SET_LED 0x01
#define VENDOR_REQ_FPGA_LOAD 0x02

#define FL_BEGIN 0
#define FL_XFER 1
#define FL_END 2

#define OZY_IO_TIMEOUT 500
#define MAX_EPO_PACKET_SIZE 64

static int init=0;

static libusb_device_handle* ozy_handle;
static libusb_context* context;

int ozy_open(void) {
    int rc;

    if(init==0) {
        rc=libusb_init(NULL);
        if(rc<0) {
            fprintf(stderr,"libusb_init failed: %d\n",rc);
            return rc;
        }
        init=1;
    }

    ozy_handle=libusb_open_device_with_vid_pid(NULL, OZY_VID, OZY_PID);
    if(ozy_handle==NULL) {
        fprintf(stderr,"libusbio: cannot find ozy device\n");
        return -1;
    }

    rc=libusb_detach_kernel_driver(ozy_handle,0);
    if(rc<0) {
//        fprintf(stderr,"libusb_detach_kernel_driver failed: %d\n",rc);
    }

    rc=libusb_claim_interface(ozy_handle,0);
    if(rc<0) {
        fprintf(stderr,"libusb_claim_interface failed: %d\n",rc);
        return rc;
    }

    return 0;
}

int ozy_close() {
    int rc;

    rc=libusb_attach_kernel_driver(ozy_handle,0);
    if(rc<0) {
//        fprintf(stderr,"libusb_attach_kernel_driver failed: %d\n",rc);
    }

    libusb_close(ozy_handle);
}

int ozy_get_firmware_string(char* buffer,int buffer_size) {
    int rc;

    rc=libusb_control_transfer(ozy_handle, VRT_VENDOR_IN, VRQ_SDR1K_CTL, SDR1KCTRL_READ_VERSION, 0, buffer, buffer_size, OZY_IO_TIMEOUT);
    if(rc<0) {
        fprintf(stderr,"ozy__get_firmware_string failed: %d\n",rc);
        return rc;
    }
    buffer[rc]='\0';

    return 0;
}

int ozy_write(int ep,unsigned char* buffer,int buffer_size) {
    int rc;
    int bytes;

    rc = libusb_bulk_transfer(ozy_handle,(unsigned char)ep,buffer,buffer_size,&bytes,OZY_IO_TIMEOUT);
    if(rc==0) {
        rc=bytes;
    }

    return rc;
}

int ozy_read(int ep,unsigned char* buffer,int buffer_size) {
    int rc;
    int bytes;

    rc = libusb_bulk_transfer(ozy_handle,(unsigned char)ep,buffer,buffer_size,&bytes,OZY_IO_TIMEOUT);
    if(rc==0) {
        rc=bytes;
    }

    return rc;
}

int ozy_write_ram(int fx2_start_addr, char *bufp, int count) {
        int pkt_size = MAX_EPO_PACKET_SIZE;
        int len = count;
        int bytes_written = 0;
        int addr;
        int bytes_written_this_write;
        int nsize;

        for ( addr = fx2_start_addr; addr < fx2_start_addr + len; addr += pkt_size, bufp += pkt_size ) {
                nsize = len + fx2_start_addr - addr;
                if ( nsize > pkt_size ) nsize = pkt_size;
                bytes_written_this_write = libusb_control_transfer(ozy_handle, 0x40, 0xa0, addr, 0, bufp, nsize, OZY_IO_TIMEOUT);
                if ( bytes_written_this_write >= 0  ) {
                        bytes_written += bytes_written_this_write;
                }
                else {
                        return bytes_written_this_write;
                }
        }
        return bytes_written;
}

int ozy_reset_cpu(int reset) {
        char write_buf;

        if ( reset ) write_buf = 1;
        else write_buf = 0;

        if ( ozy_write_ram(0xe600, &write_buf, 1) != 1 ) return 0;
        else return 1;

}

static unsigned int hexitToUInt(char c) {
        c = tolower(c);
        if ( c >= '0' && c <= '9' ) {
                return c - '0';
        }
        else if ( c >= 'a' && c <= 'f' ) {
                return 10 + (c - 'a');
        }
        return 0;
}

static int ishexit(c) {
        c = tolower(c);
        if ( c >= '0' && c <= '9' ) return 1;
        if ( c >= 'a' && c <= 'f' ) return 1;
        return 0;
}

static int hexitsToUInt(char *p, int count) {
        unsigned int result = 0;
        int i;
        char c;
        unsigned int this_hex;
        for ( i = 0; i < count; i++ ) {
                c = *p;
                ++p;
                if ( !ishexit(c) ) {
                        return -1;
                }
                this_hex = hexitToUInt(c);
                result *= 16;
                result += this_hex;
        }
        return result;
}

int ozy_load_firmware(char *fnamep) {
        FILE *ifile;
        int linecount = 0;
        int length;
        int addr;
        int type;
        char readbuf[1030];
        char wbuf[256];
        unsigned char my_cksum;
        unsigned char cksum;
        int this_val;
        int i;

fprintf(stderr,"loading ozy firmware: %s\n",fnamep);

        ifile = fopen(fnamep, "r");
        if ( ifile == NULL ) {
                fprintf(stderr, "Could not open: \'%s\'\n", fnamep);
                return 0;
        }

        while (  fgets(readbuf, sizeof(readbuf), ifile) != NULL ) {
                ++linecount;
                if ( readbuf[0] != ':' ) {
                        fprintf(stderr, "ozy_upload_firmware: bad record\n");
                        return 0;
                }
                length = hexitsToUInt(readbuf+1, 2);
                addr = hexitsToUInt(readbuf+3, 4);
                type = hexitsToUInt(readbuf+7, 2);
                if ( length < 0 || addr < 0 || type < 0 ) {
                        fprintf(stderr, "ozy_upload_firmware: bad length, addr or type\n");
                        return 0;
                }
                switch ( type ) {
                        case 0: /* record */
                                my_cksum = (unsigned char)(length + (addr & 0xff) + (addr >>8 + type));
                                for ( i = 0; i < length; i++ ) {
                                        this_val = hexitsToUInt(readbuf+9+(i*2),2);
#if 0
                                        printf("i: %d val: 0x%02x\n", i, this_val);
#endif

                                        if ( this_val < 0 ) {
                                                fprintf(stderr, "ozy_upload_firmware: bad record data\n");
                                                return 0;
                                        }
                                        wbuf[i] = (unsigned char)this_val;
                                        my_cksum += wbuf[i];
                                }

                                this_val = hexitsToUInt(readbuf+9+(length*2),2);
                                if ( this_val < 0 ) {
                                        fprintf(stderr, "ozy_upload_firmware: bad checksum data\n");
                                        return 0;
                                }
                                cksum = (unsigned char)this_val;
#if 0
                                printf("\n%s", readbuf);
                                printf("len: %d (0x%02x) addr: 0x%04x mychk: 0x%02x chk: 0x%02x",
                                                length, length, addr, my_cksum, cksum);
#endif

                                if ( ((cksum + my_cksum) & 0xff) != 0 ) {
                                        fprintf(stderr, "ozy_upload_firmware: bad checksum\n");
                                        return 0;
                                }
                                if ( ozy_write_ram(addr, wbuf, length) < 1 ) {
                                        fprintf(stderr, "ozy_upload_firmware: bad write\n");
                                        return 0;
                                }
                                break;

                        case 1: /* EOF */
                                break;

                        default: /* invalid */
                                fprintf(stderr, "ozy_upload_firmware: invalid type\n");
                                return 0;

                }
        }
//        fprintf(stderr, "ozy_upload_firmware: Processed %d lines.\n", linecount);
        return linecount;
}

int ozy_set_led(int which, int on) {
        int rc;
        int val;

        if ( on ) {
                val = 1;
        }
        else {
                val = 0;
        }

        rc = libusb_control_transfer(ozy_handle, VENDOR_REQ_TYPE_OUT, VENDOR_REQ_SET_LED,
                                                 val, which, NULL, 0, OZY_IO_TIMEOUT);

        if ( rc < 0 ) {
                return 0;
        }
        return 1;
}

int ozy_load_fpga(char *rbf_fnamep) {

        FILE *rbffile;
        char buf[MAX_EPO_PACKET_SIZE];
        int bytes_read;
        int total_bytes_xferd = 0;
        int rc;

fprintf(stderr,"loading ozy fpga: %s\n",rbf_fnamep);

        rbffile = fopen(rbf_fnamep, "rb");
        if ( rbffile == NULL ) {
                fprintf(stderr, "Failed to open: \'%s\'\n", rbf_fnamep);
                return 0;
        }

        rc = libusb_control_transfer(ozy_handle, VENDOR_REQ_TYPE_OUT, VENDOR_REQ_FPGA_LOAD,
                                     0, FL_BEGIN, NULL, 0, OZY_IO_TIMEOUT);

        if ( rc < 0 ) {
                fprintf(stderr, "ozy_load_fpga: failed @ FL_BEGIN rc=%d\n",rc);
                fclose(rbffile);
                return 0;
        }

        /*
         *  read the rbf and send it over the wire, 64 bytes at a time
         */
        while ( (bytes_read = fread(buf, 1, sizeof(buf), rbffile)) > 0 ) {
                rc = libusb_control_transfer(ozy_handle, VENDOR_REQ_TYPE_OUT, VENDOR_REQ_FPGA_LOAD,
                                                 0, FL_XFER, buf, bytes_read, OZY_IO_TIMEOUT);
                total_bytes_xferd += bytes_read;
                if ( rc < 0 ) {
                        fprintf(stderr, "ozy_load_fpga: failed @ FL_XFER\n");
                        fclose(rbffile);
                        return 0;
                }
        }
        printf("%d bytes transferred.\n", total_bytes_xferd);
        fclose(rbffile);
        rc = libusb_control_transfer(ozy_handle, VENDOR_REQ_TYPE_OUT, VENDOR_REQ_FPGA_LOAD,
                                          0, FL_END, NULL, 0, OZY_IO_TIMEOUT);
        if ( rc < 0 ) {
                fprintf(stderr, "ozy_load_fpga: failed @ FL_END\n");
                return 0;
        }      

        return 1;
}

