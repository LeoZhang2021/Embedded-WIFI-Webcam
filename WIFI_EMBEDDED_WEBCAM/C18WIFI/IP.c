/*********************************************************************
 *
 *  Internet Protocol (IP) Version 4 Communications Layer
 *  Module for Microchip TCP/IP Stack
 *   -Provides a transport for TCP, UDP, and ICMP messages
 *	 -Reference: RFC 791
 *
 *********************************************************************
 * FileName:        IP.c
 * Dependencies:    Network Layer interface (ENC28J60.c, ETH97J60.c, 
 *					ENCX24J600.c or WFMac.c)
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
 * Howard Schlunder		8/31/04	Beta Rev 0.9 (See version.log for detail)
 * Howard Schlunder		1/5/06	Improved DMA checksum efficiency
 * Darren Rook			9/21/06	Corrected IPHeaderLen not being 
 * 								initialized when NON_MCHP_MAC defined.
 ********************************************************************/
#define __IP_C

#include "TCPIP.h"
 BYTE LCDText[16];
// This is left shifted by 4.  Actual value is 0x04.
#define IPv4                (0x40u)
#define IP_VERSION          IPv4

// IHL (Internet Header Length) is # of DWORDs in a header.
// Since, we do not support options, our IP header length will be
// minimum i.e. 20 bytes : IHL = 20 / 4 = 5.
#define IP_IHL              (0x05)

#define IP_SERVICE_NW_CTRL  (0x07)
#define IP_SERVICE_IN_CTRL  (0x06)
#define IP_SERVICE_ECP      (0x05)
#define IP_SERVICE_OVR      (0x04)
#define IP_SERVICE_FLASH    (0x03)
#define IP_SERVICE_IMM      (0x02)
#define IP_SERVICE_PRIOR    (0x01)
#define IP_SERVICE_ROUTINE  (0x00)

#define IP_SERVICE_N_DELAY  (0x00)
#define IP_SERCICE_L_DELAY  (0x08)
#define IP_SERVICE_N_THRPT  (0x00)
#define IP_SERVICE_H_THRPT  (0x10)
#define IP_SERVICE_N_RELIB  (0x00)
#define IP_SERVICE_H_RELIB  (0x20)
#define IP      0x0800
#define IP_SERVICE          (IP_SERVICE_ROUTINE | IP_SERVICE_N_DELAY)
#define IP_TCP  0x06
#if defined(STACK_USE_ZEROCONF_MDNS_SD)
  #define MY_IP_TTL           (255)  // Time-To-Live in hops 
  // IP TTL is set to 255 for Multicast DNS compatibility. See mDNS-draft-08, section 4.
#else
  #define MY_IP_TTL           (100)  // Time-To-Live in hops
#endif



static WORD _Identifier = 0;
static WORD sequence = 0;
static BYTE IPHeaderLen;


