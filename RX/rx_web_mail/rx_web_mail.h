/*************************user modify settings****************************/
byte addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char  HopCH[3] = {105,76,108};//Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx

#define DATA_LENGTH				4                    //use fixed data length 1-32
#define BUZZON					100                //set lenght of the buzz
#define BUZZOFF					30000            //set interval of the buzz
#define RFID_NUMBER				8


#if defined(__AVR_ATmega2560__)	
#define DEGBUG_OUTPUT
#define TIME_OUT_CLOSE_DOOR		20        //s
#define IN_OUT_CLEAN			10        //s
#define WIFI_SERIAL Serial3
#else
#define WIFI_SERIAL Serial
#define TIME_OUT_CLOSE_DOOR		60        //s
#define IN_OUT_CLEAN			50        //s
#endif

#define TIME_SYNC_TIME_OUT 86400

#define MAIL_TIME_OUT  30


#define COMPENSATION_MS_IN_ONE_SECOND 0 //+9;-10;...    2560:-9
#define COMPENSATION_SECOND_IN 5775//05775 //second			2560:1452
#define COMPENSATION_SECOND_DIRECTION -- //  ++;--


/*****************************************************/

#define USE_CONFIG_FILE

#if defined USE_CONFIG_FILE
#include "D:\GitHub\Private\config.h"
#else
char NameList[][RFID_NUMBER] = {"Unknown","Unknown","Unknown","Unknown","Unknown","Unknown","Unknown","Unknown"}; 
#endif







/*************************include*******************/
#include <SPI.h>
#include "RF24.h"
#include <printf.h>
#include "TimeLib.h"
#include <EEPROM.h>
#include <avr/wdt.h>


/*****************************************************/



/*************************nRF Module*******************/
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long LastChangeCHTime = 0;
unsigned long LastGetTime[RFID_NUMBER] = {0};
bool RfidOnline[RFID_NUMBER] = {false};

//Look out direction of RFID
unsigned long LastOnlineTime[RFID_NUMBER] = {0};
bool LastOnlineAtUp[RFID_NUMBER];
unsigned char LastOnlineStoreIndex[RFID_NUMBER] = {0};

unsigned long CurrTime = 0;
unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
/**********************************************************/

/*************************Output*******************/
#define DOOR 10
//bool DoorRelease = false;
bool DoorLastState = false;
//unsigned char  DoorLastOnID = 0;

#define BUZZ 9
bool BuzzOutputHigh;
bool AlarmOn;
/**********************************************************/

/*************************Record*******************/
struct tRecord
{
	unsigned long tag_time = 0;
	unsigned char code = 0; // //0 up,1 down 2 in 3 out
	unsigned char ID = 0;
	unsigned int volt = 0;
};
tRecord Record;
unsigned char RecordLength = sizeof(Record);
unsigned char RecordCounter = 1024/RecordLength;
unsigned char NextRecord = 0;
char CharRecord[100];
unsigned char CharRecordLen;
unsigned char ReadedRecord = 0;
unsigned char RecordShoudRead = 0;
unsigned char LatestRecord = 0;
#define SHORT_LIST_NUMBER 16

/**********************************************************/

/*************************Time*******************/
time_t t;
unsigned long SecondsSinceStart = 0;
unsigned long Start1970OffSet = 0;
unsigned long TempSecondsSince1970 = 0;
unsigned long LastSyncTime = 0;
signed long LastSyncOffSet = 0;
bool NtpWorking = false;
bool NtpSync = false;
unsigned char NtpDataIndex = 0;
/**********************************************************/

/*************************WiFi*******************/
typedef enum
{
	STEP_WIFI_IDLE,
	STEP_TRUN_OFF_TRANS,
	STEP_SET_MODE,
	STEP_RESET,
	STEP_CHECK_WIFI_GOT_IP,
	STEP_CONNECT_AP,
	STEP_CONNECT_SERVER,
	STEP_SEND_TIME_REQUEST_LEN,
	STEP_SEND_TIME_REQUEST,
	STEP_GET_TIME,
	STEP_SET_MUX,
	STEP_SERVER_SET,
	STEP_PROCESS_TIME,
	STEP_WAIT_REQUEST,
	STEP_SEND_HTTP_HEAD,
	STEP_SEND_HTTP_TITLE,
	STEP_SEND_HTTP_TITLE_LEN,
	STEP_SEND_HTTP_BODY_LEN,
	STEP_SEND_HTTP_BODY,
	STEP_SEND_HTTP_END_LEN,
	STEP_SEND_HTTP_END,
	STEP_WAIT_CLOSED,
	STEP_HTTP_CLOSE,
} eWiFiStep;


eWiFiStep WiFiNextStep = STEP_WIFI_IDLE;
bool WiFiLastReslut = false;
bool WiFiOK = false;



unsigned char TimeOut = 0;
unsigned long ActiveTime = 0;
eWiFiStep TimeOutStep;

bool bWebStarted = false;


const unsigned char NTP_Request[] PROGMEM = {0xdb, 0x00 ,0x0a ,0xfa ,0x00 ,0x00 ,0x4c ,0x5a ,0x00 ,0x08 ,0xca ,0xac ,0x00 ,0x00 ,0x00 ,0x00 ,
0xdd ,0x35 ,0x79 ,0xb8 ,0xaf ,0xa1 ,0x91 ,0xb5 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xdd ,0x35 ,0x79 ,0xb9 ,0xb7 ,0x4f ,0xa6 ,0x30
};

const char HttpResponseHead[]   ="HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html lang=\"zh-cn\">\r\n<head>\r\n<meta charset=\"GB2312\">\r\n<title>Room</title>\r\n</head>\r\n<body>\r\n";
char HttpResponseEnd[] ="</body>\r\n</html>\r\n";

/**********************************************************/



/************************Mail**********************************/
//bool MailSending = false;

#define MAIL_QUEUE_NUMBER 8
unsigned char MailQueue[MAIL_QUEUE_NUMBER];
unsigned char QueueNumber = 0;
unsigned char CurrentMailMemIndex = 0;
/**********************************************************/



void Mail_task();
void Door_task();
void nRF_task();
void Buzz_task();
void InsertRecord(tRecord * pRecord);
void WriteRecord(unsigned char index,tRecord * pRecord);
void ReadRecord(unsigned char index,tRecord * pRecord);
unsigned char FindLastRecord();
void SecondsSinceStartTask();
void WebStart();
void ConnectAp();
void ResetWiFi();
void StartNtp();
void ProcessNTP(unsigned long data);
void CheckTimeOut();
void CheckForInput();
bool CheckResponse(unsigned char data,const char * ret);
bool CheckParameter(unsigned char data,const char * ret,unsigned char * pPara);
void OnWiFiData(unsigned char data);
unsigned char CharLength(char * MyChar);
void OnSecond();
void NonStopTask();
bool SendMail();
void PushMailQueue(unsigned char MemIndex);
unsigned char PopMailQueue();
void RfidOnLine(unsigned char ID,bool OnlineAtUp);
void InOUtCleanTask();

