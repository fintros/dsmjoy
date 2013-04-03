/*
  Copyright 2013  Alexey Spirkov (me [at] alsp [dot] net)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/* USB processing functions
*/

#include "DSMJoy.h"


static USB_JoystickReport_Data_t JoystickReportData;

static void UpdateReportData(void)
{
	if(rx_channels_changed)
	{
		for(int i = 0; i < 8; i++)
			JoystickReportData.channels[i] = rx_channels[i] - CHANNEL_MAX/2;
		
		JoystickReportData.buttons = ((rx_channels[8]>(CHANNEL_MAX/2))?0x01: 0) |
									 ((rx_channels[9]>(CHANNEL_MAX/2))?0x02: 0) |
									 ((rx_channels[10]>(CHANNEL_MAX/2))?0x04: 0) |
									 ((rx_channels[11]>(CHANNEL_MAX/2))?0x08: 0) |
									 ((rx_channels[12]>(CHANNEL_MAX/2))?0x10: 0) |
									 ((rx_channels[13]>(CHANNEL_MAX/2))?0x20: 0) |
									 ((rx_channels[14]>(CHANNEL_MAX/2))?0x40: 0) |
									 ((rx_channels[15]>(CHANNEL_MAX/2))?0x80: 0) 									 
									 ;
		rx_channels_changed = 0;					
	}
}

typedef struct
{
	USB_Descriptor_Configuration_Header_t Config;

	// Joystick HID Interface
	USB_Descriptor_Interface_t            HID_Interface;
	USB_HID_Descriptor_HID_t              HID_JoystickHID;
	USB_Descriptor_Endpoint_t             HID_ReportINEndpoint;
} USB_Descriptor_Configuration_t;

/* Macros: */
/** Endpoint address of the Joystick HID reporting IN endpoint. */
#define JOYSTICK_EPADDR           (ENDPOINT_DIR_IN | 1)

/** Descriptor header type value, to indicate a HID class HID descriptor. */
#define DTYPE_HID                 0x21

/** Descriptor header type value, to indicate a HID class HID report descriptor. */
#define DTYPE_Report              0x22




/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Device_Connect(void)
{
	LEDs_TurnOnLEDs(LEDS_LED2);
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs and stops the USB management and joystick reporting tasks.
 */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_TurnOffLEDs(LEDS_LED2);
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured and the joystick reporting task started.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	/* Setup HID Report Endpoint */
	if (Endpoint_ConfigureEndpoint(JOYSTICK_EPADDR, EP_TYPE_INTERRUPT, sizeof(JoystickReportData), 1))
		LEDs_TurnOnLEDs(LEDS_LED2);
	else
		LEDs_TurnOffLEDs(LEDS_LED2);
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
	/* Handle HID Class specific requests */
	switch (USB_ControlRequest.bRequest)
	{
		case HID_REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();

				UpdateReportData();
				/* Write the report data to the control endpoint */
				Endpoint_Write_Control_Stream_LE(&JoystickReportData, sizeof(JoystickReportData));
				Endpoint_ClearOUT();
			}

			break;
	}
}

/** Function to manage HID report generation and transmission to the host. */
void HID_Task(void)
{
	/* Device must be connected and configured for the task to run */
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

	/* Select the Joystick Report Endpoint */
	Endpoint_SelectEndpoint(JOYSTICK_EPADDR);

	/* Check to see if the host is ready for another packet */
	if (Endpoint_IsINReady())
	{
		UpdateReportData();
		/* Write Joystick Report Data */
		Endpoint_Write_Stream_LE(&JoystickReportData, sizeof(JoystickReportData), NULL);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();
	}
}


/** HID class report descriptor. This is a special descriptor constructed with values from the
 *  USBIF HID class specification to describe the reports and capabilities of the HID device. This
 *  descriptor is parsed by the host and its contents used to determine what data (and in what encoding)
 *  the device will send, and what it may be sent back from the host. Refer to the HID specification for
 *  more details on HID report descriptors.
 */
