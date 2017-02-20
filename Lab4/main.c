/* File Name:    main.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      02/19/2017
 * Description:  Get Weather Application
 *               
 * 
 * Lab Number: MW 330-500
 * TA: Mahesh
 * 
 * Hardware Configurations:
 * ST7735R LCD:
 *     Backlight    (pin 10) connected to +3.3 V
 *     MISO         (pin 9) unconnected
 *     SCK          (pin 8) connected to PA2 (SSI0Clk)
 *     MOSI         (pin 7) connected to PA5 (SSI0Tx)
 *     TFT_CS       (pin 6) connected to PA3 (SSI0Fss)
 *     CARD_CS      (pin 5) unconnected
 *     Data/Command (pin 4) connected to PA6 (GPIO)
 *     RESET        (pin 3) connected to PA7 (GPIO)
 *     VCC          (pin 2) connected to +3.3 V
 *     Gnd          (pin 1) connected to ground
 *		 PE2					 Potentiometer
 
 
 
 */
#include "..\cc3100\simplelink\include\simplelink.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "application_commands.h"
#include "LED.h"
#include "Nokia5110.h"
#include <string.h>
#include "ST7735.h"
#include "ADCSWTrigger.h"
#include "SysTick.h"
#include <stdbool.h>
#include "fixed.h"


//#define SSID_NAME  "valvanoAP" /* Access point name to connect to */
#define SEC_TYPE   SL_SEC_TYPE_WPA
//#define PASSKEY    "12345678"  /* Password in case of secure AP */ 
#define SSID_NAME  "Verizon-SM-G900V-8A0F"
#define PASSKEY    "lbyi901*"
#define BAUD_RATE   115200
void UART_Init(void){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTStdioConfig(0,BAUD_RATE,50000000);
}

#define MAX_RECV_BUFF_SIZE  1024
#define MAX_SEND_BUFF_SIZE  512
#define MAX_HOSTNAME_SIZE   40
#define MAX_PASSKEY_SIZE    32
#define MAX_SSID_SIZE       32


#define SUCCESS             0

#define CONNECTION_STATUS_BIT   0
#define IP_AQUIRED_STATUS_BIT   1

void getVoltageData(void);
/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,/* Choosing this number to avoid overlap w/ host-driver's error codes */

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


/* Status bits - These are used to set/reset the corresponding bits in 'g_Status' */
typedef enum{
    STATUS_BIT_CONNECTION =  0, /* If this bit is:
                                 *      1 in 'g_Status', the device is connected to the AP
                                 *      0 in 'g_Status', the device is not connected to the AP
                                 */

    STATUS_BIT_IP_AQUIRED,       /* If this bit is:
                                 *      1 in 'g_Status', the device has acquired an IP
                                 *      0 in 'g_Status', the device has not acquired an IP
                                 */

}e_StatusBits;


#define SET_STATUS_BIT(status_variable, bit)    status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~(1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & (1<<(bit))))
#define IS_CONNECTED(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_CONNECTION)
#define IS_IP_AQUIRED(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_IP_AQUIRED)

typedef struct{
    UINT8 SSID[MAX_SSID_SIZE];
    INT32 encryption;
    UINT8 password[MAX_PASSKEY_SIZE];
}UserInfo;

/*
 * GLOBAL VARIABLES -- Start
 */

char Recvbuff[MAX_RECV_BUFF_SIZE];
char SendBuff[MAX_SEND_BUFF_SIZE];
char HostName[MAX_HOSTNAME_SIZE];
unsigned long DestinationIP;
int SockID;

bool firstrun = true;
typedef struct Time{
    uint32_t min;
    uint32_t max;
    uint32_t total;
		uint32_t iteration;
		uint32_t sampledata[10];
} Time;

Time WeatherTimings ={0XFFFFFFFF,0,0,0,0};
Time VoltageTiming = {0XFFFFFFFF,0,0,0,0};



uint32_t lostpackets =0;
uint32_t timesqueried = 0;
typedef enum{
    CONNECTED = 0x01,
    IP_AQUIRED = 0x02,
    IP_LEASED = 0x04,
    PING_DONE = 0x08

}e_Status;
UINT32  g_Status = 0;
/*
 * GLOBAL VARIABLES -- End
 */


 /*
 * STATIC FUNCTION DEFINITIONS  -- Start
 */

