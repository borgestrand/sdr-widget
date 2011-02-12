/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Management of the USB stream.
 *
 * "USB stream" means that a device and a host are reading/writing information
 * through the USB at a specific sampling rate. But, in reality, even if the
 * sampling rate is identical for both products, there is no guaranty that they
 * are strictly equivalent. More over, some jitter may also appear. The aim of
 * the usb_stream_control module is to ensure a good audio playback by gently
 * re-synchronising the stream.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a USB module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ***************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */

//_____  I N C L U D E S ___________________________________________________

#include <stdio.h>
#include "conf_usb.h"
#include "usb_drv.h"
#include "usb_stream_control.h"

#if USB_HOST_FEATURE == ENABLED

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


//_____ D E C L A R A T I O N S ____________________________________________

//! The USB stream audio FIFO is composed of buffers.
volatile S8   usb_stream_audio_buffer[     USB_STREAM_BUFFER_NUMBER][USB_STREAM_REAL_BUFFER_SIZE];

//! Corrector factor used to ensure a good synchronization between stream creator and stream reader.
         U16  usb_stream_corrector_factor[ USB_STREAM_BUFFER_NUMBER];

//! Holds the data size of the audio packets stored into the buffers.
volatile U16  usb_stream_audio_buffer_size[USB_STREAM_BUFFER_NUMBER];

//! Reader buffer id.
volatile U8   usb_stream_rd_id;

//! Writer buffer id.
volatile U8   usb_stream_wr_id;

//! Boolean telling if synchro is acting or not.
volatile Bool usb_stream_start=FALSE;

//! Driver status.
volatile int  usb_stream_status;

static   U8   usb_stream_bits_per_sample;
static   U16  usb_stream_cur_cpt;
static   U8   usb_stream_channel_count;



//!
//! @brief This callback function is called when the PWM DAC interrupt has sent
//! the buffer 'n-1' and switches to buffer 'n'. The aim of this function is thus to prepare
//! the buffer 'n+1'; so that there is always a pending buffer for the interrupt.
//!
void pwm_dac_sample_sent_cb( void )
{
   // Previous buffer has been sent by PWM DAC driver.
   usb_stream_fifo_pull();

   if( usb_stream_fifo_get_used_room()!=0 )
   {
      void* buffer;
      U16   size;

      usb_stream_fifo_get(&buffer, &size);
      pwm_dac_output(buffer, size/(usb_stream_channel_count*usb_stream_bits_per_sample/8));

   }
   else
   {
      usb_stream_start  = FALSE;
      usb_stream_status = USB_STREAM_STATUS_NOT_SYNCHRONIZED;
   }
}


//!
//! @brief This callback function is called when the PWM DAC interrupt does not have
//! any more audio samples (i.e. "famine").
//!
void pwm_dac_underflow_cb( void )
{
   usb_stream_start  = FALSE;
   usb_stream_status = USB_STREAM_STATUS_NOT_SYNCHRONIZED;
}


