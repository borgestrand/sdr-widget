/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Processing of USB device specific enumeration requests.
 *
 * This file contains the specific request decoding for enumeration process.
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
 * Additions and Modifications to ATMEL AVR32-SoftwareFramework-AT32UC3 are:
 *
 * Copyright (C) Alex Lee
 *
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"


#if USB_DEVICE_FEATURE == ENABLED

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "usart.h"
#include "pm.h"
#include "Mobo_config.h"
#include "usb_audio.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

extern pm_freq_param_t   pm_freq_param;
extern Bool mute, spk_mute;
extern S16 volume, spk_volume;

#define VOL_MIN      (S16)0x8000
#define VOL_MAX      (S16)0x7FFF
#define VOL_RES      0x000A

//_____ P R I V A T E   D E C L A R A T I O N S ____________________________

U8 usb_feature_report[3];
U8 usb_report[3];

U8 g_u8_report_rate=0;


static U8    wValue_msb;
static U8    wValue_lsb;
static U16   wIndex;
static U16   wLength;


extern const    void *pbuffer;
extern          U16   data_to_transfer;

U8 dg8saqBuffer[32];	// 32 bytes long return buffer for DG8SAQ commands


//_____ D E C L A R A T I O N S ____________________________________________



//! @brief This function manages hid set idle request.
//!
//! @param Duration     When the upper byte of wValue is 0 (zero), the duration is indefinite else from 0.004 to 1.020 seconds
//! @param Report ID    0 the idle rate applies to all input reports, else only applies to the Report ID
//!
void usb_hid_set_idle (U8 u8_report_id, U8 u8_duration )
{
   Usb_ack_setup_received_free();
  
   if( wIndex == DSC_INTERFACE_HID )
     g_u8_report_rate = u8_duration;
   
   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());
}


