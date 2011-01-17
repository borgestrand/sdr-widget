/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief USB identifiers.
 *
 * This file contains the USB parameters that uniquely identify the USB
 * application through descriptor tables.
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

//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"


#if USB_DEVICE_FEATURE == ENABLED

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "usb_audio.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

// usb_hid_report_descriptor
const U8 usb_hid_report_descriptor[USB_HID_REPORT_DESC] =
{
	  	0x06, 0xA0, 0xFF,	// Usage page (vendor defined)
	  	0x09, 0x01,	// Usage ID (vendor defined)
	  	0xA1, 0x01,	// Collection (application)

		// The Input report
        0x09, 0x03,     	// Usage ID - vendor defined
        0x15, 0x00,     	// Logical Minimum (0)
        0x26, 0xFF, 0x00,   // Logical Maximum (255)
        0x75, 0x08,     	// Report Size (8 bits)
        0x95, 0x02,     	// Report Count (2 fields)
        0x81, 0x02,     	// Input (Data, Variable, Absolute)

		// The Output report
        0x09, 0x04,     	// Usage ID - vendor defined
        0x15, 0x00,     	// Logical Minimum (0)
        0x26, 0xFF, 0x00,   // Logical Maximum (255)
        0x75, 0x08,     	// Report Size (8 bits)
        0x95, 0x02,     	// Report Count (2 fields)
        0x91, 0x02,      	// Output (Data, Variable, Absolute)

		// The Feature report
        0x09, 0x05,     	// Usage ID - vendor defined
        0x15, 0x00,     	// Logical Minimum (0)
        0x26, 0xFF, 0x00,   // Logical Maximum (255)
        0x75, 0x08,			// Report Size (8 bits)
        0x95, 0x02, 		// Report Count	(2 fields)
        0xB1, 0x02,     	// Feature (Data, Variable, Absolute)

	  	0xC0	// end collection

};



// usb_user_device_descriptor
const S_usb_device_descriptor usb_dev_desc =
{
  sizeof(S_usb_device_descriptor),
  DEVICE_DESCRIPTOR,
  Usb_format_mcu_to_usb_data(16, USB_SPECIFICATION),
  DEVICE_CLASS,
  DEVICE_SUB_CLASS,
  DEVICE_PROTOCOL,
  EP_CONTROL_LENGTH,
  Usb_format_mcu_to_usb_data(16, VENDOR_ID),
  Usb_format_mcu_to_usb_data(16, PRODUCT_ID),
  Usb_format_mcu_to_usb_data(16, RELEASE_NUMBER),
  MAN_INDEX,
  PROD_INDEX,
  SN_INDEX,
  NB_CONFIGURATION
};



