/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Management of the USB host mouse HID task.
 *
 * This file manages the USB host mouse HID task.
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

#include "conf_usb.h"
#include "debug.h"

#if USB_HOST_FEATURE == ENABLED

#include "board.h"
#ifdef FREERTOS_USED
#  include "FreeRTOS.h"
#  include "task.h"
#endif

#include "usb_drv.h"
#include "usb_host_enum.h"
#include "usb_host_task.h"
#include "usb_audio.h"
#include "host_audio_task.h"
#include <stdio.h>
#include "pwm_dac.h"
#include "pm.h"
#include "usb_stream_control.h"

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

extern pm_freq_param_t pm_freq_param;



//_____ D E C L A R A T I O N S ____________________________________________

volatile U8  audio_cpt_sof;

//! Define for each 'feature unit' the max numbers of bmaControls
//! One of them is used by the microphone!
#define MAX_FEATURE_UNITS 3
volatile cs_feature_unit_t g_cs_feature[MAX_FEATURE_UNITS];
volatile U8                g_cs_num_features_unit=0;

volatile U32 g_sample_freq[      MAX_INTERFACE_SUPPORTED];
volatile U8  g_n_channels[       MAX_INTERFACE_SUPPORTED];
volatile U8  g_n_bits_per_sample[MAX_INTERFACE_SUPPORTED];
         U16 byte_count=0;

U8  pipe_audio_in;
U8  interf_audio_stream;

volatile Bool audio_new_device_connected;
volatile Bool audio_connected;


//!
//! @brief This function initializes the host mouse HID task.
//!
void host_audio_task_init(void)
{
  audio_new_device_connected = FALSE;
  audio_connected = FALSE;

#ifdef FREERTOS_USED
  xTaskCreate(host_audio_task,
              configTSK_USB_HAUDIO_NAME,
              configTSK_USB_HAUDIO_STACK_SIZE,
              NULL,
              configTSK_USB_HAUDIO_PRIORITY,
              NULL);
#endif  // FREERTOS_USED
}


//!
//! @brief This function manages the host mouse HID task.
//!
#ifdef FREERTOS_USED
void host_audio_task(void *pvParameters)
#else
void host_audio_task(void)
#endif
{
   U32  i, j;
   U32  fifo_cnt;
   int  stream_status;

#ifdef FREERTOS_USED
   portTickType xLastWakeTime;
   xLastWakeTime = xTaskGetTickCount();

   while (TRUE)
   {
      vTaskDelayUntil(&xLastWakeTime, configTSK_USB_HAUDIO_PERIOD);

#endif  // FREERTOS_USED
   // First, check the host controller is in full operating mode with the
   // B-device attached and enumerated
   if (Is_host_ready())
   {
      // New device connection (executed only once after device connection)
      if (audio_new_device_connected)
      {
         audio_new_device_connected = FALSE;

         // For all supported interfaces
         for (i = 0; i < Get_nb_supported_interface(); i++)
         {
            // Audio Streaming Interface
            // Select the audio streaming interface that has an IN PIPE
            if((Get_class(i)==AUDIO_CLASS)
            && (Get_subclass(i)==AUDIOSTREAMING_SUBCLASS)
            && (Get_nb_ep(i) != 0))
            {
               for (j=0 ; j<Get_nb_ep(i) ; j++)
               {
                  if (Is_ep_in(i,j))
                  {
                    // Log in device
                    audio_connected=TRUE;
                    LED_On(LED0);
                    Host_enable_sof_interrupt();

                    // Select and enable ISOCHRONOUS pipe
                    pipe_audio_in = Get_ep_pipe(i,j);
                    Host_enable_continuous_in_mode(pipe_audio_in);
                    Host_unfreeze_pipe(pipe_audio_in);

                    // Enable alternate streaming interface
                    interf_audio_stream = Get_interface_number(i);                     // store interface number
                    host_set_interface(interf_audio_stream,1);   // enable streaming interface with "alternate 1" on Device
                    break;
                  }
               }
            }
         }

         printf("Detecting Microphone settings:\r\n");
         printf(". Sample freq      =%ld Hz.\r\n", g_sample_freq[     interf_audio_stream]);
         printf(". Num of channels  =%d.\r\n", g_n_channels[       interf_audio_stream]);
         printf(". Bits per sample  =%d.\r\n", g_n_bits_per_sample[interf_audio_stream]);

         // Unmuting all features unit and setting default volume to MAX
         //
         for( i=0 ; i<g_cs_num_features_unit ; i++ )
         {
            for( j=0 ; j<g_cs_feature[i].n_bmaControls ; j++ )
            {
               U16 max;
               U8  bmaControls= g_cs_feature[i].bmaControls[j];

               if( bmaControls==0 )
                  continue;

               if( bmaControls & 0x01)
               {
                  host_audio_set_cur_mute(g_cs_feature[i].unit, FALSE);  // Unmute channels
               }

               if( bmaControls & 0x02)
               {
                  max= host_audio_get_max(g_cs_feature[i].unit, j);
                  host_audio_set_cur(g_cs_feature[i].unit, j, max);
               }
            }
         }
         host_set_sampling_rate(Host_get_pipe_endpoint_number(pipe_audio_in)|MSK_EP_DIR, g_sample_freq[interf_audio_stream]);

         usb_stream_init(
           g_sample_freq[      interf_audio_stream]
         , g_n_channels[       interf_audio_stream]
         , g_n_bits_per_sample[interf_audio_stream]
         , FALSE
         , pm_freq_param.pba_f
         );
      }


      if( audio_connected )
      {
         if((Is_host_in_received(pipe_audio_in) )
         && (Is_host_stall(pipe_audio_in)==FALSE))
         {
            byte_count=Host_byte_count(pipe_audio_in);
            if(byte_count!=0)
            {
               stream_status = usb_stream_input(pipe_audio_in, byte_count, &fifo_cnt);
               if( USB_STREAM_STATUS_OK == stream_status )
               {
                  LED_Off( LED0 );
                  LED_On(  LED1 );
                  LED_Off( LED2 );
               }
               else if( USB_STREAM_STATUS_SPEED_UP == stream_status )
               {
                  LED_On(  LED0 );
                  LED_Off( LED1 );
                  LED_Off( LED2 );
               }
               else if( USB_STREAM_STATUS_SLOW_DOWN == stream_status )
               {
                  LED_Off( LED0 );
                  LED_Off( LED1 );
                  LED_On(  LED2 );
               }
               else if( USB_STREAM_STATUS_NOT_SYNCHRONIZED == stream_status )
               {
                  LED_On( LED0 );
                  LED_On( LED1 );
                  LED_On( LED2 );
               }
            }
            Host_ack_in_received(pipe_audio_in);
            Host_free_in(pipe_audio_in);
         }
      
         if(Is_host_nak_received(pipe_audio_in))
         {
            Host_ack_nak_received(pipe_audio_in);
         }
      }
   }
#ifdef FREERTOS_USED
   }
#endif
}