static int32_t configureSimpleLinkToDefaultState(char *);


/*
 * STATIC FUNCTION DEFINITIONS -- End
 */

void Crash(uint32_t time){
  while(1){
    for(int i=time;i;i--){};
    LED_RedToggle();
  }
}


/*Helper Function to extract Temperature from getWeather API*/

void extracTemp(char* Recvbuff, char* tempbuffer){
	const char TEMP[] = "\"temp\"";
	const char COMMA[] = ",";
	char* start = strstr(Recvbuff,TEMP);
	char* end = strstr(start,COMMA);
	start += strlen(TEMP)*sizeof(char)+1;
	int i =0;
	while(*start != *end)
	{
		tempbuffer[i++] = *start;
		start ++;
	}
	}

/*Helper function to turn ADC Data into string format */
void itoa(uint32_t voltage, char buffer[]){
	char const digits[] = "0123456789";
	char* pos = buffer;
	int counter = voltage;
	do{
		++pos;
		counter = counter/10;
	}while(counter);
	*pos = '\0';
	do{
		*--pos = digits[voltage%10];
		voltage /= 10;
}while(voltage);
}

void queryWebserver(char* Host,char* MessageRequest)
{
			
	int32_t retVal;  SlSecParams_t secParams;
  char *pConfig = NULL; INT32 ASize = 0; SlSockAddrIn_t  Addr; //Socket creates a scoet
		retVal = sl_NetAppDnsGetHostByName(Host,strlen(Host),
                                                &DestinationIP, SL_AF_INET);
	timesqueried++;
	if(retVal == 0){
      Addr.sin_family = SL_AF_INET;
      Addr.sin_port = sl_Htons(80);
      Addr.sin_addr.s_addr = sl_Htonl(DestinationIP);// IP to big endian 
      ASize = sizeof(SlSockAddrIn_t);
      SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
      if( SockID >= 0 ){
        retVal = sl_Connect(SockID, ( SlSockAddr_t *)&Addr, ASize);
      }
      if((SockID >= 0)&&(retVal >= 0)){
        strcpy(SendBuff,MessageRequest); 
        sl_Send(SockID, SendBuff, strlen(SendBuff), 0);// Send the HTTP GET 
        sl_Recv(SockID, Recvbuff, MAX_RECV_BUFF_SIZE, 0);// Receive response 
        sl_Close(SockID);
        UARTprintf("This line reached");
      }
			else
			{
				lostpackets ++;
			}
	
}
	
}
	


	
/*
*
*/

void process_Times(unsigned long difference,Time* Timing)
{
		
		if(difference > Timing->max)
		{
			Timing->max = difference;
		}
		if(difference < Timing->min)
		{
			Timing->min = difference;
		}
		Timing->total+= difference;	
		
		if(Timing->iteration < 10){
		Timing->sampledata[Timing->iteration++] = difference/50000;
		}
		
	}

	
	
/* Initializes clock,uarts,leds,and ADC Channels
 *
 */	
	void init_All(){
	initClk();        // PLL 50 MHz
  UART_Init();      // Send data to PC, 115200 bps (baud rate)
  LED_Init();       // initialize LaunchPad I/O 
	ADC0_InitSWTriggerSeq3_Ch9(); //initialize for ADC Sampling 
	SysTick_Init();
	ST7735_InitR(INITR_REDTAB);
	}
	
/* Connect to the wifi modules
 *
 */
	
	void connect_internet(){
	int32_t retVal;  SlSecParams_t secParams;
  char *pConfig = NULL; INT32 ASize = 0; SlSockAddrIn_t  Addr;
		  retVal = configureSimpleLinkToDefaultState(pConfig); // set policies
  if(retVal < 0)Crash(4000000);
  retVal = sl_Start(0, pConfig, 0);
  if((retVal < 0) || (ROLE_STA != retVal) ) Crash(8000000);
  secParams.Key = PASSKEY;
  secParams.KeyLen = strlen(PASSKEY);
  secParams.Type = SEC_TYPE; // OPEN, WPA, or WEP
  sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
  while((0 == (g_Status&CONNECTED)) || (0 == (g_Status&IP_AQUIRED))){
    _SlNonOsMainLoopTask();
  }
  UARTprintf("Connected\n");
	}
	
