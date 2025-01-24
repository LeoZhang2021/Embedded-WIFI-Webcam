/*********************************************************************
 *
 *        TCP/IP Stack Manager
 *  Module for Microchip TCP/IP Stack
 *         -Handles internal RX packet pre-processing prior to dispatching 
 *    to upper application layers.
 *         -Reference: AN833
 *
 *********************************************************************
 * FileName:        StackTsk.c
 * Dependencies:    ARP, IP, Network layer interface (ENC28J60.c, 
 *                                        ETH97J60.c, ENCX24J600.c, or WFMac.c)
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     8/14/01     Original (Rev. 1.0)
 * Nilesh Rajbharti     2/9/02      Cleanup
 * Nilesh Rajbharti     5/22/02     Rev 2.0 (See version.log for detail)
 * Nilesh Rajbharti     12/5/02     Modified UDPProcess() and TCPProcess()
 *                                  to include localIP as third param.
 *                                  This was done to allow these functions
 *                                  to calculate checksum correctly.
 * Nilesh Rajbharti     7/26/04     Added code in StackTask() to not
 *                                  clear statically IP address if link is
 *                                  removed and DHCP module is disabled
 *                                  at runtime.
 * Howard Schlunder                03/16/07        Rewrote stack manager to be linear
********************************************************************/
#define __STACKTSK_C

#include "TCPIP.h"


   

// Stack FSM states.
typedef enum
{
    SM_STACK_IDLE,
    SM_STACK_MAC,
    SM_STACK_IP,
    SM_STACK_ARP,
    SM_STACK_TCP,
    SM_STACK_UDP
} SM_STACK;
static SM_STACK smStack;

NODE_INFO remoteNode;



/*********************************************************************
 * Function:        void StackInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Stack and its componets are initialized
 *
 * Side Effects:    None
 *
 * Note:            This function must be called before any of the
 *                  stack or its component routines are used.
 *
 ********************************************************************/
void StackInit(void)
{
    smStack                     = SM_STACK_IDLE;


    AppConfig.Flags.bInConfigMode = TRUE;

    MACInit();
     ARPInit();
     UDPInit();
    DHCPInit(0);
    if(!AppConfig.Flags.bIsDHCPEnabled)
    {
        DHCPDisable(0);
    }






}


/*********************************************************************
 * Function:        void StackTask(void)
 *
 * PreCondition:    StackInit() is already called.
 *
 * Input:           None
 *
 * Output:          Stack FSM is executed.
 *
 * Side Effects:    None
 *
 * Note:            This FSM checks for new incoming packets,
 *                  and routes it to appropriate stack components.
 *                  It also performs timed operations.
 *
 *                  This function must be called periodically to
 *                  ensure timely responses.
 *
 ********************************************************************/
void StackTask(void)
{
   WORD dataCount,size;
    IP_ADDR tempLocalIP;
	BYTE cFrameType;
	BYTE cIPFrameType;
   
  
        MACProcess();
   if(AppConfig.Flags.bIsDHCPEnabled)
	{
		static BOOL bLastLinkState = FALSE;
		BOOL bCurrentLinkState;
		
		bCurrentLinkState = MACIsLinked();
		if(bCurrentLinkState != bLastLinkState)
		{
			bLastLinkState = bCurrentLinkState;
			if(!bCurrentLinkState)
			{
				AppConfig.MyIPAddr.Val = AppConfig.DefaultIPAddr.Val;
				AppConfig.MyMask.Val = AppConfig.DefaultMask.Val;
				AppConfig.Flags.bInConfigMode = TRUE;
				DHCPInit(0);
			}
		}
	
		// DHCP must be called all the time even after IP configuration is
		// discovered.
		// DHCP has to account lease expiration time and renew the configuration
		// time.
		DHCPTask();
		
		if(DHCPIsBound(0))
			AppConfig.Flags.bInConfigMode = FALSE;
	}
      
         UDPTask();


while(1)
	{
		//if using the random module, generate entropy
		
			UDPDiscard();
		

		// Fetch a packet (throws old one away, if not thrown away 
		// yet)
		if(!MACGetHeader(&remoteNode.MACAddr, &cFrameType))
			break;

		// When using a WiFi module, filter out all incoming packets that have 
		// the same source MAC address as our own MAC address.  This is to 
		// prevent receiving and passing our own broadcast packets up to other 
		// layers and avoid, for example, having our own gratuitous ARPs get 
		// answered by ourself.
		
			if(memcmp((void*)&remoteNode.MACAddr, (void*)&AppConfig.MyMACAddr, 6) == 0u)
				continue;
		

		// Dispatch the packet to the appropriate handler
		switch(cFrameType)
		{
			case MAC_ARP:
				ARPProcess();
				break;
	
			case MAC_IP:
				if(!IPGetHeader(&tempLocalIP, &remoteNode, &cIPFrameType, &dataCount ,&size))
					break;

				
				if(cIPFrameType == IP_PROT_ICMP)
				{
					
					// Process this ICMP packet if it the destination IP address matches our address or one of the broadcast IP addressees
					if( (tempLocalIP.Val == AppConfig.MyIPAddr.Val) ||
						(tempLocalIP.Val == 0xFFFFFFFF) ||

						(tempLocalIP.Val == ((AppConfig.MyIPAddr.Val & AppConfig.MyMask.Val) | ~AppConfig.MyMask.Val)))
					{
						ICMPProcess(&remoteNode, dataCount);
					}

					break;
				}
				
				
			
				if(cIPFrameType == IP_PROT_TCP)
				{

                                 decode_tcp(size,&remoteNode);
				//	TCPProcess(&remoteNode, &tempLocalIP, dataCount);
					break;
				}
			
				
				
				if(cIPFrameType == IP_PROT_UDP)
				{
					// Stop processing packets if we came upon a UDP frame with application data in it
					if(UDPProcess(&remoteNode, &tempLocalIP, dataCount))
						return;
				}
				

				break;
		}
	}











  
}

/*********************************************************************
 * Function:        void StackApplications(void)
 *
 * PreCondition:    StackInit() is already called.
 *
 * Input:           None
 *
 * Output:          Calls all loaded application modules.
 *
 * Side Effects:    None
 *
 * Note:            This function must be called periodically to
 *                  ensure timely responses.
 *
 ********************************************************************/
void StackApplications(void)
{
        
        
        
        
}