/*********************************************************************
 *
 *                  IP Defs for Microchip TCP/IP Stack
 *
 *********************************************************************
 * FileName:        IP.h
 * Dependencies:    StackTsk.h
 *                  MAC.h
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
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
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
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
 * Nilesh Rajbharti     4/27/01 Original        (Rev 1.0)
 * Nilesh Rajbharti     2/9/02  Cleanup
 * Nilesh Rajbharti     5/22/02 Rev 2.0 (See version.log for detail)
 ********************************************************************/
#ifndef __IP_H
#define __IP_H
#include"TCPIP.h"
#define IP_PROT_ICMP    (1u)
#define IP_PROT_TCP     (6u)
#define IP_PROT_UDP     (17u)


// IP packet header definition
typedef struct _IP_HEADER
{
    BYTE    VersionIHL;
    BYTE    TypeOfService;
    WORD    TotalLength;
    WORD    Identification;
    WORD    FragmentInfo;
    BYTE    TimeToLive;
    BYTE    Protocol;
    WORD    HeaderChecksum;
    IP_ADDR SourceAddress;
    IP_ADDR DestAddress;
} IP_HEADER;

// IP Pseudo header as defined by RFC 793 (needed for TCP and UDP 
// checksum calculations/verification)
typedef struct _PSEUDO_HEADER
{
    IP_ADDR SourceAddress;
    IP_ADDR DestAddress;
    BYTE Zero;
    BYTE Protocol;
    WORD Length;
} PSEUDO_HEADER;

#define SwapPseudoHeader(h)  (h.Length = swaps(h.Length))


/*********************************************************************
 * Function:        BOOL IPIsTxReady(BOOL HighPriority)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if transmit buffer is empty
 *                  FALSE if transmit buffer is not empty
 *
 * Side Effects:    None
 *
 * Note:            None
 *
 ********************************************************************/
#define IPIsTxReady()       MACIsTxReady()


/*********************************************************************
 * Macro:           IPSetTxBuffer(a, b)
 *
 * PreCondition:    None
 *
 * Input:           a       - Buffer identifier
 *                  b       - Offset
 *
 * Output:          Next Read/Write access to transmit buffer 'a'
 *                  set to offset 'b'
 *
 * Side Effects:    None
 *
 * Note:            None
 *
 ********************************************************************/
#define IPSetTxBuffer(b) MACSetWritePtr(b + BASE_TX_ADDR + sizeof(ETHER_HEADER) + sizeof(IP_HEADER))



/*********************************************************************
 * Function:        WORD IPPutHeader(   IP_ADDR *Dest,
 *                                      BYTE    Protocol,
 *                                      WORD    Identifier,
 *                                      WORD    DataLen)
 *
 * PreCondition:    IPIsTxReady() == TRUE
 *
 * Input:           Src         - Destination node address
 *                  Protocol    - Current packet protocol
 *                  Identifier  - Current packet identifier
 *                  DataLen     - Current packet data length
 *
 * Output:          Handle to current packet - For use by
 *                  IPSendByte() function.
 *
 * Side Effects:    None
 *
 * Note:            Only one IP message can be transmitted at any
 *                  time.
 *                  Caller may not transmit and receive a message
 *                  at the same time.
 *
 ********************************************************************/
WORD    IPPutHeader(NODE_INFO *remote,
                    BYTE protocol,
                    WORD len);


/*********************************************************************
 * Function:        BOOL IPGetHeader( IP_ADDR    *localIP,
 *                                    NODE_INFO  *remote,
 *                                    BYTE        *Protocol,
 *                                    WORD        *len)
 *
 * PreCondition:    MACGetHeader() == TRUE
 *
 * Input:           localIP     - Local node IP Address as received
 *                                in current IP header.
 *                                If this information is not required
 *                                caller may pass NULL value.
 *                  remote      - Remote node info
 *                  Protocol    - Current packet protocol
 *                  len         - Current packet data length
 *
 * Output:          TRUE, if valid packet was received
 *                  FALSE otherwise
 *
 * Side Effects:    None
 *
 * Note:            Only one IP message can be received.
 *                  Caller may not transmit and receive a message
 *                  at the same time.
 *
 ********************************************************************/