///***______________________ESP module Function Declarations__________________**///
void _esp8266_putch( char);
bool _esp8266_getch(char * RetData);   
/** Function prototypes **/
bool esp8266_isStarted(void);        // Check if the module is started (AT)
bool esp8266_restart(void);          // Restart module (AT+RST)
void esp8266_echoCmds(bool);        // Enabled/disable command echoing (ATE)
// WIFI Mode (station/softAP/station+softAP) (AT+CWMODE)
void esp8266_mode( char);
// Connect to AP (AT+CWJAP)
void esp8266_connect( char*,  char*);
// Disconnect from AP (AT+CWQAP)
void esp8266_disconnect(void);
// Local IP (AT+CIFSR)
void esp8266_ip(char*);
// Create connection (AT+CIPSTART)
bool esp8266_start( char protocol, char* ip,  char port);
// Send data (AT+CIPSEND)
bool esp8266_send( char*);
// Receive data (+IPD)
void esp8266_receive( char*, uint16_t, bool);
/** Functions for internal use only **/
// Print a string to the output
void _esp8266_print(const char*);
void _esp8266_print_nc(char*);
// Wait for a certain string on the input
bool _esp8266_waitFor(const char *);
// Wait for any response on the input
bool _esp8266_waitResponse(void);
//void Lcd_Set_Cursor(char , char b);
//void Lcd_Print_Char(char);
//void Lcd_Print_String(char *);
bool _esp8266_login_mail( char*,  char*);
bool _esp8266_mail_sendID( char*);
bool _esp8266_mail_recID( char*);
bool _esp8266_mail_subject( char*);
bool _esp8266_mail_body( char*);

bool _esp8266_create_server(); //Create a server on port 80
bool _esp8266_enale_MUX();
bool _esp8266_connect_SMPT2GO();
bool _esp8266_disconnect_SMPT2GO();
bool _esp8266_start_mail();
bool _esp8266_End_mail();
//********__________________End of Function Declaration_________________********///






void setup()
{





#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_WiFi_ID_Read"));
	printf_begin();
#endif
	WIFI_SERIAL.begin(115200);

	pinMode(DOOR, OUTPUT);


	radio.begin();
	radio.setPALevel(RF24_PA_MIN);
	radio.setAddressWidth(5);
	radio.setPayloadSize(4);
	radio.setDataRate(RF24_2MBPS); //RF24_250KBPS  //RF24_2MBPS     //RF24_1MBPS
	radio.setChannel(105);
	radio.setCRCLength(RF24_CRC_8); //RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit

	//Open a writing and reading pipe on each radio, with opposite addresses
	radio.openWritingPipe(addresses);
	radio.openReadingPipe(0,addresses);
	radio.closeReadingPipe(1);
	radio.closeReadingPipe(2);
	radio.closeReadingPipe(3);
	radio.closeReadingPipe(4);
	radio.closeReadingPipe(5);
	radio.setAutoAck(1,false);
	radio.setAutoAck(2,false);
	radio.setAutoAck(3,false);
	radio.setAutoAck(4,false);
	radio.setAutoAck(5,false);
	radio.setAutoAck(0,false);
	//Start the radio listening for data
	radio.startListening();


	NextRecord = FindLastRecord();

	wdt_enable(WDTO_2S);



}

void loop()
{





	if ((!WiFiOK)&&(WiFiNextStep == STEP_WIFI_IDLE))
	{
		ConnectAp();
	}

	if ((WiFiOK)&&(!NtpWorking)&&(!NtpSync))
	{
#ifdef DEGBUG_OUTPUT
		printf("WiFi OK, send NTP request \r\n");
#endif
		StartNtp();
		NtpWorking = true;
	}

	if ((WiFiOK)&&(NtpSync)&&(!bWebStarted))
	{
#ifdef DEGBUG_OUTPUT
		printf("WiFi OK, NtpSync,start web \r\n");
#endif
		WebStart();
		bWebStarted = true;
	}





	NonStopTask();


	CheckForInput();
	Mail_task();


} // Loop

void NonStopTask()
{
	nRF_task();
	SecondsSinceStartTask();
	Buzz_task();

}
void Mail_task()
{
	//static bool RfidOnlineSent[RFID_NUMBER] = {false};

	if ((QueueNumber > 0)&&(WiFiNextStep == STEP_WAIT_REQUEST))
	{
		CurrentMailMemIndex = PopMailQueue();
		SendMail();
	}



}

void nRF_task()
{
	if( radio.available())
	{
		// Variable for the received timestamp
		while (radio.available())                                     // While there is data ready
		{
			radio.read( GotData,DATA_LENGTH);             // Get the payload
		}
		Volt=1.2*(GotData[DATA_LENGTH-2]*256+GotData[DATA_LENGTH-1])*3*1000/4096;

		if ((GotData[0] == 0)&&(GotData[1]<RFID_NUMBER))
		{
			LastGetTime[GotData[1]] = SecondsSinceStart;
		}

		PackageCounter ++;

		Door_task();


#ifdef DEGBUG_OUTPUT
		Serial.print(PackageCounter);
		Serial.print(" ");
		Serial.print(F("Get data "));
		for(char i=0; i<DATA_LENGTH ; i++)
		{
			printf("%d,",GotData[i]);
		}
		printf("Volt:%d ",Volt);
		printf("CH:%d\r\n",CurrCH);
#endif

	}


	CurrTime = millis();
	if (abs(CurrTime - LastChangeCHTime>1000))//RF_HOP
	{
		CurrCH++;
		if (CurrCH>2)
		{
			CurrCH = 0;
		}
		LastChangeCHTime = millis();
		radio.stopListening();
		radio.setChannel(HopCH[CurrCH]);
		radio.startListening();
	}


}

void Door_task()
{

	bool DoorShouldOpen = false;
	for(unsigned char i = 0;i < RFID_NUMBER;i++)
	{
		if ((LastGetTime[i]!=0)&&( SecondsSinceStart - LastGetTime[i] < TIME_OUT_CLOSE_DOOR))
		{
			if (!RfidOnline[i])//offline before
			{
#ifdef DEGBUG_OUTPUT
				printf("RFID %d online. \r\n",i);
#endif
				if (NtpSync)
				{
					RfidOnLine(GotData[1],true);
				}
			} 
			RfidOnline[i] = true;
			DoorShouldOpen = true;
		}
		else
		{
			if (RfidOnline[i])//online before
			{	

				RfidOnline[i] = false;
#ifdef DEGBUG_OUTPUT
				printf("RFID %d offline. \r\n",i);

#endif

			}
		}
	}

	if (DoorLastState != DoorShouldOpen)//update relay when door status change
	{
		DoorLastState = DoorShouldOpen;
		if(DoorShouldOpen)
		{
			digitalWrite(DOOR, HIGH);
			AlarmOn = true;
		}
		else
		{
#ifdef DEGBUG_OUTPUT
			printf("Close door  \r\n");
#endif

			digitalWrite(DOOR, LOW);
			AlarmOn = false;
		}
	}
}


void Buzz_task()
{

	static signed int BuzzOn;
	static signed int BuzzOff;
	if (AlarmOn)
	{
		if (BuzzOutputHigh)
		{
			BuzzOn++;
			if (BuzzOn > BUZZON)
			{
				BuzzOutputHigh = false;
				BuzzOn = 0;
			}
			else
			{
				BuzzOutputHigh = true;
			}
		}
		else
		{
			BuzzOff++;
			if (BuzzOff > BUZZOFF)
			{
				BuzzOutputHigh = true;
				BuzzOff = 0;
			}
			else
			{
				BuzzOutputHigh = false;
			}
		}
	}
	else
	{
		BuzzOn = 0;
		BuzzOff = 0;
		BuzzOutputHigh = false;
	}

	if (BuzzOutputHigh)
	{
		digitalWrite(BUZZ, HIGH);
	}
	else
	{
		digitalWrite(BUZZ, LOW);
	}
}