static void SwapIPHeader(IP_HEADER* h);

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_MAX_SIZE 1450
WORD size,hit_count;
#define  CARRY  (STATUS & 0x01)
WORD sendr_port, dest_port,rx_seq1,rx_seq0,rx_ack1,rx_ack0 ;
BYTE sendr_ip[4],sendr_ha[6];
WORD  id,seq_number, ne_check2,ne_check1,ne_count1,ne_count2;
BYTE ne_byte,type1;
BYTE  code;
BOOL bin_print,isr_last; 
#define MPFS_DATA          (0x00u)
#define MPFS_DELETED       (0x01u)
#define MPFS_DLE           (0x03u)
#define MPFS_ETX           (0x04u)
typedef ROM BYTE* MPFS;
#define MPFS_INVALID                (MPFS)(0xffffff)
MPFS _currentHandle;



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
                 WORD *len,WORD *len1 )
{
    WORD_VAL    CalcChecksum;
    IP_HEADER   header;

#if defined(NON_MCHP_MAC)
    WORD_VAL    ReceivedChecksum;
    WORD        checksums[2];
    BYTE        optionsLen;
	#define MAX_OPTIONS_LEN     (40u)            // As per RFC 791.
    BYTE        options[MAX_OPTIONS_LEN];
#endif

    // Read IP header.
    MACGetArray((BYTE*)&header, sizeof(header));

    // Make sure that this is an IPv4 packet.
    if((header.VersionIHL & 0xf0) != IP_VERSION)
    	return FALSE;

	// Throw this packet away if it is a fragment.  
	// We don't have enough RAM for IP fragment reconstruction.
	if(header.FragmentInfo & 0xFF1F)
		return FALSE;

	IPHeaderLen = (header.VersionIHL & 0x0f) << 2;

#if !defined(NON_MCHP_MAC)
	// Validate the IP header.  If it is correct, the checksum 
	// will come out to 0x0000 (because the header contains a 
	// precomputed checksum).  A corrupt header will have a 
	// nonzero checksum.
	CalcChecksum.Val = MACCalcRxChecksum(0, IPHeaderLen);

	// Seek to the end of the IP header
	MACSetReadPtrInRx(IPHeaderLen);

    if(CalcChecksum.Val)
#else
    // Calculate options length in this header, if there is any.
    // IHL is in terms of numbers of 32-bit DWORDs; i.e. actual
    // length is 4 times IHL.
    optionsLen = IPHeaderLen - sizeof(header);

    // If there is any option(s), read it so that we can include them
    // in checksum calculation.
    if ( optionsLen > MAX_OPTIONS_LEN )
        return FALSE;

    if ( optionsLen > 0u )
        MACGetArray(options, optionsLen);

    // Save header checksum; clear it and recalculate it ourselves.
    ReceivedChecksum.Val = header.HeaderChecksum;
    header.HeaderChecksum = 0;

    // Calculate checksum of header including options bytes.
    checksums[0] = ~CalcIPChecksum((BYTE*)&header, sizeof(header));

    // Calculate Options checksum too, if they are present.
    if ( optionsLen > 0u )
        checksums[1] = ~CalcIPChecksum((BYTE*)options, optionsLen);
    else
        checksums[1] = 0;

    CalcChecksum.Val  = CalcIPChecksum((BYTE*)checksums,
                                            2 * sizeof(WORD));

    // Make sure that checksum is correct
    if ( ReceivedChecksum.Val != CalcChecksum.Val )
#endif
    {
        // Bad packet. The function caller will be notified by means of the FALSE 
        // return value and it should discard the packet.
        return FALSE;
    }

    // Network to host conversion.
    SwapIPHeader(&header);

    // If caller is intrested, return destination IP address
    // as seen in this IP header.
    if ( localIP )
        localIP->Val    = header.DestAddress.Val;

    remote->IPAddr.Val  = header.SourceAddress.Val;
    *protocol           = header.Protocol;
    *len 				= header.TotalLength - IPHeaderLen;
    *len1                =header.TotalLength;
    return TRUE;
}




/*********************************************************************
 * Function: WORD IPPutHeader(NODE_INFO *remote,
 *           				  BYTE protocol,
 *                			  WORD len)
 *
 * PreCondition:    IPIsTxReady() == TRUE
 *
 * Input:           *remote     - Destination node address
 *                  protocol    - Current packet protocol
 *                  len         - Current packet data length
 *
 * Output:          (WORD)0
 *
 * Side Effects:    None
 *
 * Note:            Only one IP message can be transmitted at any
 *                  time.
 ********************************************************************/
WORD IPPutHeader(NODE_INFO *remote,
                 BYTE protocol,
                 WORD len)
{
    IP_HEADER   header;
    
    IPHeaderLen = sizeof(IP_HEADER);

    header.VersionIHL       = IP_VERSION | IP_IHL;
    header.TypeOfService    = IP_SERVICE;
    header.TotalLength      = sizeof(header) + len;
    header.Identification   = ++_Identifier;
    header.FragmentInfo     = 0;
    header.TimeToLive       = MY_IP_TTL;
    header.Protocol         = protocol;
    header.HeaderChecksum   = 0;
	header.SourceAddress 	= AppConfig.MyIPAddr;

    header.DestAddress.Val = remote->IPAddr.Val;

    SwapIPHeader(&header);

    header.HeaderChecksum   = CalcIPChecksum((BYTE*)&header, sizeof(header));

    MACPutHeader(&remote->MACAddr, MAC_IP, (sizeof(header)+len));
    MACPutArray((BYTE*)&header, sizeof(header));

    return 0x0000;

}

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
void IPSetRxBuffer(WORD Offset) 
{
	MACSetReadPtrInRx(Offset+IPHeaderLen);
}



