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
 * Modified by Alex Lee and sdr-widget team since Feb 2010.  Copyright General Purpose Licence v2.
 * Please refer to http://code.google.com/p/sdr-widget/
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
#include "device_audio_task.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

extern pm_freq_param_t   pm_freq_param;


//_____ P R I V A T E   D E C L A R A T I O N S ____________________________

U8 usb_feature_report[3];
U8 usb_report[3];

U8 g_u8_report_rate=0;

S_line_coding   line_coding;
U8 clock_selected = 1;
Bool clock_changed = FALSE;

S_freq current_freq;
Bool freq_changed = FALSE;

static U8    wValue_msb;
static U8    wValue_lsb;
static U16   wIndex;
static U16   wLength;


extern const    void *pbuffer;
extern          U16   data_to_transfer;

U8 dg8saqBuffer[32];	// 32 bytes long return buffer for DG8SAQ commands


int Speedx[38] = {\
0x03,0x00,				//Size
0x80,0xbb,0x00,0x00,	//48k Min
0x80,0xbb,0x00,0x00,	//48k Max
0x00,0x00,0x00,0x00,	// 0 Res

0x00,0x77,0x01,0x00,	//96k Min
0x00,0x77,0x01,0x00,	//96k Max
0x00,0x00,0x00,0x00,	// 0 Res

0x00,0xee,0x02,0x00,	//192k Min
0x00,0xee,0x02,0x00,	//192k Max
0x00,0x00,0x00,0x00	// 0 Res
};



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

   if( Is_usb_full_speed_mode() )
   {
     (void)Usb_configure_endpoint(EP_HID_TX,
                                  EP_ATTRIBUTES_4,
                                  DIRECTION_IN,
                                  EP_SIZE_4_FS,
                                  SINGLE_BANK);
     (void)Usb_configure_endpoint(EP_HID_RX,
                                  EP_ATTRIBUTES_5,
                                  DIRECTION_OUT,
                                  EP_SIZE_5_FS,
                                  SINGLE_BANK);
     (void)Usb_configure_endpoint(EP_CDC_COMM,
                                  EP_ATTRIBUTES_1,
                                  DIRECTION_IN,
                                  EP_SIZE_1_FS,
                                  SINGLE_BANK);
     (void)Usb_configure_endpoint(EP_CDC_TX,
                                  EP_ATTRIBUTES_2,
                                  DIRECTION_IN,
                                  EP_SIZE_2_FS,
                                  DOUBLE_BANK);
     (void)Usb_configure_endpoint(EP_CDC_RX,
                                  EP_ATTRIBUTES_3,
                                  DIRECTION_OUT,
                                  EP_SIZE_3_FS,
                                  DOUBLE_BANK);
     (void)Usb_configure_endpoint(EP_AUDIO_IN,
                            EP_ATTRIBUTES_6,
                            DIRECTION_IN,
                            EP_SIZE_6_FS,
                            DOUBLE_BANK);
//     (void)Usb_configure_endpoint(EP_AUDIO_OUT,
//                            EP_ATTRIBUTES_7,
//                            DIRECTION_OUT,
//                            EP_SIZE_7_FS,
//                            DOUBLE_BANK);

   }else{
     (void)Usb_configure_endpoint(EP_HID_TX,
                                  EP_ATTRIBUTES_4,
                                  DIRECTION_IN,
                                  EP_SIZE_4_HS,
                                  SINGLE_BANK);
     (void)Usb_configure_endpoint(EP_HID_RX,
                                   EP_ATTRIBUTES_5,
                                   DIRECTION_OUT,
                                   EP_SIZE_5_HS,
                                   SINGLE_BANK);
     (void)Usb_configure_endpoint(EP_CDC_COMM,
                                  EP_ATTRIBUTES_1,
                                  DIRECTION_IN,
                                  EP_SIZE_1_HS,
                                  SINGLE_BANK);
     (void)Usb_configure_endpoint(EP_CDC_TX,
                                  EP_ATTRIBUTES_2,
                                  DIRECTION_IN,
                                  EP_SIZE_2_HS,
                                  DOUBLE_BANK);
     (void)Usb_configure_endpoint(EP_CDC_RX,
                                  EP_ATTRIBUTES_3,
                                  DIRECTION_OUT,
                                  EP_SIZE_3_HS,
                                  DOUBLE_BANK);
     (void)Usb_configure_endpoint(EP_AUDIO_IN,
                             EP_ATTRIBUTES_6,
                             DIRECTION_IN,
                             EP_SIZE_6_HS,
                             DOUBLE_BANK);
//     (void)Usb_configure_endpoint(EP_AUDIO_OUT,
//                              EP_ATTRIBUTES_7,
//                              DIRECTION_OUT,
//                              EP_SIZE_7_HS,
//                              DOUBLE_BANK);

   }
}