void InsertRecord(tRecord * pRecord)
{


	WriteRecord(NextRecord,pRecord);


	NextRecord++;
	if(NextRecord>=RecordCounter)
	{
		NextRecord = 0;
	}

}


void WriteRecord(unsigned char index,tRecord * pRecord)
{
	unsigned char i;
	for(i=0; i<RecordLength; i++)
	{
		EEPROM.write(index*RecordLength+i,*(((unsigned char *)(pRecord))+i));
	}
	t = pRecord->tag_time;
#ifdef DEGBUG_OUTPUT
	printf("Write Record! Index %d. Date Time = %d-%d-%d %d:%d:%d Code:%d. ID:%d. Volt:%d mV\r\n",index,year(t) ,month(t),day(t),hour(t),minute(t),second(t),pRecord->code,pRecord->ID,pRecord->volt);
#endif
}

void ReadRecord(unsigned char index,tRecord * pRecord)
{
	unsigned char i;
	for(i=0; i<RecordLength; i++)
	{
		*(((unsigned char *)(pRecord))+i) = EEPROM.read(index*RecordLength+i);
	}

	t = pRecord->tag_time;
#ifdef DEGBUG_OUTPUT
	printf("Read Record! Index %03d. Date Time = %d-%02d-%02d %02d:%02d:%02d Code:%d. ID:%d. Volt:%d mV\r\n",index,year(t) ,month(t),day(t),hour(t),minute(t),second(t),pRecord->code,pRecord->ID,pRecord->volt);
#endif
}

unsigned char FindLastRecord()
{
	tRecord Record;
	unsigned char i;
	unsigned char MinIndex = 0;
	unsigned long MinTime = 0xFFFFFFFF;
	for(i=0; i<RecordCounter; i++)
	{
		ReadRecord(i,&Record);
		if(MinTime>Record.tag_time)
		{
			MinTime = Record.tag_time;
			MinIndex = i;
		}
	}

	t = MinTime;
#ifdef DEGBUG_OUTPUT
	printf("Min Record index %03d! Date Time = %d-%02d-%02d %02d:%02d:%02d \r\n",MinIndex,year(t) ,month(t),day(t),hour(t),minute(t),second(t));
#endif
	return MinIndex;

}

void OnSecond()
{
	if (SecondsSinceStart%COMPENSATION_SECOND_IN == 0)
	{
		Start1970OffSet COMPENSATION_SECOND_DIRECTION;
	}



	//Record.tag_time=0;
	//Record.code= 0;
	//Record.ID =  0;
	//Record.volt = 0;
	//InsertRecord(&Record);

	wdt_reset();

	Door_task();

	CheckTimeOut();

	InOUtCleanTask();

}

unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if(abs(CurrentMillis-LastMillis)> 1000 + COMPENSATION_MS_IN_ONE_SECOND)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;
		OnSecond();

		time_t t = SecondsSinceStart + Start1970OffSet;
#ifdef DEGBUG_OUTPUT
		//printf("Date Time = %d-%02d-%02d %02d:%02d:%02d \r\n",year(t) ,month(t),day(t),hour(t),minute(t),second(t));
		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
#endif
	}
}



void ResetWiFi()
{
	WiFiOK = false;
	NtpWorking = false;
	NtpSync = false;
	bWebStarted = false;
#ifdef DEGBUG_OUTPUT
	printf("\r\n set all to false, reset wifi \r\n");
#endif
	WiFiNextStep = STEP_RESET;
	OnWiFiData('O');
	OnWiFiData('K');

}

void WebStart()
{
	WIFI_SERIAL.print(F("AT+CIPMUX=1\r\n"));
	//TimeOut = 1;
	//WiFiTask = TASK_WEB_SERVER;
	WiFiNextStep = STEP_SET_MUX;
}

void ConnectAp()
{
	WIFI_SERIAL.print(F("+++"));
	delay(100);
	WIFI_SERIAL.print(F("AT+CWMODE=1\r\n"));
	WiFiNextStep = STEP_RESET;
}

void StartNtp()
{
	WIFI_SERIAL.print(F("AT+CIPSTART=\"UDP\",\"24.56.178.140\",123\r\n"));
	WiFiNextStep = STEP_SEND_TIME_REQUEST_LEN;
	NtpDataIndex = 0;
	TempSecondsSince1970 = 0;
}

void ProcessNTP(unsigned long data)
{

	if (NtpDataIndex==40)
	{
		//TempSecondsSince1970 = TempSecondsSince1970 + data * 0xFFFFFF;
		*(((unsigned char *)(&TempSecondsSince1970))+3) = data;
	}
	if (NtpDataIndex==41)
	{
		//TempSecondsSince1970 = TempSecondsSince1970 + data * 0xFFFF;
		*(((unsigned char *)(&TempSecondsSince1970))+2) = data;
	}
	if (NtpDataIndex==42)
	{
		//TempSecondsSince1970 = TempSecondsSince1970 + data * 0xFF;
		*(((unsigned char *)(&TempSecondsSince1970))+1) = data;
	}
	if (NtpDataIndex==43)
	{
		//TempSecondsSince1970 = TempSecondsSince1970 + data ;
		*(((unsigned char *)(&TempSecondsSince1970))) = data;
		//LastSyncTime = TempSecondsSince1970-2208988800+8*60+8*3600;//-145;
		LastSyncTime = TempSecondsSince1970-0x83aa7e80+8*3600;
		LastSyncOffSet = SecondsSinceStart+Start1970OffSet-LastSyncTime;
		Start1970OffSet = LastSyncTime-SecondsSinceStart;
		NtpSync = true;
		NtpWorking = false;
		TimeOut = 5;
		time_t t = LastSyncTime;
#ifdef DEGBUG_OUTPUT
		printf("\r\nDate Time = %d-%02d-%02d %02d:%02d:%02d \r\n",year(t) ,month(t),day(t),hour(t),minute(t),second(t));
#endif
	}
	NtpDataIndex++;
}


void CheckTimeOut()
{

	if ((SecondsSinceStart + Start1970OffSet - LastSyncTime > TIME_SYNC_TIME_OUT)&&(NtpSync))
	{
		ResetWiFi();
	}



	if (TimeOut!=0)
	{
		if(SecondsSinceStart - ActiveTime > TimeOut)
		{
			if ((WiFiNextStep == STEP_GET_TIME) ||  (WiFiNextStep == STEP_PROCESS_TIME))
			{
				WiFiNextStep = STEP_SEND_TIME_REQUEST_LEN;
				OnWiFiData('O');
				OnWiFiData('K');
				TimeOut = 0;
			}
			if ((WiFiNextStep > STEP_WAIT_REQUEST)&&(WiFiNextStep < STEP_HTTP_CLOSE))
			{
#ifdef DEGBUG_OUTPUT
				printf("\r\n close when time out \r\n");
#endif
				WIFI_SERIAL.print(F("AT+CIPCLOSE=1\r\n"));
				WiFiNextStep = STEP_WAIT_REQUEST;
				TimeOut = 0;
			}

		}
	}

}


void CheckForInput()
{
	while (WIFI_SERIAL.available() > 0)
	{
		unsigned char RetData = WIFI_SERIAL.read();

		OnWiFiData(RetData);

#ifdef DEGBUG_OUTPUT
		//Serial.write(RetData);
#endif

	}
}

bool CheckResponse(unsigned char GetData,const char * ExpectResponse)
{
	static  char CheckedBytePostion = -1;
	if (GetData == ExpectResponse[CheckedBytePostion+1])
	{
		CheckedBytePostion++;
		if(ExpectResponse[CheckedBytePostion+1] == 0)
		{
			return true;
		}
	}
	else
	{
		CheckedBytePostion = -1;
	}

	return false;
}

