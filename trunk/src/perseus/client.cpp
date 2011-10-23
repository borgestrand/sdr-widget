/**
* @file client.c
* @brief Handle client connection
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "client.h"
#include "receiver.h"
#include "messages.h"
#include "perseus_audio.h"


typedef union {
	struct __attribute__((__packed__)) {
		int32_t	i;
		int32_t	q;
		} iq;
	struct __attribute__((__packed__)) {
		uint8_t		i1;
		uint8_t		i2;
		uint8_t		i3;
		uint8_t		i4;
		uint8_t		q1;
		uint8_t		q2;
		uint8_t		q3;
		uint8_t		q4;
		};
} iq_sample;

static int counter = 0;

// 2^24 / 2 -1 = 8388607.0

#define SCALE_FACTOR 8388607.0

#define ADC_CLIP 0x00000001

int user_data_callback(void *buf, int buf_size, void *extra)
{
	// The buffer received contains 24-bit IQ samples (6 bytes per sample)
	// Here as a demonstration we save the received IQ samples as 32 bit 
	// (msb aligned) integer IQ samples.

	// At the maximum sample rate (2 MS/s) the hard disk should be capable
	// of writing data at a rate of at least 16 MB/s (almost 1 GB/min!)

	uint8_t	*samplebuf 	= (uint8_t*)buf;
	//FILE *fout 			= (FILE*)extra;
	int nSamples 		= buf_size/6;
	int k;
    int adc_clip = 0;
	iq_sample s;
	RECEIVER *pRec = (RECEIVER *)extra;

    //fprintf (stderr, ">>>>>>>>>>>>> %s\n", __FUNCTION__);

    if (pRec == 0) {
        fprintf (stderr, "%s: no receiver data.\n", __FUNCTION__);
        return 0;
    }

	s.i1 = s.q1 = 0;

	for (k=0; k < nSamples; k++) {
		s.i2 = *samplebuf++;
		s.i3 = *samplebuf++;
		s.i4 = *samplebuf++;
		s.q2 = *samplebuf++;
		s.q3 = *samplebuf++;
		s.q4 = *samplebuf++;

		//fwrite(&s.iq, 1, sizeof(iq_sample), fout);

        pRec->input_buffer[pRec->samples]             = ((float) (s.iq.q)) / SCALE_FACTOR;
        pRec->input_buffer[pRec->samples+BUFFER_SIZE] = ((float) (s.iq.i)) / SCALE_FACTOR;
//        pRec->input_buffer[pRec->samples]             = (float) 0;
//        pRec->input_buffer[pRec->samples+BUFFER_SIZE] = (float) 0;

        if (adc_clip == 0) adc_clip = s.iq.i & ADC_CLIP; 

        if ((counter++ % 204800) == 0) {
           fprintf (stderr, ">>>>>>>>>>>>>> %s: k: %d i: %d q: %d\n", __FUNCTION__, k, s.iq.i, s.iq.q);  
           fprintf (stderr, ">>>>>>>>>>>>>> LSB first: i: %02x%02x%02x%02x q: %02x%02x%02x%02x\n",
                    s.i1, s.i2, s.i3, s.i4, s.q1, s.q2, s.q3, s.q4 );  
           fprintf (stderr, ">>>>>>>>>>>>>> i: %f q: %f\n", 
                    pRec->input_buffer[pRec->samples], pRec->input_buffer[pRec->samples+BUFFER_SIZE]);
           fflush(stderr);
        }

        pRec->samples++;

     // if(timing) {
     //     sample_count++;
     //     if(sample_count==sample_rate) {
     //         ftime(&end_time);
     //         fprintf(stderr,"%d samples in %ld ms\n",sample_count,((end_time.time*1000)+end_time.millitm)-((start_time.time*1000)+start_time.millitm));
     //         sample_count=0;
     //         ftime(&start_time);
     // }
     // }

        // when we have enough samples send them to the clients
        if(pRec->samples==BUFFER_SIZE) {
            // send I/Q data to clients
            //fprintf (stderr, "%s: sending data.\n", __FUNCTION__);
            send_IQ_buffer(pRec);
            pRec->samples=0;
        }


	}
    return 0;
}


short audio_port=AUDIO_PORT;

const char* parse_command(CLIENT* client,char* command);
void* audio_thread(void* arg);

void* client_thread(void* arg) {
    CLIENT* client=(CLIENT*)arg;
    char command[80];
    int bytes_read;
    const char* response;

fprintf(stderr,"%s: client connected: %s:%d\n", __FUNCTION__, inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));

    client->state=RECEIVER_DETACHED;

    while(1) {
        bytes_read=recv(client->socket,command,sizeof(command),0);
        if(bytes_read<=0) {
            break;
        }
        command[bytes_read]=0;
        response=parse_command(client,command);
        send(client->socket,response,strlen(response),0);

//fprintf(stderr,"%s: response: '%s'\n", __FUNCTION__, response);

        if (strcmp(response, QUIT_ASAP) == 0) {
            break;
        }
    }

    close(client->socket);

    fprintf(stderr,"%s: client disconnected: %s:%d\n", __FUNCTION__, inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));

    if(client->state==RECEIVER_ATTACHED) {
        //int rx = client->receiver;
        //free(client);
        //receiver[rx].client=(CLIENT*)NULL;
        //client->state=RECEIVER_DETACHED;

        detach_receiver (client->receiver, client);

    }
    return 0;
}

const char* parse_command(CLIENT* client,char* command) {
    
    char* token;

    fprintf(stderr,"parse_command: '%s'\n",command);

    token=strtok(command," \r\n");
    if(token!=NULL) {
        if(strcmp(token,"attach")==0) {
            // select receiver
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                int rx=atoi(token);
                return attach_receiver(rx,client);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"detach")==0) {
            // select receiver
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                int rx=atoi(token);
                return detach_receiver(rx,client);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"frequency")==0) {
            // set frequency
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long f=atol(token);
               return set_frequency (client,f);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"start")==0) {
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if(strcmp (token,"iq")==0) {
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->iq_port=atoi(token);
                    }

                    if(pthread_create(&receiver[client->receiver].audio_thread_id,NULL,audio_thread,&receiver[client->receiver])!=0) {
                        fprintf(stderr,"failed to create audio thread for rx %d\n",client->receiver);
                        exit(1);
                    }
                    printf("***************************Starting async data acquisition... CLIENT REQUESTED %d port\n", client->iq_port);

                    (receiver[client->receiver]).samples = 0;
                 	if ( perseus_start_async_input ( (receiver[client->receiver]).pPd, 16320, user_data_callback, &(receiver[client->receiver])) < 0 ) {
                 		printf("start async input error: %s\n", perseus_errorstr());
                        return INVALID_COMMAND;
                 	} else {
                 		printf("start async input: %s\n", "STARTED");
                    }


                    return OK;
                } else if(strcmp(token,"bandscope")==0) {
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->bs_port=atoi(token);
                    }
                    return OK;
                } else {
                    // invalid command string
                    return INVALID_COMMAND;
                }
            } else {
                // invalid command string
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"preamp")==0) {
            // set frequency
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if (strcmp(token,"on")==0) {
                    return set_preamp (client,true);
                }
                if (strcmp(token,"off")==0) {
                    return set_preamp (client,false);
                }
                return INVALID_COMMAND;
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"dither")==0) {
            // set frequency
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if (strcmp(token,"on")==0) {
                    return set_dither (client,true);
                }
                if (strcmp(token,"off")==0) {
                    return set_dither (client,false);
                }
                return INVALID_COMMAND;
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"setattenuator")==0) {
            // set attenuator
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long av=atol(token);
               return set_attenuator (client,av);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"random")==0) {
            // set frequency
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if (strcmp(token,"on")==0) {
                    return set_random (client,true);
                }
                if (strcmp(token,"off")==0) {
                    return set_random (client,false);
                }
                return INVALID_COMMAND;
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"stop")==0) {
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if(strcmp(token,"iq")==0) {
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->iq_port=-1;
                    }
                    // try to terminate audio thread
                    close ((receiver[client->receiver]).audio_socket);
                    printf("Quitting...\n");
                    perseus_close((receiver[client->receiver]).pPd);
                    return OK;
                } else if(strcmp(token,"bandscope")==0) {
                    client->bs_port=-1;
                } else {
                    // invalid command string
                    return INVALID_COMMAND;
                }
            } else {
                // invalid command string
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"quit")==0) {
            return QUIT_ASAP;
        } else {
            // invalid command string
            return INVALID_COMMAND;
        }
    } else {
        // empty command string
        return INVALID_COMMAND;
    }
    return INVALID_COMMAND;
}

void* audio_thread(void* arg) {
    RECEIVER          *rx=(RECEIVER*)arg;
    struct sockaddr_in audio;
    socklen_t          audio_length;

    int old_state, old_type;
    int bytes_read;
    int on=1;

    fprintf(stderr,"audio_thread port=%d\n",audio_port+(rx->id*2));

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);

    rx->audio_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(rx->audio_socket<0) {
        perror("create socket failed for server audio socket");
        exit(1);
    }

    setsockopt(rx->audio_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    audio_length=sizeof(audio);
    memset(&audio,0,audio_length);
    audio.sin_family=AF_INET;
    audio.sin_addr.s_addr=htonl(INADDR_ANY);
    audio.sin_port=htons(audio_port+(rx->id*2));

    if(bind(rx->audio_socket,(struct sockaddr*)&audio,audio_length)<0) {
        perror("bind socket failed for server audio socket");
        exit(1);
    }

    fprintf(stderr,"listening for rx %d audio on port %d\n",rx->id,audio_port+(rx->id*2));

    while(1) {
        // get audio from a client
        bytes_read=recvfrom (rx->audio_socket,
                             rx->output_buffer,
                             sizeof(rx->output_buffer),
                             0,
                             (struct sockaddr*)&audio,
                             &audio_length);
        if(bytes_read<0) {
            perror("recvfrom socket failed for audio buffer");
            exit(1);
        } else {
            if (bytes_read != sizeof(rx->output_buffer)) {
                fprintf (stderr, "Arrrgh: %d bytes read instead of %d\n", bytes_read, sizeof(rx->output_buffer));
            }
        }

        //process_ozy_output_buffer(rx->output_buffer,&rx->output_buffer[BUFFER_SIZE]);
        //rx->ppa->write_3 (rx->output_buffer,&rx->output_buffer[BUFFER_SIZE]);
        rx->ppa->write_sr (rx->output_buffer,&rx->output_buffer[BUFFER_SIZE]);

    }
}

