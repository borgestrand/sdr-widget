/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
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
//#include "features.h"

#if USB_DEVICE_FEATURE == ENABLED

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "uac1_usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "usb_audio.h"
//#include "hid.h"

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


// usb_user_device_descriptor
const S_usb_device_descriptor uac1_dg8saq_usb_dev_desc =
{
	sizeof(S_usb_device_descriptor),
	DEVICE_DESCRIPTOR,
	// BSB Changed 20110901 according to mail from Roger
	Usb_format_mcu_to_usb_data(16, USB_1_1_SPECIFICATION),
	//	Usb_format_mcu_to_usb_data(16, USB_SPECIFICATION),
	DEVICE_CLASS,
	DEVICE_SUB_CLASS,
	DEVICE_PROTOCOL,
	EP_CONTROL_LENGTH,
	Usb_format_mcu_to_usb_data(16, DG8SAQ_VENDOR_ID),
	Usb_format_mcu_to_usb_data(16, DG8SAQ_PRODUCT_ID),
	Usb_format_mcu_to_usb_data(16, RELEASE_NUMBER),
	MAN_INDEX,
	PROD_INDEX,
	SN_INDEX,
	NB_CONFIGURATION
};

const S_usb_device_descriptor uac1_audio_usb_dev_desc =
{
	sizeof(S_usb_device_descriptor),
	DEVICE_DESCRIPTOR,
	// BSB Changed 20110901 according to mail from Roger
	Usb_format_mcu_to_usb_data(16, USB_1_1_SPECIFICATION),
	//	Usb_format_mcu_to_usb_data(16, USB_SPECIFICATION),
	DEVICE_CLASS,
	DEVICE_SUB_CLASS,
	DEVICE_PROTOCOL,
	EP_CONTROL_LENGTH,
	Usb_format_mcu_to_usb_data(16, AUDIO_VENDOR_ID),

	// BSB 20120928 new VID/PID system
	#if defined (FEATURE_PRODUCT_SDR_WIDGET) // AUDIO_PRODUCT_ID_1 and _2
	  Usb_format_mcu_to_usb_data(16, AUDIO_PRODUCT_ID_1),
	#elif defined (FEATURE_PRODUCT_USB9023) // AUDIO_PRODUCT_ID_3 and _4
	  Usb_format_mcu_to_usb_data(16, AUDIO_PRODUCT_ID_3),
	#elif defined (FEATURE_PRODUCT_USB5102) // AUDIO_PRODUCT_ID_5 and _6
	  Usb_format_mcu_to_usb_data(16, AUDIO_PRODUCT_ID_5),
	#elif defined (FEATURE_PRODUCT_USB8741) // AUDIO_PRODUCT_ID_7 and _8
	  Usb_format_mcu_to_usb_data(16, AUDIO_PRODUCT_ID_7),
	#elif defined (FEATURE_PRODUCT_AB1x)    // AUDIO_PRODUCT_ID_9 and _10
	  Usb_format_mcu_to_usb_data(16, AUDIO_PRODUCT_ID_9),
	#elif defined (FEATURE_PRODUCT_AMB)     // AUDIO_PRODUCT_ID_13 and _14
	  Usb_format_mcu_to_usb_data(16, AUDIO_PRODUCT_ID_13),
	#else
	#error No recognized FEATURE_PRODUCT... is defined in Makefile, aborting.
	#endif

	Usb_format_mcu_to_usb_data(16, RELEASE_NUMBER),
	MAN_INDEX,
	PROD_INDEX,
	SN_INDEX,
	NB_CONFIGURATION
};