static void SwapIPHeader(IP_HEADER* h)
{
    h->TotalLength      = swaps(h->TotalLength);
    h->Identification   = swaps(h->Identification);
    h->HeaderChecksum   = swaps(h->HeaderChecksum);
}
 void decode_tcp(WORD size,NODE_INFO *remote){
// BYTE readPtrL;
 BYTE i;
  
// decode tcp message in ram buffer
// size - number of bytes of message to decode
  BYTE flags;
  for(i=0;i<4;i++)
  sendr_ip[i]= remote->IPAddr.v[i];
  for(i=0;i<6;i++)
  sendr_ha[i]= remote->MACAddr.v[i];
  sprintf(LCDText,"remote-mac =%x:%x:%x:%x:%x:%x\r\n",remote->MACAddr.v[0],remote->MACAddr.v[1],remote->MACAddr.v[2],remote->MACAddr.v[3],remote->MACAddr.v[4],remote->MACAddr.v[5]);
  putsUART(LCDText);
  sprintf(LCDText,"remote-ip =%x:%x:%x:%x\r\n",remote->IPAddr.v[0],remote->IPAddr.v[1],remote->IPAddr.v[2],remote->IPAddr.v[3]);
  putsUART(LCDText);
  //int fl;
  // get source and destination port
  sendr_port = inport();
   sprintf(LCDText,"sender_ip=%x:%x:\r\n",sendr_port>>8,sendr_port &0xff );
   putsUART(LCDText);
  dest_port = inport();
  // sequence and acknowledge data
  rx_seq1 = inport();
  rx_seq0 = inport();
  rx_ack1 = inport();
  rx_ack0 = inport();
  // header length and flags
   inportb();//&0x3f;
  flags=inportb() ;
   if (flags==TCP_SYN) {
    putrsUART("RX:TCP-SYN\r\n");
    // web browser opening socket
   // sequence number
       rx_seq0++;
      if (rx_seq0 == 0)
      rx_seq1++;
    // send reply
    reply_tcp(TCP_ACK);


  } else if (flags==TCP_ACK) {
    // web browser socket is open
      putrsUART("RX:TCP-ACK\r\n");
  } else if (flags==(TCP_ACK|TCP_PSH)) {
    // web browser with request
    // absorb window, checksum and urgent pointer
    putrsUART("RX:TCP-ACK-PSH\r\n");
    inport();
    inport();
    inport();
       // sequence number
    rx_seq0 += size-40;  // increment sequence number by size of rx packet
    if (CARRY)
    rx_seq1++;
     reply_tcp(0);
  } else if (flags&TCP_FIN) {
    // web browser closing socket
  putrsUART("RX:TCP-FIN\r\n");
       // sequence number
    rx_seq0 += size-40;  // increment sequence number by size of rx packet
    if (CARRY)
      rx_seq1++;
    reply_tcp(TCP_FIN);
  } else {
    // unhandled TCP type
   putrsUART("RX:UNKNOWN\r\n");

  }

 }

void write_mac_hdr(BYTE tx1) {
// write mac (ethernet) header
// len - total amount of data to send in the ethernet packet
  // setup to write ram buffer
//set ETXST to transmit window
 if (tx1==1){
 //	while(!MACIsTxReady());
//	MACSetWritePtr(BASE_TX_ADDR);
     
}


  	while(!MACIsTxReady());
      wifi_set();
/*
  g_txBufferFlushed = FALSE;
     // Set the SPI write pointer to the beginning of the transmit buffer (post WF_TX_PREAMBLE_SIZE)
    g_encIndex[ENC_WT_PTR_ID] = TXSTART + WF_TX_PREAMBLE_SIZE;
    SyncENCPtrRAWState(ENC_WT_PTR_ID);
*/
       
  // write hardware addresses
  outportb(sendr_ha[0]);
  outportb(sendr_ha[1]);
  outportb(sendr_ha[2]);
  outportb(sendr_ha[3]);
  outportb(sendr_ha[4]);
  outportb(sendr_ha[5]);
  outportb(0xaa);
  outportb(0xaa);
  outportb(0x03);
  outportb(0x00);
  outportb(0x00);
  outportb(0x00);
 
}



void write_ip_hdr(WORD len,BYTE protocol) {
// write ip header into output ram buffer
// len - number of bytes in IP portion.
// protocol - type of IP message (ICMP, TCP etc)
 WORD index;

  // message type : IP
  outport(IP);//0c-0d
  while(!IPIsTxReady());
  // IP header, starting with prelim info (IP version, header size etc)
  outport(0x4500);//0e-0f
  // total size
  outport(len);  //10-11
  // identification
  outport(++sequence);//12-13
  // fragmentation flags and offset
  outport(0x0000);//14-15
  // time to live and protocol
  outport(0x2000+protocol);//16-17
  // checksum placeholder, will be calculated with calc_ip_check()
  outport(0x0000);//18-19
  // source IP
 for (index=0;index<4;index++)
   outportb(AppConfig.MyIPAddr.v[index]);
 // outport(local_ip[0]);//1a-1b
//  outport(local_ip[1]);//1c-1d
  // destination IP
  for (index=0;index<4;index++)
   outportb(sendr_ip[index]);//1e-1f,20-21
}
/*void write_ip_check(BYTE tx1){
WORD check;

MACSetReadPtr(BASE_TX_ADDR+0x0e );
check = CalcIPBufferChecksum(20);
//CalcIPBufferChecksum(WORD len)
//check=MACCalcTxChecksum(0x0e,20);
  check = swaps(check);
 MACSetWritePtr(BASE_TX_ADDR+0x18);
 outport(check);
}*/



