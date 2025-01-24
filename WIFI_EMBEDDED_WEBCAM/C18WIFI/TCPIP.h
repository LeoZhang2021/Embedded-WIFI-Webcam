#include <p18cxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define COMPILER_MPLAB_C18
#define ROM   const
 #define GetSystemClock()	(40000000ul)			// Hz
 #define GetInstructionClock()	(GetSystemClock()/4)	// Should be GetSystemClock()/4 for PIC18
 #define GetPeripheralClock()	(GetSystemClock()/4)	// Should be GetSystemClock()/4 for PIC18
#define PTR_BASE		unsigned short
#define ROM_PTR_BASE	unsigned short long
#define memcmppgm2ram(a,b,c)	memcmp(a,b,c)
#define strcmppgm2ram(a,b)		strcmp(a,b)
	#define memcpypgm2ram(a,b,c)	memcpy(a,b,c)
	#define strcpypgm2ram(a,b)		strcpy(a,b)
	#define strncpypgm2ram(a,b,c)	strncpy(a,b,c)
	#define strstrrampgm(a,b)		strstr(a,b)
	#define	strlenpgm(a)			strlen(a)
	#define strchrpgm(a,b)			strchr(a,b)
	#define strcatpgm2ram(a,b)		strcat(a,b)
#define MAX_UDP_SOCKETS     (9u)
#define MAX_HTTP_CONNECTIONS	(2u)
#define MY_DEFAULT_MAC_BYTE1            (0x00)	// Use the default of 00-04-A3-00-00-00
#define MY_DEFAULT_MAC_BYTE2            (0x04)	// if using an ENCX24J600, MRF24WB0M, or
#define MY_DEFAULT_MAC_BYTE3            (0xA3)	// PIC32MX6XX/7XX internal Ethernet 
#define MY_DEFAULT_MAC_BYTE4            (0x00)	// controller and wish to use the 
#define MY_DEFAULT_MAC_BYTE5            (0x00)	// internal factory programmed MAC
#define MY_DEFAULT_MAC_BYTE6            (0x00)	// address instead.

#define MY_DEFAULT_IP_ADDR_BYTE1        (169ul)
#define MY_DEFAULT_IP_ADDR_BYTE2        (254ul)
#define MY_DEFAULT_IP_ADDR_BYTE3        (1ul)
#define MY_DEFAULT_IP_ADDR_BYTE4        (1ul)

#define MY_DEFAULT_MASK_BYTE1           (255ul)
#define MY_DEFAULT_MASK_BYTE2           (255ul)
#define MY_DEFAULT_MASK_BYTE3           (0ul)
#define MY_DEFAULT_MASK_BYTE4           (0ul)

#define MY_DEFAULT_GATE_BYTE1           (169ul)
#define MY_DEFAULT_GATE_BYTE2           (254ul)
#define MY_DEFAULT_GATE_BYTE3           (1ul)
#define MY_DEFAULT_GATE_BYTE4           (1ul)

#define MY_DEFAULT_PRIMARY_DNS_BYTE1	(169ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE2	(254ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE3	(1ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE4	(1ul)

#define MY_DEFAULT_SECONDARY_DNS_BYTE1	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE2	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE3	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE4	(0ul)	
#include "xlcd.h"
#include "Delay.h"
#include "Hardware.h"
#include  "UART.h"
#include  "Tick.h"
#include "GenericTypeDefs.h"
//#include "WFMac.h"
#include "Mac.h"
#include "StackTsk.h"
#include "WF_Config.h"  
#include "WFApi.h"
#include "WFDriverPrv.h"
#include "WFMgmtMsg.h"
#include "WFRaw.h"
#include "ARP.h"
#include "IP.h"
#include "DHCP.h"
#include "ICMP.h" 
#include "UDP.h"    