//!
//! @brief This function controls the Mute feature of a particular unit
//!
void host_audio_set_cur_mute(U16 unit, Bool cs_mute)
{
   data_stage[0] = cs_mute;
   usb_request.bmRequestType   = 0x21;
   usb_request.bRequest        = BR_REQUEST_SET_CUR;
   usb_request.wValue          = CS_MUTE;
   usb_request.wIndex          = (unit)<<8;
   usb_request.wLength         = 1;
   usb_request.incomplete_read = FALSE;
   host_transfer_control(data_stage);
}


//!
//! @brief This function set the 'current setting' feature of a particular unit
//!
void host_audio_set_cur(U16 unit, U16 channel_number, U16 cur)
{
   data_stage[0] = LSB(cur);
   data_stage[1] = MSB(cur);
   usb_request.bmRequestType   = 0x21;
   usb_request.bRequest        = BR_REQUEST_SET_CUR;
   usb_request.wValue          = CS_VOLUME | channel_number;
   usb_request.wIndex          = (unit)<<8;
   usb_request.wLength         = 2;
   usb_request.incomplete_read = FALSE;
   host_transfer_control(data_stage);
}


//!
//! @brief This function returns the 'MAX setting' feature of a particular unit
//!
U16 host_audio_get_max(U16 unit, U16 channel_number)
{
   U16 max;
   usb_request.bmRequestType   = 0xA1;
   usb_request.bRequest        = BR_REQUEST_GET_MAX;
   usb_request.wValue          = CS_VOLUME | channel_number;
   usb_request.wIndex          = (unit)<<8;
   usb_request.wLength         = 2;
   usb_request.incomplete_read = FALSE;
   host_transfer_control(data_stage);
   LSB(max)= data_stage[0];
   MSB(max)= data_stage[1];
   return max;
}


//!
//! @brief This function selects one of the sampling rate of the streaming interface.
//!
void host_set_sampling_rate(U16 endpoint, U32 sampling_rate)
{
   data_stage[0] = MSB3(sampling_rate);
   data_stage[1] = MSB2(sampling_rate);
   data_stage[2] = MSB1(sampling_rate);
   usb_request.bmRequestType   = 0x22;
   usb_request.bRequest        = 0x01;
   usb_request.wValue          = DEVICE_DESCRIPTOR << 8;
   usb_request.wIndex          = endpoint;
   usb_request.wLength         = 3;
   usb_request.incomplete_read = FALSE;
   host_transfer_control(data_stage);
}



#endif  // USB_HOST_FEATURE == ENABLED
