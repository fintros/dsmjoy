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

/* This file proceed all functionality related to DSM satellite communiction
   USART and TIMER is used for processing
*/

#include "DSMJoy.h"

// global variables
int8_t sat_is_connected=0;
USB_JoystickReport_Data_t JoystickReportData;

// local variables
static int8_t byte_in_frame=0;  // byte number in frame
static int8_t counter=0;		// byte counter
static int8_t timer_event;		// kind of timer interrupt


// Binding sequence - do not works with HW v.0.1 - power issues
void SAT_Bind()
{
	LEDs_TurnOnLEDs( LEDS_LED2);
   
	DDRD &= ~(1 << SAT_PIN);     // Rx is input
	PORTD |= (1 << SAT_PIN);   // enable pull-up to detect connection of sat by zero impulse
	
	_delay_ms(200); // allow pins to switch mode

	// wait for zero on pin
	while(1)
	{
		if (!(PIND & (1 << SAT_PIN))) {
		   break; 
		}    
	}	
	  
	int delaycounter = 0;

	// proceed bing sequence Thanks to mikrokopter.de for idea
	DDRD |= (1 << SAT_PIN);     // Rx as output

	_delay_ms(90);
	for (delaycounter = 0; delaycounter < 100; delaycounter++) PORTD |= (1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 400; delaycounter++) PORTD &= ~(1 << SAT_PIN);

	for (delaycounter = 0; delaycounter < 10; delaycounter++) PORTD |= (1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 10; delaycounter++) PORTD &= ~(1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 400; delaycounter++) PORTD |= (1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 400; delaycounter++) PORTD &= ~(1 << SAT_PIN);

	for (delaycounter = 0; delaycounter < 10; delaycounter++) PORTD |= (1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 10; delaycounter++) PORTD &= ~(1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 400; delaycounter++) PORTD |= (1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 400; delaycounter++) PORTD &= ~(1 << SAT_PIN);

	for (delaycounter = 0; delaycounter < 10; delaycounter++) PORTD |= (1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 10; delaycounter++) PORTD &= ~(1 << SAT_PIN);
	for (delaycounter = 0; delaycounter < 400; delaycounter++) PORTD |= (1 << SAT_PIN);
  
	DDRD &= ~(1 << SAT_PIN);      // RX as input
	PORTD &= ~(1 << SAT_PIN);

	LEDs_TurnOffLEDs( LEDS_LED2);    
	
}

  
void Sat_Init(void)
{
	if(Button_GetStatus())
		SAT_Bind();

	byte_in_frame=0;
	counter = 0;
	sat_is_connected = 0;

    for(int i = 0; i<16; i++)
       JoystickReportData.channels[i] = 0;    

	// Serial processing 115200	
	UBRR1H = 0;
	UBRR1L = 8; 

	// enable RX and set interrupts on rx complete
	UCSR1B = (1 << RXEN1) | (1 << RXCIE1);

	// 8-bit, 1 stop bit, no parity, asynchronous UART
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10) | (0 << USBS1) |
			(0 << UPM11) | (0 << UPM10) | (0 << UMSEL11) |
			(0 << UMSEL10);
	
	// to detect connection - use timer:
	TIMSK1 |= (1 << TOIE1); // Enable overflow interrupt 
	
	TCNT1 = MAX_UART_TIMEOUT; // Preload timer with precalculated value 
	timer_event = T_DISCONNECT_EVENT;

    TCCR1B |= ((1 << CS10) | (1 << CS11)); // Set up timer at Fcpu/64  == 250 kHz
}
  
ISR(USART1_RX_vect)
{
   	TCNT1 = MAX_UART_FRAME_TIMEOUT; // Reset disconnect timer 
 	timer_event = T_FRAME_EVENT;
	sat_is_connected = 1;
	
    static int bytes[2];

    bytes[counter] = UDR1; 
    counter++;
    byte_in_frame++;

	if(counter>1)
    {
	  counter = 0;
	  if((byte_in_frame != 2) && (byte_in_frame != 18)) // skip first 2 bytes in frame - control sum + signal level
	  {
		int value = (((bytes[0]&0x3)<<8) | bytes[1]) - CHANNEL_MAX/2;
		int chanid = (bytes[0] >>2) & 0x0F;
		JoystickReportData.channels[chanid] = value;
	  }
	} 
}

ISR(TIMER1_OVF_vect) 
{ 
   if(timer_event==T_FRAME_EVENT)
   {
		timer_event = T_DISCONNECT_EVENT;
		TCNT1  = MAX_UART_TIMEOUT;
   }
   else
		sat_is_connected = 0;

   byte_in_frame = 0;
   counter=0;
}
