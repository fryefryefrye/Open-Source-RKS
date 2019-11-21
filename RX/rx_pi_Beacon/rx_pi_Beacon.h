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
#define IN_OUT_CLEAN			180        //s
#define WIFI_SERIAL Serial3

#define COMPENSATION_MS_IN_ONE_SECOND -9
#define COMPENSATION_SECOND_IN 1452
#define COMPENSATION_SECOND_DIRECTION --

#else
#define WIFI_SERIAL Serial
#define TIME_OUT_CLOSE_DOOR		15        //s
#define IN_OUT_CLEAN			180        //s

#define COMPENSATION_MS_IN_ONE_SECOND 0 //+9;-10;...    2560:-9
#define COMPENSATION_SECOND_IN 5775//05775 //second			2560:1452
#define COMPENSATION_SECOND_DIRECTION -- //  ++;--
#endif



/*****************************************************/



#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 0xFF;
tKeyLessData KeyLessData;





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
unsigned long LastKeyGetTime = 0;
unsigned long RandDelay = 0;
unsigned long LastGetTime[RFID_NUMBER] = {0};
bool RfidOnline[RFID_NUMBER] = {false};
unsigned long SecondsSinceStart;

unsigned long CurrTime = 0;
unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
/**********************************************************/

/*************************Output*******************/
#define DOOR 10
bool DoorLastState = false;


#define BUZZ 9
bool BuzzOutputHigh;
bool AlarmOn;
/**********************************************************/


void Door_task();
void nRF_task();
void Buzz_task();
void WiFi_task();
void SecondsSinceStartTask();
void SendBeacon();
void OnKeyPress();

void SendKeyLessData();
void OnWiFiData(unsigned char data);
unsigned char CharLength(char * MyChar);
void OnSecond();
void NonStopTask();
bool InitEsp();
void _esp8266_putch( char);
bool _esp8266_getch(char * RetData);   
void _esp8266_print(const char*);
void _esp8266_print_nc(char*);
bool _esp8266_waitFor(const char *);
unsigned int WaitForTimeOut = 30;

bool WifiOK = false;




void setup()
{

#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_WiFi_ID_Read"));
	printf_begin();
#endif
	WIFI_SERIAL.begin(115200);

#ifdef DEGBUG_OUTPUT
	printf("InitEsp ConnectAp \r\n");
#endif
	InitEsp();
#ifdef DEGBUG_OUTPUT
	printf("InitEsp finished \r\n");
#endif

	KeyLessData.DataType = 14;

	pinMode(DOOR, OUTPUT);

	KeyLessData.Mac[0] = 2;


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

	wdt_enable(WDTO_2S);
}

void loop()
{
	WiFi_task();
	NonStopTask();
} // Loop

void NonStopTask()
{
	nRF_task();
	SecondsSinceStartTask();
	Buzz_task();
}



