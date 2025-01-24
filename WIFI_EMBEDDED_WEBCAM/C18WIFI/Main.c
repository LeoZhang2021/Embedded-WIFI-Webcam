

#include "TCPIP.h"
#pragma config OSC=HSPLL, WDT=OFF, MCLRE=ON, PBADEN=OFF, LVP=OFF
#define WF_MODULE_NUMBER   WF_MODULE_MAIN_DEMO
#define BAUD_RATE       (57600)     // bps
static void BoardInit(void);
//static void InitializeBoard(void);
static void InitAppConfig(void);
static void WF_Connect(void);
static  BYTE a;
#pragma interruptlow LowISR
	void LowISR(void)
	{
	    TickUpdate();
	}

#pragma interruptlow HighISR
	void HighISR(void)

	{
	    
		WFEintISR();
		
	}

#pragma code lowVector=0x18
void LowVector(void){_asm goto LowISR _endasm}
#pragma code highVector=0x8
void HighVector(void){_asm goto HighISR _endasm}
#pragma code // Return to default code section
APP_CONFIG AppConfig;
static unsigned short wOriginalAppConfigChecksum;

void main(){
static DWORD dwLastIP = 0;
static DWORD t = 0;

unsigned char  LCD[16];
a=0;
BoardInit();
LED0_IO = 1;
POWERON=0;
xlcd_init();
TickInit();
InitAppConfig();
StackInit();
WF_Connect();
lcd_putc('\f'); //clear screen
sprintf(LCD,"WIFI WEBCAMSERVER");
LCDPutString (LCD);

while(1){

  if(TickGet() - t >= TICK_SECOND/2ul)
        {
            t = TickGet();
            LED0_IO ^= 1;
        }
       StackTask();
  if(dwLastIP != AppConfig.MyIPAddr.Val)
		{
			dwLastIP = AppConfig.MyIPAddr.Val;
		     putrsUART((ROM char*)"\r\nNew IP Address: ");
			sprintf(LCD,"%u:%u:%u:%u",AppConfig.MyIPAddr.v[0],AppConfig.MyIPAddr.v[1],AppConfig.MyIPAddr.v[2],AppConfig.MyIPAddr.v[3]);
            putsUART(LCD);
			putrsUART((ROM char*)"\r\n");
			lcd_putc('\f');
            LCDPutString (LCD);


			
		}
    

}

}
static void BoardInit(void){

ADCON1 = 0xff;
INTCON2bits.RBPU = 0;
TRISD=0;
TRISE=0;
LED0_TRIS=0;
POWERON_TRIS=0;
WF_CS_IO=1;
WF_CS_TRIS=0;	
OSCTUNE = 0x40;
 TXSTA = 0b00100000;     // Low BRG speed
 RCSTA = 0b10010000;
 #if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
		SPBRG = (GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1;
		TXSTAbits.BRGH = 1;
	#else	// Use the low baud rate setting
		SPBRG = (GetPeripheralClock()+8*BAUD_RATE)/BAUD_RATE/16 - 1;
	#endif
// SPBRG = SPBRG_VAL;
RW=0;
RCONbits.IPEN = 1;		// Enable interrupt priorities
INTCONbits.GIEH = 1;
INTCONbits.GIEL = 1;






}
static void WF_Connect(void)
{
    UINT8 ConnectionProfileID;
    UINT8 channelList[] = MY_DEFAULT_CHANNEL_LIST;
    #if defined(WF_USE_POWER_SAVE_FUNCTIONS)
    BOOL  PsPollEnabled;
    #endif
    
    /* create a Connection Profile */
    WF_CPCreate(&ConnectionProfileID);

   
    putrsUART("Set SSID (");
    sprintf(AppConfig.MySSID,"kathir");
    putsUART(AppConfig.MySSID);
    putrsUART(")\r\n");
  
    WF_CPSetSsid(ConnectionProfileID, 
                 AppConfig.MySSID, 
                 AppConfig.SsidLength);

   
    putrsUART("Set Network Type\r\n");
	
    WF_CPSetNetworkType(ConnectionProfileID, MY_DEFAULT_NETWORK_TYPE);
    
	putrsUART("Set Scan Type\r\n");
	
    WF_CASetScanType(MY_DEFAULT_SCAN_TYPE);
    
   
    putrsUART("Set Channel List\r\n");
     
    WF_CASetChannelList(channelList, sizeof(channelList));
    
   
    putrsUART("Set list retry count\r\n");
  
    WF_CASetListRetryCount(MY_DEFAULT_LIST_RETRY_COUNT);

          
    putrsUART("Set Event Notify\r\n");    
   
    WF_CASetEventNotificationAction(MY_DEFAULT_EVENT_NOTIFICATION_LIST);
    
#if defined(WF_USE_POWER_SAVE_FUNCTIONS)
    PsPollEnabled = (MY_DEFAULT_PS_POLL == WF_ENABLED);
    if (!PsPollEnabled)
    {    
        /* disable low power (PS-Poll) mode */
       
        putrsUART("Disable PS-Poll\r\n");        
       
        WF_PsPollDisable();
    }    
    else
    {
        /* Enable low power (PS-Poll) mode */
        
        putrsUART("Enable PS-Poll\r\n");        
       
        WF_PsPollEnable(TRUE);
    }    
#endif

   
    putrsUART("Set Beacon Timeout\r\n");
   
    WF_CASetBeaconTimeout(40);
    
    /* Set Security */
    #if (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_OPEN)
       
        putrsUART("Set Security (Open)\r\n");
       
    #elif (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WEP_40)
       
        putrsUART("Set Security (WEP40)\r\n");
       
    #elif (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WEP_104)
      
        putrsUART("Set Security (WEP104)\r\n");
       
    #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_WITH_KEY 
        
        putrsUART("Set Security (WPA with key)\r\n");
        
    #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA2_WITH_KEY 
        
        putrsUART("Set Security (WPA2 with key)\r\n");
       
    #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_WITH_PASS_PHRASE
       
        putrsUART("Set Security (WPA with pass phrase)\r\n");
       
    #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA2_WITH_PASS_PHRASE
        
        putrsUART("Set Security (WPA2 with pass phrase)\r\n");    
       
    #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_AUTO_WITH_KEY
        
        putrsUART("Set Security (WPA with key, auto-select)\r\n");
       
    #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE
       
     putrsUART("Set Security (WPA with pass phrase, auto-select)\r\n");
       
    #endif 

    WF_CPSetSecurity(ConnectionProfileID,
                     AppConfig.SecurityMode,
                     AppConfig.WepKeyIndex,   /* only used if WEP enabled */
                     AppConfig.SecurityKey,
                     AppConfig.SecurityKeyLength);
                  
    putrsUART("Start WiFi Connect\r\n");        
   
    WF_CMConnect(ConnectionProfileID);
} 
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6}; 
 
