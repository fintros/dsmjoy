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

/* Main entry point file
*/

#include "DSMJoy.h"
		
/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	Buttons_Init();
	LEDs_Init();	
	Sat_Init();
}


/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	GlobalInterruptDisable();

	SetupHardware();

	LEDs_SetAllLEDs(0);
	GlobalInterruptEnable();

	// Proceed USB functions only if transmitter is ON
	int8_t prev_connected = 0;
	
	for (;;)
	{
		if(prev_connected != sat_is_connected)
		{
			if(sat_is_connected)
				USB_Init();
			else
				USB_Disable();
			
			prev_connected = sat_is_connected;
		}
		
		if(sat_is_connected)
		{
			LEDs_TurnOnLEDs(LEDS_LED1);        
			HID_Task();
			USB_USBTask();
		}
		else
			LEDs_TurnOffLEDs(LEDS_LED1);
 
	}
}