//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
Bool usb_user_read_request(U8 type, U8 request)
{   int i;

   // Read wValue
   wValue_lsb = Usb_read_endpoint_data(EP_CONTROL, 8);
   wValue_msb = Usb_read_endpoint_data(EP_CONTROL, 8);
   wIndex = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
   wLength = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

   if (type == IN_CL_INTERFACE || type == OUT_CL_INTERFACE){    // process Class Specific Interface

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
		   } // end request
		   return FALSE;
	   }
   }  // end wIndex ==  HID Interface

   if (wIndex == DSC_INTERFACE_CDC_comm ){
	   switch (request)
	   {

		   case GET_LINE_CODING:
	       cdc_get_line_coding();
	       return TRUE;
	       // No need to break here !

		   case SET_LINE_CODING:
	       cdc_set_line_coding();
	       return TRUE;
	       // No need to break here !

	       case SET_CONTROL_LINE_STATE:
	       cdc_set_control_line_state();
	       return TRUE;
	       // No need to break here !
	       default:
	    	   return FALSE;
	   }
   } // end wIndex == CDC Interface

//  request for AUDIO interface

//   if ( (wIndex % 256) == DSC_INTERFACE_AUDIO){	// low byte wIndex is Interface number
												// high byte is for EntityID
	   if (TRUE){								// Temporary hack, as alsa 1.0.23 driver assumes Interface
											    // number 0 !!!
	   if (type == IN_CL_INTERFACE){			// get controls
		   switch (wIndex /256){
			   case CSD_ID_1:
				   if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
					  Usb_ack_setup_received_free();

					  Usb_reset_endpoint_fifo_access(EP_CONTROL);
					  Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[3]); // 0x0000bb80 is 48khz
					  Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[2]); // 0x00017700 is 96khz
					  Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[1]); // 0x0002ee00 is 192khz
					  Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[0]);
					  Usb_ack_control_in_ready_send();

					  while (!Is_usb_control_out_received());
					  Usb_ack_control_out_received_free();
					  return TRUE;
				   }
				   else if (wValue_msb == AUDIO_CS_CONTROL_CLOCK_VALID && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
					  Usb_ack_setup_received_free();

					  Usb_reset_endpoint_fifo_access(EP_CONTROL);
					  Usb_write_endpoint_data(EP_CONTROL, 8, TRUE);	// always valid
					  // temp hack to give total # of bytes requested
					  for (i = 0; i < (wLength - 1); i++)
						  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					  Usb_ack_control_in_ready_send();
					  while (!Is_usb_control_out_received());
					  Usb_ack_control_out_received_free();
					  return TRUE;
				   }
				   else if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_RANGE){
						  Usb_ack_setup_received_free();

						  Usb_reset_endpoint_fifo_access(EP_CONTROL);

						  // give total # of bytes requested
							  for (i = 0; i < (wLength); i++)
								  Usb_write_endpoint_data(EP_CONTROL, 8, Speedx[i]);
//							  LED_Toggle(LED0);
						  Usb_ack_control_in_ready_send();

						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;

				   } else return FALSE;

			   case CSD_ID_2:
				   if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
					  Usb_ack_setup_received_free();
					  Usb_reset_endpoint_fifo_access(EP_CONTROL);
					  Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[3]); // 0x0000bb80 is 48khz
					  Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[2]); // 0x00017700 is 96khz
					  Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[1]); // 0x0002ee00 is 192khz
					  Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[0]);
					  Usb_ack_control_in_ready_send();
					  while (!Is_usb_control_out_received());
					  Usb_ack_control_out_received_free();
					  return TRUE;
				   }
				   else if (wValue_msb == AUDIO_CS_CONTROL_CLOCK_VALID && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
					  Usb_ack_setup_received_free();
					  Usb_reset_endpoint_fifo_access(EP_CONTROL);
					  Usb_write_endpoint_data(EP_CONTROL, 8, TRUE);	// always valid
					  // temp hack to give total # of bytes requested
					  for (i = 0; i < (wLength - 1); i++)
						  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);

					  Usb_ack_control_in_ready_send();
					  while (!Is_usb_control_out_received());
					  Usb_ack_control_out_received_free();
					  return TRUE;
				   }
				   else if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_RANGE){
						  Usb_ack_setup_received_free();

						  Usb_reset_endpoint_fifo_access(EP_CONTROL);

						  // give total # of bytes requested
							  for (i = 0; i < (wLength); i++)
								  Usb_write_endpoint_data(EP_CONTROL, 8, Speedx[i]);
//							  LED_Toggle(LED0);
						  Usb_ack_control_in_ready_send();

						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;



				   } else return FALSE;


			   case CSX_ID:
				   if (wValue_msb == AUDIO_CX_CLOCK_SELECTOR && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
						  Usb_ack_setup_received_free();
						  Usb_reset_endpoint_fifo_access(EP_CONTROL);
						  Usb_write_endpoint_data(EP_CONTROL, 8, clock_selected);
						  // temp hack to give total # of bytes requested
						  for (i = 0; i < (wLength - 1); i++)
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						  Usb_ack_control_in_ready_send();
						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;
				   } else return FALSE;

			   case MIC_FEATURE_UNIT_ID:
				   if (wValue_msb == AUDIO_FU_CONTROL_CS_MUTE
						   && request == AUDIO_CS_REQUEST_CUR){
						  Usb_ack_setup_received_free();
						  Usb_reset_endpoint_fifo_access(EP_CONTROL);

						  Usb_write_endpoint_data(EP_CONTROL, 8, mute);
						  // temp hack to give total # of bytes requested
						  for (i = 0; i < (wLength - 1); i++)
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);

						  Usb_ack_control_in_ready_send();
						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;
				   } else return FALSE;

			   case INPUT_TERMINAL_ID:
				   if (wValue_msb == AUDIO_TE_CONTROL_CS_CLUSTER && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
						  Usb_ack_setup_received_free();
						  Usb_reset_endpoint_fifo_access(EP_CONTROL);
						  if (usb_alternate_setting == 1) {
							  Usb_write_endpoint_data(EP_CONTROL, 8, INPUT_TERMINAL_NB_CHANNELS);
							  Usb_write_endpoint_data(EP_CONTROL, 8, (U8) INPUT_TERMINAL_CHANNEL_CONF);
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							  Usb_write_endpoint_data(EP_CONTROL, 8, INPUT_TERMINAL_STRING_DESC);
						  }
						  else {			// zero's at startup alt setting 0
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						  };
						  Usb_ack_control_in_ready_send();
						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;
				   } else return FALSE;

			   default:
				   return FALSE;
		   } // end switch EntityID
	   } else if (type == OUT_CL_INTERFACE){		// set controls
		   switch (wIndex /256){
		   case CSD_ID_1:							// set CUR freq
		   case CSD_ID_2:
			   if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
					   && request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received());
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					current_freq.freq_bytes[3]=Usb_read_endpoint_data(EP_CONTROL, 8);		// read 4 bytes freq to set
					current_freq.freq_bytes[2]=Usb_read_endpoint_data(EP_CONTROL, 8);
					current_freq.freq_bytes[1]=Usb_read_endpoint_data(EP_CONTROL, 8);
					current_freq.freq_bytes[0]=Usb_read_endpoint_data(EP_CONTROL, 8);
					freq_changed = TRUE;
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
					while (!Is_usb_control_in_ready()); //!< waits for status phase done
					return TRUE;
			   }
			   else return FALSE;
		   case CSX_ID:
			   if (wValue_msb == AUDIO_CX_CLOCK_SELECTOR && wValue_lsb == 0
					   && request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received());
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						clock_selected = Usb_read_endpoint_data(EP_CONTROL, 8);
						clock_changed = TRUE;
						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready()); //!< waits for status phase done
						if (clock_selected < 1 || clock_selected > CSX_INPUT_PINS)
							clock_selected = 1;
						return TRUE;
			   } else return FALSE;
		   case MIC_FEATURE_UNIT_ID:
			   if (wValue_msb == AUDIO_FU_CONTROL_CS_MUTE
					   && request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received());
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					mute = Usb_read_endpoint_data(EP_CONTROL, 8);
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
					while (!Is_usb_control_in_ready()); //!< waits for status phase done
					return TRUE;
			   } else return FALSE;
		   default:
			   return FALSE;
		   }
	   } // end OUT_CL_INTERFACE

   } // end Audio Control Interface

   if (wIndex == DSC_INTERFACE_AS){
	   if (type == IN_CL_INTERFACE){			// get controls

				   if (wValue_msb == AUDIO_AS_VAL_ALT_SETTINGS && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
					  Usb_ack_setup_received_free();

					  Usb_reset_endpoint_fifo_access(EP_CONTROL);
					  Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					  Usb_write_endpoint_data(EP_CONTROL, 8, 0b00000011); // alt 0 and 1 valid
					  Usb_ack_control_in_ready_send();

					  while (!Is_usb_control_out_received());
					  Usb_ack_control_out_received_free();
					  return TRUE;
				   } else if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
						  Usb_ack_setup_received_free();
						  Usb_reset_endpoint_fifo_access(EP_CONTROL);
						  Usb_write_endpoint_data(EP_CONTROL, 8, usb_alternate_setting);
						  Usb_ack_control_in_ready_send();
						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;
				   } else if (wValue_msb == AUDIO_AS_AUDIO_DATA_FORMAT && wValue_lsb == 0
						   && request == AUDIO_CS_REQUEST_CUR){
						  Usb_ack_setup_received_free();
						  Usb_reset_endpoint_fifo_access(EP_CONTROL);
						  Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
						  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);	// only PCM format
						  Usb_ack_control_in_ready_send();
						  while (!Is_usb_control_out_received());
						  Usb_ack_control_out_received_free();
						  return TRUE;
				   } else return FALSE;

	   } else if (type == OUT_CL_INTERFACE){		// set controls
		   if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS
				   && request == AUDIO_CS_REQUEST_CUR){
				Usb_ack_setup_received_free();
				while (!Is_usb_control_out_received());
				Usb_reset_endpoint_fifo_access(EP_CONTROL);
				usb_alternate_setting = Usb_read_endpoint_data(EP_CONTROL, 8);
				usb_alternate_setting_changed = TRUE;
				Usb_ack_control_out_received_free();
				Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
				while (!Is_usb_control_in_ready()); //!< waits for status phase done
		   return FALSE;
		   }
	   } // end OUT_CL_INTERFACE
   } // end DSC_INTERFACE_AS
   } // end CL_INTERFACE

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

    case CLOCK_SOURCE_1_INDEX:
      data_to_transfer = sizeof(usb_user_clock_source_1);
      pbuffer = &usb_user_clock_source_1;
      break;

    case CLOCK_SOURCE_2_INDEX:
      data_to_transfer = sizeof(usb_user_clock_source_2);
      pbuffer = &usb_user_clock_source_2;
      break;

    case CLOCK_SELECTOR_INDEX:
       data_to_transfer = sizeof(usb_user_clock_selector);
       pbuffer = &usb_user_clock_selector;
       break;

    case WL_INDEX:
       data_to_transfer = sizeof(usb_user_wl);
       pbuffer = &usb_user_wl;
       break;

    case AIT_INDEX:
       data_to_transfer = sizeof(usb_user_ait);
       pbuffer = &usb_user_ait;
       break;

    case AOT_INDEX:
       data_to_transfer = sizeof(usb_user_aot);
       pbuffer = &usb_user_aot;
       break;

    case AIN_INDEX:
       data_to_transfer = sizeof(usb_user_ain);
       pbuffer = &usb_user_ain;
       break;

    case AIA_INDEX:
       data_to_transfer = sizeof(usb_user_aia);
       pbuffer = &usb_user_aia;
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