// usb_user_configuration_descriptor FS
const S_usb_user_configuration_descriptor uac1_usb_conf_desc_fs =
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
		ENDPOINT_NB_1,
		EP_ATTRIBUTES_1,
		Usb_format_mcu_to_usb_data(16, EP_SIZE_1_FS),
		EP_INTERVAL_1
	},
	{
		sizeof(S_usb_endpoint_descriptor),
		ENDPOINT_DESCRIPTOR,
		ENDPOINT_NB_2,
		EP_ATTRIBUTES_2,
		Usb_format_mcu_to_usb_data(16, EP_SIZE_2_FS),
		EP_INTERVAL_2
	}
	,

	{ sizeof(S_usb_interface_association_descriptor)
	  ,  DESCRIPTOR_IAD
	  ,  FIRST_INTERFACE1					// bFirstInterface
	  ,  INTERFACE_COUNT1 				// bInterfaceCount
	  ,  INTERFACE_CLASS2
	  ,  INTERFACE_SUB_CLASS2
	  ,  INTERFACE_PROTOCOL2
	  ,  INTERFACE_INDEX2
	},

	{  sizeof(S_usb_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  INTERFACE_NB2
	   ,  ALTERNATE_NB2
	   ,  NB_ENDPOINT2
	   ,  INTERFACE_CLASS2
	   ,  INTERFACE_SUB_CLASS2
	   ,  INTERFACE_PROTOCOL2
	   ,  INTERFACE_INDEX2
    }
	,


    {  sizeof(S_usb_ac_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  HEADER_SUB_TYPE
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_CLASS_REVISION)

	   // BSB 20130604 disabling UAC1 IN
//	   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor_1)+2*sizeof(S_usb_in_ter_descriptor_1)+
//		 2*sizeof(S_usb_feature_unit_descriptor_1)+2*sizeof(S_usb_out_ter_descriptor_1))
	   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor_1)+sizeof(S_usb_in_ter_descriptor_1)+
		 sizeof(S_usb_feature_unit_descriptor_1)+sizeof(S_usb_out_ter_descriptor_1))


       ,  NB_OF_STREAMING_INTERFACE
	   ,  BELONGS_AUDIO_INTERFACE_OUT
	   // BSB 20130604 disabling UAC1 IN 	   ,  BELONGS_AUDIO_INTERFACE_IN
    }
	,
	// BSB 20130604 disabling UAC1 IN
	/*{  sizeof(S_usb_in_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  INPUT_TERMINAL_SUB_TYPE
	   ,  INPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_TYPE)
	   ,  INPUT_TERMINAL_ASSOCIATION
	   ,  INPUT_TERMINAL_NB_CHANNELS
	   ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_CHANNEL_CONF)
	   ,  0x00
	   ,  INPUT_TERMINAL_CH_NAME_ID
    }
	,
    {  sizeof(S_usb_feature_unit_descriptor_1)
	   ,  CS_INTERFACE
	   ,  FEATURE_UNIT_SUB_TYPE
	   ,  MIC_FEATURE_UNIT_ID
	   ,  MIC_FEATURE_UNIT_SOURCE_ID
	   ,  MIC_FEATURE_UNIT_CONTROL_SIZE
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_0)
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_1)
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_2)
	   ,  MIC_FEATURE_UNIT_CH_NAME_ID
    }
	,
    {  sizeof(S_usb_out_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  OUTPUT_TERMINAL_SUB_TYPE
	   ,  OUTPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, OUTPUT_TERMINAL_TYPE)
	   ,  OUTPUT_TERMINAL_ASSOCIATION
	   ,  OUTPUT_TERMINAL_SOURCE_ID
	   ,  0x00
    }
	,*/
	{  sizeof(S_usb_in_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  INPUT_TERMINAL_SUB_TYPE
	   ,  SPK_INPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, SPK_INPUT_TERMINAL_TYPE)
	   ,  SPK_INPUT_TERMINAL_ASSOCIATION
	   ,  SPK_INPUT_TERMINAL_NB_CHANNELS
	   ,  Usb_format_mcu_to_usb_data(16, SPK_INPUT_TERMINAL_CHANNEL_CONF)
	   ,  0x00
	   ,  SPK_INPUT_TERMINAL_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_feature_unit_descriptor_1)
	   ,  CS_INTERFACE
	   ,  FEATURE_UNIT_SUB_TYPE
	   ,  SPK_FEATURE_UNIT_ID
	   ,  SPK_FEATURE_UNIT_SOURCE_ID
	   ,  SPK_FEATURE_UNIT_CONTROL_SIZE
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_0)
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_1)
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_2)
	   ,  SPK_FEATURE_UNIT_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_out_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  OUTPUT_TERMINAL_SUB_TYPE
	   ,  SPK_OUTPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, SPK_OUTPUT_TERMINAL_TYPE)
	   ,  SPK_OUTPUT_TERMINAL_ASSOCIATION
	   ,  SPK_OUTPUT_TERMINAL_SOURCE_ID
	   ,  0x00
	}
	,
    {  sizeof(S_usb_as_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  STD_AS_INTERFACE_OUT
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
	   ,  STD_AS_INTERFACE_OUT
	   ,  ALT1_AS_INTERFACE_INDEX
	   ,  SPK_ALT1_AS_NB_ENDPOINT
	   ,  ALT1_AS_INTERFACE_CLASS
	   ,  ALT1_AS_INTERFACE_SUB_CLASS
	   ,  ALT1_AS_INTERFACE_PROTOCOL
	   ,  0x00
    }
	,
    {  sizeof(S_usb_as_g_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  GENERAL_SUB_TYPE
	   ,  SPK_AS_TERMINAL_LINK
	   ,  SPK_AS_DELAY
	   ,  Usb_format_mcu_to_usb_data(16, SPK_AS_FORMAT_TAG)
    }
	,
    {  sizeof(S_usb_format_type_1)
	   ,  CS_INTERFACE
	   ,  FORMAT_SUB_TYPE
	   ,  FORMAT_TYPE
	   ,  FORMAT_NB_CHANNELS
	   ,  FORMAT_FRAME_SIZE
	   ,  FORMAT_BIT_RESOLUTION
	   ,  FORMAT_SAMPLE_FREQ_NB
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_441)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_48)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
    }
	,
	{   sizeof(S_usb_endpoint_audio_descriptor_1)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_3
		,   EP_ATTRIBUTES_3
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_3_FS)
		,   EP_INTERVAL_3_FS
		,   EP_REFRESH_3_FS // BSB 20130530 added as define. Was 0x05
		,   ENDPOINT_NB_5
	}
	,
	{  sizeof(S_usb_endpoint_audio_specific_1)
	   ,  CS_ENDPOINT
	   ,  GENERAL_SUB_TYPE
	   ,  AUDIO_EP_ATRIBUTES
	   ,  AUDIO_EP_DELAY_UNIT
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
	}
	  ,
      {   sizeof(S_usb_endpoint_audio_descriptor_1)
      ,   ENDPOINT_DESCRIPTOR
      ,   ENDPOINT_NB_5
      ,   EP_ATTRIBUTES_5
      ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_5_FS)
      ,   EP_INTERVAL_5_FS
      ,   EP_REFRESH_5_FS
      ,   0x00
      }
