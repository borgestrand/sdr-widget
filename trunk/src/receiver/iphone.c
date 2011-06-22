/** 
* @file iphone.c
* @brief iPhone network interface
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// iphone.c

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

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include "filter.h"
#include "xvtr.h"
#include "band.h"
#include "mode.h"
#include "ozy.h"
#include "spectrum_update.h"
#include "vfo.h"
#include "main.h"
#include "soundcard.h"

static pthread_t iphone_thread_id;

int port=8000;

int serverSocket;
int clientSocket;
struct sockaddr_in server;
struct sockaddr_in client;
int addrlen;

void* iphone_thread(void* arg);
void iphone_send_samples(gpointer data);

#define PREFIX 48
#define BUFFER_SIZE 480
unsigned char iphone_samples[BUFFER_SIZE+PREFIX];

int rejectAddress(char* address) {
    int result=0;
    if(strcmp(address,"222.208.183.218")==0) result=1;
    if(strcmp(address,"221.195.73.68")==0) result=1;
    if(strcmp(address,"61.183.15.9")==0) result=1;
    return result;
}

void iphone_init() {

    int rc;

    clientSocket=-1;
    rc=pthread_create(&iphone_thread_id,NULL,iphone_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on iphone_thread: rc=%d\n", rc);
    }

}

void* iphone_thread(void* arg) {

    int bytesRead;
    char message[32];
    int on=1;

fprintf(stderr,"iphone_thread\n");

    serverSocket=socket(AF_INET,SOCK_STREAM,0);
    if(serverSocket==-1) {
        perror("iphone socket");
        return;
    }

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&server,0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(port);

    if(bind(serverSocket,(struct sockaddr *)&server,sizeof(server))<0) {
        perror("iphone bind");
        return;
    }

    while(1) {
//fprintf(stderr,"iphone_thread: listen\n");
        if (listen(serverSocket, 5) == -1) {
            perror("iphone listen");
            break;
        }

//fprintf(stderr,"iphone_thread: accept\n");
        addrlen = sizeof(client); 
	if ((clientSocket = accept(serverSocket,(struct sockaddr *)&client,&addrlen)) == -1) {
		perror("iphone accept");
	} else {

            time_t tt;
            struct tm *tod;
            time(&tt);
            tod=localtime(&tt);
            fprintf(stderr,"%02d/%02d/%02d %02d:%02d:%02d iphone connection from %s:%d\n",tod->tm_mday,tod->tm_mon+1,tod->tm_year+1900,tod->tm_hour,tod->tm_min,tod->tm_sec,inet_ntoa(client.sin_addr),ntohs(client.sin_port));

            if(rejectAddress(inet_ntoa(client.sin_addr))) {
                fprintf(stderr,"connection rejected!\n");
            } else {
                while(1) {
                    bytesRead=recv(clientSocket, message, sizeof(message), 0);
                    if(bytesRead<=0) {
                        //perror("iphone recv");
                        break;
                    }
                    message[bytesRead]=0;
//fprintf(stderr,"iphone message: %s\n",message);

                    if(strncmp(message,"getSpectrum",11)==0) {
                        iphone_send_samples((gpointer)NULL);
                    } else if(strncmp(message,"setMode",7)==0) {
                        int *mode=malloc(sizeof(int));
                        char* stringMode=&message[8];
                        if(strcmp(stringMode,"LSB")==0) {
                            *mode=modeLSB;
                        } else if(strcmp(stringMode,"USB")==0) {
                            *mode=modeUSB;
                        } else if(strcmp(stringMode,"DSB")==0) {
                            *mode=modeDSB;
                        } else if(strcmp(stringMode,"CWL")==0) {
                            *mode=modeCWL;
                        } else if(strcmp(stringMode,"CWU")==0) {
                            *mode=modeCWU;
                        } else if(strcmp(stringMode,"SPEC")==0) {
                            *mode=modeSPEC;
                        } else if(strcmp(stringMode,"DIGL")==0) {
                            *mode=modeDIGL;
                        } else if(strcmp(stringMode,"DIGU")==0) {
                            *mode=modeDIGU;
                        } else if(strcmp(stringMode,"DRM")==0) {
                            *mode=modeDRM;
                        } else if(strcmp(stringMode,"AM")==0) {
                            *mode=modeAM;
                        } else if(strcmp(stringMode,"SAM")==0) {
                            *mode=modeSAM;
                        } else if(strcmp(stringMode,"FMN")==0) {
                            *mode=modeFMN;
                        }
                        g_idle_add(setMode,(gpointer)mode);
                    } else if(strncmp(message,"setAFrequency",12)==0) {
                        long long*frequency=malloc(sizeof(long long));
                        *frequency=atoll(&message[13]);
                        g_idle_add(setAFrequency,(gpointer)frequency);
                    } else if(strncmp(message,"scrollFrequency",15)==0) {
                        long *increment=malloc(sizeof(long));
                        *increment=atol(&message[16]);
//fprintf(stderr,"scrollFrequency %ld\n",*increment);
                        g_idle_add(vfoStepFrequency,(gpointer)increment);
                    } else if(strncmp(message,"band",4)==0) {
                        // select a band
                        int *band=malloc(sizeof(int));
                        *band=atoi(&message[5]);
//fprintf(stderr,"select band %d\n",*band);
                        g_idle_add(remoteSetBand,(gpointer)band);
                    } else {
fprintf(stderr,"iphone_thread: invalid command: %s\n",message);
                        break;
                    }
                }
            }

            close(clientSocket);
            time(&tt);
            tod=localtime(&tt);
            fprintf(stderr,"%02d/%02d/%02d %02d:%02d:%02d iphone disconnected from %s:%d\n",tod->tm_mday,tod->tm_mon+1,tod->tm_year+1900,tod->tm_hour,tod->tm_min,tod->tm_sec,inet_ntoa(client.sin_addr),ntohs(client.sin_port));
        }
        clientSocket=-1;

    }
    
}

void iphone_send_samples(gpointer data) {
    int rc;
    if(clientSocket!=-1) {
//fprintf(stderr,"iphone_send_samples\n");
        rc=send(clientSocket,iphone_samples,BUFFER_SIZE+PREFIX,0);
        if(rc<0) {
            perror("iphone send failed");
        }
    } else {
fprintf(stderr,"iphone_send_samples: clientSocket==-1\n");
    }
}

void iphone_set_samples(float* samples) {
    int i,j;
    float slope;
    float max;
    int lindex,rindex;

    // first 14 bytes contain the frequency
    sprintf(iphone_samples,"% 4lld.%03lld.%03lld",frequencyA/1000000LL,(frequencyA%1000000LL)/1000LL,frequencyA%1000LL);

    // next 6 bytes contain the filter low
    sprintf(&iphone_samples[14],"%d",filterLow);

    // next 6 bytes contain the filter high
    sprintf(&iphone_samples[20],"%d",filterHigh);

    // next 6 bytes contain the mode
    sprintf(&iphone_samples[26],"%s",modeToString());

    // next 8 bytes contain the sample rate
    sprintf(&iphone_samples[32],"%d",sampleRate);

    // next 8 bytes contain the band
    sprintf(&iphone_samples[40],"%d",band);

    slope=(float)SPECTRUM_BUFFER_SIZE/(float)BUFFER_SIZE;
    for(i=0;i<BUFFER_SIZE;i++) {
        max=-10000.0F;
        lindex=(int)floor((float)i*slope);
        rindex=(int)floor(((float)i*slope)+slope);
        if(rindex>SPECTRUM_BUFFER_SIZE) rindex=SPECTRUM_BUFFER_SIZE;
        for(j=lindex;j<rindex;j++) {
            if(samples[j]>max) max=samples[j];
        }
        iphone_samples[i+PREFIX]=(unsigned char)-(max+displayCalibrationOffset+preampOffset);
    }

}