// usb_user_configuration_descriptor FS
const S_usb_user_configuration_descriptor usb_conf_desc_fs =
{
  {
    sizeof(S_usb_configuration_descriptor),
    CONFIGURATION_DESCRIPTOR,
    Usb_format_mcu_to_usb_data(16, sizeof(S_usb_user_configuration_descriptor)),
    NB_INTERFACE,
    CONF_NB,
    CONF_INDEX,
    CONF_ATTRIBUTES,
    MAX_POWER
  },

	{
	sizeof(S_usb_interface_descriptor),
	INTERFACE_DESCRIPTOR,
	INTERFACE_NB0,
	ALTERNATE_NB0,
	NB_ENDPOINT0,
	INTERFACE_CLASS0,
	INTERFACE_SUB_CLASS0,
	INTERFACE_PROTOCOL0,
	INTERFACE_INDEX0
	}
,
	{ sizeof(S_usb_interface_association_descriptor)
	,  DESCRIPTOR_IAD
	,  FIRST_INTERFACE					// bFirstInterface
	,  INTERFACE_COUNT 				// bInterfaceCount
	,  INTERFACE_CLASS1
	,  INTERFACE_SUB_CLASS1
	,  INTERFACE_PROTOCOL1
	,  INTERFACE_INDEX1
   },

	{
	sizeof(S_usb_interface_descriptor),
	INTERFACE_DESCRIPTOR,
    INTERFACE_NB1,
    ALTERNATE_NB1,
    NB_ENDPOINT1,
    INTERFACE_CLASS1,
    INTERFACE_SUB_CLASS1,
    INTERFACE_PROTOCOL1,
    INTERFACE_INDEX1
  },
	{
	sizeof(S_usb_cdc_header_func_descriptor),
	DSC_TYPE_CS_INTERFACE,					// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CDC_HEADER_FUNC,			// bDescriptorSubtype (Header Functional)
	Usb_format_mcu_to_usb_data(16,0x0110)	// bcdCDC (CDC spec release number, 1.1
	},
 {										// Call Management Functional Descriptor
	sizeof(S_usb_cdc_call_man_func_descriptor),		// bLength
	DSC_TYPE_CS_INTERFACE,					// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CDC_CALL_MAN,			// bDescriptorSubtype (Call Management)
	0x02,									// bmCapabilities (only over Communication Class IF / handles itself)
	DSC_INTERFACE_CDC_data,					// bDataInterface (Interface number of Data Class interface)
  },
  {										// Abstract Control Management Functional Descriptor
	sizeof(S_usb_cdc_abst_control_mana_descriptor),	// bLength
	DSC_TYPE_CS_INTERFACE,					// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CDC_ABST_CNTRL,			// bDescriptorSubtype (Abstract Control Management)
	0x06,									// bmCapabilities (Supports Send_Break, Set_Line_Coding, Set_Control_Line_State,
											// Get_Line_Coding, and the notification Serial_State)
  },

  {
	sizeof(S_usb_cdc_union_func_descriptor),			// bLength
	DSC_TYPE_CS_INTERFACE,					// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CDC_UNION_FUNC,			// bDescriptorSubtype (Union Functional)
	DSC_INTERFACE_CDC_comm,					// bMasterInterface (Interface number master interface in the union)
	DSC_INTERFACE_CDC_data,					// bSlaveInterface0 (Interface number slave interface in the union)
  },


  {
    sizeof(S_usb_endpoint_descriptor),
    ENDPOINT_DESCRIPTOR,
    ENDPOINT_NB_1,
    EP_ATTRIBUTES_1,
    Usb_format_mcu_to_usb_data(16, EP_SIZE_1_FS),
    EP_INTERVAL_1
  },

  {
     sizeof(S_usb_interface_descriptor),
     INTERFACE_DESCRIPTOR,
     INTERFACE_NB2,
     ALTERNATE_NB2,
     NB_ENDPOINT2,
     INTERFACE_CLASS2,
     INTERFACE_SUB_CLASS2,
     INTERFACE_PROTOCOL2,
     INTERFACE_INDEX2
   },

  {
    sizeof(S_usb_endpoint_descriptor),
    ENDPOINT_DESCRIPTOR,
    ENDPOINT_NB_2,
    EP_ATTRIBUTES_2,
    Usb_format_mcu_to_usb_data(16, EP_SIZE_2_FS),
    EP_INTERVAL_2
  },
  {
    sizeof(S_usb_endpoint_descriptor),
    ENDPOINT_DESCRIPTOR,
    ENDPOINT_NB_3,
    EP_ATTRIBUTES_3,
    Usb_format_mcu_to_usb_data(16, EP_SIZE_3_FS),
    EP_INTERVAL_3
  },

  {
    sizeof(S_usb_interface_descriptor),
    INTERFACE_DESCRIPTOR,
    INTERFACE_NB3,
    ALTERNATE_NB3,
    NB_ENDPOINT3,
    INTERFACE_CLASS3,
    INTERFACE_SUB_CLASS3,
    INTERFACE_PROTOCOL3,
    INTERFACE_INDEX3
  },

  {
    sizeof(S_usb_hid_descriptor),
    HID_DESCRIPTOR,
    Usb_format_mcu_to_usb_data(16, HID_VERSION),
    HID_COUNTRY_CODE,
    HID_NUM_DESCRIPTORS,
    HID_REPORT_DESCRIPTOR,
    Usb_format_mcu_to_usb_data(16, sizeof(usb_hid_report_descriptor))
  },

  {
    sizeof(S_usb_endpoint_descriptor),
    ENDPOINT_DESCRIPTOR,
    ENDPOINT_NB_4,
    EP_ATTRIBUTES_4,
    Usb_format_mcu_to_usb_data(16, EP_SIZE_4_FS),
    EP_INTERVAL_4
  },
  {
    sizeof(S_usb_endpoint_descriptor),
    ENDPOINT_DESCRIPTOR,
    ENDPOINT_NB_5,
    EP_ATTRIBUTES_5,
    Usb_format_mcu_to_usb_data(16, EP_SIZE_5_FS),
    EP_INTERVAL_5
  }
  ,

	{ sizeof(S_usb_interface_association_descriptor)
	,  DESCRIPTOR_IAD
	,  FIRST_INTERFACE1					// bFirstInterface
	,  INTERFACE_COUNT1 				// bInterfaceCount
	,  INTERFACE_CLASS4
	,  INTERFACE_SUB_CLASS4
	,  INTERFACE_PROTOCOL4
	,  INTERFACE_INDEX4
	},

  {  sizeof(S_usb_interface_descriptor)
    ,  INTERFACE_DESCRIPTOR
    ,  INTERFACE_NB4
    ,  ALTERNATE_NB4
    ,  NB_ENDPOINT4
    ,  INTERFACE_CLASS4
    ,  INTERFACE_SUB_CLASS4
    ,  INTERFACE_PROTOCOL4
    ,  INTERFACE_INDEX4
    }
  ,


    {  sizeof(S_usb_ac_interface_descriptor)
    ,  CS_INTERFACE
    ,  HEADER_SUB_TYPE
    ,  Usb_format_mcu_to_usb_data(16, AUDIO_CLASS_REVISION_2)
    ,  MIC_CATEGORY
    ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor)
    		+ 2*sizeof(S_usb_clock_source_descriptor) + sizeof(S_usb_clock_selector_descriptor)
    		+sizeof(S_usb_mic_in_ter_descriptor)
            +sizeof(S_usb_feature_unit_descriptor)+sizeof(S_usb_mic_out_ter_descriptor))
    ,  MIC_LATENCY_CONTROL
    }
 , {  sizeof (S_usb_clock_source_descriptor)
    ,  CS_INTERFACE
    ,  DESCRIPTOR_SUBTYPE_AUDIO_AC_CLOCK_SOURCE
    ,  CSD_ID_1
    ,  CSD_ID_1_TYPE
    ,  CSD_ID_1_CONTROL
    ,  INPUT_TERMINAL_ID
    ,  0x00
    }
  , {  sizeof (S_usb_clock_source_descriptor)
     ,  CS_INTERFACE
     ,  DESCRIPTOR_SUBTYPE_AUDIO_AC_CLOCK_SOURCE
     ,  CSD_ID_2
     ,  CSD_ID_2_TYPE
     ,  CSD_ID_2_CONTROL
     ,  INPUT_TERMINAL_ID
     ,  0x00
     }
  ,

  {  sizeof (S_usb_clock_selector_descriptor)
    ,  CS_INTERFACE
    ,  DESCRIPTOR_SUBTYPE_AUDIO_AC_CLOCK_SELECTOR
    ,  CSX_ID
    ,  CSX_INPUT_PINS
    ,  CSX_SOURCE_1
    ,  CSX_SOURCE_2
    ,  CSX_CONTROL
    ,  0x00
    }
  ,
    {  sizeof(S_usb_mic_in_ter_descriptor)
    ,  CS_INTERFACE
    ,  INPUT_TERMINAL_SUB_TYPE
    ,  INPUT_TERMINAL_ID
    ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_TYPE)
    ,  INPUT_TERMINAL_ASSOCIATION
    ,  CSX_ID
    ,  INPUT_TERMINAL_NB_CHANNELS
    ,  Usb_format_mcu_to_usb_data(32, INPUT_TERMINAL_CHANNEL_CONF)
    ,  INPUT_TERMINAL_CH_NAME_ID
    ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_CONTROLS)
    ,  INPUT_TERMINAL_STRING_DESC
    }
 ,
    {  sizeof(S_usb_feature_unit_descriptor)
    ,  CS_INTERFACE
    ,  FEATURE_UNIT_SUB_TYPE
    ,  MIC_FEATURE_UNIT_ID
    ,  MIC_FEATURE_UNIT_SOURCE_ID
    ,  Usb_format_mcu_to_usb_data(32, MIC_BMA_CONTROLS)
    ,  Usb_format_mcu_to_usb_data(32, MIC_BMA_CONTROLS_CH_1)
    ,  Usb_format_mcu_to_usb_data(32, MIC_BMA_CONTROLS_CH_2)
    ,  0x00
    }
 ,
    {  sizeof(S_usb_mic_out_ter_descriptor)
    ,  CS_INTERFACE
    ,  OUTPUT_TERMINAL_SUB_TYPE
    ,  OUTPUT_TERMINAL_ID
    ,  Usb_format_mcu_to_usb_data(16, OUTPUT_TERMINAL_TYPE)
    ,  OUTPUT_TERMINAL_ASSOCIATION
    ,  OUTPUT_TERMINAL_SOURCE_ID
    ,  CSX_ID
    ,  Usb_format_mcu_to_usb_data(16,OUTPUT_TERMINAL_CONTROLS)
    ,  0x00
    }
 ,
    {  sizeof(S_usb_as_interface_descriptor)
    ,  INTERFACE_DESCRIPTOR
    ,  STD_AS_INTERFACE_NB
    ,  ALT0_AS_INTERFACE_INDEX
    ,  ALT0_AS_NB_ENDPOINT
    ,  ALT0_AS_INTERFACE_CLASS
    ,  ALT0_AS_INTERFACE_SUB_CLASS
    ,  ALT0_AS_INTERFACE_PROTOCOL
    ,  0x00
    }
 ,
    {  sizeof(S_usb_as_interface_descriptor)
    ,  INTERFACE_DESCRIPTOR
    ,  STD_AS_INTERFACE_NB
    ,  ALT1_AS_INTERFACE_INDEX
    ,  ALT1_AS_NB_ENDPOINT
    ,  ALT1_AS_INTERFACE_CLASS
    ,  ALT1_AS_INTERFACE_SUB_CLASS
    ,  ALT1_AS_INTERFACE_PROTOCOL
    ,  0x00
    }
 ,
    {  sizeof(S_usb_as_g_interface_descriptor)
    ,  CS_INTERFACE
    ,  GENERAL_SUB_TYPE
    ,  AS_TERMINAL_LINK
    ,  AS_CONTROLS
    ,  AS_FORMAT_TYPE
    ,  Usb_format_mcu_to_usb_data(32, AS_FORMATS)
    ,  AS_NB_CHANNELS
    ,  Usb_format_mcu_to_usb_data(32,AS_CHAN_CONFIG)
    ,  0x00
    }
 ,
    {  sizeof(S_usb_format_type)
    ,  CS_INTERFACE
    ,  FORMAT_SUB_TYPE
    ,  FORMAT_TYPE_1
    ,  FORMAT_SUBSLOT_SIZE_1
    ,  FORMAT_BIT_RESOLUTION_1
    }
  ,
      {   sizeof(S_usb_endpoint_audio_descriptor)
      ,   ENDPOINT_DESCRIPTOR
      ,   ENDPOINT_NB_6
      ,   EP_ATTRIBUTES_6
      ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_6_FS)
      ,   EP_INTERVAL_6_FS
      }
   ,
      {  sizeof(S_usb_endpoint_audio_specific)
      ,  CS_ENDPOINT
      ,  GENERAL_SUB_TYPE
      ,  AUDIO_EP_ATRIBUTES
      ,  AUDIO_EP_DELAY_UNIT
      ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
      }
  ,
     {  sizeof(S_usb_as_interface_descriptor)
     ,  INTERFACE_DESCRIPTOR
     ,  STD_AS_INTERFACE_OUT_NB
     ,  ALT0_AS_INTERFACE_INDEX
     ,  ALT0_AS_NB_ENDPOINT
     ,  ALT0_AS_INTERFACE_CLASS
     ,  ALT0_AS_INTERFACE_SUB_CLASS
     ,  ALT0_AS_INTERFACE_PROTOCOL
     ,  0x00
     }
  ,
     {  sizeof(S_usb_as_interface_descriptor)
     ,  INTERFACE_DESCRIPTOR
     ,  STD_AS_INTERFACE_OUT_NB
     ,  ALT1_AS_INTERFACE_INDEX
     ,  ALT1_AS_NB_ENDPOINT
     ,  ALT1_AS_INTERFACE_CLASS
     ,  ALT1_AS_INTERFACE_SUB_CLASS
     ,  ALT1_AS_INTERFACE_PROTOCOL
     ,  0x00
     }
  ,
     {  sizeof(S_usb_as_g_interface_descriptor)
     ,  CS_INTERFACE
     ,  GENERAL_SUB_TYPE
     ,  SPK_INPUT_TERMINAL_ID
     ,  AS_CONTROLS
     ,  AS_FORMAT_TYPE
     ,  Usb_format_mcu_to_usb_data(32, AS_FORMATS)
     ,  AS_NB_CHANNELS
     ,  Usb_format_mcu_to_usb_data(32,AS_CHAN_CONFIG)
     ,  0x00
     }
  ,
     {  sizeof(S_usb_format_type)
     ,  CS_INTERFACE
     ,  FORMAT_SUB_TYPE
     ,  FORMAT_TYPE_1
     ,  FORMAT_SUBSLOT_SIZE_1
     ,  FORMAT_BIT_RESOLUTION_1
     }
   ,
       {   sizeof(S_usb_endpoint_audio_descriptor)
       ,   ENDPOINT_DESCRIPTOR
       ,   ENDPOINT_NB_7
       ,   EP_ATTRIBUTES_7
       ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_7_FS)
       ,   EP_INTERVAL_7_FS
       }
    ,
       {  sizeof(S_usb_endpoint_audio_specific)
       ,  CS_ENDPOINT
       ,  GENERAL_SUB_TYPE
       ,  AUDIO_EP_ATRIBUTES
       ,  AUDIO_EP_DELAY_UNIT
       ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
       }
};