//	,
	// BSB 20130604 disabling UAC1 IN
	/*{  sizeof(S_usb_as_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  STD_AS_INTERFACE_IN
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
	   ,  STD_AS_INTERFACE_IN
	   ,  ALT1_AS_INTERFACE_INDEX
	   ,  ALT1_AS_NB_ENDPOINT
	   ,  ALT1_AS_INTERFACE_CLASS
	   ,  ALT1_AS_INTERFACE_SUB_CLASS
	   ,  ALT1_AS_INTERFACE_PROTOCOL
	   ,  0x00
	}
	,
	{  sizeof(S_usb_as_g_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  GENERAL_SUB_TYPE
	   ,  AS_TERMINAL_LINK
	   ,  AS_DELAY
	   ,  Usb_format_mcu_to_usb_data(16, AS_FORMAT_TAG)
	}
	,
	{  sizeof(S_usb_format_type_1)
	   ,  CS_INTERFACE
	   ,  FORMAT_SUB_TYPE
	   ,  FORMAT_TYPE
	   ,  FORMAT_NB_CHANNELS
	   ,  FORMAT_FRAME_SIZE
	   ,  FORMAT_BIT_RESOLUTION
	   ,  FORMAT_SAMPLE_FREQ_NB
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_441)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_48)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
	}
    ,
    {   sizeof(S_usb_endpoint_audio_descriptor_1)
        ,   ENDPOINT_DESCRIPTOR
        ,   ENDPOINT_NB_4
        ,   EP_ATTRIBUTES_4
        ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_4_FS)
        ,   EP_INTERVAL_4_FS
        ,   0x00
        ,   0x00
	}
	,
	{  sizeof(S_usb_endpoint_audio_specific_1)
	   ,  CS_ENDPOINT
	   ,  GENERAL_SUB_TYPE
	   ,  AUDIO_EP_ATRIBUTES
	   ,  AUDIO_EP_DELAY_UNIT
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
	}*/
};


#if (USB_HIGH_SPEED_SUPPORT==ENABLED)

