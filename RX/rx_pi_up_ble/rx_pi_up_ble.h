/*************************user modify settings****************************/
byte addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char  HopCH[3] = {105,76,108};//Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx

#define DATA_LENGTH				4                    //use fixed data length 1-32
#define BUZZON					100                //set lenght of the buzz
#define BUZZOFF					30000            //set interval of the buzz
#define RFID_MAX_NUMBER				8
#define RFID_MAX_TYPE				4

//#define DEBUG_WITHOUT_WIFI
//#define SOFT_SERI

//#if defined SOFT_SERI
#define DEGBUG_OUTPUT
#include <SoftwareSerial.h>
SoftwareSerial SoftSerial(2, 3); //rx ,tx 
#define WIFI_SERIAL SoftSerial
//#else


//#if defined(__AVR_ATmega2560__)	
//#define DEGBUG_OUTPUT
//#define TIME_OUT_CLOSE_DOOR		20        //s
//#define IN_OUT_CLEAN			180        //s
//#define WIFI_SERIAL Serial3
//
//
//#else
//
//#if defined DEBUG_WITHOUT_WIFI//on off for
//
//#define DEGBUG_OUTPUT
//#define WIFI_SERIAL Serial
//#define TIME_OUT_CLOSE_DOOR		15        //s
//#define IN_OUT_CLEAN			180        //s
//
//#else
//
//#define WIFI_SERIAL Serial
//#define TIME_OUT_CLOSE_DOOR		15        //s
//#define IN_OUT_CLEAN			180        //s
//
//#endif
//#endif
//#endif




/*****************************************************/



#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 22;
tBleKeyLessData BleKeyLessData;
// #include "nrf_ble.h"
// tTagData TagData;



/*************************include*******************/
#include <SPI.h>
#include "RF24.h"
#include <printf.h>
#include "TimeLib.h"
#include <EEPROM.h>
#include <avr/wdt.h>



unsigned char TagTimeOut[RFID_MAX_TYPE][RFID_MAX_NUMBER] = {0};
bool RfidOnline[RFID_MAX_TYPE][RFID_MAX_NUMBER] = {false};
unsigned long SecondsSinceStart;


//unsigned char GotData[DATA_LENGTH];
//unsigned long Volt;   //unit: mV,
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



//#if ! defined DEBUG_WITHOUT_WIFI
	WIFI_SERIAL.begin(115200);

//#ifdef DEGBUG_OUTPUT
	Serial.println(F("InitEsp ConnectAp \r\n"));
//#endif
//#endif



//
//#if ! defined DEBUG_WITHOUT_WIFI
	InitEsp();
//#ifdef DEGBUG_OUTPUT
	Serial.println(F("InitEsp finished \r\n"));
//#endif
//#endif




	BleKeyLessData.DataType = 21;

	pinMode(DOOR, OUTPUT);

	BleKeyLessData.Mac[0] = 1;

/*	BLEnRfInit();*/



	wdt_enable(WDTO_2S);
}

void loop()
{

	WiFi_task();

	//NonStopTask();
} 

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
			Serial.println(F("Wifi is connected \r\n"));
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
			Serial.println(F("wait IP time out, reset wifi \r\n"));
#endif
			WIFI_SERIAL.print(F("AT+RST\r\n"));
			WaitForTimeOut = 30;
		}
	}



}