#if (USB_HIGH_SPEED_SUPPORT==ENABLED)

// usb_user_configuration_descriptor HS
const S_usb_user_configuration_descriptor usb_conf_desc_hs =
{
  {
    sizeof(S_usb_configuration_descriptor),
    CONFIGURATION_DESCRIPTOR,
    Usb_format_mcu_to_usb_data(16, sizeof(S_usb_user_configuration_descriptor)),
    NB_INTERFACE,
    CONF_NB,
    CONF_INDEX,
    CONF_ATTRIBUTES,
    MAX_POWER
  },
	{
	sizeof(S_usb_interface_descriptor),
	INTERFACE_DESCRIPTOR,
  INTERFACE_NB0,
  ALTERNATE_NB0,
  NB_ENDPOINT0,
  INTERFACE_CLASS0,
  INTERFACE_SUB_CLASS0,
  INTERFACE_PROTOCOL0,
  INTERFACE_INDEX0
},
	{ sizeof(S_usb_interface_association_descriptor)
	,  DESCRIPTOR_IAD
	,  FIRST_INTERFACE					// bFirstInterface
	,  INTERFACE_COUNT 				// bInterfaceCount
	,  INTERFACE_CLASS0
	,  INTERFACE_SUB_CLASS0
	,  INTERFACE_PROTOCOL0
	,  INTERFACE_INDEX0
   },

	{
	sizeof(S_usb_interface_descriptor),
	INTERFACE_DESCRIPTOR,
	INTERFACE_NB1,
	ALTERNATE_NB1,
	NB_ENDPOINT1,
	INTERFACE_CLASS1,
	INTERFACE_SUB_CLASS1,
	INTERFACE_PROTOCOL1,
	INTERFACE_INDEX1
},
	{
	sizeof(S_usb_cdc_header_func_descriptor),
	DSC_TYPE_CS_INTERFACE,					// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CDC_HEADER_FUNC,			// bDescriptorSubtype (Header Functional)
	Usb_format_mcu_to_usb_data(16,0x0110)	// bcdCDC (CDC spec release number, 1.1
	},
{										// Call Management Functional Descriptor
	sizeof(S_usb_cdc_call_man_func_descriptor),		// bLength
	DSC_TYPE_CS_INTERFACE,					// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CDC_CALL_MAN,			// bDescriptorSubtype (Call Management)
	0x02,									// bmCapabilities (only over Communication Class IF / handles itself)
	DSC_INTERFACE_CDC_data,					// bDataInterface (Interface number of Data Class interface)
},
{										// Abstract Control Management Functional Descriptor
	sizeof(S_usb_cdc_abst_control_mana_descriptor),	// bLength
	DSC_TYPE_CS_INTERFACE,					// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CDC_ABST_CNTRL,			// bDescriptorSubtype (Abstract Control Management)
	0x06,									// bmCapabilities (Supports Send_Break, Set_Line_Coding, Set_Control_Line_State,
											// Get_Line_Coding, and the notification Serial_State)
},

{
	sizeof(S_usb_cdc_union_func_descriptor),			// bLength
	DSC_TYPE_CS_INTERFACE,					// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CDC_UNION_FUNC,			// bDescriptorSubtype (Union Functional)
	DSC_INTERFACE_CDC_comm,					// bMasterInterface (Interface number master interface in the union)
	DSC_INTERFACE_CDC_data,					// bSlaveInterface0 (Interface number slave interface in the union)
},

{
  sizeof(S_usb_endpoint_descriptor),
  ENDPOINT_DESCRIPTOR,
  ENDPOINT_NB_1,
  EP_ATTRIBUTES_1,
  Usb_format_mcu_to_usb_data(16, EP_SIZE_1_HS),
  EP_INTERVAL_1
},

{
    sizeof(S_usb_interface_descriptor),
    INTERFACE_DESCRIPTOR,
    INTERFACE_NB2,
    ALTERNATE_NB2,
    NB_ENDPOINT2,
    INTERFACE_CLASS2,
    INTERFACE_SUB_CLASS2,
    INTERFACE_PROTOCOL2,
    INTERFACE_INDEX2
  },

  {
    sizeof(S_usb_endpoint_descriptor),
    ENDPOINT_DESCRIPTOR,
    ENDPOINT_NB_2,
    EP_ATTRIBUTES_2,
    Usb_format_mcu_to_usb_data(16, EP_SIZE_2_HS),
    EP_INTERVAL_2
  },
  {
    sizeof(S_usb_endpoint_descriptor),
    ENDPOINT_DESCRIPTOR,
    ENDPOINT_NB_3,
    EP_ATTRIBUTES_3,
    Usb_format_mcu_to_usb_data(16, EP_SIZE_3_HS),
    EP_INTERVAL_3
  },

  {
    sizeof(S_usb_interface_descriptor),
    INTERFACE_DESCRIPTOR,
    INTERFACE_NB3,
    ALTERNATE_NB3,
    NB_ENDPOINT3,
    INTERFACE_CLASS3,
    INTERFACE_SUB_CLASS3,
    INTERFACE_PROTOCOL3,
    INTERFACE_INDEX3
  },

  {
    sizeof(S_usb_hid_descriptor),
    HID_DESCRIPTOR,
    Usb_format_mcu_to_usb_data(16, HID_VERSION),
    HID_COUNTRY_CODE,
    HID_NUM_DESCRIPTORS,
    HID_REPORT_DESCRIPTOR,
    Usb_format_mcu_to_usb_data(16, sizeof(usb_hid_report_descriptor))
  },

  {
    sizeof(S_usb_endpoint_descriptor),
    ENDPOINT_DESCRIPTOR,
    ENDPOINT_NB_4,
    EP_ATTRIBUTES_4,
    Usb_format_mcu_to_usb_data(16, EP_SIZE_4_HS),
    EP_INTERVAL_4
  },
  {
     sizeof(S_usb_endpoint_descriptor),
     ENDPOINT_DESCRIPTOR,
     ENDPOINT_NB_5,
     EP_ATTRIBUTES_5,
     Usb_format_mcu_to_usb_data(16, EP_SIZE_5_HS),
     EP_INTERVAL_5
   }
//! Here is where Audio Class 2 specific stuff is
,
	{ sizeof(S_usb_interface_association_descriptor) // 4.6
	,  DESCRIPTOR_IAD
	,  FIRST_INTERFACE1					// bFirstInterface
	,  INTERFACE_COUNT1 				// bInterfaceCount
	,  FUNCTION_CLASS
	,  FUNCTION_SUB_CLASS
	,  FUNCTION_PROTOCOL
	,  FUNCTION_INDEX
},

  {  sizeof(S_usb_interface_descriptor) //4.7
    ,  INTERFACE_DESCRIPTOR
    ,  INTERFACE_NB4
    ,  ALTERNATE_NB4
    ,  NB_ENDPOINT4
    ,  INTERFACE_CLASS4
    ,  INTERFACE_SUB_CLASS4
    ,  INTERFACE_PROTOCOL4
    , WL_INDEX
  }
  ,

  {  sizeof(S_usb_ac_interface_descriptor)
   ,  CS_INTERFACE
   ,  HEADER_SUB_TYPE
   ,  Usb_format_mcu_to_usb_data(16, AUDIO_CLASS_REVISION_2)
   ,  MIC_CATEGORY
   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor)
   		+ 2*sizeof(S_usb_clock_source_descriptor) + sizeof(S_usb_clock_selector_descriptor)
   		+sizeof(S_usb_mic_in_ter_descriptor)
           +sizeof(S_usb_feature_unit_descriptor)+sizeof(S_usb_mic_out_ter_descriptor))
   ,  MIC_LATENCY_CONTROL
   }

