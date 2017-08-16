/*************************user modify settings****************************/
byte addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char  HopCH[3] = {105,76,108};//Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define TIME_OUT_CLOSE_DOOR 5000        //ms
#define DATA_LENGTH 4                    //use fixed data length 1-32
#define BUZZON 1000                //set lenght of the buzz
#define BUZZOFF 30000            //set interval of the buzz
#define RFID_NUMBER 8


#define DEGBUG_OUTPUT
#define WIFI_SERIAL Serial3
#define TIME_SYNC_TIME_OUT 86400
/*****************************************************/









/*************************include*******************/
#include <SPI.h>
#include "RF24.h"
#include <printf.h>
#include "TimeLib.h"
#include <EEPROM.h>
/*****************************************************/



/*************************nRF Module*******************/
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long LastChangeCHTime = 0;
unsigned long LastGetTime[RFID_NUMBER] = {0};
bool RfidOnline[RFID_NUMBER] = {false};

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
	unsigned char code = 0;
	unsigned char ID = 0;
	unsigned int volt = 0;
};
tRecord Record;
unsigned char RecordLength = sizeof(Record);
unsigned char RecordCounter = 16;//1024/RecordLength;
unsigned char NextRecord = 0;
char CharRecord[100];
unsigned char CharRecordLen;
unsigned char ReadedRecord = 0;
unsigned char LatestRecord = 0;

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

const char HttpResponseHead[]   ="HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html lang=\"zh-cn\">\r\n<head>\r\n<title>HTML</title>\r\n</head>\r\n<body>\r\n";
char HttpResponseEnd[] ="</body>\r\n</html>\r\n";

/**********************************************************/




void Door_task();
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




void setup()
{




#ifdef DEGBUG_OUTPUT
	Serial.begin(9600);
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



}

void loop()
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



	if ((!WiFiOK)&&(WiFiNextStep == STEP_WIFI_IDLE))
	{
		ConnectAp();
	}

	if ((WiFiOK)&&(!NtpWorking)&&(!NtpSync))
	{
		printf("WiFi OK, send NTP request \r\n");
		StartNtp();
		NtpWorking = true;
	}

	if ((WiFiOK)&&(NtpSync)&&(!bWebStarted))
	{
		printf("WiFi OK, NtpSync,start web \r\n");
		WebStart();
		bWebStarted = true;
	}


	CheckForInput();
	SecondsSinceStartTask();
	Buzz_task();
} // Loop

void Door_task()
{

	bool DoorShouldOpen = false;
	for(unsigned char i = 0;i < RFID_NUMBER;i++)
	{
		if ((LastGetTime[i]!=0)&&( SecondsSinceStart - LastGetTime[i] < TIME_OUT_CLOSE_DOOR))
		{
			if (!RfidOnline[i])//offline before
			{
				printf("RFID %d online. \r\n",i);
				if (NtpSync)
				{
					Record.tag_time=SecondsSinceStart+Start1970OffSet;
					Record.code= GotData[0];
					Record.ID =  GotData[1];
					Record.volt = Volt;
					InsertRecord(&Record);
				}
			} 
			RfidOnline[i] = true;
			DoorShouldOpen = true;
		}
		else
		{
			if (!RfidOnline[i])//online before
			{	
				RfidOnline[i] = false;
			}
		}
	}

	if (DoorLastState != DoorShouldOpen)//update relay when door status change
	{
		DoorLastState = DoorShouldOpen;
		if(DoorShouldOpen)
		{
			digitalWrite(DOOR, HIGH);
		}
		else
		{
			printf("Close door  \r\n");
			digitalWrite(DOOR, LOW);
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
	printf("Write Record! Index %d. Date Time = %d-%d-%d %d:%d:%d Code:%d. ID:%d. Volt:%d mV\r\n",index,year(t) ,month(t),day(t),hour(t),minute(t),second(t),pRecord->code,pRecord->ID,pRecord->volt);
}

void ReadRecord(unsigned char index,tRecord * pRecord)
{
	unsigned char i;
	for(i=0; i<RecordLength; i++)
	{
		*(((unsigned char *)(pRecord))+i) = EEPROM.read(index*RecordLength+i);
	}

	t = pRecord->tag_time;
	printf("Read Record! Index %03d. Date Time = %d-%02d-%02d %02d:%02d:%02d Code:%d. ID:%d. Volt:%d mV\r\n",index,year(t) ,month(t),day(t),hour(t),minute(t),second(t),pRecord->code,pRecord->ID,pRecord->volt);
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
	printf("Min Record index %03d! Date Time = %d-%02d-%02d %02d:%02d:%02d \r\n",MinIndex,year(t) ,month(t),day(t),hour(t),minute(t),second(t));

	return MinIndex;

}

void OnSecond()
{
	if (SecondsSinceStart%1452 == 0)
	{
		Start1970OffSet--;
	}


	Door_task();

	CheckTimeOut();

}

unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	//unsigned long CurrentMillis = millis();
	//if
	//	(
	//	(CurrentMillis>=LastMillis)
	//	?
	//	((CurrentMillis-LastMillis)> 1000)
	//	:
	//((0xFFFFFFFF-LastMillis)+CurrentMillis>1000)
	//	)
	//{
	//	LastMillis = (CurrentMillis/1000)*1000;


	//unsigned long CurrentMillis = millis()/1000;
	//if(CurrentMillis != LastMillis)
	//{
	//	LastMillis = CurrentMillis;

	unsigned long CurrentMillis = millis();
	if(abs(CurrentMillis-LastMillis)> 1000-9)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;
		OnSecond();

		time_t t = SecondsSinceStart + Start1970OffSet;
		printf("Date Time = %d-%02d-%02d %02d:%02d:%02d \r\n",year(t) ,month(t),day(t),hour(t),minute(t),second(t));
		printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);

	}
}