//!
//! @brief This function initializes the USB Stream driver.
//!
void usb_stream_init(
   U32 sample_rate_hz
,  U8 num_channels
,  U8 bits_per_sample
,  Bool swap_channels
,  U32 pba_hz)
{
   U32 i, j, cumul;

   // Compute corrector factor and make it big to improve precision in the following steps
   cumul = 2*(USB_STREAM_MAX_EXCURSION*1024)/(USB_STREAM_BUFFER_NUMBER-USB_STREAM_IDLE_BUFFER_NUMBER);

   usb_stream_cur_cpt=
   usb_stream_rd_id=
   usb_stream_wr_id= 0;
   usb_stream_start= FALSE;
   usb_stream_bits_per_sample=bits_per_sample;
   usb_stream_channel_count =num_channels;

   for( i=0 ; i<USB_STREAM_BUFFER_NUMBER ; i++ )
      usb_stream_audio_buffer_size[i] = 0;

   i= (USB_STREAM_BUFFER_NUMBER - USB_STREAM_IDLE_BUFFER_NUMBER)/2;
   while( i!=( USB_STREAM_BUFFER_NUMBER ) )
   {
      if( i<( (USB_STREAM_BUFFER_NUMBER - USB_STREAM_IDLE_BUFFER_NUMBER)/2 + USB_STREAM_IDLE_BUFFER_NUMBER ) )
         usb_stream_corrector_factor[i] = 0;
      else
      {
         U32 coef;
         // Compute excursion for each buffer
         coef = (i+1-((USB_STREAM_BUFFER_NUMBER - USB_STREAM_IDLE_BUFFER_NUMBER)/2 + USB_STREAM_IDLE_BUFFER_NUMBER) );
         coef = coef*cumul/1024;
         // Finally convert excursion into threshold after each 1 byte is added/removed
         coef = (1000*(usb_stream_bits_per_sample/8))/coef;
         usb_stream_corrector_factor[i] = (coef + num_channels - 1) / num_channels * num_channels;
      }
      i++;
   }

   j = ( (USB_STREAM_BUFFER_NUMBER - USB_STREAM_IDLE_BUFFER_NUMBER)/2 + USB_STREAM_IDLE_BUFFER_NUMBER );
   for( i=(USB_STREAM_BUFFER_NUMBER - USB_STREAM_IDLE_BUFFER_NUMBER)/2 ; i!=0 ; i--, j++ )
   {  usb_stream_corrector_factor[i-1] = usb_stream_corrector_factor[j]; }

#if 0
   for( i=0 ; i<USB_STREAM_BUFFER_NUMBER ; i++ )
   {  /*printf("Corrector factor[%ld]: %d\r\n", i, usb_stream_corrector_factor[i]);*/ }
#endif

   pwm_dac_start(
      sample_rate_hz
   ,  num_channels
   ,  bits_per_sample
   ,  swap_channels
   ,  pwm_dac_underflow_cb
   ,  pwm_dac_sample_sent_cb
   ,  pba_hz
   );
}