// usb_user_configuration_descriptor HS
const S_usb_user_configuration_descriptor uac1_usb_conf_desc_hs =
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
		ENDPOINT_NB_1,
		EP_ATTRIBUTES_1,
		Usb_format_mcu_to_usb_data(16, EP_SIZE_1_HS),
		EP_INTERVAL_1
	},
	{
		sizeof(S_usb_endpoint_descriptor),
		ENDPOINT_DESCRIPTOR,
		ENDPOINT_NB_2,
		EP_ATTRIBUTES_2,
		Usb_format_mcu_to_usb_data(16, EP_SIZE_2_HS),
		EP_INTERVAL_2
	}

	,
	{ sizeof(S_usb_interface_association_descriptor)
	  ,  DESCRIPTOR_IAD
	  ,  FIRST_INTERFACE1					// bFirstInterface
	  ,  INTERFACE_COUNT1 				// bInterfaceCount
	  ,  INTERFACE_CLASS2
	  ,  INTERFACE_SUB_CLASS2
	  ,  INTERFACE_PROTOCOL2
	  ,  INTERFACE_INDEX2
	},

	{  sizeof(S_usb_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  INTERFACE_NB2
	   ,  ALTERNATE_NB2
	   ,  NB_ENDPOINT2
	   ,  INTERFACE_CLASS2
	   ,  INTERFACE_SUB_CLASS2
	   ,  INTERFACE_PROTOCOL2
	   ,  INTERFACE_INDEX2
    }
	,

	{  sizeof(S_usb_ac_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  HEADER_SUB_TYPE
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_CLASS_REVISION)
// BSB 20130604 disabling UAC1 IN
//	   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor_1)+2*sizeof(S_usb_in_ter_descriptor_1)+
//			 2*sizeof(S_usb_feature_unit_descriptor_1)+2*sizeof(S_usb_out_ter_descriptor_1))
	   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor_1)+sizeof(S_usb_in_ter_descriptor_1)+
			 sizeof(S_usb_feature_unit_descriptor_1)+sizeof(S_usb_out_ter_descriptor_1))

	   ,  NB_OF_STREAMING_INTERFACE
	   ,  BELONGS_AUDIO_INTERFACE_OUT
	   // BSB 20130604 disabling UAC1 IN	   ,  BELONGS_AUDIO_INTERFACE_IN
	}
	,
	// BSB 20130604 disabling UAC1 IN
	/*{  sizeof(S_usb_in_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  INPUT_TERMINAL_SUB_TYPE
	   ,  INPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_TYPE)
	   ,  INPUT_TERMINAL_ASSOCIATION
	   ,  INPUT_TERMINAL_NB_CHANNELS
	   ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_CHANNEL_CONF)
	   ,  0x00
	   ,  INPUT_TERMINAL_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_feature_unit_descriptor_1)
	   ,  CS_INTERFACE
	   ,  FEATURE_UNIT_SUB_TYPE
	   ,  MIC_FEATURE_UNIT_ID
	   ,  MIC_FEATURE_UNIT_SOURCE_ID
	   ,  MIC_FEATURE_UNIT_CONTROL_SIZE
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_0)
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_1)
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_2)
	   ,  MIC_FEATURE_UNIT_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_out_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  OUTPUT_TERMINAL_SUB_TYPE
	   ,  OUTPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, OUTPUT_TERMINAL_TYPE)
	   ,  OUTPUT_TERMINAL_ASSOCIATION
	   ,  OUTPUT_TERMINAL_SOURCE_ID
	   ,  0x00
	}
	,*/
	{  sizeof(S_usb_in_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  INPUT_TERMINAL_SUB_TYPE
	   ,  SPK_INPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, SPK_INPUT_TERMINAL_TYPE)
	   ,  SPK_INPUT_TERMINAL_ASSOCIATION
	   ,  SPK_INPUT_TERMINAL_NB_CHANNELS
	   ,  Usb_format_mcu_to_usb_data(16, SPK_INPUT_TERMINAL_CHANNEL_CONF)
	   ,  0x00
	   ,  SPK_INPUT_TERMINAL_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_feature_unit_descriptor_1)
	   ,  CS_INTERFACE
	   ,  FEATURE_UNIT_SUB_TYPE
	   ,  SPK_FEATURE_UNIT_ID
	   ,  SPK_FEATURE_UNIT_SOURCE_ID
	   ,  SPK_FEATURE_UNIT_CONTROL_SIZE
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_0)
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_1)
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_2)
	   ,  SPK_FEATURE_UNIT_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_out_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  OUTPUT_TERMINAL_SUB_TYPE
	   ,  SPK_OUTPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, SPK_OUTPUT_TERMINAL_TYPE)
	   ,  SPK_OUTPUT_TERMINAL_ASSOCIATION
	   ,  SPK_OUTPUT_TERMINAL_SOURCE_ID
	   ,  0x00
	}
	,
	{  sizeof(S_usb_as_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  STD_AS_INTERFACE_OUT
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
	   ,  STD_AS_INTERFACE_OUT
	   ,  ALT1_AS_INTERFACE_INDEX
	   ,  SPK_ALT1_AS_NB_ENDPOINT
	   ,  ALT1_AS_INTERFACE_CLASS
	   ,  ALT1_AS_INTERFACE_SUB_CLASS
	   ,  ALT1_AS_INTERFACE_PROTOCOL
	   ,  0x00
	}
	,
	{  sizeof(S_usb_as_g_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  GENERAL_SUB_TYPE
	   ,  SPK_AS_TERMINAL_LINK
	   ,  SPK_AS_DELAY
	   ,  Usb_format_mcu_to_usb_data(16, SPK_AS_FORMAT_TAG)
	}
	,
	{  sizeof(S_usb_format_type_1)
	   ,  CS_INTERFACE
	   ,  FORMAT_SUB_TYPE
	   ,  FORMAT_TYPE
	   ,  FORMAT_NB_CHANNELS
	   ,  FORMAT_FRAME_SIZE
	   ,  FORMAT_BIT_RESOLUTION
	   ,  FORMAT_SAMPLE_FREQ_NB
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_441)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_48)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
	}
	,
    {   sizeof(S_usb_endpoint_audio_descriptor_1)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_3
		,   EP_ATTRIBUTES_3
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_3_HS)
		,   EP_INTERVAL_3_HS
		,   EP_REFRESH_3_HS // BSB 20130530 added as define. Was 0x05
		,   ENDPOINT_NB_5
    }
	,
    {  sizeof(S_usb_endpoint_audio_specific_1)
	   ,  CS_ENDPOINT
	   ,  GENERAL_SUB_TYPE
	   ,  AUDIO_EP_ATRIBUTES
	   ,  AUDIO_EP_DELAY_UNIT
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
    }
	  ,
      {   sizeof(S_usb_endpoint_audio_descriptor_1)
      ,   ENDPOINT_DESCRIPTOR
      ,   ENDPOINT_NB_5
      ,   EP_ATTRIBUTES_5
      ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_5_HS)
      ,   EP_INTERVAL_5_HS
      ,   EP_REFRESH_5_HS
      ,   0x00
      }
