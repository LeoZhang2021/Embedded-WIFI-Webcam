/*********************************************************************
 *
 *                  External LCD access routines
 *
 *********************************************************************
 * FileName:        XLCD.c
 * Dependencies:    xlcd.h
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
 * HiTech PICC18 Compiler Options excluding device selection:
 *                  -FAKELOCAL -G -E -C
 *
 *
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     5/8/02  Original        (Rev 1.0)
 * Nilesh Rajbharti     7/10/02 Optimized
 ********************************************************************/
#include "TCPIP.h"
   unsigned char const LCD_INIT_STRING[4] = {0x20 | (lcd_type << 2), 0xc, 1, 6};
void lcd_send_nibble( unsigned char n ) {
      LCD_DATA_IO  = n;
      Nop();;
      E = 1;
      Nop();
      Nop();
      E = 0;
}




 void lcd_send_byte( unsigned char address, unsigned char n ) {
  RS = address;
   Nop();
   E = 0;
   lcd_send_nibble(n >> 4);
   lcd_send_nibble(n & 0xf);
   DelayMs(200);
}

 
void xlcd_init() {
    unsigned char i;
    RS = 0;
    E = 0;
    DelayMs(15);
    for(i=1;i<=3;++i) {
     lcd_send_nibble(3);
      DelayMs(10);
    }
    lcd_send_nibble(2);
    for(i=0;i<=3;++i) {
    lcd_send_byte(0,LCD_INIT_STRING[i]);
         DelayMs(10);
         }
}

void lcd_gotoxy( unsigned char x, unsigned char y) {
   unsigned char address;

   if(y!=1)
     address=lcd_line_two;
   else
     address=0;
   address+=x-1;
   lcd_send_byte(0,0x80|address);
}

void lcd_putc( char c) {
   switch (c) {
     case '\f'   : lcd_send_byte(0,1);
                   DelayMs(2);
                                           break;
     case '\n'   : lcd_gotoxy(1,2);        break;
     case '\b'   : lcd_send_byte(0,0x10);  break;
     default     : lcd_send_byte(1,c);     break;
   }
}
void LCDPutString(char *string)
{
    char v;

    while( v = *string )
    {
      lcd_putc(v);
        string++;
    }
}