, {  sizeof (S_usb_clock_source_descriptor)
  ,  CS_INTERFACE
  ,  DESCRIPTOR_SUBTYPE_AUDIO_AC_CLOCK_SOURCE
  ,  CSD_ID_1
  ,  CSD_ID_1_TYPE
  ,  CSD_ID_1_CONTROL
  ,  OUTPUT_TERMINAL_ID
  ,  CLOCK_SOURCE_1_INDEX
  }

  , {  sizeof (S_usb_clock_source_descriptor)
    ,  CS_INTERFACE
    ,  DESCRIPTOR_SUBTYPE_AUDIO_AC_CLOCK_SOURCE
    ,  CSD_ID_2
    ,  CSD_ID_2_TYPE
    ,  CSD_ID_2_CONTROL
    ,  OUTPUT_TERMINAL_ID
    ,  CLOCK_SOURCE_2_INDEX
    }
,
{  sizeof (S_usb_clock_selector_descriptor)
  ,  CS_INTERFACE
  ,  DESCRIPTOR_SUBTYPE_AUDIO_AC_CLOCK_SELECTOR
  ,  CSX_ID
  ,  CSX_INPUT_PINS
  ,  CSX_SOURCE_1
  ,  CSX_SOURCE_2
  ,  CSX_CONTROL
  ,  CLOCK_SELECTOR_INDEX
  }