//	,
	// BSB 20130604 disabling UAC1 IN
	/*{  sizeof(S_usb_as_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  STD_AS_INTERFACE_IN
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
	   ,  STD_AS_INTERFACE_IN
	   ,  ALT1_AS_INTERFACE_INDEX
	   ,  ALT1_AS_NB_ENDPOINT
	   ,  ALT1_AS_INTERFACE_CLASS
	   ,  ALT1_AS_INTERFACE_SUB_CLASS
	   ,  ALT1_AS_INTERFACE_PROTOCOL
	   ,  0x00
    }
	,
    {  sizeof(S_usb_as_g_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  GENERAL_SUB_TYPE
	   ,  AS_TERMINAL_LINK
	   ,  AS_DELAY
	   ,  Usb_format_mcu_to_usb_data(16, AS_FORMAT_TAG)
    }
	,
    {  sizeof(S_usb_format_type_1)
	   ,  CS_INTERFACE
	   ,  FORMAT_SUB_TYPE
	   ,  FORMAT_TYPE
	   ,  FORMAT_NB_CHANNELS
	   ,  FORMAT_FRAME_SIZE
	   ,  FORMAT_BIT_RESOLUTION
	   ,  FORMAT_SAMPLE_FREQ_NB
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_441)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_48)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
    }
	,
	{   sizeof(S_usb_endpoint_audio_descriptor_1)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_4
		,   EP_ATTRIBUTES_4
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_4_HS)
		,   EP_INTERVAL_4_HS
		,   0x00
		,   0x00
	}
	,
	{  sizeof(S_usb_endpoint_audio_specific_1)
	   ,  CS_ENDPOINT
	   ,  GENERAL_SUB_TYPE
	   ,  AUDIO_EP_ATRIBUTES
	   ,  AUDIO_EP_DELAY_UNIT
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
	}*/

};


// usb_user_configuration_descriptor HS
const S_usb_user_configuration_descriptor uac1_usb_conf_desc_hs_widget =
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
		ENDPOINT_NB_1,
		EP_ATTRIBUTES_1,
		Usb_format_mcu_to_usb_data(16, EP_SIZE_1_HS),
		EP_INTERVAL_1
	},
	{
		sizeof(S_usb_endpoint_descriptor),
		ENDPOINT_DESCRIPTOR,
		ENDPOINT_NB_2,
		EP_ATTRIBUTES_2,
		Usb_format_mcu_to_usb_data(16, EP_SIZE_2_HS),
		EP_INTERVAL_2
	}

	,
	{ sizeof(S_usb_interface_association_descriptor)
	  ,  DESCRIPTOR_IAD
	  ,  FIRST_INTERFACE1					// bFirstInterface
	  ,  INTERFACE_COUNT1 				// bInterfaceCount
	  ,  INTERFACE_CLASS2
	  ,  INTERFACE_SUB_CLASS2
	  ,  INTERFACE_PROTOCOL2
	  ,  INTERFACE_INDEX2
	},

	{  sizeof(S_usb_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  INTERFACE_NB2
	   ,  ALTERNATE_NB2
	   ,  NB_ENDPOINT2
	   ,  INTERFACE_CLASS2
	   ,  INTERFACE_SUB_CLASS2
	   ,  INTERFACE_PROTOCOL2
	   ,  INTERFACE_INDEX2
    }
	,

	{  sizeof(S_usb_ac_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  HEADER_SUB_TYPE
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_CLASS_REVISION)
// BSB 20130604 disabling UAC1 IN
//	   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor_1)+2*sizeof(S_usb_in_ter_descriptor_1)+
//		2*sizeof(S_usb_feature_unit_descriptor_1)+2*sizeof(S_usb_out_ter_descriptor_1))
       ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor_1)+sizeof(S_usb_in_ter_descriptor_1)+
		sizeof(S_usb_feature_unit_descriptor_1)+sizeof(S_usb_out_ter_descriptor_1))
	   ,  NB_OF_STREAMING_INTERFACE
	   ,  BELONGS_AUDIO_INTERFACE_OUT
	   // BSB 20130604 disabling UAC1 IN	   ,  BELONGS_AUDIO_INTERFACE_IN
	}
	,
	// BSB 20130604 disabling UAC1 IN
	/*{  sizeof(S_usb_in_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  INPUT_TERMINAL_SUB_TYPE
	   ,  INPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_TYPE)
	   ,  INPUT_TERMINAL_ASSOCIATION
	   ,  INPUT_TERMINAL_NB_CHANNELS
	   ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_CHANNEL_CONF)
	   ,  0x00
	   ,  INPUT_TERMINAL_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_feature_unit_descriptor_1)
	   ,  CS_INTERFACE
	   ,  FEATURE_UNIT_SUB_TYPE
	   ,  MIC_FEATURE_UNIT_ID
	   ,  MIC_FEATURE_UNIT_SOURCE_ID
	   ,  MIC_FEATURE_UNIT_CONTROL_SIZE
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_0)
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_1)
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_2)
	   ,  MIC_FEATURE_UNIT_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_out_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  OUTPUT_TERMINAL_SUB_TYPE
	   ,  OUTPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, OUTPUT_TERMINAL_TYPE)
	   ,  OUTPUT_TERMINAL_ASSOCIATION
	   ,  OUTPUT_TERMINAL_SOURCE_ID
	   ,  0x00
	}
	,*/
	{  sizeof(S_usb_in_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  INPUT_TERMINAL_SUB_TYPE
	   ,  SPK_INPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, SPK_INPUT_TERMINAL_TYPE)
	   ,  SPK_INPUT_TERMINAL_ASSOCIATION
	   ,  SPK_INPUT_TERMINAL_NB_CHANNELS
	   ,  Usb_format_mcu_to_usb_data(16, SPK_INPUT_TERMINAL_CHANNEL_CONF)
	   ,  0x00
	   ,  SPK_INPUT_TERMINAL_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_feature_unit_descriptor_1)
	   ,  CS_INTERFACE
	   ,  FEATURE_UNIT_SUB_TYPE
	   ,  SPK_FEATURE_UNIT_ID
	   ,  SPK_FEATURE_UNIT_SOURCE_ID
	   ,  SPK_FEATURE_UNIT_CONTROL_SIZE
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_0)
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_1)
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_2)
	   ,  SPK_FEATURE_UNIT_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_out_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  OUTPUT_TERMINAL_SUB_TYPE
	   ,  SPK_OUTPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, SPK_OUTPUT_TERMINAL_TYPE)
	   ,  SPK_OUTPUT_TERMINAL_ASSOCIATION
	   ,  SPK_OUTPUT_TERMINAL_SOURCE_ID
	   ,  0x00
	}
	,
	{  sizeof(S_usb_as_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  STD_AS_INTERFACE_OUT
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
	   ,  STD_AS_INTERFACE_OUT
	   ,  ALT1_AS_INTERFACE_INDEX
	   ,  SPK_ALT1_AS_NB_ENDPOINT
	   ,  ALT1_AS_INTERFACE_CLASS
	   ,  ALT1_AS_INTERFACE_SUB_CLASS
	   ,  ALT1_AS_INTERFACE_PROTOCOL
	   ,  0x00
	}
	,
	{  sizeof(S_usb_as_g_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  GENERAL_SUB_TYPE
	   ,  SPK_AS_TERMINAL_LINK
	   ,  SPK_AS_DELAY
	   ,  Usb_format_mcu_to_usb_data(16, SPK_AS_FORMAT_TAG)
	}
	,
	{  sizeof(S_usb_format_type_1)
	   ,  CS_INTERFACE
	   ,  FORMAT_SUB_TYPE
	   ,  FORMAT_TYPE
	   ,  FORMAT_NB_CHANNELS
	   ,  FORMAT_FRAME_SIZE
	   ,  FORMAT_BIT_RESOLUTION
	   ,  0x01
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_48)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
	}
	,
    {   sizeof(S_usb_endpoint_audio_descriptor_1)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_3
		,   EP_ATTRIBUTES_3
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_3_HS)
		,   EP_INTERVAL_3_HS
		,   0x00
		,   EP_BSYNC_ADDRESS_3
    }
	,
    {  sizeof(S_usb_endpoint_audio_specific_1)
	   ,  CS_ENDPOINT
	   ,  GENERAL_SUB_TYPE
	   ,  AUDIO_EP_ATRIBUTES
	   ,  AUDIO_EP_DELAY_UNIT
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
    }
	
	  ,
      {   sizeof(S_usb_endpoint_audio_descriptor_1)
      ,   ENDPOINT_DESCRIPTOR
      ,   ENDPOINT_NB_5
      ,   EP_ATTRIBUTES_5
      ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_5_HS)
      ,   EP_INTERVAL_5_HS
      ,   EP_REFRESH_5_HS
      ,   0x00
      }
	