#define REQUEST "GET /data/2.5/weather?q=Austin%20Texas&APPID=1bc54f645c5f1c75e681c102ed4bbca4&units=metric HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n"
	
	
	/*getWeather()
	 *Queries openweather map for the current weather
	 */
	
	void getWeather(){
		strcpy(HostName,"api.openweathermap.org"); // works 9/2016
		unsigned long startTime =  NVIC_ST_CURRENT_R;
		queryWebserver(HostName,REQUEST);
		unsigned long endTime = NVIC_ST_CURRENT_R;
		
		process_Times(endTime-startTime,&WeatherTimings);
		char tempbuffer[50] = " ";
				
				extracTemp(Recvbuff,tempbuffer);
				ST7735_SetCursor(0,1);
				ST7735_OutString("Temperature ");
				strcat(tempbuffer," C");
				ST7735_OutString(tempbuffer);
		
	}
	

	
	#define SENDSTRING1 "GET /query?city=Austin%2C%20Texas&id=Kevin%20and%20Stefan&greet=" 
#define SENDSTRING2 " HTTP/1.1\r\nUser-Agent: Keil\r\nHost: ee445l-kjy252.appspot.com\r\n\r\n"
#define SERVER "ee445l-kjy252.appspot.com"
#define REQUESTT "GET /query?city=Austin%2C%20Texas&id=Kevin%20and%20Stefan&greet=FUCKFINALLYYESSSSFUCKK HTTP/1.1\r\nUser-Agent: Keil\r\nHost: ee445l-kjy252.appspot.com\r\n\r\n"
void getVoltageData(){
	uint32_t ADCval = ADC0_InSeq3();
	ST7735_SetCursor(0,2);
	ST7735_OutString("Voltage ");

	
	char voltagebuffer[10];
	char TCPPACKET[200] ="";
	itoa(ADCval,voltagebuffer);
	ST7735_OutString(" ");
	ST7735_OutString(voltagebuffer);
	
	strcat(TCPPACKET,SENDSTRING1);
	strcat(TCPPACKET,voltagebuffer);
	strcat(TCPPACKET,SENDSTRING2);
	
	//UARTprintf(TCPPACKET);

	

	strcpy(HostName,SERVER);
		unsigned long startTime =  NVIC_ST_CURRENT_R;
		queryWebserver(HostName,TCPPACKET);;
		unsigned long endTime = NVIC_ST_CURRENT_R;
		process_Times(endTime-startTime,&VoltageTiming);
	
	}

	
	
	void printHeader(){
		ST7735_SetCursor(0,0);
		ST7735_OutString("Number of runs ");
		
		
	}
	
	void processData(){
		ST7735_SetCursor(0,0);
		ST7735_OutString("Number of runs ");
		ST7735_OutUDec(timesqueried/2);
		ST7735_SetCursor(0,3);
		ST7735_OutString("Packets Lost ");
		ST7735_OutUDec(lostpackets);
		ST7735_SetCursor(0,4);
		ST7735_OutString("Percentage Lost ");
		int percentlost = lostpackets/timesqueried;
		ST7735_OutUDec(percentlost);
		
		ST7735_SetCursor(0,5);
		ST7735_OutString("Weather Download ");
		ST7735_SetCursor(0,6);
		ST7735_OutString("Max ");
		ST7735_OutUDec(WeatherTimings.max*1/50000);
		ST7735_OutString(" ms");
		ST7735_SetCursor(0,7);
		ST7735_OutString("Min ");
		ST7735_OutUDec(WeatherTimings.min*1/50000);
		ST7735_OutString(" ms");
		ST7735_SetCursor(0,8);
		ST7735_OutString("Average ");
		ST7735_OutUDec(WeatherTimings.total/(timesqueried/2)*1/50000);
		ST7735_OutString(" ms");
		
		ST7735_SetCursor(0,10);
		ST7735_OutString("Voltage Upload ");
		ST7735_SetCursor(0,11);
		ST7735_OutString("Max ");
		ST7735_OutUDec(VoltageTiming.max*1/50000);
		ST7735_OutString(" ms");
		ST7735_SetCursor(0,12);
		ST7735_OutString("Min ");
		ST7735_OutUDec(VoltageTiming.min*1/50000);
		ST7735_OutString(" ms");
		ST7735_SetCursor(0,13);
		ST7735_OutString("Average ");
		ST7735_OutUDec(VoltageTiming.total/(timesqueried/2)*1/50000);
		ST7735_OutString(" ms");
	}

