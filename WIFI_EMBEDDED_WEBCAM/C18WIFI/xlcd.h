/*********************************************************************
 *
 *                  External LCD access routines defs
 *
 *********************************************************************
 * FileName:        XLCD.h
 * Dependencies:    compiler.h
 * Processor:       PIC18
 * Complier:        MCC18 v1.00.50 or higher
 *                  HITECH PICC-18 V8.10PL1 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PICmicro® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     5/8/02  Original        (Rev 1.0)
 * Nilesh Rajbharti     7/10/02 Improved
 ********************************************************************/

typedef struct
	{
		unsigned char data : 4;	// Bits 0 through 3
		unsigned char : 4;		// Bits 4 through 7
	} LCD_DATA;

    #define LCD_DATA_TRIS		(((volatile LCD_DATA*)&TRISD)->data)
	#define LCD_DATA_IO			(((volatile LCD_DATA*)&LATD)->data)
	#define LCD_RD_WR_TRIS		(TRISEbits.TRISE1)
	#define RW		         (LATEbits.LATE1)
	#define LCD_RS_TRIS			(TRISEbits.TRISE0)
	#define RS			(LATEbits.LATE0)
	#define LCD_E_TRIS			(TRISEbits.TRISE2)
	#define E			(LATEbits.LATE2)
    #define FOUR_BIT_MODE



/*
#define D7 PORTD.F3
#define D6 PORTD.F2
#define D5 PORTD.F1
#define D4 PORTD.F0
#define E  PORTE.F2
#define RS PORTE.F0
#define RW PORTE.F1*/

#define lcd_type 2           // 0=5x7, 1=5x10, 2=2 lines
//unsigned char const LCD_INIT_STRING[4] = {0x20 | (lcd_type << 2), 0xc, 1, 6};
#define lcd_line_two 0x40
void lcd_send_nibble( unsigned char n );
void lcd_send_byte( unsigned char address, unsigned char n );
void xlcd_init(void);
void lcd_gotoxy( unsigned char x, unsigned char y);
void lcd_putc( char c);
void LCDPutString(char *string);