//	,
	// BSB 20130604 disabling UAC1 IN
	/*{  sizeof(S_usb_as_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  STD_AS_INTERFACE_IN
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
	   ,  STD_AS_INTERFACE_IN
	   ,  ALT1_AS_INTERFACE_INDEX
	   ,  ALT1_AS_NB_ENDPOINT
	   ,  ALT1_AS_INTERFACE_CLASS
	   ,  ALT1_AS_INTERFACE_SUB_CLASS
	   ,  ALT1_AS_INTERFACE_PROTOCOL
	   ,  0x00
    }
	,
    {  sizeof(S_usb_as_g_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  GENERAL_SUB_TYPE
	   ,  AS_TERMINAL_LINK
	   ,  AS_DELAY
	   ,  Usb_format_mcu_to_usb_data(16, AS_FORMAT_TAG)
    }
	,
    {  sizeof(S_usb_format_type_1)
	   ,  CS_INTERFACE
	   ,  FORMAT_SUB_TYPE
	   ,  FORMAT_TYPE
	   ,  FORMAT_NB_CHANNELS
	   ,  FORMAT_FRAME_SIZE
	   ,  FORMAT_BIT_RESOLUTION
	   ,  0x01
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_48)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
    }
	,
	{   sizeof(S_usb_endpoint_audio_descriptor_1)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_4
		,   EP_ATTRIBUTES_4
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_4_HS)
		,   EP_INTERVAL_4_HS
		,   0x00
		,   0x00
	}
	,
	{  sizeof(S_usb_endpoint_audio_specific_1)
	   ,  CS_ENDPOINT
	   ,  GENERAL_SUB_TYPE
	   ,  AUDIO_EP_ATRIBUTES
	   ,  AUDIO_EP_DELAY_UNIT
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
	}*/

};
#endif

