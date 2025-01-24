/*********************************************************************
 *
 *     UART access routines for C18 and C30
 *
 *********************************************************************
 * FileName:        UART.c
 * Dependencies:    UART.h
 * Processor:       PIC18, PIC24F/H, dsPIC30F, dsPIC33F
 * Complier:        Microchip C18 v3.03 or higher
 * Complier:        Microchip C30 v2.01 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * This software is owned by Microchip Technology Inc. ("Microchip") 
 * and is supplied to you for use exclusively as described in the 
 * associated software agreement.  This software is protected by 
 * software and other intellectual property laws.  Any use in 
 * violation of the software license may subject the user to criminal 
 * sanctions as well as civil liability.  Copyright 2006 Microchip
 * Technology Inc.  All rights reserved.
 *
 * This software is provided "AS IS."  MICROCHIP DISCLAIMS ALL 
 * WARRANTIES, EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, NOT LIMITED 
 * TO MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND 
 * INFRINGEMENT.  Microchip shall in no event be liable for special, 
 * incidental, or consequential damages.
 *
 *
 * Author               Date                   Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder                4/04/06                Copied from dsPIC30 libraries
 * Howard Schlunder                6/16/06                Added PIC18
********************************************************************/
#define THIS_IS_UART


#include "TCPIP.h"

char BusyUSART(void)
{
 return !TXSTAbits.TRMT;
}

void CloseUSART(void)
{
  RCSTA &= 0b01001111;  // Disable the receiver
 TXSTAbits.TXEN = 0;     // and transmitter

  PIE1 &= 0b11001111;   // Disable both interrupts
}

char DataRdyUSART(void)
{
   if(RCSTAbits.OERR)
	{
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}
  return PIR1bits.RCIF; 

  
}

char ReadUSART(void)
{
  return RCREG;                     // Return the received data
}

void WriteUSART(char data1)
{
  TXREG = data1;      // Write the data byte to the USART
}

void getsUSART(char *buffer, unsigned char len)
{
  char i;    // Length counter
  unsigned char data1;

  for(i=0;i<len;i++)  // Only retrieve len characters
  {
    while(!DataRdyUSART());// Wait for data to be received

    data1 = getcUART();    // Get a character from the USART
                           // and save in the string
    *buffer = data1;
    buffer++;              // Increment the string pointer
  }
}

void putsUSART( char *data1)
{
  do
  {  // Transmit a byte
    while(BusyUSART());
    putcUART(*data1);
  } while( *data1++ );
}

void putrsUSART(const rom char *data)
{
  do
  {  // Transmit a byte
    while(BusyUSART());
    putcUART(*data);
  } while( *data++ );
}