/*
 * Application's entry point
 */
// 1) change Austin Texas to your city
// 2) you can change metric to imperial if you want temperature in F


// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID

	
int main(void){
	UARTprintf("Weather App\n");
	init_All();
  connect_internet();
  while(1){
			printHeader();
			getVoltageData();
			getWeather();
			processData();
			LED_GreenOn();
		while(Board_Input()==0){}; // wait for touch
			LED_GreenOff();
			ST7735_FillScreen(0);
			
		
      }
   
  }




/*!
    \brief This function puts the device in its default state. It:
           - Set the mode to STATION
           - Configures connection policy to Auto and AutoSmartConfig
           - Deletes all the stored profiles
           - Enables DHCP
           - Disables Scan policy
           - Sets Tx power to maximum
           - Sets power policy to normal
           - Unregister mDNS services
    \param[in]      none
    \return         On success, zero is returned. On error, negative is returned
*/
static int32_t configureSimpleLinkToDefaultState(char *pConfig){
  SlVersionFull   ver = {0};
  UINT8           val = 1;
  UINT8           configOpt = 0;
  UINT8           configLen = 0;
  UINT8           power = 0;

  INT32           retVal = -1;
  INT32           mode = -1;

  mode = sl_Start(0, pConfig, 0);


    /* If the device is not in station-mode, try putting it in station-mode */
  if (ROLE_STA != mode){
    if (ROLE_AP == mode){
            /* If the device is in AP mode, we need to wait for this event before doing anything */
      while(!IS_IP_AQUIRED(g_Status));
    }

        /* Switch to STA role and restart */
    retVal = sl_WlanSetMode(ROLE_STA);

    retVal = sl_Stop(0xFF);

    retVal = sl_Start(0, pConfig, 0);

        /* Check if the device is in station again */
    if (ROLE_STA != retVal){
            /* We don't want to proceed if the device is not coming up in station-mode */
      return DEVICE_NOT_IN_STATION_MODE;
    }
  }
    /* Get the device's version-information */
  configOpt = SL_DEVICE_GENERAL_VERSION;
  configLen = sizeof(ver);
  retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (unsigned char *)(&ver));

    /* Set connection policy to Auto + SmartConfig (Device's default connection policy) */
  retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);

    /* Remove all profiles */
  retVal = sl_WlanProfileDel(0xFF);

    /*
     * Device in station-mode. Disconnect previous connection if any
     * The function returns 0 if 'Disconnected done', negative number if already disconnected
     * Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
     */
  retVal = sl_WlanDisconnect();
  if(0 == retVal){
        /* Wait */
     while(IS_CONNECTED(g_Status));
  }

    /* Enable DHCP client*/
  retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&val);

    /* Disable scan */
  configOpt = SL_SCAN_POLICY(0);
  retVal = sl_WlanPolicySet(SL_POLICY_SCAN , configOpt, NULL, 0);

    /* Set Tx power level for station mode
       Number between 0-15, as dB offset from max power - 0 will set maximum power */
  power = 0;
  retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&power);

    /* Set PM policy to normal */
  retVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);

    /* TBD - Unregister mDNS services */
  retVal = sl_NetAppMDNSUnRegisterService(0, 0);


  retVal = sl_Stop(0xFF);


  g_Status = 0;
  memset(&Recvbuff,0,MAX_RECV_BUFF_SIZE);
  memset(&SendBuff,0,MAX_SEND_BUFF_SIZE);
  memset(&HostName,0,MAX_HOSTNAME_SIZE);
  DestinationIP = 0;;
  SockID = 0;


  return retVal; /* Success */
}




