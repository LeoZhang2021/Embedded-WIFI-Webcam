/*********************************************************************
 *
 *     UART access routines for C18 and C30
 *
 *********************************************************************
 * FileName:        UART.h
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
 * Howard Schlunder                6/16/06                Original
********************************************************************/
#ifndef __UART_H
#define __UART_H
#define CLOCK_FREQ                (40000000)      // Hz
#define INSTR_FREQ                        (CLOCK_FREQ/4)
//#define GetPeripheralClock()	(CLOCK_FREQ/4)
#define BAUD_RATE       (57600)     // bps
/*#define USART_USE_BRGH_LOW
#if defined(USART_USE_BRGH_LOW)
 #define SPBRG_VAL   ( ((INSTR_FREQ/BAUD_RATE)/16) - 1)
#else
    #define SPBRG_VAL   ( ((INSTR_FREQ/BAUD_RATE)/4) - 1)
#endif*/
/*	#if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
		SPBRG = (GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1;
		TXSTAbits.BRGH = 1;
	#else	// Use the low baud rate setting
		SPBRG = (GetPeripheralClock()+8*BAUD_RATE)/BAUD_RATE/16 - 1;
	#endif
*/
char BusyUSART(void);
void CloseUSART(void);
char DataRdyUSART(void);
char ReadUSART(void);
void WriteUSART(char data1);
void getsUSART(char *buffer, unsigned char len);
void putsUSART(char *data1);
void putrsUSART(const rom char *data);
#define getcUART()                                ReadUSART()
#define putcUART(a)                               WriteUSART(a)
#define putrsUART(a)		putrsUSART((far rom char*)a)
#define putsUART(a)			putsUSART(a)


#endif