static void InitAppConfig(void)
{

	while(1)
	{
		// Start out zeroing all AppConfig bytes to ensure all fields are 
		// deterministic for checksum generation
		memset((void*)&AppConfig, 0x00, sizeof(AppConfig));
		
		AppConfig.Flags.bIsDHCPEnabled = TRUE;
		AppConfig.Flags.bInConfigMode = TRUE;
		memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));

		AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
		AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
		AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
		AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
		AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
		AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
		AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;
	
	
		// SNMP Community String configuration
	
	
		// Load the default NetBIOS Host Name
	//	memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
	//	FormatNetBIOSName(AppConfig.NetBIOSName);
	
	
			// Load the default SSID Name
			WF_ASSERT(sizeof(MY_DEFAULT_SSID_NAME) <= sizeof(AppConfig.MySSID));
			memcpypgm2ram(AppConfig.MySSID, (ROM void*)MY_DEFAULT_SSID_NAME, sizeof(MY_DEFAULT_SSID_NAME));
			AppConfig.SsidLength = sizeof(MY_DEFAULT_SSID_NAME) - 1;
	       
	        AppConfig.SecurityMode = MY_DEFAULT_WIFI_SECURITY_MODE;
	        AppConfig.WepKeyIndex  = MY_DEFAULT_WEP_KEY_INDEX;
	        
	        #if (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_OPEN)
	            memset(AppConfig.SecurityKey, 0x00, sizeof(AppConfig.SecurityKey));
	            AppConfig.SecurityKeyLength = 0;
	
	        #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WEP_40
	            memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_WEP_KEYS_40, sizeof(MY_DEFAULT_WEP_KEYS_40) - 1);
	            AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_WEP_KEYS_40) - 1;
	
	        #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WEP_104
			    memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_WEP_KEYS_104, sizeof(MY_DEFAULT_WEP_KEYS_104) - 1);
			    AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_WEP_KEYS_104) - 1;
	
	        #elif (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_WITH_KEY)       || \
	              (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA2_WITH_KEY)      || \
	              (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_AUTO_WITH_KEY)
			    memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_PSK, sizeof(MY_DEFAULT_PSK) - 1);
			    AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_PSK) - 1;
	
	        #elif (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_WITH_PASS_PHRASE)     || \
	              (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA2_WITH_PASS_PHRASE)    || \
	              (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE)
	            memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_PSK_PHRASE, sizeof(MY_DEFAULT_PSK_PHRASE) - 1);
	            AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_PSK_PHRASE) - 1;
	
	        #else 
	            #error "No security defined"
	        #endif /* MY_DEFAULT_WIFI_SECURITY_MODE */
	
	

		// Compute the checksum of the AppConfig defaults as loaded from ROM
		wOriginalAppConfigChecksum = CalcIPChecksum((BYTE*)&AppConfig, sizeof(AppConfig));

	     s:
           Nop();
		break;
	}
}
