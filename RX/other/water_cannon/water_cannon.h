
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
//#define TIME_OUT_TURN_OFF_BIKE 300		//0.1s
//#define TIME_OUT_HOME 60
//#define WAIT_KEY_IN_HOME 30

#define TIME_WATER 10
#define TIME_WAIT 4
#define TIME_AIR 2

#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 4000				//set lenght of the buzz 2000
#define BUZZOFF 7000			//set interval of the buzz 5000
#define DEGBUG_OUTPUT

#define BUZZ 9
#define RELAY_WATER 10
#define RELAY_AIR 6

/*****************************************************/


#include <SPI.h>
#include "RF24.h"
#include <printf.h>


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

unsigned char Alarm = 0;
bool Alarmed = false;

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;

unsigned long LastTagGetTime = 0;
unsigned long LastHomeGetTime = 0;
unsigned long LastKeyGetTime = 0; //ms

unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long TenthSecondsSinceStart;

//bool LastOn = false;
//bool Auto = true;
bool Shooting = false;

void TenthSecondsSinceStartTask();
void nRFTask();
void ChHopTask();
void RF_task();
void Buzz_task();
void Shoot_task();
void OnKeyPress();

void setup()
{

    pinMode(RELAY_WATER, OUTPUT);
	pinMode(RELAY_AIR, OUTPUT);
    pinMode(BUZZ, OUTPUT);

#ifdef DEGBUG_OUTPUT
    Serial.begin(115200);
    Serial.println(F("RF24_Water_Cannon"));
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

    TenthSecondsSinceStartTask();
    nRFTask();
    Shoot_task();
    ChHopTask();
    Buzz_task();


} // Loop

void Shoot_task()
{
	static unsigned char Step = 0;
	static unsigned long TenthStart;
	if (Shooting)
	{
		switch(Step)
		{
		case 0:
			printf("Water in start \r\n");
			digitalWrite(RELAY_WATER, HIGH);
			Step++;
			TenthStart = TenthSecondsSinceStart;
			break;
		case 1:
			if (TenthSecondsSinceStart - TenthStart > TIME_WATER)
			{
				printf("Water in stop \r\n");
				digitalWrite(RELAY_WATER, LOW);
				Step++;
				TenthStart = TenthSecondsSinceStart;
			}
			break;
		case 2:
			if (TenthSecondsSinceStart - TenthStart > TIME_WAIT)
			{
				printf("Air in start \r\n");
				digitalWrite(RELAY_AIR, HIGH);
				Step++;
				TenthStart = TenthSecondsSinceStart;
			}
			break;
		case 3:
			if (TenthSecondsSinceStart - TenthStart > TIME_AIR)
			{
				printf("Air in stop \r\n");
				digitalWrite(RELAY_AIR, LOW);
				Step++;
				TenthStart = TenthSecondsSinceStart;
			}
			break;
		default:
			Shooting = false;
			break;
		}
	} 
	else
	{
		digitalWrite(RELAY_WATER, LOW);
		digitalWrite(RELAY_AIR, LOW);
		Step = 0;
	}
}

//void CheckTime_task()
//{
//
//
//
//    if (!Auto)
//    {
//        return;
//    }
//
//	if ((TenthSecondsSinceStart - LastHomeGetTime < TIME_OUT_HOME) && (LastHomeGetTime != 0))
//	{
//		Home = true;
//	}
//	else
//	{
//		Home = false;
//	}
//
//
//    if (LastOn)
//    {
//        if ((TenthSecondsSinceStart - LastTagGetTime == TIME_OUT_TURN_OFF_BIKE/2) && (LastTagGetTime != 0)&&(Alarm == 0))
//        {
//            Alarm = 3;
//			Alarmed = true;
//#ifdef DEGBUG_OUTPUT
//			printf("Power off warning \r\n");
//#endif
//        }
//
//		if ((TenthSecondsSinceStart - LastTagGetTime < TIME_OUT_TURN_OFF_BIKE/2) && (LastTagGetTime != 0)&&(Alarm == 0)&&Alarmed)
//		{
//			Alarm = 1;
//			Alarmed = false;
//#ifdef DEGBUG_OUTPUT
//			printf("Power off warning clear \r\n");
//#endif
//		}
//
//		if ((TenthSecondsSinceStart - LastTagGetTime == TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0)&&(Alarm == 0))
//		{
//			Alarm = 2;
//#ifdef DEGBUG_OUTPUT
//			printf("Power off soon \r\n");
//#endif
//		}
//
//        if ((TenthSecondsSinceStart - LastTagGetTime > TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0))
//        {
//
//            digitalWrite(RELAY_WATER, LOW);
//            LastOn = false;
//#ifdef DEGBUG_OUTPUT
//            printf("Turn OFF \r\n");
//#endif
//        }
//    }
//    else // last OFF
//    {
//        if ((TenthSecondsSinceStart - LastTagGetTime < TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0))
//        {
//            Alarm = 1;
//
//#ifdef DEGBUG_OUTPUT
//            printf("Turn ON \r\n");
//#endif
//            digitalWrite(RELAY_WATER, HIGH);
//            LastOn = true;
//			Alarmed = false;
//        }
//    }
//}

void OnKeyPress()
{

	Alarm = 1;
	Shooting = true;
	printf("OnKeyPress \r\n");


//
//    if (LastOn)
//    {
//        Alarm = 3;
//        digitalWrite(RELAY_WATER, LOW);
//		digitalWrite(RELAY_AIR, LOW);
//        LastOn = false;
//        Auto = false;
//
//#ifdef DEGBUG_OUTPUT
//        printf("Turn OFF manually \r\n");
//#endif
//    }
//    else
//    {
//        Alarm = 1;
//        digitalWrite(RELAY_WATER, HIGH);
//		digitalWrite(RELAY_AIR, HIGH);
//        LastOn = true;
//        Auto = true;
//
//#ifdef DEGBUG_OUTPUT
//        printf("Turn ON manually \r\n");
//#endif
//    }
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
			//if (Alarmed)
			//{
			//	Alarm = 1;
			//}
			
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
            if (millis() - LastKeyGetTime > 500)
            {
                LastKeyGetTime = millis();
                OnKeyPress();
            }
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
    if (TenthSecondsSinceStart - LastChangeCHTime > 10)             //RF_HOP every seconds
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
void TenthSecondsSinceStartTask()
{
    unsigned long CurrentMillis = millis();
    if (abs(CurrentMillis - LastMillis) > 100)
    {
        LastMillis = CurrentMillis;
        TenthSecondsSinceStart++;
        //printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
        //printf("LastTagGetTime = %d \r\n",LastTagGetTime);
    }
}

void Buzz_task()
{

    static signed int BuzzOn;
    static signed int BuzzOff;
    static bool BuzzHigh;

    if (Alarm > 0)
    {
        if (BuzzHigh)
        {
            BuzzOn++;
            if (BuzzOn > BUZZON)
            {
                BuzzOn = 0;
                BuzzHigh = false;
                digitalWrite(BUZZ, LOW);
                Alarm--;
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