//! @brief This function manages hid get idle request.
//!
//! @param Report ID    0 the idle rate applies to all input reports, else only applies to the Report ID
//!
void usb_hid_get_idle (U8 u8_report_id)
{
	Usb_ack_setup_received_free();
   
   if( (wLength != 0) && (wIndex == DSC_INTERFACE_HID) )
   {
      Usb_write_endpoint_data(EP_CONTROL, 8, g_u8_report_rate);
      Usb_ack_control_in_ready_send();
   }
   
   while (!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}



//! @brief This function configures the endpoints of the device application.
//! This function is called when the set configuration request has been received.
//!
void usb_user_endpoint_init(U8 conf_nb)
{
	if (Is_usb_full_speed_mode()){
    (void)Usb_configure_endpoint(EP_HID_TX,
                                 EP_ATTRIBUTES_1,
                                 DIRECTION_IN,
                                 EP_SIZE_1_FS,
                                 SINGLE_BANK, 0);
    (void)Usb_configure_endpoint(EP_HID_RX,
                                 EP_ATTRIBUTES_2,
                                 DIRECTION_OUT,
                                 EP_SIZE_2_FS,
                                 SINGLE_BANK, 0);
    (void)Usb_configure_endpoint(EP_AUDIO_OUT,
                           EP_ATTRIBUTES_3,
                           DIRECTION_OUT,
                           EP_SIZE_3_FS,
                           DOUBLE_BANK, 0);
    (void)Usb_configure_endpoint(EP_AUDIO_IN,
                            EP_ATTRIBUTES_4,
                            DIRECTION_IN,
                            EP_SIZE_4_FS,
                            DOUBLE_BANK, 0);

    /*
    (void)Usb_configure_endpoint(EP_AUDIO_OUT_FB,
                           EP_ATTRIBUTES_5,
                           DIRECTION_OUT,
                           EP_SIZE_5_FS,
                           SINGLE_BANK, 0);
         */
	}
	else {
		   (void)Usb_configure_endpoint(EP_HID_TX,
		                                 EP_ATTRIBUTES_1,
		                                 DIRECTION_IN,
		                                 EP_SIZE_1_HS,
		                                 SINGLE_BANK, 0);
		    (void)Usb_configure_endpoint(EP_HID_RX,
		                                 EP_ATTRIBUTES_2,
		                                 DIRECTION_OUT,
		                                 EP_SIZE_2_HS,
		                                 SINGLE_BANK, 0);
		    (void)Usb_configure_endpoint(EP_AUDIO_OUT,
		                           EP_ATTRIBUTES_3,
		                           DIRECTION_OUT,
		                           EP_SIZE_3_HS,
		                           DOUBLE_BANK, 0);
		    (void)Usb_configure_endpoint(EP_AUDIO_IN,
		                           EP_ATTRIBUTES_4,
		                           DIRECTION_IN,
		                           EP_SIZE_4_HS,
		                           DOUBLE_BANK, 0);

		    /*
		    (void)Usb_configure_endpoint(EP_AUDIO_OUT_FB,
		                            EP_ATTRIBUTES_5,
		                            DIRECTION_OUT,
		                            EP_SIZE_5_HS,
		                            SINGLE_BANK, 0);
		      */
	}

}


//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
Bool usb_user_read_request(U8 type, U8 request)
{

   // Read wValue
   wValue_lsb = Usb_read_endpoint_data(EP_CONTROL, 8);
   wValue_msb = Usb_read_endpoint_data(EP_CONTROL, 8);
   wIndex = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
   wLength = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

   //** Specific request from Class HID
   if( wIndex == DSC_INTERFACE_HID )   // Interface number of HID
   {

	   if( type == OUT_CL_INTERFACE ) // USB_SETUP_SET_CLASS_INTER
	   {
		   switch( request )
		   {

		   case HID_SET_REPORT:
			   // The MSB wValue field specifies the Report Type
			   // The LSB wValue field specifies the Report ID
			   switch (wValue_msb)
			   {
				   case HID_REPORT_INPUT:
					   // TODO
					   break;

				   case HID_REPORT_OUTPUT:
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received());
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					usb_report[0] = Usb_read_endpoint_data(EP_CONTROL, 8);
					usb_report[1] = Usb_read_endpoint_data(EP_CONTROL, 8);
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_in_ready());
					return TRUE;

					case HID_REPORT_FEATURE:
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received());
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					usb_feature_report[0] = Usb_read_endpoint_data(EP_CONTROL, 8);
					usb_feature_report[1] = Usb_read_endpoint_data(EP_CONTROL, 8);
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
					while (!Is_usb_control_in_ready()); //!< waits for status phase done
					return TRUE;

			   }
			   break;

			   case HID_SET_IDLE:
				   usb_hid_set_idle(wValue_lsb, wValue_msb);
				   return TRUE;
   
			   case HID_SET_PROTOCOL:
				   // TODO
				   break;
		   }
	   }
	   if( type == IN_CL_INTERFACE) // USB_SETUP_GET_CLASS_INTER
	   {
		   switch( request )
		   {
			   case HID_GET_REPORT:
				   switch (wValue_msb)
				   {
					   case HID_REPORT_INPUT:
						  Usb_ack_setup_received_free();

						  Usb_reset_endpoint_fifo_access(EP_CONTROL);
						  Usb_write_endpoint_data(EP_CONTROL, 8, usb_report[0]);
						  Usb_write_endpoint_data(EP_CONTROL, 8, usb_report[1]);
						  Usb_ack_control_in_ready_send();

						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;

					 case HID_REPORT_OUTPUT:
					 break;

					 case HID_REPORT_FEATURE:
						  Usb_ack_setup_received_free();

						  Usb_reset_endpoint_fifo_access(EP_CONTROL);
						  Usb_write_endpoint_data(EP_CONTROL, 8, usb_feature_report[0]);
						  Usb_write_endpoint_data(EP_CONTROL, 8, usb_feature_report[1]);
						  Usb_ack_control_in_ready_send();

						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;
				   }
				   break;
			 case HID_GET_IDLE:
				 usb_hid_get_idle(wValue_lsb);
				 return TRUE;
			 case HID_GET_PROTOCOL:
				 // TODO
				 break;
		   }
	   }
   }  // if wIndex ==  HID Interface


   //  assume all other requests are for AUDIO interface

	   switch (request)
	    {
	       case BR_REQUEST_SET_CUR:
	          audio_set_cur();
	          return TRUE;
	          // No need to break here !

	       case BR_REQUEST_SET_MIN:     //! Set MIN,MAX and RES not supported
	       case BR_REQUEST_SET_MAX:
	       case BR_REQUEST_SET_RES:
	          return FALSE;
	          // No need to break here !

	       case BR_REQUEST_GET_CUR:
	          audio_get_cur();
	          return TRUE;
	          // No need to break here !

	       case BR_REQUEST_GET_MIN:
	          audio_get_min();
	          return TRUE;
	          // No need to break here !

	       case BR_REQUEST_GET_MAX:
	          audio_get_max();
	          return TRUE;
	          // No need to break here !

	       case BR_REQUEST_GET_RES:
	          audio_get_res();
	          return TRUE;
	          // No need to break here !

	       default:
	          return FALSE;
	          // No need to break here !
	    }

   return FALSE;  // No supported request
}


