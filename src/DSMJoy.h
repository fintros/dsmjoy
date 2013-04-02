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

/* DSMJoy header file 
*/

#ifndef _DSMJOY_H_
#define _DSMJOY_H_

#define MAX_UART_TIMEOUT (65535-25000) 		// 100 ms
#define MAX_UART_FRAME_TIMEOUT (65535-1250) // 5ms
#define CHANNEL_MAX 1023

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>

#include <LUFA/Drivers/USB/USB.h>

#define SAT_PIN (1<<2)

enum
{
   T_FRAME_EVENT=1,
   T_DISCONNECT_EVENT
};

typedef struct
{
	int16_t  channels[8]; 
} USB_JoystickReport_Data_t;


extern USB_JoystickReport_Data_t JoystickReportData;
extern int8_t sat_is_connected;

void HID_Task(void);
void Sat_Init(void);

#include <avr/io.h>

#define BUTTONS_BUTTON1      (1 << 3)

static inline void Buttons_Init(void)
{
	DDRD  &= ~BUTTONS_BUTTON1;
	PORTD |=  BUTTONS_BUTTON1;
}

static inline void Buttons_Disable(void)
{
	DDRD  &= ~BUTTONS_BUTTON1;
	PORTD &= ~BUTTONS_BUTTON1;
}

static inline uint8_t Button_GetStatus(void)
{
	return ((PIND & BUTTONS_BUTTON1) ^ BUTTONS_BUTTON1);
}


#define LEDS_LED1        (1 << 4)
#define LEDS_LED2        (1 << 5)

#define LEDS_ALL_LEDS    (LEDS_LED1 | LEDS_LED2)

static inline void LEDs_Init(void)
{
	DDRC  |=  LEDS_ALL_LEDS;
	PORTC &= ~LEDS_ALL_LEDS;
}

static inline void LEDs_Disable(void)
{
	DDRC  &= ~LEDS_ALL_LEDS;
	PORTC &= ~LEDS_ALL_LEDS;
}

static inline void LEDs_TurnOnLEDs(const uint8_t LedMask)
{
	PORTC |= LedMask;
}

static inline void LEDs_TurnOffLEDs(const uint8_t LedMask)
{
	PORTC &= ~LedMask;
}

static inline void LEDs_SetAllLEDs(const uint8_t LedMask)
{
	PORTC = ((PORTC & ~LEDS_ALL_LEDS) | LedMask);
}

static inline void LEDs_ChangeLEDs(const uint8_t LedMask,
								   const uint8_t ActiveMask)
{
	PORTC = ((PORTC & ~LedMask) | ActiveMask);
}

static inline void LEDs_ToggleLEDs(const uint8_t LEDMask)
{
	PINC  = LEDMask;
}

static inline uint8_t LEDs_GetLEDs(void)
{
	return (PORTC & LEDS_ALL_LEDS);
}



#endif