const S_usb_user_configuration_descriptor uac1_usb_conf_desc_fs_widget =
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
		ENDPOINT_NB_1,
		EP_ATTRIBUTES_1,
		Usb_format_mcu_to_usb_data(16, EP_SIZE_1_FS),
		EP_INTERVAL_1
	},
	{
		sizeof(S_usb_endpoint_descriptor),
		ENDPOINT_DESCRIPTOR,
		ENDPOINT_NB_2,
		EP_ATTRIBUTES_2,
		Usb_format_mcu_to_usb_data(16, EP_SIZE_2_FS),
		EP_INTERVAL_2
	}
	,

	{ sizeof(S_usb_interface_association_descriptor)
	  ,  DESCRIPTOR_IAD
	  ,  FIRST_INTERFACE1					// bFirstInterface
	  ,  INTERFACE_COUNT1 				// bInterfaceCount
	  ,  INTERFACE_CLASS2
	  ,  INTERFACE_SUB_CLASS2
	  ,  INTERFACE_PROTOCOL2
	  ,  INTERFACE_INDEX2
	},

	{  sizeof(S_usb_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  INTERFACE_NB2
	   ,  ALTERNATE_NB2
	   ,  NB_ENDPOINT2
	   ,  INTERFACE_CLASS2
	   ,  INTERFACE_SUB_CLASS2
	   ,  INTERFACE_PROTOCOL2
	   ,  INTERFACE_INDEX2
    }
	,


    {  sizeof(S_usb_ac_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  HEADER_SUB_TYPE
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_CLASS_REVISION)
// BSB 20130604 disabling UAC1 IN
//	   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor_1)+2*sizeof(S_usb_in_ter_descriptor_1)+
//		2*sizeof(S_usb_feature_unit_descriptor_1)+2*sizeof(S_usb_out_ter_descriptor_1))
	   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_ac_interface_descriptor_1)+sizeof(S_usb_in_ter_descriptor_1)+
		sizeof(S_usb_feature_unit_descriptor_1)+sizeof(S_usb_out_ter_descriptor_1))
	   ,  NB_OF_STREAMING_INTERFACE
	   ,  BELONGS_AUDIO_INTERFACE_OUT
	   // BSB 20130604 disabling UAC1 IN	   ,  BELONGS_AUDIO_INTERFACE_IN
    }
	,
	// BSB 20130604 disabling UAC1 IN
	/*{  sizeof(S_usb_in_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  INPUT_TERMINAL_SUB_TYPE
	   ,  INPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_TYPE)
	   ,  INPUT_TERMINAL_ASSOCIATION
	   ,  INPUT_TERMINAL_NB_CHANNELS
	   ,  Usb_format_mcu_to_usb_data(16, INPUT_TERMINAL_CHANNEL_CONF)
	   ,  0x00
	   ,  INPUT_TERMINAL_CH_NAME_ID
    }
	,
    {  sizeof(S_usb_feature_unit_descriptor_1)
	   ,  CS_INTERFACE
	   ,  FEATURE_UNIT_SUB_TYPE
	   ,  MIC_FEATURE_UNIT_ID
	   ,  MIC_FEATURE_UNIT_SOURCE_ID
	   ,  MIC_FEATURE_UNIT_CONTROL_SIZE
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_0)
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_1)
	   ,  Usb_format_mcu_to_usb_data(16, MIC_BMA_CONTROLS_2)
	   ,  MIC_FEATURE_UNIT_CH_NAME_ID
    }
	,
    {  sizeof(S_usb_out_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  OUTPUT_TERMINAL_SUB_TYPE
	   ,  OUTPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, OUTPUT_TERMINAL_TYPE)
	   ,  OUTPUT_TERMINAL_ASSOCIATION
	   ,  OUTPUT_TERMINAL_SOURCE_ID
	   ,  0x00
    }
	,*/
	{  sizeof(S_usb_in_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  INPUT_TERMINAL_SUB_TYPE
	   ,  SPK_INPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, SPK_INPUT_TERMINAL_TYPE)
	   ,  SPK_INPUT_TERMINAL_ASSOCIATION
	   ,  SPK_INPUT_TERMINAL_NB_CHANNELS
	   ,  Usb_format_mcu_to_usb_data(16, SPK_INPUT_TERMINAL_CHANNEL_CONF)
	   ,  0x00
	   ,  SPK_INPUT_TERMINAL_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_feature_unit_descriptor_1)
	   ,  CS_INTERFACE
	   ,  FEATURE_UNIT_SUB_TYPE
	   ,  SPK_FEATURE_UNIT_ID
	   ,  SPK_FEATURE_UNIT_SOURCE_ID
	   ,  SPK_FEATURE_UNIT_CONTROL_SIZE
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_0)
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_1)
	   ,  Usb_format_mcu_to_usb_data(16, SPK_FEATURE_UNIT_BMA_CONTROLS_2)
	   ,  SPK_FEATURE_UNIT_CH_NAME_ID
	}
	,
	{  sizeof(S_usb_out_ter_descriptor_1)
	   ,  CS_INTERFACE
	   ,  OUTPUT_TERMINAL_SUB_TYPE
	   ,  SPK_OUTPUT_TERMINAL_ID
	   ,  Usb_format_mcu_to_usb_data(16, SPK_OUTPUT_TERMINAL_TYPE)
	   ,  SPK_OUTPUT_TERMINAL_ASSOCIATION
	   ,  SPK_OUTPUT_TERMINAL_SOURCE_ID
	   ,  0x00
	}
	,
    {  sizeof(S_usb_as_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  STD_AS_INTERFACE_OUT
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
	   ,  STD_AS_INTERFACE_OUT
	   ,  ALT1_AS_INTERFACE_INDEX
	   ,  SPK_ALT1_AS_NB_ENDPOINT
	   ,  ALT1_AS_INTERFACE_CLASS
	   ,  ALT1_AS_INTERFACE_SUB_CLASS
	   ,  ALT1_AS_INTERFACE_PROTOCOL
	   ,  0x00
    }
	,
    {  sizeof(S_usb_as_g_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  GENERAL_SUB_TYPE
	   ,  SPK_AS_TERMINAL_LINK
	   ,  SPK_AS_DELAY
	   ,  Usb_format_mcu_to_usb_data(16, SPK_AS_FORMAT_TAG)
    }
	,
    {  sizeof(S_usb_format_type_1)
	   ,  CS_INTERFACE
	   ,  FORMAT_SUB_TYPE
	   ,  FORMAT_TYPE
	   ,  FORMAT_NB_CHANNELS
	   ,  FORMAT_FRAME_SIZE
	   ,  FORMAT_BIT_RESOLUTION
	   ,  0x01
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_48)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
    }
	,
	{   sizeof(S_usb_endpoint_audio_descriptor_1)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_3
		,   EP_ATTRIBUTES_3
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_3_FS)
		,   EP_INTERVAL_3_FS
		,   0x00
		,   EP_BSYNC_ADDRESS_3
	}
	,
	{  sizeof(S_usb_endpoint_audio_specific_1)
	   ,  CS_ENDPOINT
	   ,  GENERAL_SUB_TYPE
	   ,  AUDIO_EP_ATRIBUTES
	   ,  AUDIO_EP_DELAY_UNIT
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
	}
	
	  ,
      {   sizeof(S_usb_endpoint_audio_descriptor_1)
      ,   ENDPOINT_DESCRIPTOR
      ,   ENDPOINT_NB_5
      ,   EP_ATTRIBUTES_5
      ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_5_FS)
      ,   EP_INTERVAL_5_FS
      ,   EP_REFRESH_5_FS
      ,   0x00
      }