void write_ip_check(BYTE tx1) {
// writes the IP checksum.  Assumes MAC header is 14 bytes, and IP header is 20
// bytes (thus no options), IP checksum is 12 words into the ethernet packet, and
// existing checksum is 0x00.  Must be called just before sending ethernet packet.
  WORD check,prev_check;
  WORD datum;
//  WORD datum1;
  BYTE index;
  check = 0;
//  datum1=0x2f3e;
  

   MACSetReadPtr(BASE_TX_ADDR+0x0e);
  //  WriteReg(ERXRDPT, TXSTART+0x0e);  
   
   
   if (tx1==2){
  //  WriteReg(ERXRDPT,0X1c0e);  
         }

  // calc checksum
  for (index = 0;index<10;index++) {
    datum = inport();
  //  sprintf(LCDText,"ip_check =%x%x\r\n",(datum >>8) &0xff,datum &0xff); 
  //  putsUART(LCDText);
    prev_check = check;
    check += datum;
    // check carry bit
    if (CARRY)
      check++;  // wrap for one's complement addition
  }
  check = ~check;  // one's complement
   // setup to write checks
 // printf("ip-chk =%x%x\r\n",(check >>8) &0xff,check &0xff); 

  
     MACSetWritePtr(BASE_TX_ADDR+0x18);
  //  WriteReg(EGPWRPT, TXSTART+0X18);  
  
      
    if (tx1==2){
  //  WriteReg(EGPWRPT,0X1c18);  
  
        }

     // write checksum
        outport(check);
 
}

void write_tcp_check(WORD len,BYTE tx1) {
// writes the TCP checksum.  Assumes MAC header is 14 bytes, and IP header is 20
// bytes (thus no options), TCP checksum is 25 words into the ethernet packet, and
// only the TCP header needs to be added to sum.  Must be called just before 
// sending ethernet packet.
// len - number of words in TCP header
  BYTE index;
  WORD ne_check,prev_check;
       
  if (tx1==1)
    ne_check = ne_check1;
  if (tx1==2)
    ne_check = ne_check2;
  
  // setup to read output ram buffer

  
    MACSetReadPtr(BASE_TX_ADDR+0x1a);
 





  // calc checksum
  for (index = 0;index<len;index++) {
    prev_check = ne_check;
    ne_check += inport1();
//       printf("tcp =%x%x\r\n",(ne_check >>8) &0xff,ne_check &0xff); 
    // check carry bit
    if (ne_check < prev_check)
      ne_check++;  // wrap for one's complement addition
  }
  // protocol (0x6/0x5) and TCP header length (0x14) already added in existing checksum
  ne_check = ~ne_check;  // one's complement
  // setup to write checks
//    printf("tcpchk =%x%x\r\n",(ne_check >>8) &0xff,ne_check &0xff); 
   
     MACSetWritePtr(BASE_TX_ADDR+0x32);
 //    WriteReg(EWRPTL, LOW(TXSTART)+0x32);
 //   WriteReg(EWRPTH, HIGH(TXSTART));  
 //  WriteReg(EGPWRPT, TXSTART+0X32);  
 
      
 

 
      // write checksum
        outport(ne_check);
    

}

void ne_print(char text) {
  WORD sendata;
  WORD prev_check;
if (text == '^' && !bin_print)
      text = '"';
  if (ne_count2 &0X0001) {
    // prepare data to send
    sendata = ne_byte;
    sendata = (sendata<<8)+text;
    outport(sendata);
    // calc checksum
    prev_check = ne_check2;
    ne_check2 += sendata;
    // check carry bit
    if (ne_check2 < prev_check)
      ne_check2++;  // wrap for one's complement addition
  } else {
    ne_byte = text;
  }
  ne_count2++;
  // check if first tx buffer full
  if (ne_count2==TCP_MAX_SIZE && ne_count1==0) {
    // write remaining data into second buffer
    // stats for first tx buffer are generated using 2nd buffer's stats.
    ne_count1 = ne_count2;
    ne_check1 = ne_check2;
    ne_count2 = 0;
    ne_check2 = 0;



    // setup to write remaining payload into second tx buffer
     MACSetWritePtr(BASE_TX_ADDR+0x36);
    //   WriteReg(EWRPTL, 0x36);
   //   WriteReg(EWRPTH, 0x1c);  
    //     WriteReg(ERXRDPT,0X1C36);
    //     WriteReg(EGPWRPT,0X1C36);



  }

 }


