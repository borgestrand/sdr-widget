/**
* @file metis.c
* @brief Metis protocol implementation
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
#ifdef __linux
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <pthread.h>
#else
#include "pthread.h"
#endif

#include <string.h>
#include <errno.h>

#include "metis.h"
#include "ozy.h"

#define MAX_METIS_CARDS 10
METIS_CARD metis_cards[MAX_METIS_CARDS];

#define DISCOVER_IDLE 0
#define DISCOVER_SENT 1
static int discover_state=DISCOVER_IDLE;

#define PORT 1024
#define DISCOVERY_SEND_PORT PORT
#define DISCOVERY_RECEIVE_PORT PORT
#define DATA_PORT PORT

static int discovery_socket;
static struct sockaddr_in discovery_addr;
static int discovery_length;

static int discovering;

static unsigned char hw_address[6];
static long ip_address;

static int data_socket;
static struct sockaddr_in data_addr;
static int data_addr_length;

static unsigned char buffer[70];

static pthread_t receive_thread_id;
static int found=0;

int ep;
long sequence=-1;

void* metis_receive_thread(void* arg);
void metis_send_buffer(char* buffer,int length);

#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])

static int get_addr(int sock, char * ifname) {

  struct ifreq *ifr;
  struct ifreq ifrr;
  struct sockaddr_in sa;
  unsigned char      *u;
  int i;

  ifr = &ifrr;
  ifrr.ifr_addr.sa_family = AF_INET;
  strncpy(ifrr.ifr_name, ifname, sizeof(ifrr.ifr_name));

  if (ioctl(sock, SIOCGIFADDR, ifr) < 0) {
    printf("No %s interface.\n", ifname);
    return -1;
  }

  ip_address=inaddrr(ifr_addr.sa_data).s_addr;

  if (ioctl(sock, SIOCGIFHWADDR, ifr) < 0) {
    printf("No %s interface.\n", ifname);
    return -1;
  }

  u = (unsigned char *) &ifr->ifr_addr.sa_data;

  for(i=0;i<6;i++)
      hw_address[i]=u[i];


  return 0;
}

void metis_discover(char* interface,char* metisip) {
    int rc;
    int i;
    int on=1;
    struct ifreq ifr;

    fprintf(stderr,"Looking for Metis card on interface %s\n",interface);

    discovering=1;
    
    // send a broadcast to locate metis boards on the network
    discovery_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(discovery_socket<0) {
        perror("create socket failed for discovery_socket\n");
        exit(1);
    }

    // get my MAC address and IP address
    if(get_addr(discovery_socket,interface)<0) {
        exit(1);
    }

    printf("%s IP Address: %ld.%ld.%ld.%ld\n",
              interface,
              ip_address&0xFF,
              (ip_address>>8)&0xFF,
              (ip_address>>16)&0xFF,
              (ip_address>>24)&0xFF);

    printf("%s MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
         interface,
         hw_address[0], hw_address[1], hw_address[2], hw_address[3], hw_address[4], hw_address[5]);


    // start a receive thread to get discovery responses
    rc=pthread_create(&receive_thread_id,NULL,metis_receive_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on metis_receive_thread: rc=%d\n", rc);
        exit(1);
    }

    // bind to this interface
    struct sockaddr_in name={0};
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = ip_address;
    name.sin_port = htons(DISCOVERY_SEND_PORT);
    bind(discovery_socket,(struct sockaddr*)&name,sizeof(name));


    // allow broadcast on the socket
    rc=setsockopt(discovery_socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    if(rc != 0) {
        fprintf(stderr,"cannot set SO_BROADCAST: rc=%d\n", rc);
        exit(1);
    }

    discovery_length=sizeof(discovery_addr);
    memset(&discovery_addr,0,discovery_length);
    discovery_addr.sin_family=AF_INET;
    discovery_addr.sin_port=htons(DISCOVERY_SEND_PORT);
    discovery_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);

    buffer[0]=0xEF;
    buffer[1]=0xFE;
    buffer[2]=0x02;
    for(i=0;i<60;i++) {
        buffer[i+3]=0x00;
    }

    if(sendto(discovery_socket,buffer,63,0,(struct sockaddr*)&discovery_addr,discovery_length)<0) {
        perror("sendto socket failed for discovery_socket\n");
        exit(1);
    }
}

int metis_found() {
    return found;
}

char* metis_ip_address(int entry) {
    if(entry>=0 && entry<found) {
        return metis_cards[entry].ip_address;
    }
    return NULL;
}

char* metis_mac_address(int entry) {
    if(entry>=0 && entry<found) {
        return metis_cards[entry].mac_address;
    }
    return NULL;
}

void metis_start_receive_thread() {
    int i;
    int rc;
    struct hostent *h;

    fprintf(stderr,"Metis starting receive thread\n");

    discovering=0;

    h=gethostbyname(metis_cards[0].ip_address);
    if(h==NULL) {
        fprintf(stderr,"metis_start_receiver_thread: unknown host %s\n",metis_cards[0].ip_address);
        exit(1);
    }

    data_addr_length=sizeof(data_addr);
    memset(&data_addr,0,data_addr_length);
    data_addr.sin_family=AF_INET;
    data_addr.sin_port=htons(DATA_PORT);
    memcpy((char *)&data_addr.sin_addr.s_addr,h->h_addr_list[0],h->h_length);

    ozy_prime();
    
    // send a packet to start the stream
    buffer[0]=0xEF;
    buffer[1]=0xFE;
    buffer[2]=0x04;    // data send state
    buffer[3]=0x01;    // send (0x00=stop)

    for(i=0;i<60;i++) {
        buffer[i+4]=0x00;
    }

    if(sendto(discovery_socket,buffer,64,0,(struct sockaddr*)&data_addr,data_addr_length)<0) {
        perror("sendto socket failed for start\n");
        exit(1);
    }

}

void* metis_receive_thread(void* arg) {
    struct sockaddr_in addr;
    int length;
    unsigned char buffer[2048];
    int bytes_read;

    length=sizeof(addr);
    while(1) {
   	bytes_read=recvfrom(discovery_socket,buffer,sizeof(buffer),0,(struct sockaddr*)&addr,&length);
        if(bytes_read<0) {
            perror("recvfrom socket failed for metis_receive_thread");
            exit(1);
        }

        if(buffer[0]==0xEF && buffer[1]==0xFE) {
            switch(buffer[2]) {
                case 1:
                    if(!discovering) {
                        // get the end point
                        ep=buffer[3]&0xFF;

                        // get the sequence number
                        sequence=((buffer[4]&0xFF)<<24)+((buffer[5]&0xFF)<<16)+((buffer[6]&0xFF)<<8)+(buffer[7]&0xFF);
                        //fprintf(stderr,"received data ep=%d sequence=%ld\n",ep,sequence);

                        switch(ep) {
                            case 6:
                                // process the data
                                process_ozy_input_buffer(&buffer[8]);
                                process_ozy_input_buffer(&buffer[520]);
                                break;
                            case 4:
                                fprintf(stderr,"EP4 data\n");
                                break;
                            default:
                                fprintf(stderr,"unexpected EP %d length=%d\n",ep,bytes_read);
                                break;
                        }
                    } else {
                        fprintf(stderr,"unexpected data packet when in discovery mode\n");
                    }
                    break;
                case 2:  // response to a discovery packet
                    if(discovering) {
                        if(found<MAX_METIS_CARDS) {
                            // get MAC address from reply
                            sprintf(metis_cards[found].mac_address,"%02X:%02X:%02X:%02X:%02X:%02X",
                                       buffer[3]&0xFF,buffer[4]&0xFF,buffer[5]&0xFF,buffer[6]&0xFF,buffer[7]&0xFF,buffer[8]&0xFF);
                            fprintf(stderr,"Metis MAC address %s\n",metis_cards[found].mac_address);
    
                            // get ip address from packet header
                            sprintf(metis_cards[found].ip_address,"%d.%d.%d.%d",
                                       addr.sin_addr.s_addr&0xFF,
                                       (addr.sin_addr.s_addr>>8)&0xFF,
                                       (addr.sin_addr.s_addr>>16)&0xFF,
                                       (addr.sin_addr.s_addr>>24)&0xFF);
                            fprintf(stderr,"Metis IP address %s\n",metis_cards[found].ip_address);
                            found++;
                        } else {
                            fprintf(stderr,"too many metis cards!\n");
                        }
                    } else {
                        fprintf(stderr,"unexepected discovery response when not in discovery mode\n");
                    }
                    break;
                default:
                    fprintf(stderr,"unexpected packet type: 0x%02X\n",buffer[2]);
                    break;
            }
        } else {
            fprintf(stderr,"received bad header bytes on data port %02X,%02X\n",buffer[0],buffer[1]);
        }

    }
    
}

static unsigned char output_buffer[1032];
static long send_sequence=-1;
static int offset=8;

int metis_write(unsigned char ep,char* buffer,int length) {
    int i;

//fprintf(stderr,"metis_write\n");
    if(offset==8) {

        send_sequence++;
        output_buffer[0]=0xEF;
        output_buffer[1]=0xFE;
        output_buffer[2]=0x01;
        output_buffer[3]=ep;
        output_buffer[4]=(send_sequence>>24)&0xFF;
        output_buffer[5]=(send_sequence>>16)&0xFF;
        output_buffer[6]=(send_sequence>>8)&0xFF;
        output_buffer[7]=(send_sequence)&0xFF;

        // copy the buffer over
        for(i=0;i<512;i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=520;
    } else {
        // copy the buffer over
        for(i=0;i<512;i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=8;

        // send the buffer
        metis_send_buffer(&output_buffer[0],1032);

    }

    return length;
}

void metis_send_buffer(char* buffer,int length) {
//fprintf(stderr,"metis_send_buffer\n");
    if(sendto(discovery_socket,buffer,length,0,(struct sockaddr*)&data_addr,data_addr_length)<0) {
        perror("sendto socket failed for metis_send_data\n");
        exit(1);
    }
}