//	,
	// BSB 20130604 disabling UAC1 IN
	/*{  sizeof(S_usb_as_interface_descriptor)
	   ,  INTERFACE_DESCRIPTOR
	   ,  STD_AS_INTERFACE_IN
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
	   ,  STD_AS_INTERFACE_IN
	   ,  ALT1_AS_INTERFACE_INDEX
	   ,  ALT1_AS_NB_ENDPOINT
	   ,  ALT1_AS_INTERFACE_CLASS
	   ,  ALT1_AS_INTERFACE_SUB_CLASS
	   ,  ALT1_AS_INTERFACE_PROTOCOL
	   ,  0x00
	}
	,
	{  sizeof(S_usb_as_g_interface_descriptor_1)
	   ,  CS_INTERFACE
	   ,  GENERAL_SUB_TYPE
	   ,  AS_TERMINAL_LINK
	   ,  AS_DELAY
	   ,  Usb_format_mcu_to_usb_data(16, AS_FORMAT_TAG)
	}
	,
	{  sizeof(S_usb_format_type_1)
	   ,  CS_INTERFACE
	   ,  FORMAT_SUB_TYPE
	   ,  FORMAT_TYPE
	   ,  FORMAT_NB_CHANNELS
	   ,  FORMAT_FRAME_SIZE
	   ,  FORMAT_BIT_RESOLUTION
	   ,  0x01
	   ,  Usb_format_mcu_to_usb_data(16, FORMAT_LSBYTE_SAMPLE_FREQ_48)
	   ,  FORMAT_MSBYTE_SAMPLE_FREQ
	}
    ,
    {   sizeof(S_usb_endpoint_audio_descriptor_1)
        ,   ENDPOINT_DESCRIPTOR
        ,   ENDPOINT_NB_4
        ,   EP_ATTRIBUTES_4
        ,   Usb_format_mcu_to_usb_data(16, EP_SIZE_4_FS)
        ,   EP_INTERVAL_4_FS
        ,   0x00
        ,   0x00
	}
	,
	{  sizeof(S_usb_endpoint_audio_specific_1)
	   ,  CS_ENDPOINT
	   ,  GENERAL_SUB_TYPE
	   ,  AUDIO_EP_ATRIBUTES
	   ,  AUDIO_EP_DELAY_UNIT
	   ,  Usb_format_mcu_to_usb_data(16, AUDIO_EP_LOCK_DELAY)
	}*/
};


#if (USB_HIGH_SPEED_SUPPORT==ENABLED)
// usb_qualifier_desc FS
const S_usb_device_qualifier_descriptor uac1_usb_qualifier_desc =
{
	sizeof(S_usb_device_qualifier_descriptor),
	DEVICE_QUALIFIER_DESCRIPTOR,
	// BSB Changed 20110901 according to mail from Roger
	Usb_format_mcu_to_usb_data(16, USB_1_1_SPECIFICATION),
	//	Usb_format_mcu_to_usb_data(16, USB_SPECIFICATION),
	DEVICE_CLASS,
	DEVICE_SUB_CLASS,
	DEVICE_PROTOCOL,
	EP_CONTROL_LENGTH,
	NB_CONFIGURATION,
	0
};
#endif
#endif

