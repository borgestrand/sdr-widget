/** 
* @file ozy.h
* @brief Header files for the Ozy interface functions 
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-03-10
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


#ifndef _OZY_H
#define	_OZY_H

#ifdef	__cplusplus
extern "C" {
#endif

// Added by Alex Lee 18 Aug 2010
#define LO_OFFSET 9000

#define SYNC 0x7F
#define OZY_BUFFER_SIZE 512

// ozy command and control
#define MOX_DISABLED    0x00
#define MOX_ENABLED     0x01

#define MIC_SOURCE_JANUS 0x00
#define MIC_SOURCE_PENELOPE 0x80
#define CONFIG_NONE     0x00
#define CONFIG_PENELOPE 0x20
#define CONFIG_MERCURY  0x40
#define CONFIG_BOTH     0x60
#define PENELOPE_122_88MHZ_SOURCE 0x00
#define MERCURY_122_88MHZ_SOURCE  0x10
#define ATLAS_10MHZ_SOURCE        0x00
#define PENELOPE_10MHZ_SOURCE     0x04
#define MERCURY_10MHZ_SOURCE      0x08
#define SPEED_48KHZ               0x00
#define SPEED_96KHZ               0x01
#define SPEED_192KHZ              0x02

#define MODE_CLASS_E              0x01
#define MODE_OTHERS               0x00

#define ALEX_ATTENUATION_0DB      0x00
#define ALEX_ATTENUATION_10DB     0x01
#define ALEX_ATTENUATION_20DB     0x02
#define ALEX_ATTENUATION_30DB     0x03
#define LT2208_GAIN_OFF           0x00
#define LT2208_GAIN_ON            0x04
#define LT2208_DITHER_OFF         0x00
#define LT2208_DITHER_ON          0x08
#define LT2208_RANDOM_OFF         0x00
#define LT2208_RANDOM_ON          0x10

typedef struct _buffer {
    unsigned long long sequence;
    unsigned short offset;
    unsigned short length;
    unsigned char data[500];
} BUFFER;

int speed;
int class;
int lt2208Dither;
int lt2208Random;
int alexAttenuation;
int micSource;
int clock10MHz;
int clock122_88MHz;
int preamp;

int sampleRate;

int mox;

int forwardPower;

char server_address[64];


/* --------------------------------------------------------------------------*/
/** 
* @brief Initialize the Ozy interface 
* 
* @return 
*/
extern int ozy_init();


/* --------------------------------------------------------------------------*/
/** 
* @brief Close the Ozy interface 
* 
* @return 
*/
extern void ozyClose();

/* --------------------------------------------------------------------------*/
/** 
* @brief Disconnect the Ozy interface 
* 
* @return 
*/
extern void ozyDisconnect();

/* --------------------------------------------------------------------------*/
/** 
* @brief set frequency
* 
* @return 
*/
extern void setFrequency(long f);

/* --------------------------------------------------------------------------*/
/** 
* @brief set rx frequency
* 
* @return 
*/
extern void setRxFrequency(long f);

/* --------------------------------------------------------------------------*/
/** 
* @brief set tx frequency
* 
* @return 
*/
extern void setTxFrequency(long f);

/* --------------------------------------------------------------------------*/
/** 
* @brief set duplex
* 
* @return 
*/
extern void setDuplex(int d);

/* --------------------------------------------------------------------------*/
/** 
* @brief set preamp
* 
* @return 
*/
extern void setPreamp();

/* --------------------------------------------------------------------------*/
/** 
* @brief Detect a ADC Overflow 
* 
* @return 
*/
extern int getADCOverflow();

/* --------------------------------------------------------------------------*/
/** 
* @brief Set 122.88 MHz clock source
*
* @param source
* 
* @return 
*/
extern void set122MHzSource(int source);

/* --------------------------------------------------------------------------*/
/**
* @brief Get Bandscope spectrum samples
*
* @return
*/
void getSpectrumSamples(char *samples);


/* --------------------------------------------------------------------------*/
/**
* @brief Get Ozy FX2 software version
*
* @return
*/
char* get_ozy_firmware_version();

/* --------------------------------------------------------------------------*/
/**
* @brief Get Mercury software version
*
* @return
*/
int get_mercury_software_version();

/* --------------------------------------------------------------------------*/
/**
* @brief Get Penelope software version
*
* @return
*/
int get_penelope_software_version();

/* --------------------------------------------------------------------------*/
/**
* @brief Get Ozy software version
*
* @return
*/
int get_ozy_software_version();

/* --------------------------------------------------------------------------*/
/**
* @brief save Ozy state
*
* @return
*/
void ozySaveState();

/* --------------------------------------------------------------------------*/
/**
* @brief restore Ozy state
*
* @return
*/
void ozyRestoreState();

/* --------------------------------------------------------------------------*/
/**
* @brief set speed
*
* @return
*/
void setSpeed(int s);

#ifdef	__cplusplus
}
#endif


int set_frequency();

void ozy_set_local_audio(int state);
void ozy_set_port_audio(int state);
void ozy_set_debug(int state);

#endif	/* _OZY_H */