void finish_tcp(char type, BYTE tx1) {

  WORD ne_count;
  WORD i;

  if (tx1==1)
    ne_count = ne_count1;
  if(tx1==2)
    ne_count = ne_count2;
    // start with mac header
  write_mac_hdr(tx1);
  if (type == TCP_ACK)
    write_ip_hdr(44,IP_TCP);
  else
    write_ip_hdr(ne_count+40,IP_TCP);

  // TCP header, ports first
  outport(dest_port);//22-23
  outport(sendr_port);//24-25
  // sequence and ack numbers
  outport(rx_ack1);//39  26-27
  outport(rx_ack0);//41  28-29
  outport(rx_seq1);//43  2a-2b
  outport(rx_seq0);//45  2c-2d
  // flags, note connection close with regular message
  if (type==TCP_ACK)
    // ACK message from client, reply with ACK-SYN
    outport(0x6012);//2e-2f  47 // the '6' means header length = 6x4 = 24 = 0x18
  else if (type == TCP_FIN)
    // close connection and socket
    outport(0x5014); //47 header length = 5x4 = 20 = 0x14
  else {
    // send data packet to client
    if (isr_last==TRUE)
      // last packet
      outport(0x5019); //47 header length = 5x4 = 20 = 0x14
    else
      // not last packet
      outport(0x5018); //47 header length = 5x4 = 20 = 0x14
  }
  // window size
  outport(0x2000);//49  30-31
  // checksum, instead of 0, put pseudo header in here
  if (type==TCP_ACK)
     outport(0x1E);  //32-3351 0x06(protocol) + 0x18(TCP header length)
  else
    outport(0x1A+ne_count);  //51 0x6(protocol) + 0x14(TCP header length) + num bytes in data
  // urgent pointer
  outport(0x0000);//53  34-35


  if (type==TCP_ACK) {
    // options
    outport(0x0204);//55  36-37
    outport(0x05b4);//57  38-39
    // data (fill) not in any checksum calculations nor packet lengths
    outport(0x0000);//59  3a-3b
    write_ip_check(tx1);
    ne_check1 = 0;
   write_tcp_check(16,tx1);   // does not include fill data
    ne_send(59,tx1);           // include fill data
   } else {
    // send packet if not waiting for image data.
    // NOTE!that ne_count must be >=6 to meet min. packet size
    write_ip_check(tx1);
    write_tcp_check(14,tx1);
    ne_send(ne_count+54,tx1);
}

}

void flush_print(void) {
// finish up use of 'ne_count2' and 'ne_check2'
  WORD sendata,prev_check;
  // flush write-HTML buffer
  if ((ne_count2 & 0x0001)) {
    sendata = ne_byte;
    sendata = sendata<<8;
    outport(sendata);
    ne_count2++;

    // calc checksum
    prev_check = ne_check2;
    ne_check2 += sendata;
    // check carry bit
    if (ne_check2 < prev_check)
      ne_check2++;  // wrap for one's complement addition
  }
}


