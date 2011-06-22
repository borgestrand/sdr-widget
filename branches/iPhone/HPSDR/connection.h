/*
 *  connection.h
 *  SDR
 *
 *  Created by John Melton on 02/07/2009.
 *  Copyright 2009 G0ORX. All rights reserved.
 *
 */

#define PREFIX 48
#define BUFFER_SIZE 480

#define SPECTRUM_BUFFER 0
#define AUDIO_BUFFER 1

char host[64];
int port;

long frequency;
int filterLow;
int filterHigh;
int mode;
int gain;

int currentBand;
int displayedBand;

int samplesReceived;

void setHost(char * id);
void setPort(int p);
void makeConnection();
void disconnect();
void newConnection(const char *newHost,const char *newPort);
int isConnected();
void sendCommand(char *command);
void setFrequency(long f);
void setFilter(int low,int high);
void setMode(int m);
void setGain(int g);