bool CheckParameter(unsigned char GetData,const char * pPrefix,unsigned char * pPara)
{
	static bool PrefixFound = false;
	if(!PrefixFound)
	{
		if (CheckResponse(GetData,pPrefix))
		{
			PrefixFound = true;
		}
	}
	else
	{
		*pPara = GetData;
		PrefixFound = false;
		return true;
	}
	return false;

}

void OnWiFiData(unsigned char GetData)
{
	switch  (WiFiNextStep)
	{
		//    case STEP_TRUN_OFF_TRANS:
		//        printf("\r\n  STEP_TRUN_OFF_TRANS time out =%d; \r\n",SecondsSinceStart - SentTime);
		//        
		//        //if(SecondsSinceStart - SentTime > TimeOut)
		//        //{
		//
		//        //}
		//        break;

	case STEP_RESET:
		if (CheckResponse(GetData,"OK"))
		{
			WIFI_SERIAL.print(F("AT+RST\r\n"));
			//WIFI_SERIAL.print(F("AT+CWJAP=\"frye\",\"52150337\"\r\n"));
			WiFiNextStep = STEP_CHECK_WIFI_GOT_IP;
		}
		break;
	case STEP_CHECK_WIFI_GOT_IP:
		if (CheckResponse(GetData,"WIFI GOT IP"))
		{
			WiFiNextStep = STEP_WIFI_IDLE;
			WiFiOK = true;
		}
		break;

	case STEP_SEND_TIME_REQUEST_LEN:
		if (CheckResponse(GetData,"OK"))
		{
			WIFI_SERIAL.print(F("AT+CIPSEND=48\r\n"));
			WiFiNextStep = STEP_SEND_TIME_REQUEST;
		}
		break;

	case STEP_SEND_TIME_REQUEST:
		if (CheckResponse(GetData,">"))
		{
			for(unsigned char i = 0; i<48 ; i++)
			{
				//WIFI_SERIAL.print(NTP_Request[i]);
				WIFI_SERIAL.print(pgm_read_byte_near(NTP_Request + i));

			}

			ActiveTime = SecondsSinceStart;
			TimeOut = 5;
			WiFiNextStep = STEP_GET_TIME;
			return;
		}
		break;
	case STEP_GET_TIME:
		if (CheckResponse(GetData,"+IPD,48:"))
		{
			WiFiNextStep = STEP_PROCESS_TIME;
		}
		break;

	case STEP_PROCESS_TIME:
		ProcessNTP(GetData);
		break;

	case STEP_SET_MUX:
		if (CheckResponse(GetData,"OK"))
		{
			WIFI_SERIAL.print(F("AT+CIPSERVER=1,8081\r\n"));
			WiFiNextStep = STEP_SERVER_SET;
		}
		break;
	case STEP_SERVER_SET:
		if (CheckResponse(GetData,"OK"))
		{
			WiFiNextStep = STEP_WAIT_REQUEST;
		}
		break;

	case STEP_WAIT_REQUEST:
		unsigned char Para;
		if (CheckParameter(GetData,"GET /",&Para))
		{
#ifdef DEGBUG_OUTPUT
			printf("\r\n Get para = \"%c\"  value = %d \r\n",Para,Para);
#endif
			if (Para == ' ')
			{
#ifdef DEGBUG_OUTPUT
				printf("\r\n request home page , send head len\r\n");
#endif
				delay(100);
				WIFI_SERIAL.print(F("AT+CIPSEND=1,"));
				//WIFI_SERIAL.print(strlen_P(HttpResponseHead));
				WIFI_SERIAL.print(sizeof(HttpResponseHead));
				WIFI_SERIAL.print(F("\r\n"));

				RecordShoudRead = SHORT_LIST_NUMBER;

				WiFiNextStep = STEP_SEND_HTTP_HEAD;

				ActiveTime = SecondsSinceStart;
				TimeOut = 15;
			}
			else if(Para == 'a')
			{
#ifdef DEGBUG_OUTPUT
				printf("\r\n request home page , send head len\r\n");
#endif
				delay(100);
				WIFI_SERIAL.print(F("AT+CIPSEND=1,"));
				//WIFI_SERIAL.print(strlen_P(HttpResponseHead));
				WIFI_SERIAL.print(sizeof(HttpResponseHead));
				WIFI_SERIAL.print(F("\r\n"));

				RecordShoudRead = RecordCounter;

				WiFiNextStep = STEP_SEND_HTTP_HEAD;

				ActiveTime = SecondsSinceStart;
				TimeOut = 15;

			}
			else if((Para >= '0')&&(Para <= '9'))
			{
#ifdef DEGBUG_OUTPUT
				printf("\r\n Got ID online from wifi \r\n");
#endif
				RfidOnLine(Para-'0',false);
				WIFI_SERIAL.print(F("AT+CIPCLOSE=1\r\n"));
				//WiFiNextStep = STEP_WAIT_REQUEST;
			}
			else
			{
#ifdef DEGBUG_OUTPUT
				printf("\r\n close not request home page \r\n");
#endif
				WIFI_SERIAL.print(F("AT+CIPCLOSE=1\r\n"));
				//WiFiNextStep = STEP_WAIT_REQUEST;
			}
		}

		break;

	case STEP_SEND_HTTP_HEAD:
		if (CheckResponse(GetData,">"))
		{
#ifdef DEGBUG_OUTPUT
			printf("\r\n send response head \r\n");
#endif
			//for(unsigned char i = 0; i<strlen_P(HttpResponseHead) ; i++)
			for(unsigned char i = 0; i<sizeof(HttpResponseHead) ; i++)
			{
				//WIFI_SERIAL.print(pgm_read_byte_near(HttpResponseHead+i));
				WIFI_SERIAL.print(HttpResponseHead[i]);
			}
			WiFiNextStep = STEP_SEND_HTTP_TITLE_LEN;
			ReadedRecord = 0;
			LatestRecord = (NextRecord>0?NextRecord-1:RecordCounter-1);

#ifdef DEGBUG_OUTPUT
			printf("\r\n LatestRecord = %d \r\n",LatestRecord);
#endif

			ActiveTime = SecondsSinceStart;
		}
		break;


	case STEP_SEND_HTTP_TITLE_LEN:
		if (CheckResponse(GetData,"SEND OK"))
		{
#ifdef DEGBUG_OUTPUT
			printf("\r\n send TITLE len \r\n");
#endif
			t = LastSyncTime;
			time_t t2 = SecondsSinceStart+Start1970OffSet;
			sprintf(CharRecord,"Time: %d-%02d-%02d %02d:%02d:%02d <br>\r\nSync: %d-%02d-%02d %02d:%02d:%02d   OffSet: %d<br>---<br>\r\n",year(t2) ,month(t2),day(t2),hour(t2),minute(t2),second(t2),year(t) ,month(t),day(t),hour(t),minute(t),second(t),LastSyncOffSet);
			CharRecordLen = CharLength(CharRecord);
			WIFI_SERIAL.print(F("AT+CIPSEND=1,"));
			WIFI_SERIAL.print(CharRecordLen);
			WIFI_SERIAL.print(F("\r\n"));

			WiFiNextStep = STEP_SEND_HTTP_TITLE;
			ActiveTime = SecondsSinceStart;
		}
		break;

	case STEP_SEND_HTTP_TITLE:
		if (CheckResponse(GetData,">"))
		{
#ifdef DEGBUG_OUTPUT
			printf("\r\n send HTTP_TITLE\r\n");
#endif

			TimeOut = 15;
			for(unsigned char i = 0; i<CharRecordLen ; i++)
			{
				WIFI_SERIAL.print(CharRecord[i]);
			}
			WiFiNextStep = STEP_SEND_HTTP_BODY_LEN;
			ActiveTime = SecondsSinceStart;
		}
		break;

	case STEP_SEND_HTTP_BODY_LEN:
		if (CheckResponse(GetData,"SEND OK"))
		{
#ifdef DEGBUG_OUTPUT
			printf("\r\n send body len \r\n");
			printf("\r\n ReadedRecord = %d \r\n",ReadedRecord);
#endif

			unsigned char ReadIndex;
			if(LatestRecord >= ReadedRecord)
			{
				ReadIndex = LatestRecord-ReadedRecord;

			}
			else
			{
				ReadIndex = RecordCounter-(ReadedRecord-LatestRecord);
			}
			//ReadIndex = 
			ReadRecord(ReadIndex,&Record);
			t = Record.tag_time;

			char * Direction;

			//0 up,1 down 2 in 3 out
			if (Record.code == 0)
			{
				Direction = "上";
			}
			if (Record.code == 1)
			{
				Direction = "下";
			}
			if (Record.code == 2)
			{
				Direction = "进";
			}
			if (Record.code == 3)
			{
				Direction = "出";
			}
			//sprintf(CharRecord,"%03d %d-%02d-%02d %02d:%02d:%02d Code:%d. ID:%d. Volt:%d mV<br>\r\n",ReadedRecord,year(t) ,month(t),day(t),hour(t),minute(t),second(t),Record.code,Record.ID,Record.volt);
			sprintf(CharRecord,"%03d %d-%02d-%02d %02d:%02d:%02d ID:%d. %s %s %dmV<br>\r\n",ReadedRecord,year(t) ,month(t),day(t),hour(t),minute(t),second(t),Record.ID,NameList[Record.ID],Direction,Record.volt);


			CharRecordLen = CharLength(CharRecord);
			WIFI_SERIAL.print(F("AT+CIPSEND=1,"));
			WIFI_SERIAL.print(CharRecordLen);
			WIFI_SERIAL.print(F("\r\n"));

			WiFiNextStep = STEP_SEND_HTTP_BODY;
			ActiveTime = SecondsSinceStart;
		}
		break;


	case STEP_SEND_HTTP_BODY:
		if (CheckResponse(GetData,">"))
		{
#ifdef DEGBUG_OUTPUT
			printf("\r\n send body\r\n");
#endif

			for(unsigned char i = 0; i<CharRecordLen ; i++)
			{
				WIFI_SERIAL.print(CharRecord[i]);
			}

			if (ReadedRecord<RecordShoudRead-1)
			{
				ReadedRecord++;
				WiFiNextStep = STEP_SEND_HTTP_BODY_LEN;
			}
			else
			{
				WiFiNextStep = STEP_SEND_HTTP_END_LEN;
			}
			ActiveTime = SecondsSinceStart;
		}
		break;




	case STEP_SEND_HTTP_END_LEN:
		if (CheckResponse(GetData,"SEND OK"))
		{
#ifdef DEGBUG_OUTPUT
			printf("\r\n send end len \r\n");
#endif

			WIFI_SERIAL.print(F("AT+CIPSEND=1,"));
			WIFI_SERIAL.print(sizeof(HttpResponseEnd));
			WIFI_SERIAL.print(F("\r\n"));

			WiFiNextStep = STEP_SEND_HTTP_END;
			ActiveTime = SecondsSinceStart;
		}
		break;


	case STEP_SEND_HTTP_END:
		if (CheckResponse(GetData,">"))
		{
#ifdef DEGBUG_OUTPUT
			printf("\r\n send END \r\n");
#endif

			for(unsigned char i = 0; i<sizeof(HttpResponseEnd) ; i++)
			{
				WIFI_SERIAL.print(HttpResponseEnd[i]);
			}
			WiFiNextStep = STEP_HTTP_CLOSE;
			ActiveTime = SecondsSinceStart;
		}
		break;


	case STEP_HTTP_CLOSE:
		if (CheckResponse(GetData,"SEND OK"))
		{

#ifdef DEGBUG_OUTPUT
			printf("\r\n close!!! \r\n");
#endif
			WIFI_SERIAL.print(F("AT+CIPCLOSE=1\r\n"));
			WiFiNextStep = STEP_WAIT_REQUEST;
			TimeOut = 0;
		}
		break;

	case STEP_WAIT_CLOSED:
		if (CheckResponse(GetData,"OK"))
		{

#ifdef DEGBUG_OUTPUT
			printf("\r\n close!!! \r\n");
#endif
			
			WiFiNextStep = STEP_WAIT_REQUEST;
			TimeOut = 0;
		}
		break;

		





	}
}