,
  {  sizeof(S_usb_mic_in_ter_descriptor)
  ,  CS_INTERFACE
  ,  INPUT_TERMINAL_SUB_TYPE
  ,  INPUT_TERMINAL_ID
  ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_TYPE)
  ,  INPUT_TERMINAL_ASSOCIATION
  ,  CSX_ID
  ,  INPUT_TERMINAL_NB_CHANNELS
  ,  Usb_format_mcu_to_usb_data(32, INPUT_TERMINAL_CHANNEL_CONF)
  ,  INPUT_TERMINAL_CH_NAME_ID
  ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_CONTROLS)
  , AIT_INDEX
  }
 ,
    {  sizeof(S_usb_feature_unit_descriptor)
    ,  CS_INTERFACE
    ,  FEATURE_UNIT_SUB_TYPE
    ,  MIC_FEATURE_UNIT_ID
    ,  MIC_FEATURE_UNIT_SOURCE_ID
    ,  Usb_format_mcu_to_usb_data(32, MIC_BMA_CONTROLS)
    ,  Usb_format_mcu_to_usb_data(32, MIC_BMA_CONTROLS_CH_1)
    ,  Usb_format_mcu_to_usb_data(32, MIC_BMA_CONTROLS_CH_2)
    ,  0x00   //iFeature
    }
 ,
    {  sizeof(S_usb_mic_out_ter_descriptor)
    ,  CS_INTERFACE
    ,  OUTPUT_TERMINAL_SUB_TYPE
    ,  OUTPUT_TERMINAL_ID
    ,  Usb_format_mcu_to_usb_data(16, OUTPUT_TERMINAL_TYPE)
    ,  OUTPUT_TERMINAL_ASSOCIATION
    ,  OUTPUT_TERMINAL_SOURCE_ID
    ,  CSX_ID
    ,  Usb_format_mcu_to_usb_data(16, OUTPUT_TERMINAL_CONTROLS)
    , AOT_INDEX
    }
 ,
    {  sizeof(S_usb_as_interface_descriptor)
    ,  INTERFACE_DESCRIPTOR
    ,  STD_AS_INTERFACE_NB
    ,  ALT0_AS_INTERFACE_INDEX
    ,  ALT0_AS_NB_ENDPOINT
    ,  ALT0_AS_INTERFACE_CLASS
    ,  ALT0_AS_INTERFACE_SUB_CLASS
    ,  ALT0_AS_INTERFACE_PROTOCOL
    , AIN_INDEX
    }
 ,
    {  sizeof(S_usb_as_interface_descriptor)
    ,  INTERFACE_DESCRIPTOR
    ,  STD_AS_INTERFACE_NB
    ,  ALT1_AS_INTERFACE_INDEX
    ,  ALT1_AS_NB_ENDPOINT
    ,  ALT1_AS_INTERFACE_CLASS
    ,  ALT1_AS_INTERFACE_SUB_CLASS
    ,  ALT1_AS_INTERFACE_PROTOCOL
    , AIA_INDEX
    }
 ,
    {  sizeof(S_usb_as_g_interface_descriptor)
    ,  CS_INTERFACE
    ,  GENERAL_SUB_TYPE
    ,  AS_TERMINAL_LINK
    ,  AS_CONTROLS
    ,  AS_FORMAT_TYPE
    ,  Usb_format_mcu_to_usb_data(32, AS_FORMATS)
    ,  AS_NB_CHANNELS
    ,  Usb_format_mcu_to_usb_data(32, AS_CHAN_CONFIG)
    }
 ,
    {  sizeof(S_usb_format_type)
    ,  CS_INTERFACE
    ,  FORMAT_SUB_TYPE
    ,  FORMAT_TYPE_1
    ,  FORMAT_SUBSLOT_SIZE_1
    ,  FORMAT_BIT_RESOLUTION_1

    }
 ,
    {   sizeof(S_usb_endpoint_audio_descriptor)
    ,   ENDPOINT_DESCRIPTOR
    ,   ENDPOINT_NB_6
    ,   EP_ATTRIBUTES_6
    ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_6_HS)
    ,   EP_INTERVAL_6_HS
    }
 ,
    {  sizeof(S_usb_endpoint_audio_specific)
    ,  CS_ENDPOINT
    ,  GENERAL_SUB_TYPE
    ,  AUDIO_EP_ATRIBUTES
    ,  AUDIO_EP_CONTROLS
    ,  AUDIO_EP_DELAY_UNIT
    ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
    }
  ,
      {  sizeof(S_usb_as_interface_descriptor)
      ,  INTERFACE_DESCRIPTOR
      ,  STD_AS_INTERFACE_OUT_NB
      ,  ALT0_AS_INTERFACE_INDEX
      ,  ALT0_AS_NB_ENDPOINT
      ,  ALT0_AS_INTERFACE_CLASS
      ,  ALT0_AS_INTERFACE_SUB_CLASS
      ,  ALT0_AS_INTERFACE_PROTOCOL
      ,  0x00
      }
   ,
      {  sizeof(S_usb_as_interface_descriptor)
      ,  INTERFACE_DESCRIPTOR
      ,  STD_AS_INTERFACE_OUT_NB
      ,  ALT1_AS_INTERFACE_INDEX
      ,  ALT1_AS_NB_ENDPOINT
      ,  ALT1_AS_INTERFACE_CLASS
      ,  ALT1_AS_INTERFACE_SUB_CLASS
      ,  ALT1_AS_INTERFACE_PROTOCOL
      ,  0x00
      }
   ,
      {  sizeof(S_usb_as_g_interface_descriptor)
      ,  CS_INTERFACE
      ,  GENERAL_SUB_TYPE
      ,  SPK_INPUT_TERMINAL_ID
      ,  AS_CONTROLS
      ,  AS_FORMAT_TYPE
      ,  Usb_format_mcu_to_usb_data(32, AS_FORMATS)
      ,  AS_NB_CHANNELS
      ,  Usb_format_mcu_to_usb_data(32,AS_CHAN_CONFIG)
      ,  0x00
      }
   ,
      {  sizeof(S_usb_format_type)
      ,  CS_INTERFACE
      ,  FORMAT_SUB_TYPE
      ,  FORMAT_TYPE_1
      ,  FORMAT_SUBSLOT_SIZE_1
      ,  FORMAT_BIT_RESOLUTION_1
      }
    ,
        {   sizeof(S_usb_endpoint_audio_descriptor)
        ,   ENDPOINT_DESCRIPTOR
        ,   ENDPOINT_NB_7
        ,   EP_ATTRIBUTES_7
        ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_7_HS)
        ,   EP_INTERVAL_7_HS
        }
     ,
        {  sizeof(S_usb_endpoint_audio_specific)
        ,  CS_ENDPOINT
        ,  GENERAL_SUB_TYPE
        ,  AUDIO_EP_ATRIBUTES
        ,  AUDIO_EP_DELAY_UNIT
        ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
        }
};


