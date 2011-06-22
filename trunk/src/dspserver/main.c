/** 
* \file main.c
* \brief Main file for the GHPSDR Software Defined Radio Graphic Interface. 
* \author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* \version 0.1
* \date 2009-04-11
*
*
* \mainpage GHPSDR 
*  \image html ../ghpsdr.png
*  \image latex ../ghpsdr.png "Screen shot of GHPSDR" width=10cm
*
* \section A Linux based, GTK2+, Radio Graphical User Interface to HPSDR boards through DttSP without Jack.  
* \author John Melton, G0ORX/N6LYT
* \version 0.1
* \date 2009-04-11
* 
* \author Dave Larsen, KV0S, Doxygen comments
*
* These files are design to build a simple 
* high performance  interface under the Linux  operating system.  
*
* This is still very much an Alpha version. It does still have problems and not everything is 
* completed.
*
* To build the application there is a simple Makefile.
*
* To run the application just start ghpsdr once it is built.
*
* Currently it does not include any code to load the FPGA so you must run initozy before 
* running the application. You must also have the latest FPGA code.
*
* Functionally, each band has 3 bandstacks. The frequency/mode/filter settings will be 
* saved when exiting the application for all the bandstack entries.
*
* Tuning can be accomplished by left mouse clicking in the Panadapter/Waterfall window to 
* move the selected frequency to the center of the current filter. A right mouse click will 
* move the selected frequency to the cursor. You can also use the left mouse button to drag 
* the frequency by holding it down while dragging. If you have a scroll wheel, moving the 
* scroll wheel will increment/decrement the frequency by the current step amount.
*
* You can also left mouse click on the bandscope display and it will move to the selected frequency.
* 
* The Setup button pops up a window to adjust the display settings. There are no tests 
* currently if these are set to invalid values.
*
*
* There are some problems when running at other than 48000. Sometimes the audio output will 
* stop although the Panadapter/Waterfall and bandscope continue to function. It usually 
* requires intiozy to be run again to get the audio back.
*
*
* Development of the system is documented at 
* http://javaguifordttsp.blogspot.com/
*
* This code is available at 
* svn://206.216.146.154/svn/repos_sdr_hpsdr/trunk/N6LYT/ghpsdr
*
* More information on the HPSDR project is availble at 
* http://openhpsdr.info
*
*/

/* Copyright (C) 
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
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



// main.c

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "client.h"
#include "dttsp.h"
#include "audiostream.h"
#include "soundcard.h"
#include "ozy.h"
#include "version.h"

char propertyPath[128];

struct option longOptions[] = {
    {"soundcard",required_argument, 0, 0},
    {"receiver",required_argument, 0, 1},
    {"server",required_argument, 0, 2},
    {"offset",required_argument, 0, 3},
    {"timing",no_argument, 0, 4},
    {0,0,0,0}
};

char* shortOptions="";

int optionIndex;

/* --------------------------------------------------------------------------*/
/** 
* @brief Process program arguments 
* 
* @param argc
* @param argv
*/
/* ----------------------------------------------------------------------------*/
void processCommands(int argc,char** argv) {
    int c;
    while((c=getopt_long(argc,argv,shortOptions,longOptions,&optionIndex)!=EOF)) {
        switch(optionIndex) {
            case 0:
                strcpy(soundCardName,optarg);
                break;
            case 1:
                receiver=atoi(optarg);
                break;
            case 2:
                strcpy(server_address,optarg);
                break;
            case 3:
                offset=atoi(optarg);
                break;
            case 4:
                client_set_timing();
                break;
	    default:
                fprintf(stderr,"Usage: \n");
                fprintf(stderr,"  dspserver --receivers N (default 1)\n");
                fprintf(stderr,"            --server 0.0.0.0 (default 127.0.0.1)\n");
                fprintf(stderr,"            --soundcard (machine dependent)\n");
                fprintf(stderr,"            --offset 0 \n");
                exit(1);

        }
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief  Main - it all starts here
* 
* @param argc
* @param argv[]
* 
* @return 
*/
/* ----------------------------------------------------------------------------*/
int main(int argc,char* argv[]) {
    int i;
    char directory[1024];

    strcpy(soundCardName,"HPSDR");
    strcpy(server_address,"127.0.0.1"); // localhost

    processCommands(argc,argv);

    fprintf(stderr,"gHPSDR rx %d (Version %s)\n",receiver,VERSION);

    setSoundcard(getSoundcardId(soundCardName));

    // initialize DttSP
    if(getcwd(directory, sizeof(directory))==NULL) {
        fprintf(stderr,"current working directory path is > 1024 bytes!");
        exit(1);
    }
    Setup_SDR(directory);
    Release_Update();
    SetTRX(0,0); // thread 0 is for receive
    SetTRX(1,1);  // thread 1 is for transmit
    SetRingBufferOffset(0,offset);
    SetThreadProcessingMode(0,2);
    SetThreadProcessingMode(1,2);
    SetSubRXSt(0,0,1);
    SetRXOutputGain(0,0,0.20);

    reset_for_buflen(0,1024);
    reset_for_buflen(1,1024);

    client_init(receiver);
    audio_stream_init(receiver);

    // initialize ozy
    ozy_init();

    while(1) {
        sleep(10000);
    }

    return 0;
}
