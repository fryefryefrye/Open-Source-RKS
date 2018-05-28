
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
//#define TIME_OUT_TURN_OFF_BIKE 30		//s
//#define TIME_OUT_HOME 60

#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 700				//set lenght of the buzz 2000
#define BUZZOFF 1000			//set interval of the buzz 5000
#define DEGBUG_OUTPUT


#define TIME_OUT_KEY 100  //0.1s
#define BIKE_CHECK_TIME 10  //0.1s
#define PRE_ALARM_TIME 200  //0.1s
#define ALARM_AUDIO_LIMIT 600  //0.1s
#define ALARM_FLASH_LIMIT 6000  //0.1s

#define BUZZ 9
#define LIGHT 10
#define ALARM_FLASH 6

#define BIKE_CHECK1 2
#define BIKE_CHECK2 3
#define BIKE_CHECK3 4

#define AUDIO1 A0
#define AUDIO2 A1

/*****************************************************/


#include <SPI.h>
#include "RF24.h"
#include <printf.h>


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

unsigned char Buzz = 0;
bool Buzzed = false;

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;

unsigned long LastTagGetTime = 0;
unsigned long LastHomeGetTime = 0;


unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long TenthSecondsSinceStart;


bool Tag = false;
bool AlarmOn = false;
unsigned long AlarmOnCounter = 0;
bool BikeChecked[3] = {false,false,false};
unsigned char BickCheckCounter = 0;
//bool Home = false;

void SecondsSinceStartTask();
void nRFTask();
void ChHopTask();
void RF_task();
void Buzz_task();
void CheckTime_task();
void OnKeyPress();
void OnTenthSecond();

void setup()
{

	pinMode(ALARM_FLASH, OUTPUT);
	pinMode(LIGHT, OUTPUT);
	pinMode(BUZZ, OUTPUT);

	pinMode(BIKE_CHECK1, INPUT_PULLUP);
	pinMode(BIKE_CHECK2, INPUT_PULLUP);
	pinMode(BIKE_CHECK3, INPUT_PULLUP);

	pinMode(AUDIO2, INPUT);
	pinMode(AUDIO1, INPUT);


#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_Bike_Read"));
	printf_begin();
#endif

	radio.begin();
	radio.setPALevel(RF24_PA_MIN);
	radio.setAddressWidth(5);
	radio.setPayloadSize(4);
	radio.setDataRate(RF24_2MBPS); //RF24_250KBPS  //RF24_2MBPS     //RF24_1MBPS
	radio.setChannel(105);
	radio.setCRCLength(RF24_CRC_8); //RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit

	//Open a writing and reading pipe on each radio, with opposite addresses
	radio.openWritingPipe(addresses);
	radio.openReadingPipe(0, addresses);
	radio.closeReadingPipe(1);
	radio.closeReadingPipe(2);
	radio.closeReadingPipe(3);
	radio.closeReadingPipe(4);
	radio.closeReadingPipe(5);
	radio.setAutoAck(1, false);
	radio.setAutoAck(2, false);
	radio.setAutoAck(3, false);
	radio.setAutoAck(4, false);
	radio.setAutoAck(5, false);
	radio.setAutoAck(0, false);
	//Start the radio listening for data
	radio.startListening();
}

void loop()
{

	SecondsSinceStartTask();
	nRFTask();
	CheckTime_task();
	ChHopTask();
	Buzz_task();


} // Loop

void CheckTime_task()
{

	if (TenthSecondsSinceStart - LastTagGetTime < TIME_OUT_KEY)
	{
		if (Tag == false)
		{
			printf("Get key \r\n");
			digitalWrite(LIGHT, HIGH);
			AlarmOn = false;
		}
		Tag = true;
	} 
	else
	{
		if (Tag == true)
		{
			printf("Lost key \r\n");
			digitalWrite(LIGHT, LOW);
		}
		Tag = false;
	}
}


