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


#if USB_DEVICE_FEATURE == ENABLED

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "hpsdr_usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "hpsdr_usb_specific_request.h"
#include "usb_audio.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________




// usb_user_device_descriptor
const S_usb_device_descriptor hpsdr_usb_dev_desc = {
	sizeof(S_usb_device_descriptor),
	DEVICE_DESCRIPTOR,
	Usb_format_mcu_to_usb_data(16, USB_SPECIFICATION),
	DEVICE_CLASS,
	DEVICE_SUB_CLASS,
	DEVICE_PROTOCOL,
	EP_CONTROL_LENGTH,
	Usb_format_mcu_to_usb_data(16, HPSDR_VENDOR_ID),
	Usb_format_mcu_to_usb_data(16, HPSDR_PRODUCT_ID),
	Usb_format_mcu_to_usb_data(16, RELEASE_NUMBER),
	MAN_INDEX,
	PROD_INDEX,
	SN_INDEX,
	NB_CONFIGURATION
};



// usb_user_configuration_descriptor FS
const S_usb_user_configuration_descriptor hpsdr_usb_conf_desc_fs = {
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

	{   sizeof(S_usb_endpoint_descriptor)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_1
		,   EP_ATTRIBUTES_1
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_1_FS)
		,   EP_INTERVAL_1_FS
	}
	,

	{   sizeof(S_usb_endpoint_audio_descriptor_2) // ? ep2 is declared as a S_usb_endpoint_descriptor, not this ?
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_2
		,   EP_ATTRIBUTES_2
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_2_FS)
		,   EP_INTERVAL_2_FS
	}

	,
	{   sizeof(S_usb_endpoint_descriptor)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_3
		,   EP_ATTRIBUTES_3
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_3_FS)
		,   EP_INTERVAL_3_FS
	}
};


#if (USB_HIGH_SPEED_SUPPORT==ENABLED)

// usb_user_configuration_descriptor HS
const S_usb_user_configuration_descriptor hpsdr_usb_conf_desc_hs = {
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
    {   sizeof(S_usb_endpoint_descriptor)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_1
		,   EP_ATTRIBUTES_1
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_1_HS)
		,   EP_INTERVAL_1_HS
    }
	,
	{   sizeof(S_usb_endpoint_descriptor)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_2
		,   EP_ATTRIBUTES_2
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_2_HS)
		,   EP_INTERVAL_2_HS
	}
	,
	{   sizeof(S_usb_endpoint_descriptor)
		,   ENDPOINT_DESCRIPTOR
		,   ENDPOINT_NB_3
		,   EP_ATTRIBUTES_3
		,   Usb_format_mcu_to_usb_data(16, EP_SIZE_3_HS)
		,   EP_INTERVAL_3_HS
	}
};


// usb_qualifier_desc FS
const S_usb_device_qualifier_descriptor hpsdr_usb_qualifier_desc = {
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

// usb_user_product_string_descriptor
const S_usb_product_string_descriptor hpsdr_usb_user_product_string_descriptor = {
	sizeof(S_usb_product_string_descriptor),
	STRING_DESCRIPTOR,
	HPSDR_USB_PRODUCT_NAME
};

#endif  // USB_DEVICE_FEATURE == ENABLED