unsigned char CharLength(char * MyChar)
{
	unsigned char i =0;
	while (MyChar[i] != 0)
	{
		i++;
	}
	return i;
}







#define ESP8266_STATION 0x01
#define ESP8266_SOFTAP 0x02

#define ESP8266_TCP 1
#define ESP8266_UDP 0

//#define ESP8266_OK 1
//#define ESP8266_READY 2
//#define ESP8266_FAIL 3
//#define ESP8266_NOCHANGE 4
//#define ESP8266_LINKED 5
//#define ESP8266_UNLINK 6

typedef enum
{
	ESP8266_NONE,
	ESP8266_OK,
	ESP8266_READY,
	ESP8266_FAIL,
	ESP8266_NOCHANGE,
	ESP8266_LINKED,
	ESP8266_UNLINK,
} eWiFiRespon;

int len;

bool SendMail()
{
	tRecord MailRecord;
	ReadRecord(CurrentMailMemIndex,&MailRecord);

	if (!_esp8266_connect_SMPT2GO()) return false; //Establish TCP connection with SMPT2GO
	///*LOG IN with your SMPT2GO approved mail ID*/
	///*Visit the page https://www.smtp2go.com/ and sign up using any Gmail ID
	//* Once you gmail ID is SMPT2GO approved convert your mail ID and password in 64 base format
	//* visit https://www.base64encode.org/ for converting 64 base format online
	//* FORMAT -> _esp8266_login_mail("mailID in base 64","Password in base 64");
	//* This program uses the ID-> aswinthcd@gmail.com and password -> circuitdigest as an example
	//*/
	if (!_esp8266_login_mail("YXN3aW50aGNkQGdtYWlsLmNvbQ==","Y2lyY3VpdGRpZ2VzdA==")) return false;
	///*End of Login*/


	if (!_esp8266_mail_sendID("Door@gmail.com")) return false; //The sender mail ID
	if (!_esp8266_mail_recID("fryefryefrye@foxmail.com")) return false; //The Receiver mail ID
	if (!_esp8266_start_mail()) return false;


	t = MailRecord.tag_time;
	char * Direction;

	//0 up,1 down 2 in 3 out
	if (MailRecord.code == 0)
	{
		Direction = "上";
	}
	if (MailRecord.code == 1)
	{
		Direction = "下";
	}
	if (MailRecord.code == 2)
	{
		Direction = "进";
	}
	if (MailRecord.code == 3)
	{
		Direction = "出";
	}

	//sprintf(CharRecord,"ID:%d Coming. %02d:%02d:%02d",MailRecord.ID,hour(t),minute(t),second(t));
	sprintf(CharRecord,"ID:%d. %s %s %02d:%02d:%02d",MailRecord.ID,NameList[MailRecord.ID],Direction,hour(t),minute(t),second(t));

	if (!_esp8266_mail_subject(CharRecord)) return false; //Enter the subject of your mail

	t = MailRecord.tag_time;
	sprintf(CharRecord,"%d-%02d-%02d %02d:%02d:%02d Code:%d. ID:%d. Volt:%d mV\r\n",
		year(t) ,month(t),day(t),hour(t),minute(t),second(t),MailRecord.code,MailRecord.ID,MailRecord.volt);
	if (!_esp8266_mail_body(CharRecord)) return false; //Enter the body of your mail   


	if (!_esp8266_End_mail()) return false;
	if (!_esp8266_disconnect_SMPT2GO()) return false;

	return true;

}



