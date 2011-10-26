#include <stdlib.h>
#include <stdio.h>
#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <string.h>

#include <fftw3.h>

#include "client.h"
#include "messages.h"
#include "transmitter.h"

static char response[80];

char* attach_transmitter(CLIENT* client) {

    if(client->receiver_state!=RECEIVER_ATTACHED) {
        return RECEIVER_NOT_ATTACHED;
    }

    if(client->receiver!=0) {
        return RECEIVER_NOT_ZERO;
    }

    client->transmitter_state=TRANSMITTER_ATTACHED;

    sprintf(response,"%s",OK);

    return response;
}


void process_microphone_samples(float* samples) {

    // equalizer

    // agc

    // vox

    // gain 0 to 20dB

    // I band pass filter

    // Q band pass filter
    // Q Hilbert transform

    // G3PLX speech processor

    // I band pass filter

    // Q bandpass filter


}

void set_filter(int high,int low) {
    // configure the bandpass filter
}

void bandpass_filter(float* input,float* output) {
}