void  put_eeprom(){


}
void ne_print1( char *data)
	{
	  do
	  {  // Transmit a byte
	   
	    ne_print(*data);
	  } while( *data++ );
	}

 void ne_prints(const rom char *string)
{
 
  char v;

    while( v = *string )
    {
        ne_print(v);
        string++;
    }
}
 void reply_tcp(BYTE type) {
// compose TCP reply message
// type - what type of tcp message.  Can be TCP_xxx
  WORD sendata,temp1,x,i;
  BYTE readPtrL,p;
  BYTE readPtrH;
  ne_count1 = 0;  // reset byte count
  ne_check1 = 0;  // reset checksum
  ne_count2 = 0;  // reset byte count
  ne_check2 = 0;  // reset checksum
  p=0;
  // write payload if any
  if (type == 0) {
    hit_count++;
    // web request to be answered, read request string
    if (inport()==0x4745) {
      putrsUART("GET\r\n");
      // found 'GE' from get request, absorb 'T '
      inport();
      // read file name (discard slash)
      sendata = inport();
      temp1   = inport();
      sendata = (sendata<<8)+(temp1>>8);
      if ((temp1&0xff) == 0x3f)
           put_eeprom();
     MACSetWritePtr(BASE_TX_ADDR+0x36);
 //   WriteReg(EWRPTL, LOW(TXSTART)+0x36);
 //   WriteReg(EWRPTH, HIGH(TXSTART));  
 //      WriteReg(ERXRDPT, TXSTART+0X36);
      //setup to write payload
 //      WriteReg(EGPWRPT, TXSTART+0X36);
  //        outport(0x4545);
       // write payload


  


      if (sendata==0x4545) {
         putrsUART("RX:IC\r\n");
         POWERON	=0;
        bin_print=FALSE;
        ne_prints("HTTP/1.1 200 OK\r\n");
        ne_prints("Expires: 0\r\n");
        ne_prints("Content-type:text/html\r\n\r\n");
        ne_prints("<html><body bgcolor=^AAAAFF^><center>");
        ne_prints("<BLOCKQUOTE dir=ltr style=^MARGIN-RIGHT: 0px^>");
       ne_prints("<P align=center><STRONG><FONT color=#ffff00 size=7>ELECTRONICS FOR YOU</FONT></STRONG></P>");
       ne_prints("<P align=center><STRONG>MRF24WBOMA&nbsp; BASED&nbsp; EMBEDED WIRELESS WEBCAM</STRONG></P></BLOCKQUOTE>");
   
        ne_prints("<TABLE BORDER><tr><td><<img src=IB></td></tr></table>");
  
        ne_prints("<center>");
        ne_prints("<h3><a href=/EF>SWITCH-ON</a></h3>");
   


      }

  else   if (sendata==0x4546) {
        putrsUART("RX:IB\r\n");
        POWERON=1;	
        bin_print=FALSE;
        ne_prints("HTTP/1.1 200 OK\r\n");
        ne_prints("Expires: 0\r\n");
        ne_prints("Content-type:text/html\r\n\r\n");
        ne_prints("<html><body bgcolor=^AAAAFF^><center>");
         ne_prints("<BLOCKQUOTE dir=ltr style=^MARGIN-RIGHT: 0px^>");
       ne_prints("<P align=center><STRONG><FONT color=#ffff00 size=7>ELECTRONICS FOR YOU</FONT></STRONG></P>");
       ne_prints("<P align=center><STRONG>MRF24WBOMA&nbsp; BASED&nbsp; EMBEDED WIRELESS  WEBCAM</STRONG></P></BLOCKQUOTE>");
       ne_prints("<TABLE BORDER><tr><td><<img src=IC></td></tr></table>");
   
        ne_prints("<center>");
         ne_prints("<h3><a href=/EE>SWITCH-OFF</a></h3>");
  



      }
      else if (sendata==0x4942) {
        
    //      pwron();
        // file IB, trigger camera for large picture
          bin_print=FALSE;
         ne_prints("HTTP/1.1 200 OK\r\n");
       //  ne_prints("Expires: 0\r\n");
         ne_prints("Content-type: image/jpeg\r\n\r\n");
         bin_print=TRUE;
    
       //display();
//pwroff();
//POWERON=1;	
DelayMs(10);
cameraS();        //cameraS();
/*POWER=1;
DelayMs(10);
c328syn();
DelayMs(2000);
c328setupS();
DelayMs(100);
c328snap();
DelayMs(1000);
c328getpic();
//DelayMs(10);
//c328pwroff();

//pwroff();
//delay_ms(10);*/

//POWERON=0;
  
       
s3:
 bin_print=FALSE;

// goto s;

     }

  else if (sendata==0x4943) {
           
//          pwron();
        // file IB, trigger camera for large picture
          bin_print=FALSE;
      ne_prints("HTTP/1.1 200 OK\r\n");
    //   ne_prints("Expires: 0\r\n");
      ne_prints("Content-type: image/jpeg\r\n\r\n");
        bin_print=TRUE;
////POWERON=1;	
DelayMs(10);
cameraL();
/*POWER=1;
DelayMs(10);
c328syn();
DelayMs(2000);
c328setupL();
DelayMs(100);
c328snap();
DelayMs(1000);
c328getpic();
//DelayMs(10);
//c328pwroff();
//pwroff();
//delay_ms(10);
*/
//POWERON=0;
       
     
      
s4:     

 bin_print=FALSE;

//  goto s;



     }

   else {
    bin_print=FALSE;
      putrsUART("RX:defaultpage\r\n");
    /*   BYTE    value = Read_ADC();
       BYTE    a=value * 5000/1023/1000;
       BYTE    b=value * 5000/1023%1000;*/
        // default web page
        DelayMs(10);

       ne_prints("HTTP/1.1 200 OK\r\n");
       ne_prints("Content-type:text/html\n\n");
       ne_prints("<html><body bgcolor=^FFFFCC^><center>");
   //     ne_prints("<HTML><center><h3> ENC424J600 BASED PIC WEB SERVER</h3>");
  //      printf(ne_print,"<META HTTP-EQUIV=^refresh^ CONTENT=^10^>\r");
       ne_prints("<BLOCKQUOTE dir=ltr style=^MARGIN-RIGHT: 0px^>");
       ne_prints("<P align=center><STRONG><FONT color=#ff0000 size=7>ELECTRONICS FOR YOU</FONT></STRONG></P>");
       ne_prints("<P align=center><STRONG>MRF24WBOMA&nbsp; BASED&nbsp; EMBEDED WIRELESS WEBCAM</STRONG></P></BLOCKQUOTE>");
       ne_prints("Code Compiled  by Microchip C18 Compiler</p>");
     //   printf(ne_print,"VOLTAGE= %d.%d VOLTAGE</p>",a,b);
       ne_prints("Select one:");
       ne_prints("<TABLE BORDER><tr><td><a href=/EE>SWITCH-OFF</a></td></tr><tr><td>");
     //   printf(ne_print,"<a href=IB>LI</a></td></tr><tr><td>");
     //    ne_prints("<a href=/>MAINPAGE</a></p>");
    
       ne_prints("<a href=/EF>SWITCH-ON</a></td></tr><tr><td></table>");


      }
    }
  } else if (type==TCP_FIN) {
    // setup to write fill
      MACSetWritePtr(BASE_TX_ADDR+0x36);
  //  WriteReg(EWRPTL, LOW(TXSTART)+0x36);
  //  WriteReg(EWRPTH, HIGH(TXSTART));  
  //    WriteReg(ERXRDPT, TXSTART+0X36);
  //    WriteReg(EGPWRPT, TXSTART+0X36);

       // fill to meet minimum size requirements
        ne_prints("      ");
  }

   flush_print();
  // clean up buffer counters and checksum total
    if (ne_count1==0) {

      // only first transmit buffer used, establish its counters
    ne_count1 = ne_count2;
    ne_check1 = ne_check2;
    // cleanup 2nd transmit buffer's
      ne_count2 = 0;
      ne_check2 = 0;
      isr_last = TRUE;
//      wNextPacketPointer  = ReadReg(ERXRDPT);
      finish_tcp(type,1);
//      WriteReg(ERXRDPT,wNextPacketPointer);
    }
s:
      readPtrL=0;
  //pwroff();
     }
 