//**Function to send one byte of date to UART**//
void _esp8266_putch( char bt)  
{
	WIFI_SERIAL.print(bt);
}
//_____________End of function________________//



//**Function to get one byte of date from UART**//
bool _esp8266_getch(char * RetData)   
{
	unsigned long RecvStartTime = SecondsSinceStart;
	while (1)
	{
		NonStopTask();
		if (WIFI_SERIAL.available() > 0)
		{
			*RetData = WIFI_SERIAL.read();


			unsigned char Para;
			if (CheckParameter(*RetData,"GET /",&Para))
			{
				if((Para >= '0')&&(Para <= '9'))
				{
#ifdef DEGBUG_OUTPUT
					printf("\r\n Got ID online from wifi \r\n");
#endif
					RfidOnLine(Para-'0',false);
				}
			}
#ifdef DEGBUG_OUTPUT
			Serial.write(*RetData);
#endif
			return true;
		}
		if (SecondsSinceStart - RecvStartTime > MAIL_TIME_OUT)
		{
			return false;
		}
	}
}
//_____________End of function________________//




//**Function to convert string to byte**//
void ESP8266_send_string(char* st_pt)
{
	while(*st_pt) //if there is a char
		_esp8266_putch(*st_pt++); //process it as a byte data
}
//___________End of function______________//
//**End of modified Codes**//



//**Function to configure soft_AP**//
//char esp8266_config_softAP(char* softssid, char* softpass) {
//	_esp8266_print("AT+CWSAP=\"");
//	_esp8266_print_nc(softssid);
//	_esp8266_print("\",\"");
//	_esp8266_print_nc(softpass);
//	_esp8266_print("\",5,3\r\n");
//	return _esp8266_waitResponse();
//}
//___________End of function______________//




//**Function to stations IP/MAC**//
//void esp8266_get_stationIP()
//{
//	char rex;
//	ESP8266_send_string("AT+CWLIF\r\n");
//	Lcd_Set_Cursor(1,1);
//	Lcd_Print_String("IP:");
//	do
//	{
//		rex = _esp8266_getch() ;
//		Lcd_Print_Char(rex);
//	}while(rex!=',');
//	Lcd_Set_Cursor(2,1);
//	Lcd_Print_String("MAC:");
//	do
//	{
//		rex = _esp8266_getch() ;
//		Lcd_Print_Char(rex);
//	}while(rex!='O');
//}
//___________End of function______________//



//**Function to enable multiple connections**//
bool _esp8266_enale_MUX()
{
	_esp8266_print("AT+CIPMUX=1\r\n"); //Enable Multiple Connections
	if (!_esp8266_waitResponse() )return false;


	return true;
}
//___________End of function______________//


//**Function to create server on Port 80**//
bool _esp8266_create_server()
{
	_esp8266_print("AT+CIPSERVER=1,80\r\n"); //Enable Server on port 80
	if (!_esp8266_waitResponse())return false; 


	return true;
}
//___________End of function______________//



/*Enter into Start typing the mail*/
bool _esp8266_start_mail()
{
	_esp8266_print("AT+CIPSEND=4,6\r\n");
	if (!_esp8266_waitFor("OK\r\n>")) return false;


	_esp8266_print("DATA\r\n");
	if (!_esp8266_waitResponse())return false;

	return true;
}
/*Entered into the typing mode*/


/*End the Mail using a "."*/
bool _esp8266_End_mail()
{
	_esp8266_print("AT+CIPSEND=4,3\r\n");
	if (!_esp8266_waitFor("OK\r\n>"))return false;
	_esp8266_print(".\r\n");
	if (!_esp8266_waitResponse())return false;


	return true;
}
/*End of mail*/

/*Quit Connection from SMPT server*/
bool _esp8266_disconnect_SMPT2GO()
{
	_esp8266_print("AT+CIPSEND=4,6\r\n");
	if (!_esp8266_waitFor("OK\r\n>"))return false;
	_esp8266_print("QUIT\r\n");	if (!_esp8266_waitFor("CLOSED"))return false;


	return true;
}
/*Disconnected*/




/*Connect to SMPT2GO server*/
bool _esp8266_connect_SMPT2GO()
{
	_esp8266_print("AT+CIPSTART=4,\"TCP\",\"mail.smtp2go.com\",2525\r\n");
	if (!_esp8266_waitFor("OK"))return false;
	_esp8266_print("AT+CIPSEND=4,20\r\n");
	if (!_esp8266_waitFor("OK\r\n>"))return false;
	_esp8266_print("EHLO 192.168.1.123\r\n");
	if (!_esp8266_waitResponse())return false;
	_esp8266_print("AT+CIPSEND=4,12\r\n");
	if (!_esp8266_waitFor("OK\r\n>"))return false;
	_esp8266_print("AUTH LOGIN\r\n");
	if (!_esp8266_waitResponse())return false;


	return true;
}
/*connected to Server*/

/*LOG IN with your SMPT2GO approved mail ID*/
/*Visit the page https://www.smtp2go.com/ and sign up using any Gmail ID
* Once you gmail ID is SMPT2GO approved convert your mail ID and password in 64 base format
* visit https://www.base64encode.org/ for converting 64 base format online
* FORMAT -> _esp8266_login_mail("mailID in base 64","Password in base 64");
* This program uses the ID-> aswinthcd@gmail.com and password -> circuitdigest as an example
*/
bool _esp8266_login_mail( char* mail_ID,  char* mail_Pas) {
	len = CharLength(mail_ID);
	len+= 2;
	char l2 = len%10;
	char l1 = (len/10)%10;

	_esp8266_print("AT+CIPSEND=4,");
	if ((l1+'0')>'0')
		_esp8266_putch(l1+'0');
	_esp8266_putch(l2+'0');
	_esp8266_print("\r\n");
	//if (!_esp8266_waitResponse();
	if (!_esp8266_waitFor("OK\r\n>"))return false;

	_esp8266_print_nc(mail_ID);
	_esp8266_print("\r\n");
	if (!_esp8266_waitResponse())return false;

	len = CharLength(mail_Pas);
	len+= 2;
	l2 = len%10;
	l1 = (len/10)%10;

	_esp8266_print("AT+CIPSEND=4,");
	if ((l1+'0')>'0')
		_esp8266_putch(l1+'0');
	_esp8266_putch(l2+'0');
	_esp8266_print("\r\n");
	//if (!_esp8266_waitResponse();
	if (!_esp8266_waitFor("OK\r\n>"))return false;


	_esp8266_print_nc(mail_Pas);
	_esp8266_print("\r\n");
	if (!_esp8266_waitResponse())return false;


	return true;
}
/*End of Login*/


