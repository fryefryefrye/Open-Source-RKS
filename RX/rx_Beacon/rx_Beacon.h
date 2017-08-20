/*************************user modify settings****************************/
byte addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char  HopCH[3] = {105,76,108};//Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define TIME_OUT_CLOSE_DOOR 10        //s
#define DATA_LENGTH 4                    //use fixed data length 1-32
#define BUZZON 1000                //set lenght of the buzz
#define BUZZOFF 30000            //set interval of the buzz
#define RFID_NUMBER 8


#define DEGBUG_OUTPUT
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
unsigned long RandDelay = 0;
unsigned long LastGetTime[RFID_NUMBER] = {0};
bool RfidOnline[RFID_NUMBER] = {false};

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




/*************************Time*******************/
time_t t;
unsigned long SecondsSinceStart = 0;
/**********************************************************/




void Door_task();
void Buzz_task();
void SecondsSinceStartTask();
unsigned char CharLength(char * MyChar);
void OnSecond();
void SendBeacon();



void setup()
{




#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_Beacon_ID_Read"));
	printf_begin();
#endif


	pinMode(DOOR, OUTPUT);
	pinMode(BUZZ, OUTPUT);


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

	if (abs(CurrTime - LastChangeCHTime >  RandDelay))//RF_HOP
	{
		CurrCH++;
		if (CurrCH>2)
		{
			CurrCH = 0;
		}
		LastChangeCHTime = millis();
		RandDelay = random(3000);

		printf("RandDelay:%d\r\n",RandDelay);

		radio.stopListening();

		radio.setChannel(HopCH[CurrCH]);
		delay(10);
		SendBeacon();
		delay(10);

		radio.startListening();
	}


	SecondsSinceStartTask();
	Buzz_task();
} // Loop

void SendBeacon()
{
	GotData[0] = 1;
	GotData[1] = 0;
	GotData[2] = 0;
	GotData[3] = 0;
	radio.write(GotData, 4);

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
				printf("RFID %d online. \r\n",i);
			} 
			RfidOnline[i] = true;
			DoorShouldOpen = true;
		}
		else
		{
			if (RfidOnline[i])//online before
			{	
				RfidOnline[i] = false;
				printf("RFID %d offline. \r\n",i);
			}
		}
	}

	if (DoorLastState != DoorShouldOpen)//update relay when door status change
	{
		DoorLastState = DoorShouldOpen;
		if(DoorShouldOpen)
		{
			printf("Open door  \r\n");
			digitalWrite(DOOR, HIGH);
			//AlarmOn = true;
		}
		else
		{
			printf("Close door  \r\n");
			digitalWrite(DOOR, LOW);
			//AlarmOn = false;
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
	Door_task();
}

unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if(abs(CurrentMillis-LastMillis)> 1000-9)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;
		OnSecond();
		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);

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