//CAMERA FUNCTIONS


void send(BYTE data)
{
 
    while(BusyUSART());
   WriteUSART(data);
  
}

void c328r_send_ack(BYTE command, WORD package_id)
{
  send(C328R_COMMAND_PREFIX);
  send(C328R_COMMAND_ACK);
  send(command);                  // Command to ACK
  send(0x00);
  send(package_id & 0xff);// Package to request (low byte)
  send(package_id >> 8);  // Package to request (high byte)
   
}


void c328syn(){
BYTE a[6],i;
char k[6];
   s1:
        do{
 // Send SYNC sequence
    send(C328R_COMMAND_PREFIX);
    send(C328R_COMMAND_SYNC);
    send(0x00);
    send(0x00);
    send(0x00);
    send(0x00);
    DelayMs(10);
}
while(!PIR1bits.RCIF);//}while(!DataRdyUART());
if(serial_error())
 serial_fix(); 
for(i=0;i<2;i++){
 a[i]=getcUART(); 
}   // clear
/*LCDErase();
sprintf(LCDText,"%x:%x:\n",a[0],a[1]);
LCDUpdate();
*/
   DelayMs(10);
   c328r_send_ack(C328R_COMMAND_SYNC, 0);

}
void c328setupS(){

BYTE  a[6],i;

 send(C328R_COMMAND_PREFIX);
 send(0x01);
 send(0x00);
 send(0x07);
 send(0x05);           //(0x03);
 send(5);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//while(!DataRdyUART());
if(serial_error())
 serial_fix(); 
a[i]=getcUART(); 
}
/*LCDErase();//lcd_putc('\f');
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();*/
DelayMs(10);
send(C328R_COMMAND_PREFIX);
send(C328R_COMMAND_LIGHT_FREQUENCY);
send(C328R_FREQUENCY);
send(0x00);
 send(0x00);
 send(0x00);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//hile(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}
/*LCDErase();//lcd_putc('\f');
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();*/
DelayMs(10);;
send(C328R_COMMAND_PREFIX);
send(0x06);
send(0x08);
send(0x00);
send(0x02);
send(0x00);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//hile(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}
/*LCDErase();//lcd_putc('\f');
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();
*/
}
void c328setupL(){

BYTE  a[6],i;
 int x ;
 send(C328R_COMMAND_PREFIX);
 send(0x01);
 send(0x00);
 send(0x07);
 send(0x07);//(0x03)
 send(7);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//while(!DataRdyUART());
if(serial_error())
 serial_fix(); 
a[i]=getcUART(); 
}
/*LCDErase();//lcd_putc('\f');
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();*/
DelayMs(10);
send(C328R_COMMAND_PREFIX);
send(C328R_COMMAND_LIGHT_FREQUENCY);
send(C328R_FREQUENCY);
send(0x00);
 send(0x00);
 send(0x00);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//hile(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}
/*LCDErase();//lcd_putc('\f');
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();*/
DelayMs(10);;
send(C328R_COMMAND_PREFIX);
send(0x06);
send(0x08);
send(0x00);//40
send(0x02);//00
send(0x00);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//hile(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}