bool _esp8266_mail_sendID( char* send_ID)
{
	len = CharLength(send_ID);
	len+= 14;
	char l2 = len%10;
	char l1 = (len/10)%10;

	_esp8266_print("AT+CIPSEND=4,");
	if ((l1+'0')>'0')
		_esp8266_putch(l1+'0');
	_esp8266_putch(l2+'0');
	_esp8266_print("\r\n");
	if (!_esp8266_waitFor("OK\r\n>"))return false;

	_esp8266_print("MAIL FROM:<");
	_esp8266_print_nc(send_ID);
	_esp8266_print(">\r\n");
	if (!_esp8266_waitResponse())return false;   


	return true;
} 


bool _esp8266_mail_recID( char* rec_ID)
{
	len = CharLength(rec_ID);
	len+= 12;
	char l2 = len%10;
	char l1 = (len/10)%10;

	_esp8266_print("AT+CIPSEND=4,");
	if ((l1+'0')>'0')
		_esp8266_putch(l1+'0');
	_esp8266_putch(l2+'0');
	_esp8266_print("\r\n");
	//if (!_esp8266_waitResponse();
	if (!_esp8266_waitFor("OK\r\n>"))return false;

	_esp8266_print("RCPT To:<");
	_esp8266_print_nc(rec_ID);
	_esp8266_print(">\r\n");
	if (!_esp8266_waitResponse())return false;   


	return true;
} 

bool _esp8266_mail_subject( char* subject)
{
	len = CharLength(subject);
	len+= 10;
	char l2 = len%10;
	char l1 = (len/10)%10;

	_esp8266_print("AT+CIPSEND=4,");
	if ((l1+'0')>'0')
		_esp8266_putch(l1+'0');
	_esp8266_putch(l2+'0');
	_esp8266_print("\r\n");
	if (!_esp8266_waitFor("OK\r\n>"))return false;

	_esp8266_print("Subject:");
	_esp8266_print_nc(subject);
	_esp8266_print("\r\n");
	if (!_esp8266_waitResponse())return false;   


	return true;
} 



bool _esp8266_mail_body( char* body)
{
	len = CharLength(body);
	len+= 2;
	char l2 = len%10;
	char l1 = (len/10)%10;

	_esp8266_print("AT+CIPSEND=4,");
	if ((l1+'0')>'0')
		_esp8266_putch(l1+'0');
	_esp8266_putch(l2+'0');
	_esp8266_print("\r\n");
	if (!_esp8266_waitFor("OK\r\n>"))return false;

	_esp8266_print_nc(body);
	_esp8266_print("\r\n");
	if (!_esp8266_waitResponse())return false;   


	return true;
} 




/**
* Check if the module is started
*
* This sends the `AT` command to the ESP and waits until it gets a response.
*
* @return true if the module is started, false if something went wrong
*/
//bool esp8266_isStarted(void) {
//	_esp8266_print("AT\r\n");
//	return (_esp8266_waitResponse() == ESP8266_OK);
//}

/**
* Restart the module
*
* This sends the `AT+RST` command to the ESP and waits until there is a
* response.
*
* @return true iff the module restarted properly
*/
//bool esp8266_restart(void) {
//	_esp8266_print("AT+RST\r\n");
//	if (_esp8266_waitResponse() != ESP8266_OK) {
//		return false;
//	}
//	return (_esp8266_waitResponse() == ESP8266_READY);
//}

/**
* Enable / disable command echoing.
*
* Enabling this is useful for debugging: one could sniff the TX line from the
* ESP8266 with his computer and thus receive both commands and responses.
*
* This sends the ATE command to the ESP module.
*
* @param echo whether to enable command echoing or not
*/
void esp8266_echoCmds(bool echo) {
	_esp8266_print("ATE");
	if (echo) {
		_esp8266_putch('1');
	} else {
		_esp8266_putch('0');
	}
	_esp8266_print("\r\n");
	_esp8266_waitFor("OK");
}

/**
* Set the WiFi mode.
*
* ESP8266_STATION : Station mode
* ESP8266_SOFTAP : Access point mode
*
* This sends the AT+CWMODE command to the ESP module.
*
* @param mode an ORed bitmask of ESP8266_STATION and ESP8266_SOFTAP
*/
void esp8266_mode( char mode) {
	_esp8266_print("AT+CWMODE=");
	_esp8266_putch(mode + '0');
	_esp8266_print("\r\n");
	_esp8266_waitResponse();
}

/**
* Connect to an access point.
*
* This sends the AT+CWJAP command to the ESP module.
*
* @param ssid The SSID to connect to
* @param pass The password of the network
* @return an ESP status code, normally either ESP8266_OK or ESP8266_FAIL
*/
void esp8266_connect( char* ssid,  char* pass) {
	_esp8266_print("AT+CWJAP=\"");
	_esp8266_print_nc(ssid);
	_esp8266_print("\",\"");
	_esp8266_print_nc(pass);
	_esp8266_print("\"\r\n");
	_esp8266_waitResponse();
}

/**
* Disconnect from the access point.
*
* This sends the AT+CWQAP command to the ESP module.
*/
//void esp8266_disconnect(void) {
//	_esp8266_print("AT+CWQAP\r\n");
//	_esp8266_waitFor("OK");
//}

/**
* Store the current local IPv4 address.
*
* This sends the AT+CIFSR command to the ESP module.
*
* The result will not be stored as a string but byte by byte. For example, for
* the IP 192.168.0.1, the value of store_in will be: {0xc0, 0xa8, 0x00, 0x01}.
*
* @param store_in a pointer to an array of the type  char[4]; this
* array will be filled with the local IP.
*/
//void esp8266_ip( char* store_in) {
//	_esp8266_print("AT+CIFSR\r\n");
//	char received;
//	do {
//		received = _esp8266_getch();
//	} while (received < '0' || received > '9');
//	for ( char i = 0; i < 4; i++) {
//		store_in[i] = 0;
//		do {
//			store_in[i] = 10 * store_in[i] + received - '0';
//			received = _esp8266_getch();
//		} while (received >= '0' && received <= '9');
//		received = _esp8266_getch();
//	}
//	_esp8266_waitFor("OK");
//}

/**
* Open a TCP or UDP connection.
*
* This sends the AT+CIPSTART command to the ESP module.
*
* @param protocol Either ESP8266_TCP or ESP8266_UDP
* @param ip The IP or hostname to connect to; as a string
* @param port The port to connect to
*
* @return true iff the connection is opened after this.
*/
//bool esp8266_start( char protocol, char* ip,  char port) {
//	_esp8266_print("AT+CIPSTART=\"");
//	if (protocol == ESP8266_TCP) {
//		_esp8266_print("TCP");
//	} else {
//		_esp8266_print("UDP");
//	}
//	_esp8266_print("\",\"");
//	_esp8266_print_nc(ip);
//	_esp8266_print("\",");
//	char port_str[5] = "\0\0\0\0";
//	sprintf(port_str, "%u", port);
//	_esp8266_print(port_str);
//	_esp8266_print("\r\n");
//	if (_esp8266_waitResponse() != ESP8266_OK) {
//		return 0;
//	}
//	if (_esp8266_waitResponse() != ESP8266_LINKED) {
//		return 0;
//	}
//	return 1;
//}