//! This function returns the size and the pointer on a user information
//! structure
//!
Bool usb_user_get_descriptor(U8 type, U8 string)
{
  pbuffer = NULL;

  switch (type)
  {
  case STRING_DESCRIPTOR:
    switch (string)
    {
    case LANG_ID:
      data_to_transfer = sizeof(usb_user_language_id);
      pbuffer = &usb_user_language_id;
      break;

    case MAN_INDEX:
      data_to_transfer = sizeof(usb_user_manufacturer_string_descriptor);
      pbuffer = &usb_user_manufacturer_string_descriptor;
      break;

    case PROD_INDEX:
      data_to_transfer = sizeof(usb_user_product_string_descriptor);
      pbuffer = &usb_user_product_string_descriptor;
      break;

    case SN_INDEX:
      data_to_transfer = sizeof(usb_user_serial_number);
      pbuffer = &usb_user_serial_number;
      break;

    default:
      break;
    }
    break;

  default:
    break;
  }

  return pbuffer != NULL;
}

Bool usb_user_DG8SAQ(U8 type, U8 command){

	U16 wValue, wIndex, wLength;
	U8 replyLen;
	int x;

    // Grab the wValue, wIndex / wLength
	wValue  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
	wIndex  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
	wLength  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

		//-------------------------------------------------------------------------------
		// Process USB Host to Device transmissions.  No result is returned.
		//-------------------------------------------------------------------------------
		if (type == (DRD_OUT | DRT_STD | DRT_VENDOR))
		{
		        Usb_ack_setup_received_free();
		        while (!Is_usb_control_out_received());
		        Usb_reset_endpoint_fifo_access(EP_CONTROL);

		        //for (x = 0; x<wLength;x++)
			    if (wLength>0) for (x = wLength-1; x>=0;x--)
		        {
			        dg8saqBuffer[x] = Usb_read_endpoint_data(EP_CONTROL, 8);
		        }
		        Usb_ack_control_out_received_free();
		        Usb_ack_control_in_ready_send();
		        while (!Is_usb_control_in_ready());

				// This is our all important hook - Do the magic... control Si570 etc...
				dg8saqFunctionWrite(command, wValue, wIndex, dg8saqBuffer, wLength);
		}
		//-------------------------------------------------------------------------------
		// Process USB query commands and return a result (flexible size data payload)
		//-------------------------------------------------------------------------------
		else if (type == (DRD_IN | DRT_STD | DRT_VENDOR))
		{
			// This is our all important hook - Process and execute command, read CW paddle state etc...
			replyLen = dg8saqFunctionSetup(command, wValue, wIndex, dg8saqBuffer);

			Usb_ack_setup_received_free();

	        Usb_reset_endpoint_fifo_access(EP_CONTROL);

	        // Write out if packet is larger than zero
	  		if (replyLen)
	  		{
				for (x = replyLen-1; x>=0;x--)
		        {
			        Usb_write_endpoint_data(EP_CONTROL, 8, dg8saqBuffer[x]);	// send the reply
		        }
	  		}

            Usb_ack_control_in_ready_send();
            while (!Is_usb_control_in_ready());			// handshake modified by Alex 16 May 2010

	        while ( !Is_usb_control_out_received());
	        Usb_ack_control_out_received_free();

		}

		return TRUE;
}



void audio_get_min(void)
{
   U16 cs;      // in wValue
   U16 i_unit;  // in wIndex
   U16 length;  // in wLength

   LSB(cs)=wValue_lsb;
   MSB(cs)=wValue_msb;
   i_unit = wIndex;
   length = wLength;

   Usb_ack_setup_received_free();
   Usb_reset_endpoint_fifo_access(EP_CONTROL);
   if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (cs)
      {
      case CS_MUTE:
         if( length==1 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 8, mute);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MIN));
         }
         break;
      }
    } else if ( i_unit == SPK_FEATURE_UNIT_ID)
      {
         switch (cs)
         {
         case CS_MUTE:
            if( length==1 )
            {
               Usb_write_endpoint_data(EP_CONTROL, 8, spk_mute);
            }
            break;
         case CS_VOLUME:
            if( length==2 )
            {
               Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MIN));
            }
            break;
         }
      }

   Usb_ack_control_in_ready_send();
   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}