/*LCDErase();//lcd_putc('\f');
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();*/

}
void c328snap(){
BYTE a[6],i;

send(C328R_COMMAND_PREFIX);
send(0x05);
send(0x00);
send(0x00);
send(0x00);
send(0x00);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//hile(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}
//lcd_putc('\f');
/*sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();*/
}

void c328getpic(){
BYTE lsb,msb;
char temp;
BYTE a[6],i;
WORD x;
WORD bytes,fileLength,packetLength,packetNo,j,k,l,p; ;

 send(C328R_COMMAND_PREFIX);
 send(0x04);
 send(0x01);
 send(0x00);
 send(0x00);
 send(0x00);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//while(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//while(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}
fileLength = a[4] * 256;                               // type conversions).
fileLength = fileLength +a[3];

/*sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();*/
//}
//ka
 send(C328R_COMMAND_PREFIX);
 send(0x0E);
 send(0x00);
 send(0x00);
 send(0x00);
 send(0x00); 
bytes = 0x0 ;
packetNo = 0x0;j=0;

k=0000;
p=0;
// ne_count1 = 0;  // reset byte count
//  ne_check1 = 0;  // reset checksum
//  ne_count2 = 0;  // reset byte count
//  ne_check2 = 0;  // reset checksum
  while (bytes<fileLength) {                                // Receive all the bytes, packet after  0
for(i=0;i<4;i++){
while(!PIR1bits.RCIF);//while(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}

/*sprintf(LCDText,"%x:%x:%x:%x\n",a[0],a[1],a[2],a[3]);
LCDUpdate();
*/
packetLength = a[3] * 256;                               // type conversions).
packetLength = packetLength +a[2];

        for (j=0; j< packetLength;j++){
           while(!PIR1bits.RCIF);
               if(serial_error())
               serial_fix();
               temp =getcUART();
               ne_print(temp);  
           k++;
        }   //next
         p++;
        
     flush_print();
     x = ne_count2;
    // only first transmit buffer used, establish its counters
    ne_count1 = ne_count2;
    ne_check1 = ne_check2;
    // cleanup 2nd transmit buffer's
      ne_count2 = 0;
      ne_check2 = 0; 
for(i=0;i<2;i++){
while(!PIR1bits.RCIF);//while(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}


    bytes = bytes + packetLength;                          // Account for bytes just received.
    packetNo++;
        lsb = packetNo & 0x00ff;
        msb = (packetNo & 0xff00)>>8;
 
//sprintf(LCDText,"p=%x%x:k=%x%x\n",p>>8,p&0xff,k>>8,k&0xff);
//LCDUpdate();
   
       if (bytes>=fileLength) { 
  
    send(C328R_COMMAND_PREFIX);
   send(0x0E);
   send(0x00);
   send(0x00);
   send(0xF0);
   send(0xF0); 
   
   isr_last = TRUE;

   finish_tcp(0,1); 
 
  
// sprintf(LCDText,"L2\n");
// LCDUpdate();
    
  }
 else {
   
  //k DelayMs(2);   
   isr_last = FALSE;
   finish_tcp(0,1);
   rx_ack0 += x;
 MACSetWritePtr(BASE_TX_ADDR+0x36);
  send(C328R_COMMAND_PREFIX);
  send(0x0E);
  send(0x00);
  send(0x00);
  send(lsb);
  send(msb); 
 
 
    }//else
  }   // end of while
//k DelayMs(10);
send(C328R_COMMAND_PREFIX);
send(0x09);
send(0x00);
send(0x00);
send(0x00);
send(0x00);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//hile(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}
/*LCDErase();//lcd_putc('\f');
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();
*/

}








  







void c328reset(){
BYTE a[6],i;
send(C328R_COMMAND_PREFIX);
send(0x08);
send(0x00);
send(0x00);
send(0x00);
send(0xff);
for(i=0;i<6;i++){
while(!PIR1bits.RCIF);//hile(!kbhit());
if(serial_error())
    serial_fix();
a[i]=getcUART(); 
}
lcd_putc('\f'); //clear screen
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDPutString(LCDText);
/*LCDErase();//lcd_putc('\f');
sprintf(LCDText,"%x:%x:%x:%x:%x:%x\n",a[0],a[1],a[2],a[3],a[4],a[5]);
LCDUpdate();*/

}	
 
void cameraS(void)
{
//  POWER=1;
  c328syn();
 DelayMs(500);
  c328setupS();
  c328snap();
DelayMs(100);
c328getpic();
//c328pwroff();
//  POWER=0;
}

void cameraL(void)
{
// POWER=1;
  c328syn();
 DelayMs(500);
  c328setupL();
  c328snap();
DelayMs(100);
c328getpic();
//c328pwroff();
// POWER=0;
}
