/**
* @file receiver.c
* @brief manage client attachment to receivers
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include <perseus-sdr.h>
#include "client.h"
#include "receiver.h"
#include "messages.h"
#include "perseus_audio.h"
#include "util.h"

#define SMALL_PACKETS

RECEIVER receiver[MAX_RECEIVERS];
static int iq_socket;
static struct sockaddr_in iq_addr;
static int iq_length;

static char response[80];

static unsigned long sequence=0L;

static int CORE_BANDWIDTH;

void init_receivers(PerseusConfig *ppc) {
    int i;
    for(i=0;i<MAX_RECEIVERS;i++) {
        receiver[i].client  = (CLIENT*)NULL;
        receiver[i].pPd     = NULL;
        receiver[i].samples = 0; 
    }
    CORE_BANDWIDTH = ppc->sample_rate;
    receiver[0].ppc = ppc;

    iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(iq_socket<0) {
        perror("create socket failed for iq_socket\n");
        exit(1);
    }

    iq_length=sizeof(iq_addr);
    memset(&iq_addr,0,iq_length);
    iq_addr.sin_family=AF_INET;
    iq_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    iq_addr.sin_port=htons(11002);

    if(bind(iq_socket,(struct sockaddr*)&iq_addr,iq_length)<0) {
        perror("bind socket failed for iq socket");
        exit(1);
    }

}

const char* attach_receiver(int rx, CLIENT* client) 
{
	eeprom_prodid prodid;

    if(client->state==RECEIVER_ATTACHED) {
        return CLIENT_ATTACHED;
    }

    //if(rx>=ozy_get_receivers()) {
    //    return RECEIVER_INVALID;
    //}


	// Open the first one...
	if ((receiver[rx].pPd = perseus_open(rx))==NULL) {
		printf("error: %s\n", perseus_errorstr());
        return RECEIVER_INVALID;
    }

	// Download the standard firmware to the unit
	printf("Downloading firmware...\n");
	if (perseus_firmware_download(receiver[rx].pPd,NULL)<0) {
		printf("firmware download error: %s", perseus_errorstr());
        perseus_close (receiver[rx].pPd);
        return RECEIVER_INVALID;
    }

	// Dump some information about the receiver (S/N and HW rev)
	if (receiver[rx].pPd->is_preserie == TRUE) 
		printf("The device is a preserie unit");
	else
		if (perseus_get_product_id(receiver[rx].pPd,&prodid)<0) 
			printf("get product id error: %s", perseus_errorstr());
		else
			printf("Receiver S/N: %05d-%02hX%02hX-%02hX%02hX-%02hX%02hX - HW Release:%hd.%hd\n",
					(uint16_t) prodid.sn, 
					(uint16_t) prodid.signature[5],
					(uint16_t) prodid.signature[4],
					(uint16_t) prodid.signature[3],
					(uint16_t) prodid.signature[2],
					(uint16_t) prodid.signature[1],
					(uint16_t) prodid.signature[0],
					(uint16_t) prodid.hwrel,
					(uint16_t) prodid.hwver);

	// Configure the receiver for 2 MS/s operations
	printf("Configuring FPGA...\n");

	switch (CORE_BANDWIDTH) {
		case 95000:
			if (perseus_set_sampling_rate(receiver[rx].pPd, 95000)<0) {
				printf("fpga configuration error: %s\n", perseus_errorstr());
				perseus_close (receiver[rx].pPd);
				return RECEIVER_INVALID;
			}
		break;
		case 125000:
			if (perseus_set_sampling_rate(receiver[rx].pPd, 125000)<0) {
				printf("fpga configuration error: %s\n", perseus_errorstr());
				perseus_close (receiver[rx].pPd);
				return RECEIVER_INVALID;
			}
		break;
		case 250000:
			if (perseus_set_sampling_rate(receiver[rx].pPd, 250000)<0) {
				printf("fpga configuration error: %s\n", perseus_errorstr());
				perseus_close (receiver[rx].pPd);
				return RECEIVER_INVALID;
			}
		break;
		case 500000:
			if (perseus_set_sampling_rate(receiver[rx].pPd, 500000)<0) {
				printf("fpga configuration error: %s\n", perseus_errorstr());
				perseus_close (receiver[rx].pPd);
				return RECEIVER_INVALID;
			}
		break;
        case 1000000:
            if (perseus_set_sampling_rate(receiver[rx].pPd, 1000000)<0) {
                printf("fpga configuration error: %s\n", perseus_errorstr());
                perseus_close (receiver[rx].pPd);
                return RECEIVER_INVALID;
            }
        break;
        case 2000000:
            if (perseus_set_sampling_rate(receiver[rx].pPd, 2000000)<0) {
                printf("fpga configuration error: %s\n", perseus_errorstr());
                perseus_close (receiver[rx].pPd);
                return RECEIVER_INVALID;
            }
        break;

		default:
			printf("No suitable bandwith: %d\n",CORE_BANDWIDTH );
            return RECEIVER_INVALID;
	}
    

//   // Cycle attenuator leds on the receiver front panel
//   // just to see if they indicate what they shoud
//   perseus_set_attenuator(receiver[rx].pPd, PERSEUS_ATT_0DB);
//   sleep(1);
//   perseus_set_attenuator(receiver[rx].pPd, PERSEUS_ATT_10DB);
//   sleep(1);
//   perseus_set_attenuator(receiver[rx].pPd, PERSEUS_ATT_20DB);
//   sleep(1);
//   perseus_set_attenuator(receiver[rx].pPd, PERSEUS_ATT_30DB);
//   sleep(1);
//   perseus_set_attenuator(receiver[rx].pPd, PERSEUS_ATT_0DB);
//   sleep(1);

	// Enable ADC Dither, Disable ADC Preamp
	perseus_set_adc(receiver[rx].pPd, TRUE, FALSE);

	// Do the same cycling test with the WB front panel led.
	// Enable preselection filters (WB_MODE Off)
	perseus_set_ddc_center_freq(receiver[rx].pPd, 7050000.000, 1);
//   sleep(1);
//   // Disable preselection filters (WB_MODE On)
//   perseus_set_ddc_center_freq(receiver[rx].pPd, 7000000.000, 0);
//   sleep(1);
//   // Re-enable preselection filters (WB_MODE Off)
//   perseus_set_ddc_center_freq(receiver[rx].pPd, 7000000.000, 1);


    // disable the attenuator
    perseus_set_attenuator(receiver[rx].pPd, PERSEUS_ATT_0DB);

    if(receiver[rx].client!=(CLIENT *)NULL) {
        return RECEIVER_IN_USE;
    }
    
    client->state=RECEIVER_ATTACHED;
    receiver[rx].client=client;
    client->receiver=rx;;

    // attempt to open an audio stream on a local audio card
    //perseus_audio_open (CORE_BANDWIDTH);
    receiver[rx].ppa = new PerseusAudio(CORE_BANDWIDTH);


    //sprintf(response,"%s %d",OK,ozy_get_sample_rate());
    sprintf(response,"%s %d",OK,CORE_BANDWIDTH);

    return response;
}

const char* detach_receiver (int rx, CLIENT* client) {
    if(client->state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    //if(rx>=ozy_get_receivers()) {
    //    return RECEIVER_INVALID;
    //}

    if(receiver[rx].client!=client) {
        return RECEIVER_NOT_OWNER;
    }
    printf("detach_receiver: Quitting...\n");
    perseus_stop_async_input((receiver[rx]).pPd);
    perseus_close((receiver[rx]).pPd);

    client->state=RECEIVER_DETACHED;
    receiver[rx].client = (CLIENT*)NULL;
    receiver[rx].pPd    = NULL;

    return OK;
}

const char* set_frequency (CLIENT* client, long frequency) {
    if(client->state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    if(client->receiver<0) {
        return RECEIVER_INVALID;
    }

    receiver[client->receiver].frequency=frequency;
    receiver[client->receiver].frequency_changed=1;

    //fprintf (stderr, "%s: %ld\n", __FUNCTION__, receiver[client->receiver].frequency);
    perseus_set_ddc_center_freq (receiver[client->receiver].pPd, (double)frequency, 0);

    return OK;
}


const char* set_preamp (CLIENT* client, bool preamp)
{
    receiver[client->receiver].ppc->preamp = preamp;

    perseus_set_adc (receiver[client->receiver].pPd, 
                     receiver[client->receiver].ppc->dither,
                     receiver[client->receiver].ppc->preamp
                    );
    return OK;
}

const char* set_dither (CLIENT* client, bool dither)
{
    receiver[client->receiver].ppc->dither = dither;

    perseus_set_adc (receiver[client->receiver].pPd, 
                     receiver[client->receiver].ppc->dither,
                     receiver[client->receiver].ppc->preamp
                    );
    return OK;
}

const char* set_random (CLIENT* client, bool)
{
    return NOT_IMPLEMENTED_COMMAND;
    return OK;
}

const char* set_attenuator (CLIENT* client, int new_level_in_db)
{

    perseus_set_attenuator_in_db (receiver[client->receiver].pPd, new_level_in_db);
    return OK;
}


void send_IQ_buffer (RECEIVER *pRec) {
    struct sockaddr_in client;
    int client_length;
    unsigned short offset;
    //unsigned short length;
    BUFFER buffer;
    int rc;

    //if(rx>=ozy_get_receivers()) {
    //    fprintf(stderr,"send_spectrum_buffer: invalid rx: %d\n",rx);
    //    return;
    //}

    if(pRec->client != (CLIENT*)NULL) {
        if(pRec->client->iq_port != -1) {
            // send the IQ buffer

            client_length = sizeof(client);
            memset((char*)&client,0,client_length);
            client.sin_family = AF_INET;
            client.sin_addr.s_addr = pRec->client->address.sin_addr.s_addr;
            client.sin_port = htons(pRec->client->iq_port);

#ifdef SMALL_PACKETS
            // keep UDP packets to 512 bytes or less
            //     8 bytes sequency number
            //     2 byte offset
            //     2 byte length
            offset=0;
            while(offset<sizeof(pRec->input_buffer)) {
                buffer.sequence=sequence;
                buffer.offset=offset;
                buffer.length=sizeof(pRec->input_buffer)-offset;
                if(buffer.length>500) buffer.length=500;
                memcpy ((char*)&buffer.data[0], (char*)&(pRec->input_buffer[offset/4]), buffer.length);
                rc = sendto (iq_socket, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&client,client_length);
                if(rc<=0) {
                    perror("sendto failed for iq data");
                    exit(1);
                } 
                //else {
                //    fprintf (stderr, "%s: sending packet to %s.\n", __FUNCTION__, inet_ntoa(client.sin_addr));
                //}
                offset+=buffer.length;
            }
            sequence++;

            
#else
            rc=sendto(iq_socket,pRec->input_buffer,sizeof(pRec->input_buffer),0,(struct sockaddr*)&client,client_length);
            if(rc<=0) {
                perror("sendto failed for iq data");
                exit(1);
            }
#endif
 
        }
    }
}