//!
//! @brief This function takes the stream coming from the selected USB pipe and sends
//! it to the PWM_DAC driver. Moreover, it ensures that both input and output stream
//! keep synchronized by adding or deleting samples.
//!
//! @param pipe_in       Number of the addressed pipe
//! @param byte_count    Number of bytes in the addressed pipe
//! @param pFifoCount    (return parameter) NULL or pointer to the number of used buffers at this time
//!
//! @return              status: (USB_STREAM_STATUS_OK, USB_STREAM_STATUS_NOT_SYNCHRONIZED,
//!                      USB_STREAM_STATUS_SPEED_UP, USB_STREAM_STATUS_SLOW_DOWN, USB_STREAM_STATUS_BUFFER_OVERFLOW)
//!
int usb_stream_input(U8 pipe_in, U16 byte_count, U32* pFifoCount)
{
   U16      real_byte_count, threshold, fifo_used_cnt;
   S16      delta; // Number of bytes added or removed
   U32      i;
   UnionPtr pswap;
   UnionPtr buffer;

   fifo_used_cnt=usb_stream_fifo_get_used_room();
   if (pFifoCount) *pFifoCount = fifo_used_cnt;

   if( usb_stream_fifo_get_free_room()==0 )
   {  // Fatal error: even with the synchro mechanism acting, we are in a case in which the
      // buffers are full.
      return USB_STREAM_STATUS_NOT_SYNCHRONIZED;
   }

   delta = 0;
   threshold = usb_stream_corrector_factor[fifo_used_cnt];
   pswap.s8ptr  =
   buffer.s8ptr = usb_stream_fifo_get_buffer(usb_stream_wr_id);

   Host_reset_pipe_fifo_access(pipe_audio_in);

   if( threshold==0 || !usb_stream_start )
   {
      host_read_p_rxpacket(pipe_audio_in, (void*)buffer.s8ptr, byte_count, NULL);
      usb_stream_cur_cpt=0;
      usb_stream_status = USB_STREAM_STATUS_OK;
   }
   else
   {
      if( fifo_used_cnt <(USB_STREAM_BUFFER_NUMBER/2) )
      {  // Few buffers ready. Need to accelerate stream by creating samples
         if( usb_stream_bits_per_sample==8 )
         {
            for( i=0 ; i<byte_count ; )
            {
               if( usb_stream_channel_count==1 )
               {  // Mono
                  if( usb_stream_cur_cpt >= threshold )
                  {
                     U8 sample;
                     usb_stream_cur_cpt=0;
                     sample = Host_read_pipe_data(pipe_audio_in, 8);
                     *buffer.s8ptr++ = sample;

                     *buffer.s8ptr++ = sample; // Duplicate sample
                     delta++;
                  }
                  else
                  {  *buffer.s8ptr++ = Host_read_pipe_data(pipe_audio_in, 8); }

                  i++, usb_stream_cur_cpt++;
               }
               else
               {  // Stereo
                  if( usb_stream_cur_cpt >= threshold )
                  {
                     U8 sample_l, sample_r;
                     usb_stream_cur_cpt=0;
                     sample_l = Host_read_pipe_data(pipe_audio_in, 8);
                     *buffer.s8ptr++ = sample_l;
                     sample_r = Host_read_pipe_data(pipe_audio_in, 8);
                     *buffer.s8ptr++ = sample_r;

                     *buffer.s8ptr++ = sample_l; // Duplicate left sample
                     *buffer.s8ptr++ = sample_r; // Duplicate right sample
                     delta+=2;
                  }
                  else
                  {
                     *buffer.s8ptr++ = Host_read_pipe_data(pipe_audio_in, 8);
                     *buffer.s8ptr++ = Host_read_pipe_data(pipe_audio_in, 8);
                  }

                  i+=2, usb_stream_cur_cpt+=2;
               }
            }
         }
         else if( usb_stream_bits_per_sample==16 )
         {
            for( i=0 ; i<byte_count/(16/8) ; )
            {
               if( usb_stream_channel_count==1 )
               {  // Mono
                  if( usb_stream_cur_cpt >= threshold )
                  {
                     U16 sample;
                     usb_stream_cur_cpt=0;
                     sample = Host_read_pipe_data(pipe_audio_in, 16);
                     *buffer.s16ptr++ = sample;

                     *buffer.s16ptr++ = sample; // Duplicate sample
                     delta+=2;
                  }
                  else
                  {  *buffer.s16ptr++ = Host_read_pipe_data(pipe_audio_in, 16); }

                  i++, usb_stream_cur_cpt++;
               }
               else
               {  // Stereo
                  if( usb_stream_cur_cpt >= threshold )
                  {
                     U16 sample_l, sample_r;
                     usb_stream_cur_cpt=0;
                     sample_l = Host_read_pipe_data(pipe_audio_in, 16);
                     *buffer.s16ptr++ = sample_l;
                     sample_r = Host_read_pipe_data(pipe_audio_in, 16);
                     *buffer.s16ptr++ = sample_r;

                     *buffer.s16ptr++ = sample_l; // Duplicate left sample
                     *buffer.s16ptr++ = sample_r; // Duplicate right sample
                     delta+=4;
                  }
                  else
                  {
                     *buffer.s16ptr++ = Host_read_pipe_data(pipe_audio_in, 16);
                     *buffer.s16ptr++ = Host_read_pipe_data(pipe_audio_in, 16);
                  }

                  i+=2, usb_stream_cur_cpt+=2;
               }
            }
         }
         usb_stream_status = USB_STREAM_STATUS_SPEED_UP;
      }
      else
      {  // Many buffers ready. Need to slow-down stream by deleting samples.
         if( usb_stream_bits_per_sample==8 )
         {
            for( i=0 ; i<byte_count ; )
            {
               if( usb_stream_channel_count==1 )
               {  // Mono
                  if( usb_stream_cur_cpt >= threshold )
                  {
                     usb_stream_cur_cpt=0;
                     Host_read_pipe_data(pipe_audio_in, 8); // Delete sample
                     delta--;
                  }
                  else
                  {  *buffer.s8ptr++ = Host_read_pipe_data(pipe_audio_in, 8); }

                  i++, usb_stream_cur_cpt++;
               }
               else
               {  // Stereo
                  if( usb_stream_cur_cpt >= threshold )
                  {
                     usb_stream_cur_cpt=0;
                     Host_read_pipe_data(pipe_audio_in, 8); // Delete sample
                     Host_read_pipe_data(pipe_audio_in, 8); // Delete sample
                     delta-=2;
                  }
                  else
                  {
                     *buffer.s8ptr++ = Host_read_pipe_data(pipe_audio_in, 8);
                     *buffer.s8ptr++ = Host_read_pipe_data(pipe_audio_in, 8);
                  }

                  i+=2, usb_stream_cur_cpt+=2;
               }
            }
         }
         else if( usb_stream_bits_per_sample==16 )
         {
            for( i=0 ; i<byte_count/(16/8) ; )
            {
               if( usb_stream_channel_count==1 )
               {  // Mono
                  if( usb_stream_cur_cpt >= threshold )
                  {
                     usb_stream_cur_cpt=0;
                     Host_read_pipe_data(pipe_audio_in, 16); // Delete sample
                     delta-=2;
                  }
                  else
                  {  *buffer.s16ptr++ = Host_read_pipe_data(pipe_audio_in, 16); }

                  i++, usb_stream_cur_cpt++;
               }
               else
               {  // Stereo
                  if( usb_stream_cur_cpt >= threshold )
                  {
                     usb_stream_cur_cpt=0;
                     Host_read_pipe_data(pipe_audio_in, 16); // Delete sample
                     Host_read_pipe_data(pipe_audio_in, 16); // Delete sample
                     delta-=4;
                  }
                  else
                  {
                     *buffer.s16ptr++ = Host_read_pipe_data(pipe_audio_in, 16);
                     *buffer.s16ptr++ = Host_read_pipe_data(pipe_audio_in, 16);
                  }

                  i+=2, usb_stream_cur_cpt+=2;
               }
            }
         }
         usb_stream_status = USB_STREAM_STATUS_SLOW_DOWN;
      }
   }

   // Update real byte counter in buffer
   real_byte_count=(U16)((S16)byte_count + delta);

   if( real_byte_count > USB_STREAM_REAL_BUFFER_SIZE )
   {
      real_byte_count = USB_STREAM_REAL_BUFFER_SIZE;
      usb_stream_status = USB_STREAM_STATUS_BUFFER_OVERFLOW;
   }

   // Swap samples since they are coming from the USB world
   if( usb_stream_bits_per_sample==16 )
      for( i=0 ; i<real_byte_count/(16/8) ; i++ )
         pswap.s16ptr[i] = swap16(pswap.s16ptr[i]);

   else if( usb_stream_bits_per_sample==32 )
      for( i=0 ; i<real_byte_count/(32/8) ; i++ )
         pswap.s32ptr[i] = swap32(pswap.s32ptr[i]);

   //for( i=0 ; i<real_byte_count/2 ; i++ )
   //   printf("0x%04hx ", pswap[i]);
   //printf("\r\n");

   usb_stream_fifo_push(real_byte_count);

   if( !usb_stream_start )
   {  // We have enough buffers to start the playback.
      usb_stream_status = USB_STREAM_STATUS_NOT_SYNCHRONIZED;

      if( (fifo_used_cnt+1)>=(USB_STREAM_BUFFER_NUMBER/2) )
      {
         void* buffer;
         U16   size;

         usb_stream_start=TRUE;
         usb_stream_fifo_get(&buffer, &size);
         pwm_dac_output(buffer, size/(usb_stream_channel_count*usb_stream_bits_per_sample/8));
      }
   }

   return usb_stream_status;
}

#endif