BOOL IPGetHeader(IP_ADDR *localIP,
                 NODE_INFO *remote,
                 BYTE *protocol,
                 WORD *len,WORD *len1);


/*********************************************************************
 * Macro:           IPDiscard()
 *
 * PreCondition:    MACGetHeader() == TRUE
 *
 * Input:           None
 *
 * Output:          Current packet is discarded and buffer is
 *                  freed-up
 *
 * Side Effects:    None
 *
 * Note:            None
 *
 ********************************************************************/
#define IPDiscard()         MACDiscard()



/*********************************************************************
 * Macro:           IPGetArray(a, b)
 *
 * PreCondition:    MACGetHeader() == TRUE
 *
 * Input:           a       - Data buffer
 *                  b       - Buffer length
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Note:            Data is copied from IP data to given buffer
 *
 ********************************************************************/
#define IPGetArray(a, b)    MACGetArray(a, b)




/*********************************************************************
 * Function:        IPSetRxBuffer(WORD Offset)
 *
 * PreCondition:    IPHeaderLen must have been intialized by 
 *					IPGetHeader() or IPPutHeader()
 *
 * Input:           Offset from beginning of IP data field
 *
 * Output:          Next Read/Write access to receive buffer is
 *                  set to Offset 
 *
 * Side Effects:    None
 *
 * Note:            None
 *
 ********************************************************************/
void IPSetRxBuffer(WORD Offset);

#define serial_error()  RCSTAbits.OERR            // USART error
#define serial_fix()   {RCSTAbits.CREN=0;RCSTAbits.CREN=1;}   
#define C328R_COMMAND_SYNC               0x0D
#define C328R_COMMAND_PREFIX             0xAA
#define C328R_COMMAND_ACK                0x0E
#define C328R_COMMAND_SET_BAUDRATE       0x07
#define C328R_BAUD_38400BPS              0x2f
#define C328R_BAUD_2ND_DIVIDER           0x01
#define C328R_COMMAND_LIGHT_FREQUENCY    0x13
#define C328R_FREQUENCY                  0x00
#define C328R_COMMAND_SET_PACKAGE_SIZE   0x06
#define C328R_PACKAGE_SIZE                488
#define C328R_PACKAGE_SIZE_LOW_BYTE      C328R_PACKAGE_SIZE & 0xff
#define C328R_PACKAGE_SIZE_HIGH_BYTE     C328R_PACKAGE_SIZE >> 8
#define C328R_PACKAGE_OVERHEAD            6
#define C328R_COMMAND_INITIAL            0x01
#define C328R_COLTYPE_JPEG           0x07
#define C328R_PREVIEW_160x120        0x03
#define C328R_PICTYPE_SNAPSHOT       0x01
#define C328R_COMMAND_GET_PICTURE        0x04
#define C328R_SNAPTYPE_COMPRESSED    0x00
#define C328R_COMMAND_SNAPSHOT           0x05
#define C328R_NULL                       0x00
#define C328R_LAST_PACKAGE               0xF0F0

void c328syn();
void c328r_send_ack(BYTE command, WORD package_id);
void c328setupS();
void c328setupL();
void c328snap();
void c328getpic();
void c328reset();
void c328pwroff();
void cameraS(void);
void cameraL(void);

void decode_tcp(WORD size, NODE_INFO *remote);
void reply_tcp(BYTE type);
void ne_print(char text) ;
void finish_tcp(char type, BYTE tx1);
void flush_print(void);
void  put_eeprom();
void ne_print1( char *data);
void ne_prints(const rom char *string);
//void reply_tcp(BYTE type);
void write_ip_hdr(WORD len,BYTE protocol);
void write_mac_hdr(BYTE tx1);
void write_ip_check(BYTE tx1);
void write_tcp_check(WORD len,BYTE tx1);




#endif



