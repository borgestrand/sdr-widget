/** 
* @file dttsp.h
* @brief DttSP interface definitions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
*/
// dttsp.h

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




//
// what we know about DttSP
//

#define MAXRX 4

typedef enum _trxmode { RX, TX } TRXMODE;

/* --------------------------------------------------------------------------*/
/** 
* @brief Setup_SDR  
* 
* @return
*/
extern void Setup_SDR();

/* --------------------------------------------------------------------------*/
/** 
* @brief Release_Update  
* 
* @return
*/
extern void Release_Update();

/* --------------------------------------------------------------------------*/
/** 
* @brief SetThreadCom  
*
* @param thread
* 
* @return
*/
extern void SetThreadCom(int thread);

/* --------------------------------------------------------------------------*/
/** 
* @brief DttSP audio callback 
* 
* @param input_l
* @param input_r
* @param output_l
* @param output_r
* @param nframes
* 
* @return 
*/
extern void Audio_Callback (float *input_l, float *input_r, float *output_l,
                            float *output_r, unsigned int nframes, int thread);

/* --------------------------------------------------------------------------*/
/** 
* @brief Process the spectrum 
* 
* @param thread
* @param results
* 
* @return 
*/
extern void Process_Spectrum (int thread, float *results);
/* --------------------------------------------------------------------------*/
/** 
* @brief Process Panadapter
* 
* @param thread
* @param results
* 
* @return 
*/
extern void Process_Panadapter (int thread, float *results);
/* --------------------------------------------------------------------------*/
/** 
* @brief Process Phase 
* 
* @param thread
* @param results
* @param numpoints
* 
* @return 
*/
extern void Process_Phase (int thread, float *results, int numpoints);
/* --------------------------------------------------------------------------*/
/** 
* @brief Process scope 
* 
* @param thread
* @param results
* @param numpoints
* 
* @return 
*/
extern void Process_Scope (int thread, float *results, int numpoints);
/* --------------------------------------------------------------------------*/
/** 
* @brief Calculate the RX meter 
* 
* @param subrx
* @param mt
* 
* @return 
*/
extern float CalculateRXMeter(int thread,unsigned int subrx, int mt);

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the sample rate 
* 
* @param sampleRate
* 
* @return 
*/
extern int SetSampleRate(double sampleRate);

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the Oscillator frequency 
* 
* @param frequency
* 
* @return 
*/
extern int SetRXOsc(unsigned int thread, unsigned subrx, double freq);

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the receiver output gain
* 
* @param gain
* 
* @return 
*/
extern int SetRXOutputGain(unsigned int thread, unsigned subrx, double gain);

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the receiver pan position
* 
* @param pos
* 
* @return 
*/
extern int SetRXPan(unsigned int thread, unsigned subrx, float pos);

extern int SetRingBufferOffset(unsigned int thread, int offset);