void OnTenthSecond()
{

	static unsigned char UnChangeCounter[3];
	bool BikeState[3];

	BikeState[0] = !(digitalRead(BIKE_CHECK1));
	BikeState[1] = !(digitalRead(BIKE_CHECK2));
	BikeState[2] = !(digitalRead(BIKE_CHECK3));

	for (unsigned char i = 0; i < 3; i++)
	{



		if (BikeChecked[i] != BikeState[i]) 
		{
			UnChangeCounter[i]++;

			if (UnChangeCounter[i] > BIKE_CHECK_TIME)
			{
				BikeChecked[i] = BikeState[i];
				UnChangeCounter[i] = 0;
				printf("Bike checked[%d] =  %d \r\n",i,BikeChecked[i]);

				if (BikeChecked[i])
				{
					AlarmOn = false;
					Buzz = 2;
				} 
				else//disconnect
				{
					Buzz = 1;
					if (Tag == false)
					{
						printf("Bike %d disconnect without Tag \r\n",i);
						AlarmOn = true;
						AlarmOnCounter = 0;
					}
				}
			} 
		} 
		else
		{
			UnChangeCounter[i] = 0;
		}
	}

	static bool PreAlarm = false;
	static bool AlarmAudio = false;
	static bool AlarmFlash = false;

	if (AlarmOn)
	{
		AlarmOnCounter++;


		if ((AlarmOnCounter>0)&&(AlarmOnCounter<PRE_ALARM_TIME))
		{
			if (!PreAlarm)
			{
				digitalWrite(LIGHT, HIGH);
				pinMode(AUDIO1, OUTPUT);
				pinMode(AUDIO2, INPUT);
				digitalWrite(AUDIO1, LOW);
				PreAlarm = true;
				AlarmAudio = false;
				AlarmFlash = false;
				printf("Pre_Alarm \r\n");
			} 
		}

		if ((AlarmOnCounter>PRE_ALARM_TIME)&&(AlarmOnCounter<ALARM_AUDIO_LIMIT))
		{
			if (!AlarmAudio)
			{
				digitalWrite(LIGHT, LOW);
				digitalWrite(ALARM_FLASH, HIGH);
				pinMode(AUDIO2, OUTPUT);
				pinMode(AUDIO1, INPUT);
				digitalWrite(AUDIO2, LOW);
				printf("Alarm_Audio_Flash \r\n");

				PreAlarm = false;
				AlarmAudio = true;
				AlarmFlash = false;

			}
		}

		if ((AlarmOnCounter>ALARM_AUDIO_LIMIT)&&(AlarmOnCounter<ALARM_FLASH_LIMIT))
		{
			if (!AlarmFlash)
			{
				pinMode(AUDIO2, INPUT);
				pinMode(AUDIO1, INPUT);
				printf("Alarm_Flash \r\n");
				PreAlarm = false;
				AlarmAudio = false;
				AlarmFlash = true;
			}
		}

		if (AlarmOnCounter>ALARM_FLASH_LIMIT)
		{
			printf("Alarm_Over \r\n");
			AlarmOn = false;
		}

	}
	else
	{
		if (AlarmOnCounter != 0)
		{
			AlarmOnCounter = 0;
			if (Tag == false)
			{
				digitalWrite(LIGHT, LOW);
			}
			digitalWrite(ALARM_FLASH, LOW);
			pinMode(AUDIO2, INPUT);
			pinMode(AUDIO1, INPUT);
			PreAlarm = false;
			AlarmAudio = false;
			AlarmFlash = false;
		}
	}
}


void nRFTask()
{
	if (radio.available())
	{
		// Variable for the received timestamp
		while (radio.available())                                     // While there is data ready
		{
			radio.read(GotData, DATA_LENGTH);             // Get the payload
		}

		PackageCounter++;

		Volt = 1.2 * (GotData[DATA_LENGTH - 2] * 256 + GotData[DATA_LENGTH - 1]) * 3 * 1000 / 4096;

		if (GotData[0] == 0)
		{
			LastTagGetTime = TenthSecondsSinceStart;
		}
		else if (GotData[0] == 1)
		{
#ifdef DEGBUG_OUTPUT
			printf("LastHomeGetTime offset = %d \r\n", TenthSecondsSinceStart - LastHomeGetTime);
#endif
			LastHomeGetTime = TenthSecondsSinceStart;
		}
		else if (GotData[0] == 2)
		{
			LastTagGetTime = TenthSecondsSinceStart;
			//if (millis() - LastKeyGetTime > 500)
			//{
			//    LastKeyGetTime = millis();
			//    OnKeyPress();
			//}
		}

#ifdef DEGBUG_OUTPUT
		Serial.print(PackageCounter);
		Serial.print(" ");
		Serial.print(F("Get data "));
		for (char i = 0; i < DATA_LENGTH; i++)
		{
			printf("%d,", GotData[i]);
		}
		printf("Volt:%d ", Volt);
		printf("CH:%d\r\n", CurrCH);
#endif
	}

}
void ChHopTask()
{
	if (TenthSecondsSinceStart - LastChangeCHTime > 0)             //RF_HOP every seconds
	{
		CurrCH++;
		if (CurrCH > 2)
		{
			CurrCH = 0;
		}
		LastChangeCHTime = TenthSecondsSinceStart;
		radio.stopListening();
		radio.setChannel(HopCH[CurrCH]);
		radio.startListening();

		//printf("CH change \r\n");
	}
}

unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 100)
	{
		LastMillis = CurrentMillis;
		TenthSecondsSinceStart++;
		OnTenthSecond();
		//printf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
		//printf("LastTagGetTime = %d \r\n",LastTagGetTime);
	}
}

void Buzz_task()
{

	static signed int BuzzOn;
	static signed int BuzzOff;
	static bool BuzzHigh;

	if (Buzz > 0)
	{
		if (BuzzHigh)
		{
			BuzzOn++;
			if (BuzzOn > BUZZON)
			{
				BuzzOn = 0;
				BuzzHigh = false;
				digitalWrite(BUZZ, LOW);
				Buzz--;
			}
		}
		else
		{
			BuzzOff++;
			if (BuzzOff > BUZZOFF)
			{
				BuzzOff = 0;
				BuzzHigh = true;
				digitalWrite(BUZZ, HIGH);
			}
		}
	}
	else
	{
	}
}