// usb_qualifier_desc FS
const S_usb_device_qualifier_descriptor usb_qualifier_desc =
{
  sizeof(S_usb_device_qualifier_descriptor),
  DEVICE_QUALIFIER_DESCRIPTOR,
  Usb_format_mcu_to_usb_data(16, USB_SPECIFICATION),
  DEVICE_CLASS,
  DEVICE_SUB_CLASS,
  DEVICE_PROTOCOL,
  EP_CONTROL_LENGTH,
  NB_CONFIGURATION,
  0
};
#endif


// usb_user_language_id
const S_usb_language_id usb_user_language_id =
{
  sizeof(S_usb_language_id),
  STRING_DESCRIPTOR,
  Usb_format_mcu_to_usb_data(16, LANGUAGE_ID)
};


// usb_user_manufacturer_string_descriptor
const S_usb_manufacturer_string_descriptor usb_user_manufacturer_string_descriptor =
{
  sizeof(S_usb_manufacturer_string_descriptor),
  STRING_DESCRIPTOR,
  USB_MANUFACTURER_NAME
};


// usb_user_product_string_descriptor
const S_usb_product_string_descriptor usb_user_product_string_descriptor =
{
  sizeof(S_usb_product_string_descriptor),
  STRING_DESCRIPTOR,
  USB_PRODUCT_NAME
};


