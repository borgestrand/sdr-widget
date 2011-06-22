/******************************************************************************/
/*                                                                            */
/*                          IoExample for PortTalk V2.1                       */
/*                        Version 2.1, 12th January 2002                      */
/*                          http://www.beyondlogic.org                        */
/*                                                                            */
/* Copyright © 2002 Craig Peacock. Craig.Peacock@beyondlogic.org              */
/* Any publication or distribution of this code in source form is prohibited  */
/* without prior written permission of the copyright holder. This source code */
/* is provided "as is", without any guarantee made as to its suitability or   */
/* fitness for any particular use. Permission is herby granted to modify or   */
/* enhance this sample code to produce a derivative program which may only be */
/* distributed in compiled object form only.                                  */
/******************************************************************************/

extern "C" {
#ifndef _pt_ioctl_h
#define _pt_ioctl_h

#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <porttalk_IOCTL.h>

unsigned char OpenPortTalk(void);
void ClosePortTalk(void);

void outportb(unsigned short PortAddress, unsigned char byte);
unsigned char inportb(unsigned short PortAddress);

void InstallPortTalkDriver(void);
unsigned char StartPortTalkDriver(void);

#define    inp(PortAddress)         inportb(PortAddress)
#define    outp(PortAddress, Value) outportb(PortAddress, Value)

#endif // _pt_ioctl_h
}