/*
 * * ASYNCHRONOUS EVENT HANDLERS -- Start
 */

/*!
    \brief This function handles WLAN events
    \param[in]      pWlanEvent is the event passed to the handler
    \return         None
    \note
    \warning
*/
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent){
  switch(pWlanEvent->Event){
    case SL_WLAN_CONNECT_EVENT:
    {
      SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);

            /*
             * Information about the connected AP (like name, MAC etc) will be
             * available in 'sl_protocol_wlanConnectAsyncResponse_t' - Applications
             * can use it if required
             *
             * sl_protocol_wlanConnectAsyncResponse_t *pEventData = NULL;
             * pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
             *
             */
    }
    break;

    case SL_WLAN_DISCONNECT_EVENT:
    {
      sl_protocol_wlanConnectAsyncResponse_t*  pEventData = NULL;

      CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
      CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_AQUIRED);

      pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            /* If the user has initiated 'Disconnect' request, 'reason_code' is SL_USER_INITIATED_DISCONNECTION */
      if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code){
        UARTprintf(" Device disconnected from the AP on application's request \r\n");
      }
      else{
        UARTprintf(" Device disconnected from the AP on an ERROR..!! \r\n");
      }
    }
    break;

    default:
    {
      UARTprintf(" [WLAN EVENT] Unexpected event \r\n");
    }
    break;
  }
}

/*!
    \brief This function handles events for IP address acquisition via DHCP
           indication
    \param[in]      pNetAppEvent is the event passed to the handler
    \return         None
    \note
    \warning
*/
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent){
  switch(pNetAppEvent->Event)
  {
    case SL_NETAPP_IPV4_ACQUIRED:
    {

      SET_STATUS_BIT(g_Status, STATUS_BIT_IP_AQUIRED);
        /*
             * Information about the connected AP's ip, gateway, DNS etc
             * will be available in 'SlIpV4AcquiredAsync_t' - Applications
             * can use it if required
             *
             * SlIpV4AcquiredAsync_t *pEventData = NULL;
             * pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
             * <gateway_ip> = pEventData->gateway;
             *
             */

    }
    break;

    default:
    {
            UARTprintf(" [NETAPP EVENT] Unexpected event \r\n");
    }
    break;
  }
}

/*!
    \brief This function handles callback for the HTTP server events
    \param[in]      pServerEvent - Contains the relevant event information
    \param[in]      pServerResponse - Should be filled by the user with the
                    relevant response information
    \return         None
    \note
    \warning
*/
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse){
    /*
     * This application doesn't work with HTTP server - Hence these
     * events are not handled here
     */
  UARTprintf(" [HTTP EVENT] Unexpected event \r\n");
}

/*!
    \brief This function handles general error events indication
    \param[in]      pDevEvent is the event passed to the handler
    \return         None
*/
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent){
    /*
     * Most of the general errors are not FATAL are are to be handled
     * appropriately by the application
     */
  UARTprintf(" [GENERAL EVENT] \r\n");
}

/*!
    \brief This function handles socket events indication
    \param[in]      pSock is the event passed to the handler
    \return         None
*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock){
  switch( pSock->Event )
  {
    case SL_NETAPP_SOCKET_TX_FAILED:
    {
            /*
            * TX Failed
            *
            * Information about the socket descriptor and status will be
            * available in 'SlSockEventData_t' - Applications can use it if
            * required
            *
            * SlSockEventData_t *pEventData = NULL;
            * pEventData = & pSock->EventData;
            */
      switch( pSock->EventData.status )
      {
        case SL_ECLOSE:
          UARTprintf(" [SOCK EVENT] Close socket operation failed to transmit all queued packets\r\n");
          break;


        default:
          UARTprintf(" [SOCK EVENT] Unexpected event \r\n");
          break;
      }
    }
    break;

    default:
      UARTprintf(" [SOCK EVENT] Unexpected event \r\n");
    break;
  }
}
/*
 * * ASYNCHRONOUS EVENT HANDLERS -- End
 */