const USB_Descriptor_HIDReport_Datatype_t PROGMEM JoystickReport[] =
{
	HID_RI_USAGE_PAGE(8, 0x01), /* Generic Desktop */
	HID_RI_USAGE(8, 0x04), /* Joystick */
	HID_RI_COLLECTION(8, 0x01), /* Application */
	    HID_RI_USAGE(8, 0x01), /* Pointer */
	    HID_RI_COLLECTION(8, 0x00), /* Physical */
	        HID_RI_USAGE(8, 0x30), /* Usage X */
	        HID_RI_USAGE(8, 0x31), /* Usage Y */
	        HID_RI_USAGE(8, 0x32), /* Usage Z */
	        HID_RI_USAGE(8, 0x33), /* Usage Rx */
	        HID_RI_USAGE(8, 0x34), /* Usage Ry */
	        HID_RI_USAGE(8, 0x35), /* Usage Rz */
	        HID_RI_USAGE(8, 0x36), /* Usage Slider */
	        HID_RI_USAGE(8, 0x37), /* Usage Dial */
	        HID_RI_LOGICAL_MINIMUM(16, -512),
	        HID_RI_LOGICAL_MAXIMUM(16, 511),
	        HID_RI_PHYSICAL_MINIMUM(16, 0),
	        HID_RI_PHYSICAL_MAXIMUM(16, 1023),
	        HID_RI_REPORT_COUNT(8, 8),
	        HID_RI_REPORT_SIZE(8, 16),
	        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
	    HID_RI_END_COLLECTION(0),
	    HID_RI_USAGE_PAGE(8, 0x09), /* Button */
	    HID_RI_USAGE_MINIMUM(8, 0x01),
	    HID_RI_USAGE_MAXIMUM(8, 0x02),
	    HID_RI_LOGICAL_MINIMUM(8, 0x00),
	    HID_RI_LOGICAL_MAXIMUM(8, 0x01),
	    HID_RI_REPORT_SIZE(8, 0x01),
	    HID_RI_REPORT_COUNT(8, 0x08),  /* 8 possible buttons */
	    HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
		
	HID_RI_END_COLLECTION(0),
};

/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

	.USBSpecification       = VERSION_BCD(01.10),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID               = 0x03EB,
	.ProductID              = 0xDEF0,
	.ReleaseNumber          = VERSION_BCD(00.04),

	.ManufacturerStrIndex   = 0x01,
	.ProductStrIndex        = 0x02,
	.SerialNumStrIndex      = 0x03,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
	.Config =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

			.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
			.TotalInterfaces        = 1,

			.ConfigurationNumber    = 1,
			.ConfigurationStrIndex  = NO_DESCRIPTOR,

			.ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED /*| USB_CONFIG_ATTR_SELFPOWERED*/),

			.MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
		},

	.HID_Interface =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

			.InterfaceNumber        = 0x00,
			.AlternateSetting       = 0x00,

			.TotalEndpoints         = 1,

			.Class                  = HID_CSCP_HIDClass,
			.SubClass               = HID_CSCP_NonBootSubclass,
			.Protocol               = HID_CSCP_NonBootProtocol,

			.InterfaceStrIndex      = NO_DESCRIPTOR
		},

	.HID_JoystickHID =
		{
			.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

			.HIDSpec                = VERSION_BCD(01.11),
			.CountryCode            = 0x00,
			.TotalReportDescriptors = 1,
			.HIDReportType          = HID_DTYPE_Report,
			.HIDReportLength        = sizeof(JoystickReport)
		},

	.HID_ReportINEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = JOYSTICK_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = sizeof(JoystickReportData),
			.PollingIntervalMS      = 0x05
		}
};

/** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
 *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
 *  via the language ID table available at USB.org what languages the device supports for its string descriptors.
 */
const USB_Descriptor_String_t PROGMEM LanguageString =
{
	.Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},

	.UnicodeString          = {LANGUAGE_ID_ENG}
};

/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ManufacturerString =
{
	.Header                 = {.Size = USB_STRING_LEN(4), .Type = DTYPE_String},

	.UnicodeString          = L"AlSp"
};

/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ProductString =
{
	.Header                 = {.Size = USB_STRING_LEN(22), .Type = DTYPE_String},

	.UnicodeString          = L"DSM Satellite Joystick"
};

const USB_Descriptor_String_t PROGMEM SNString =
{
#ifdef _SN
	.Header                 = {.Size = USB_STRING_LEN(sizeof(_SN)/2), .Type = DTYPE_String},

	.UnicodeString          = _SN
#else
	.Header                 = {.Size = USB_STRING_LEN(6), .Type = DTYPE_String},

	.UnicodeString          = L"SN----"
#endif	
};


/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case 0x00:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case 0x01:
					Address = &ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case 0x02:
					Address = &ProductString;
					Size    = pgm_read_byte(&ProductString.Header.Size);
					break;
				case 0x03:
					Address = &SNString;
					Size    = pgm_read_byte(&SNString.Header.Size);
					break;	
			}

			break;
		case DTYPE_HID:
			Address = &ConfigurationDescriptor.HID_JoystickHID;
			Size    = sizeof(USB_HID_Descriptor_HID_t);
			break;
		case DTYPE_Report:
			Address = &JoystickReport;
			Size    = sizeof(JoystickReport);
			break;
	}

	*DescriptorAddress = Address;
	return Size;
}