// usb_user_serial_number
const S_usb_serial_number usb_user_serial_number =
{
  sizeof(S_usb_serial_number),
  STRING_DESCRIPTOR,
  USB_SERIAL_NUMBER
};

//usb_user_clock_source_1
const S_usb_clock_source_1 usb_user_clock_source_1 =
{
	sizeof(S_usb_clock_source_1),
	STRING_DESCRIPTOR,
	USB_CLOCK_SOURCE_1
};


//usb_user_clock_source_2
const S_usb_clock_source_2 usb_user_clock_source_2 =
{
	sizeof(S_usb_clock_source_2),
	STRING_DESCRIPTOR,
	USB_CLOCK_SOURCE_2
};


//usb_user_clock_selector
const S_usb_clock_selector usb_user_clock_selector =
{
	sizeof(S_usb_clock_selector),
	STRING_DESCRIPTOR,
	USB_CLOCK_SELECTOR
};

//usb_user_wl
const S_usb_wl usb_user_wl =
{
	sizeof(S_usb_wl),
	STRING_DESCRIPTOR,
	USB_WL
};

//usb_user_ait
const S_usb_ait usb_user_ait =
{
	sizeof(S_usb_ait),
	STRING_DESCRIPTOR,
	USB_AIT
};

//usb_user_aot
const S_usb_aot usb_user_aot =
{
	sizeof(S_usb_aot),
	STRING_DESCRIPTOR,
	USB_AOT
};

//usb_user_ain
const S_usb_ain usb_user_ain =
{
	sizeof(S_usb_ain),
	STRING_DESCRIPTOR,
	USB_AIN
};

//usb_user_aia
const S_usb_aia usb_user_aia =
{
	sizeof(S_usb_aia),
	STRING_DESCRIPTOR,
	USB_AIA
};

#endif  // USB_DEVICE_FEATURE == ENABLED