// Send data (AT+CIPSEND)
/**
* Send data over a connection.
*
* This sends the AT+CIPSEND command to the ESP module.
*
* @param data The data to send
*
* @return true iff the data was sent correctly.
*/
//bool esp8266_send( char* data) {
//	char length_str[6] = "\0\0\0\0\0";
//	sprintf(length_str, "%u", CharLength(data));
//	_esp8266_print("AT+CIPSEND=");
//	_esp8266_print_nc(length_str);
//	_esp8266_print("\r\n");
//	while (_esp8266_getch() != '>');
//	_esp8266_print_nc(data);
//	if (_esp8266_waitResponse() == ESP8266_OK) {
//		return 1;
//	}
//	return 0;
//}

/**
* Read a string of data that is sent to the ESP8266.
*
* This waits for a +IPD line from the module. If more bytes than the maximum
* are received, the remaining bytes will be discarded.
*
* @param store_in a pointer to a character array to store the data in
* @param max_length maximum amount of bytes to read in
* @param discard_headers if set to true, we will skip until the first \r\n\r\n,
* for HTTP this means skipping the headers.
*/
//void esp8266_receive( char* store_in, uint16_t max_length, bool discard_headers) {
//	_esp8266_waitFor("+IPD,");
//	uint16_t length = 0;
//	char received = _esp8266_getch();
//	do {
//		length = length * 10 + received - '0';
//		received = _esp8266_getch();
//	} while (received >= '0' && received <= '9');
//
//	if (discard_headers) {
//		length -= _esp8266_waitFor("\r\n\r\n");
//	}
//
//	if (length < max_length) {
//		max_length = length;
//	}
//
//	/*sprintf(store_in, "%u,%u:%c%c", length, max_length, _esp8266_getch(), _esp8266_getch());
//	return;*/
//
//	uint16_t i;
//	for (i = 0; i < max_length; i++) {
//		store_in[i] = _esp8266_getch();
//	}
//	store_in[i] = 0;
//	for (; i < length; i++) {
//		_esp8266_getch();
//	}
//	_esp8266_waitFor("OK");
//}

/**
* Output a string to the ESP module.
*
* This is a function for internal use only.
*
* @param ptr A pointer to the string to send.
*/
void _esp8266_print(const char* ptr) {
	while (*ptr != 0) {
		_esp8266_putch(*ptr++);
	}
}

void _esp8266_print_nc(char* ptr) {
	while (*ptr != 0) {
		_esp8266_putch(*ptr++);
	}
}


/**
* Wait until we found a string on the input.
*
* Careful: this will read everything until that string (even if it's never
* found). You may lose important data.
*
* @param string
*
* @return the number of characters read
*/
bool _esp8266_waitFor(const char *string) {
	char so_far = 0;
	char received;
	int counter = 0;
	do {
		//received = _esp8266_getch();
		if (!_esp8266_getch(&received))
		{
			return false;
		}
		counter++;
		if (received == string[so_far]) {
			so_far++;
		} else {
			so_far = 0;
		}
	} while (string[so_far] != 0);
	return true;
}

/**
* Wait until we received the ESP is done and sends its response.
*
* This is a function for internal use only.
*
* Currently the following responses are implemented:
*  * OK
*  * ready
*  * FAIL
*  * no change
*  * Linked
*  * Unlink
*
* Not implemented yet:
*  * DNS fail (or something like that)
*
* @return a constant from esp8266.h describing the status response.
*/
bool _esp8266_waitResponse(void) {
	char so_far[6] = {0,0,0,0,0,0};
	const char lengths[6] = {2,5,4,9,6,6};
	const char* strings[6] = {"OK", "ready", "FAIL", "no change", "Linked", "Unlink"};
	const char responses[6] = {ESP8266_OK, ESP8266_READY, ESP8266_FAIL, ESP8266_NOCHANGE, ESP8266_LINKED, ESP8266_UNLINK};
	char received;
	char response;
	bool continue_loop = true;
	while (continue_loop) {
		//received = _esp8266_getch();
		if (!_esp8266_getch(&received))
		{
			return false;
		}

		for ( char i = 0; i < 6; i++) {
			if (strings[i][so_far[i]] == received) {
				so_far[i]++;
				if (so_far[i] == lengths[i]) {
					response = responses[i];
					continue_loop = false;
				}
			} else {
				so_far[i] = 0;
			}
		}
	}
	return true;
}





void PushMailQueue(unsigned char MemIndex)
{
	if(QueueNumber < MAIL_QUEUE_NUMBER)
	{
		QueueNumber ++;
	}
#if defined(DEGBUG_OUTPUT)	
	printf("Add queue. Then size %d. MemIndex %d \r\n",QueueNumber,MemIndex);
#else
#endif	
	for(int i = 0; i < MAIL_QUEUE_NUMBER-1;i++ )
	{
		MailQueue[MAIL_QUEUE_NUMBER-i-1] = MailQueue[MAIL_QUEUE_NUMBER-i-2]; 
	}

	MailQueue[0] = MemIndex; 
}

unsigned char PopMailQueue()
{

	if(QueueNumber != 0)
	{
		QueueNumber--;
	}

#if defined(DEGBUG_OUTPUT)	
	printf("Pop queue. Then size %d. MemIndex %d \r\n",QueueNumber,MailQueue[QueueNumber]);
#else
#endif

	return MailQueue[QueueNumber];
}


void RfidOnLine(unsigned char ID,bool OnlineAtUp)
{

	if (LastOnlineTime[ID] == 0)
	{
#if defined(DEGBUG_OUTPUT)	
		printf("Direction, first  ID = %d,Record index = %d  up = %d \r\n",ID,NextRecord,OnlineAtUp);
#endif
		LastOnlineTime[ID] = SecondsSinceStart;
		LastOnlineStoreIndex[ID] = NextRecord;

		//add record
		Record.tag_time=SecondsSinceStart+Start1970OffSet;
		Record.code= (OnlineAtUp?0:1);//0 up,1 down 2 in 3 out
		Record.ID =  ID;
		Record.volt =(OnlineAtUp?Volt:0) ;
		InsertRecord(&Record); 
	}
	else
	{
#if defined(DEGBUG_OUTPUT)	
		printf("Direction, 2ed  ID = %d,Record index = %d  up = %d \r\n",ID,NextRecord,OnlineAtUp);

		printf("PushMailQueue; Direction = %d,   ID = %d, \r\n",(OnlineAtUp?2:3),ID);
#endif



		ReadRecord(LastOnlineStoreIndex[ID],&Record);

		if (Record.code == (OnlineAtUp?0:1))
		{
#if defined(DEGBUG_OUTPUT)	
			printf("Direction, duplicate, update time only \r\n",ID,NextRecord,OnlineAtUp);
#endif
			LastOnlineTime[ID] = SecondsSinceStart;
		} 
		else
		{
			LastOnlineTime[ID] = 0;
			Record.code = (OnlineAtUp?2:3);//0 up,1 down 2 in 3 out
			if (OnlineAtUp)
			{
				Record.volt =Volt;
			}
			WriteRecord(LastOnlineStoreIndex[ID],&Record);
			PushMailQueue(LastOnlineStoreIndex[ID]);
		}




	}
}

void InOUtCleanTask()
{
	for(unsigned char i = 0;i < RFID_NUMBER;i++)
	{
		if (( SecondsSinceStart - LastOnlineTime[i] > IN_OUT_CLEAN )&&(LastOnlineTime[i] != 0))
		{
			LastOnlineTime[i] = 0;
#if defined(DEGBUG_OUTPUT)	
			printf("PushMailQueue; No Direction,   ID = %d, \r\n",i);
#endif
			PushMailQueue(LastOnlineStoreIndex[i]);
		}
	}
}