void ResetWiFi()
{
		WiFiOK = false;
		NtpWorking = false;
		NtpSync = false;
		bWebStarted = false;
		printf("\r\n set all to false, reset wifi \r\n");
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
		printf("\r\nDate Time = %d-%02d-%02d %02d:%02d:%02d \r\n",year(t) ,month(t),day(t),hour(t),minute(t),second(t));
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
				printf("\r\n close when time out \r\n");
				WIFI_SERIAL.print(F("AT+CIPCLOSE=1\r\n"));
				WiFiNextStep = STEP_WAIT_REQUEST;
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
		Serial.write(RetData);
#endif

	}
}

bool CheckResponse(unsigned char GetData,const char * ExpectResponse)
{
	static  char CheckedBytePostion = -1;
	if (GetData == ExpectResponse[CheckedBytePostion+1])
	{
		CheckedBytePostion++;
		//printf("One byte OK = %c ,OkBytePostion = %d\r\n",data,OkBytePostion);
		if(ExpectResponse[CheckedBytePostion+1] == 0)
		{
			//printf("compare to end,OK \r\n",data);
			return true;
		}
	}
	else
	{
		//printf("One byte wrong,go back = %c \r\n",data);
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
		//        //printf("\r\n  STEP_TRUN_OFF_TRANS time out =%d; \r\n",SecondsSinceStart - SentTime);
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
			//printf("\r\n Get para = \"%c\"  value = %d \r\n",Para,Para);
			if (Para == ' ')
			{
				//printf("\r\n request home page , send head len\r\n");
				delay(100);
				WIFI_SERIAL.print(F("AT+CIPSEND=1,"));
				//WIFI_SERIAL.print(strlen_P(HttpResponseHead));
				WIFI_SERIAL.print(sizeof(HttpResponseHead));
				WIFI_SERIAL.print(F("\r\n"));
				WiFiNextStep = STEP_SEND_HTTP_HEAD;

				ActiveTime = SecondsSinceStart;
				TimeOut = 5;
			}
			else
			{
				printf("\r\n close not request home page \r\n");
				WIFI_SERIAL.print(F("AT+CIPCLOSE=1\r\n"));
				WiFiNextStep = STEP_WAIT_REQUEST;
			}
		}

		break;

	case STEP_SEND_HTTP_HEAD:
		if (CheckResponse(GetData,">"))
		{
			printf("\r\n send response head \r\n");
			//for(unsigned char i = 0; i<strlen_P(HttpResponseHead) ; i++)
			for(unsigned char i = 0; i<sizeof(HttpResponseHead) ; i++)
			{
				//WIFI_SERIAL.print(pgm_read_byte_near(HttpResponseHead+i));
				WIFI_SERIAL.print(HttpResponseHead[i]);
			}
			WiFiNextStep = STEP_SEND_HTTP_TITLE_LEN;
			ReadedRecord = 0;
			LatestRecord = (NextRecord>0?NextRecord-1:RecordCounter-1);

			printf("\r\n LatestRecord = %d \r\n",LatestRecord);

			ActiveTime = SecondsSinceStart;
		}
		break;


	case STEP_SEND_HTTP_TITLE_LEN:
		if (CheckResponse(GetData,"SEND OK"))
		{
			printf("\r\n send TITLE len \r\n");
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
			printf("\r\n send HTTP_TITLE\r\n");

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
			printf("\r\n send body len \r\n");

			printf("\r\n ReadedRecord = %d \r\n",ReadedRecord);

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
			sprintf(CharRecord,"%03d %d-%02d-%02d %02d:%02d:%02d Code:%d. ID:%d. Volt:%d mV<br>\r\n",ReadedRecord,year(t) ,month(t),day(t),hour(t),minute(t),second(t),Record.code,Record.ID,Record.volt);
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
			printf("\r\n send body\r\n");

			for(unsigned char i = 0; i<CharRecordLen ; i++)
			{
				WIFI_SERIAL.print(CharRecord[i]);
			}

			if (ReadedRecord<RecordCounter-1)
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
			printf("\r\n send end len \r\n");

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
			printf("\r\n send END \r\n");

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

			printf("\r\n close!!! \r\n");
			WIFI_SERIAL.print(F("AT+CIPCLOSE=1\r\n"));
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