void audio_get_max(void)
{
   U16 i_unit;
   U16 length;
   U16 cs;

   LSB(cs)=wValue_lsb;
   MSB(cs)=wValue_msb;
   i_unit = wIndex;
   length = wLength;

   Usb_ack_setup_received_free();

   Usb_reset_endpoint_fifo_access(EP_CONTROL);
   if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (cs)
      {
      case CS_MUTE:
         if( length==1 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 8, mute);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MAX));
         }
         break;
      }
   }
   else if ( i_unit == SPK_FEATURE_UNIT_ID){
	     switch (cs)
	      {
	      case CS_MUTE:
	         if( length==1 )
	         {
	            Usb_write_endpoint_data(EP_CONTROL, 8, spk_mute);
	         }
	         break;
	      case CS_VOLUME:
	         if( length==2 )
	         {
	            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MAX));
	         }
	         break;
	      }
   }
   Usb_ack_control_in_ready_send();
   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}

void audio_get_res(void)
{
   U16 i_unit;
   U16 length;
   U16 cs;

   LSB(cs)=wValue_lsb;
   MSB(cs)=wValue_msb;
   i_unit = wIndex;
   length = wLength;

   Usb_ack_setup_received_free();

   Usb_reset_endpoint_fifo_access(EP_CONTROL);
   if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (cs)
      {
      case CS_MUTE:
         if( length==1 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 8, mute);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
         }
         break;
      }
   } else if ( i_unit==SPK_FEATURE_UNIT_ID)
   {
	     switch (cs)
	      {
	      case CS_MUTE:
	         if( length==1 )
	         {
	            Usb_write_endpoint_data(EP_CONTROL, 8, spk_mute);
	         }
	         break;
	      case CS_VOLUME:
	         if( length==2 )
	         {
	            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
	         }
	         break;
	      }
   }
   Usb_ack_control_in_ready_send();
   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}

void audio_get_cur(void)
{
   U16 i_unit;
   U16 length;
   U16 cs;

   LSB(cs)=wValue_lsb;
   MSB(cs)=wValue_msb;
   i_unit = wIndex;
   length = wLength;

   Usb_ack_setup_received_free();

   Usb_reset_endpoint_fifo_access(EP_CONTROL);
   if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (cs)
      {
      case CS_MUTE:
         if( length==1 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 8, mute);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, volume));
         }
         break;
      }
   } else if (i_unit==SPK_FEATURE_UNIT_ID){
	     switch (cs)
	      {
	      case CS_MUTE:
	         if( length==1 )
	         {
	            Usb_write_endpoint_data(EP_CONTROL, 8, spk_mute);
	         }
	         break;
	      case CS_VOLUME:
	         if( length==2 )
	         {
	            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, spk_volume));
	         }
	         break;
	      }
   }


   Usb_ack_control_in_ready_send();
   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}

void audio_set_cur(void)
{
   U16 i_unit;
   U16 length;
   U16 cs;

   LSB(cs)=wValue_lsb;
   MSB(cs)=wValue_msb;
   i_unit = wIndex;
   length = wLength;

   Usb_ack_setup_received_free();
   while(!Is_usb_control_out_received());
   Usb_reset_endpoint_fifo_access(EP_CONTROL);
   if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (cs)
      {
      case CS_MUTE:
         if( length==1 )
         {
            mute=Usb_read_endpoint_data(EP_CONTROL, 8);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            LSB(volume)= Usb_read_endpoint_data(EP_CONTROL, 8);
            MSB(volume)= Usb_read_endpoint_data(EP_CONTROL, 8);
         }
         break;
      }
   }
   else if (i_unit==SPK_FEATURE_UNIT_ID ){
	   {
	       switch (cs)
	       {
	       case CS_MUTE:
	          if( length==1 )
	          {
	             spk_mute=Usb_read_endpoint_data(EP_CONTROL, 8);
	          }
	          break;
	       case CS_VOLUME:
	          if( length==2 )
	          {
	             LSB(spk_volume)= Usb_read_endpoint_data(EP_CONTROL, 8);
	             MSB(spk_volume)= Usb_read_endpoint_data(EP_CONTROL, 8);
	          }
	          break;
	       }
	    }
   }
   Usb_ack_control_out_received_free();

   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());
}


#endif  // USB_DEVICE_FEATURE == ENABLED
