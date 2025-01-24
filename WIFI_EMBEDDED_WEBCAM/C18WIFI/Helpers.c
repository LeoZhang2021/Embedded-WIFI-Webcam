/*********************************************************************
 *
 *           Helper Functions for Microchip TCP/IP Stack
 *
 *********************************************************************
 * FileName:                Helpers.C
 * Dependencies:        None
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *                                        Microchip C30 v3.12 or higher
 *                                        Microchip C18 v3.30 or higher
 *                                        HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *                ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *                used in conjunction with a Microchip ethernet controller for
 *                the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     5/17/01 Original        (Rev 1.0)
 * Nilesh Rajbharti     2/9/02  Cleanup
 * Nilesh Rajbharti     6/25/02 Rewritten CalcIPChecksum() to avoid
 *                              multi-byte shift operation.
 * Howard Schlunder                2/9/05  Added hexatob(), btohexa_high(), and
 *                                                            btohexa_low()
 * Howard Schlunder    10/10/06 Optimized swapl()
 * Elliott Wood                   11/20/07        Added leftRotateDWORD()
 ********************************************************************/
#define __HELPERS_C

#include "TCPIP.h"


/*****************************************************************************
  Function:
        DWORD LFSRSeedRand(DWORD dwSeed)

  Summary:
        Seeds the LFSR random number generator invoked by the LFSRRand() function.  
        The prior seed is returned.

  Description:
        Seeds the LFSR random number generator invoked by the LFSRRand() function.  
        The prior seed is returned.

  Precondition:
        None

  Parameters:
        wSeed - The new 32-bit seed value to assign to the LFSR.

  Returns:
          The last seed in use.  This can be saved and restored by a subsequent call 
        to LFSRSeedRand() if you wish to use LFSRRand() in multiple contexts 
        without disrupting the random number sequence from the alternative 
        context.  For example, if App 1 needs a given sequence of random numbers 
        to perform a test, if you save and restore the seed in App 2, it is 
        possible for App 2 to not disrupt the random number sequence provided to 
        App 1, even if the number of times App 2 calls LFSRRand() varies.
          
  Side Effects:
        None
        
  Remarks:
        Upon initial power up, the internal seed is initialized to 0x1.  Using a 
        dwSeed value of 0x0 will return the same sequence of random numbers as 
        using the seed of 0x1.
  ***************************************************************************/

 /* Function:
        WORD CalcIPChecksum(BYTE* buffer, WORD count)

  Summary:
        Calculates an IP checksum value.

  Description:
        This function calculates an IP checksum over an array of input data.  The
        checksum is the 16-bit one's complement of one's complement sum of all 
        words in the data (with zero-padding if an odd number of bytes are 
        summed).  This checksum is defined in RFC 793.

  Precondition:
        buffer is WORD aligned (even memory address) on 16- and 32-bit PICs.

  Parameters:
        buffer - pointer to the data to be checksummed
        count  - number of bytes to be checksummed

  Returns:
        The calculated checksum.
        
  Internal:
        This function could be improved to do 32-bit sums on PIC32 platforms.
  ***************************************************************************/
WORD CalcIPChecksum(BYTE* buffer, WORD count)
{
        WORD i;
        WORD *val;
        union
        {
                WORD w[2];
                DWORD dw;
        } sum;

        i = count >> 1;
        val = (WORD*)buffer;

        // Calculate the sum of all words
        sum.dw = 0x00000000ul;
        while(i--)
                sum.dw += (DWORD)*val++;

        // Add in the sum of the remaining byte, if present
        if(count & 0x1)
                sum.dw += (DWORD)*(BYTE*)val;

        // Do an end-around carry (one's complement arrithmatic)
        sum.dw = sum.w[0] + sum.w[1];

        // Do another end-around carry in case if the prior add 
        // caused a carry out
        sum.w[0] += sum.w[1];

        // Return the resulting checksum
        return ~sum.w[0];
}


/*****************************************************************************
  Function:
        WORD CalcIPBufferChecksum(WORD len)

  Summary:
        Calculates an IP checksum in the MAC buffer itself.

  Description:
        This function calculates an IP checksum over an array of input data 
        existing in the MAC buffer.  The checksum is the 16-bit one's complement 
        of one's complement sum of all words in the data (with zero-padding if 
        an odd number of bytes are summed).  This checksum is defined in RFC 793.

  Precondition:
        TCP is initialized and the MAC buffer pointer is set to the start of
        the buffer.

  Parameters:
        len - number of bytes to be checksummed

  Returns:
        The calculated checksum.

  Remarks:
        All Microchip MACs should perform this function in hardware.
  ***************************************************************************/

WORD CalcIPBufferChecksum(WORD len)
{
        DWORD_VAL Checksum = {0x00000000ul};
        WORD ChunkLen;
        BYTE DataBuffer[20];        // Must be an even size
        WORD *DataPtr;

        while(len)
        {
                // Obtain a chunk of data (less SPI overhead compared 
                // to requesting one byte at a time)
                ChunkLen = len > sizeof(DataBuffer) ? sizeof(DataBuffer) : len;
                MACGetArray(DataBuffer, ChunkLen);
                len -= ChunkLen;

                // Take care of a last odd numbered data byte
                if(((WORD_VAL*)&ChunkLen)->bits.b0)
                {
                        DataBuffer[ChunkLen] = 0x00;
                        ChunkLen++;
                }

                // Calculate the checksum over this chunk
                DataPtr = (WORD*)&DataBuffer[0];
                while(ChunkLen)
                {
                        Checksum.Val += *DataPtr++;
                        ChunkLen -= 2;
                }
        }
        
        // Do an end-around carry (one's complement arrithmatic)
        Checksum.Val = (DWORD)Checksum.w[0] + (DWORD)Checksum.w[1];

        // Do another end-around carry in case if the prior add 
        // caused a carry out
        Checksum.w[0] += Checksum.w[1];

        // Return the resulting checksum
        return ~Checksum.w[0];
}
 WORD swaps(WORD v)
{
        WORD_VAL t;
        BYTE b;

        t.Val   = v;
        b       = t.v[1];
        t.v[1]  = t.v[0];
        t.v[0]  = b;

        return t.Val;
}