void nRF_task()
{
// 	if(BLEnRFDataCheckTask(&TagData))
// 	{
// 
// 		if ((TagData.Type < RFID_MAX_TYPE)&&(TagData.Id < RFID_MAX_NUMBER))
// 		{
// 
// 			if (TagTimeOut[TagData.Type][TagData.Id] > 5)
// 			{
// 				for(unsigned char i = 0;i < 4;i++)
// 				{
// 					*(((unsigned char *)(&(BleKeyLessData.KeyLessData)))+i) = *(((unsigned char *)(&(TagData)))+3-i);
// 				}
// 				BleKeyLessData.Type = TagData.Type;
// 				BleKeyLessData.DataType = 21;
// 				BleKeyLessData.Triger = true;
// 				SendKeyLessData();	
// 
// #ifdef DEGBUG_OUTPUT
// 				Serial.print(TagData.Type);
// 				Serial.print(F(" "));
// 				Serial.print(TagData.Id);
// 				Serial.print(F(" "));
// 				Serial.println(F("sent out"));
// #endif
// 			}
// 			TagTimeOut[TagData.Type][TagData.Id] = 0;
// 
// 		}
// 
// 
// 
// 
// #ifdef DEGBUG_OUTPUT
// 		printf("Code:%d ID:%d Volt:%04d Type:%d\r\n"
// 			,TagData.Code
// 			,TagData.Id
// 			,TagData.VoltH*255+TagData.VoltL
// 			,TagData.Type
// 			);
// #endif
// 
// 	}

	Door_task();




}

void SendKeyLessData()
{
	unsigned char data;
	unsigned char Len = sizeof(tBleKeyLessData);

	if (WifiOK)
	{

		WIFI_SERIAL.print(F("AT+CIPSEND="));
		WIFI_SERIAL.print(Len);
		WIFI_SERIAL.print(F("\r\n"));

		//if (!_esp8266_waitFor(">")) return;
		//printf("Sending data \r\n");
		delay(10);

		for(unsigned char i = 0; i< Len ; i++)
		{
			data = *(((char*)(&BleKeyLessData))+i);
			WIFI_SERIAL.write(data);
		}

		if (!_esp8266_waitFor("OK")) return;
	}

}

void Door_task()
{

	bool DoorShouldOpen = false;

	for(unsigned char i = 0;i < RFID_MAX_TYPE;i++)
	{
		for(unsigned char j = 0;j < RFID_MAX_NUMBER;j++)
		{
			if (TagTimeOut[i][j] < 20)
			{
				DoorShouldOpen = true;
			}
		}

	}

//	for(unsigned char i = 0;i < RFID_MAX_NUMBER;i++)
//	{
//		if ((LastGetTime[i]!=0)&&( SecondsSinceStart - LastGetTime[i] < TIME_OUT_CLOSE_DOOR))
//		{
//			if (!RfidOnline[i])//offline before
//			{
//#ifdef DEGBUG_OUTPUT
//				Serial.print(i);
//				Serial.println(F(" RFID online."));
//				//printf("RFID %d online. \r\n",i);
//#endif
//			} 
//			RfidOnline[i] = true;
//			DoorShouldOpen = true;
//		}
//		else
//		{
//			if (RfidOnline[i])//online before
//			{	
//				RfidOnline[i] = false;
//#ifdef DEGBUG_OUTPUT
//				Serial.print(i);
//				Serial.println(F(" RFID offline."));
//				//printf("RFID %d offline. \r\n",i);
//#endif
//			}
//		}
//	}

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
			//printf("Close door  \r\n");
			Serial.println(F("Close door"));
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

	for(unsigned char i = 0;i < RFID_MAX_TYPE;i++)
	{
		for(unsigned char j = 0;j < RFID_MAX_NUMBER;j++)
		{
#ifdef DEGBUG_OUTPUT
			WIFI_SERIAL.print(TagTimeOut[i][j]);
			WIFI_SERIAL.print(F(" "));
#endif
			if (TagTimeOut[i][j] < 250)
			{
				TagTimeOut[i][j]++;
			}
		}
#ifdef DEGBUG_OUTPUT
		WIFI_SERIAL.println(F(""));
#endif
	}



/*	BLEnRFSwitch();*/


	//memcpy(&(BleKeyLessData.KeyLessData),&TagData,4);


	BleKeyLessData.DataType = 21;
	BleKeyLessData.Triger = false;
	SendKeyLessData();
}

unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if(abs(CurrentMillis-LastMillis)> 1000)
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