void WiFi_task()
{
	if (!WifiOK)
	{
		//if (_esp8266_waitFor("WIFI GOT IP"))
		if (_esp8266_waitFor("GOT"))
		{

#ifdef DEGBUG_OUTPUT
			printf("Wifi is connected \r\n");
#endif
			WIFI_SERIAL.print(F("AT+CIPSTART=\"UDP\","));
			WIFI_SERIAL.print(F("\"192.168.0.17\""));
			WIFI_SERIAL.print(F(",5050\r\n"));
			WifiOK = true;
			if (!_esp8266_waitFor("OK")) return;

			//WIFI_SERIAL.print(F("AT+CIPSTAMAC?\r\n"));
			//if (!_esp8266_waitFor("OK")) return;

			WaitForTimeOut = 10;
		}
		else
		{
#ifdef DEGBUG_OUTPUT
			printf("wait IP time out, reset wifi \r\n");
#endif
			WIFI_SERIAL.print(F("AT+RST\r\n"));
			WaitForTimeOut = 30;
		}
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
			Door_task();

			KeyLessData.KeyLessData = GotData[0];
			KeyLessData.KeyLessData = KeyLessData.KeyLessData<<8;
			KeyLessData.KeyLessData = KeyLessData.KeyLessData + GotData[1];
			KeyLessData.KeyLessData = KeyLessData.KeyLessData<<8;
			KeyLessData.KeyLessData = KeyLessData.KeyLessData + GotData[2];
			KeyLessData.KeyLessData = KeyLessData.KeyLessData<<8;
			KeyLessData.KeyLessData = KeyLessData.KeyLessData + GotData[3];

			KeyLessData.DataType = 14;
			KeyLessData.Triger = true;
			SendKeyLessData();
		}
		else if (GotData[0] == 2)
		{
			if (millis() - LastKeyGetTime > 500)
			{
				LastKeyGetTime = millis();
				OnKeyPress();
			}
		}

		PackageCounter ++;








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
	if (abs(CurrTime - LastChangeCHTime>RandDelay))//RF_HOP
	{

		CurrCH++;
		if (CurrCH>2)
		{
			CurrCH = 0;
		}
		LastChangeCHTime = millis();


		RandDelay = random(3000);

		radio.stopListening();
		radio.setChannel(HopCH[CurrCH]);

		delay(10);
		SendBeacon();
		delay(10);

		radio.startListening();
	}








}

void OnKeyPress()
{
#if defined(DEGBUG_OUTPUT)	
	printf("manual off for 100ms  \r\n");
#else
#endif
	digitalWrite(DOOR, LOW);
	DoorLastState = false;
	delay(100);
}

void SendBeacon()
{
	GotData[0] = 1;
	GotData[1] = 0;
	GotData[2] = 0;
	GotData[3] = 0;
	radio.write(GotData, 4);
}

void SendKeyLessData()
{
	unsigned char data;
	unsigned char Len = sizeof(tKeyLessData);

	if (WifiOK)
	{

		WIFI_SERIAL.print(F("AT+CIPSEND="));
		WIFI_SERIAL.print(Len);
		WIFI_SERIAL.print(F("\r\n"));

		//if (!_esp8266_waitFor(">")) return;
		//printf("Sending data \r\n");
		delay(10);

		for(unsigned char i = 0; i<(sizeof(tKeyLessData)) ; i++)
		{
			data = *(((char*)(&KeyLessData))+i);
			WIFI_SERIAL.write(data);
		}

		if (!_esp8266_waitFor("OK")) return;
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



void OnSecond()
{
#ifdef DEGBUG_OUTPUT
	//printf("OnSecond %d \r\n",SecondsSinceStart);
#endif
	wdt_reset();
	Door_task();

	KeyLessData.DataType = 14;
	KeyLessData.Triger = false;
	SendKeyLessData();
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
	}
}



bool InitEsp()
{
	WIFI_SERIAL.print(F("+++"));
	delay(100);
	WIFI_SERIAL.print(F("AT+RST\r\n"));
	//if (!_esp8266_waitFor("GOT IP\r\n"))return false;
	//WIFI_SERIAL.print(F("AT+CWMODE=1\r\n"));
	//if (!_esp8266_waitFor("OK"))return false;
	//WIFI_SERIAL.print(F("AT+CIPSTART=\"UDP\","));
	//WIFI_SERIAL.print(F("\"192.168.0.17\""));
	//WIFI_SERIAL.print(F(",5050\r\n"));
	//if (!_esp8266_waitFor("CONNECT"))return false;
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



//**Function to send one byte of date to UART**//
void _esp8266_putch( char bt)  
{
	WIFI_SERIAL.print(bt);

#ifdef DEGBUG_OUTPUT
	Serial.print(bt);
#endif

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
#ifdef DEGBUG_OUTPUT
			Serial.write(*RetData);
#endif
			return true;
		}
		if (SecondsSinceStart - RecvStartTime > WaitForTimeOut)
		{
			return false;
		}
	}
}


void ESP8266_send_string(char* st_pt)
{
	while(*st_pt) //if there is a char
		_esp8266_putch(*st_pt++); //process it as a byte data
}


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