void cdc_get_line_coding(void)
{
   Usb_ack_setup_received_free();

   Usb_reset_endpoint_fifo_access(EP_CONTROL);
   Usb_write_endpoint_data(EP_CONTROL, 8, LSB0(line_coding.dwDTERate));
   Usb_write_endpoint_data(EP_CONTROL, 8, LSB1(line_coding.dwDTERate));
   Usb_write_endpoint_data(EP_CONTROL, 8, LSB2(line_coding.dwDTERate));
   Usb_write_endpoint_data(EP_CONTROL, 8, LSB3(line_coding.dwDTERate));
   Usb_write_endpoint_data(EP_CONTROL, 8, line_coding.bCharFormat);
   Usb_write_endpoint_data(EP_CONTROL, 8, line_coding.bParityType);
   Usb_write_endpoint_data(EP_CONTROL, 8, line_coding.bDataBits  );

   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());

   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}

void cdc_set_line_coding (void)
{
   Usb_ack_setup_received_free();

   while(!Is_usb_control_out_received());
   Usb_reset_endpoint_fifo_access(EP_CONTROL);

   LSB0(line_coding.dwDTERate) = Usb_read_endpoint_data(EP_CONTROL, 8);
   LSB1(line_coding.dwDTERate) = Usb_read_endpoint_data(EP_CONTROL, 8);
   LSB2(line_coding.dwDTERate) = Usb_read_endpoint_data(EP_CONTROL, 8);
   LSB3(line_coding.dwDTERate) = Usb_read_endpoint_data(EP_CONTROL, 8);
   line_coding.bCharFormat = Usb_read_endpoint_data(EP_CONTROL, 8);
   line_coding.bParityType = Usb_read_endpoint_data(EP_CONTROL, 8);
   line_coding.bDataBits = Usb_read_endpoint_data(EP_CONTROL, 8);
   Usb_ack_control_out_received_free();

   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());

   // Set the baudrate of the USART
   {
      static usart_options_t dbg_usart_options;
      U32 stopbits, parity;

      if     ( line_coding.bCharFormat==0 )   stopbits = USART_1_STOPBIT;
      else if( line_coding.bCharFormat==1 )   stopbits = USART_1_5_STOPBITS;
      else                                    stopbits = USART_2_STOPBITS;

      if     ( line_coding.bParityType==0 )   parity = USART_NO_PARITY;
      else if( line_coding.bParityType==1 )   parity = USART_ODD_PARITY;
      else if( line_coding.bParityType==2 )   parity = USART_EVEN_PARITY;
      else if( line_coding.bParityType==3 )   parity = USART_MARK_PARITY;
      else                                    parity = USART_SPACE_PARITY;

      // Options for debug USART.
      dbg_usart_options.baudrate    = line_coding.dwDTERate;
      dbg_usart_options.charlength  = line_coding.bDataBits;
      dbg_usart_options.paritytype  = parity;
      dbg_usart_options.stopbits    = stopbits;
      dbg_usart_options.channelmode = USART_NORMAL_CHMODE;

      // Initialize it in RS232 mode.
      usart_init_rs232(DBG_USART, &dbg_usart_options, pm_freq_param.pba_f);
   }
}

void cdc_set_control_line_state (void)
{
   Usb_ack_setup_received_free();
   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());
}



#endif  // USB_DEVICE_FEATURE == ENABLED
