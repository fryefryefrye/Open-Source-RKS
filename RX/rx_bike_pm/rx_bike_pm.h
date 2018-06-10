
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define TIME_OUT_TURN_OFF_BIKE 30		//s
#define TIME_OUT_HOME 60
#define WAIT_KEY_IN_HOME 30
#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 4000				//set lenght of the buzz 2000
#define BUZZOFF 7000			//set interval of the buzz 5000
#define DEGBUG_OUTPUT

#define BUZZ 9
#define RELAY 10

/*****************************************************/


#include <SPI.h>
#include "RF24.h"
#include <printf.h>

//display
#include <Wire.h>
//#include <CN_SSD1306_Wire.h>

#include "CN_SSD1306_Wire\CN_SSD1306_Wire.h"

//standy 35ma; ON 80 ; Light 260mah 320mah

CN_SSD1306_Wire Displayer(8);//HardWare I2C
unsigned long DisCharge = 0;//ma* s         max = 1190 AH
unsigned long Charge = 0;//ma* s

unsigned long LastDisCharge = 0;//ma* s         max = 1190 AH
unsigned long LastCharge = 0;//ma* s

unsigned long Capability = 72000000;
unsigned long FullVolt; //mV
unsigned long Current; //mA
bool OnCharge = false; 
bool ScreenOn = false; 
bool UnitShowed = false;
#define CHARGE_MODE_AMP 1000  //mA
#define DISCHARGE_MODE_AMP 1000  //mA


#define VOLT_INPUT 2
#define CURRENT_INPUT 6


#define CURRENT_MIDDLE 510
#define CURRENT_DEAD 3

#define CURRENT_RATE 74  //2.5/512*1000/66*1000 ma
#define VOLT_RATE 62


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
unsigned long SecondsSinceStart;

bool LastOn = false;
bool Auto = true;
bool Home = false;

void SecondsSinceStartTask();
void nRFTask();
void ChHopTask();
void RF_task();
void Buzz_task();
void CheckTime_task();
void OnKeyPress();
void ShowState();
void ShowChargeInfo();
void TenthSecondsSinceStartTask();
void GetMeasurementData();



void setup()
{

	Displayer.Initial();
	delay(10);

	Displayer.Fill_Screen(0x00,0x00);


	pinMode(RELAY, OUTPUT);
	pinMode(BUZZ, OUTPUT);

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






	//unsigned char wm=0;
	//unsigned char i=0;
	//unsigned int adder=0;

	//unsigned int ASCII1632[16];




	//for(wm = 0;wm < 16;wm++)
	//{
	//	//printf("ASCII816_2 %x \r\n", ASCII816_2[adder]);
	//	ASCII1632[wm] = 0;
	//	for(i = 0;i < 8;i++)
	//	{
	//		//printf("((((unsigned int)ASCII816_2[adder]>>i)&1)*2) %x \r\n", ((((unsigned int)ASCII816_2[adder]>>i)&1)*2))<<(2*i);

	//		//printf("2<<2 %x \r\n", 2<<(2*i);

	//		ASCII1632[wm] = ASCII1632[wm] + (((((unsigned int)ASCII816_2[adder]>>i)&1)*3)<<(2*i));
	//	}
	//	printf("ASCII1632 %x \r\n", ASCII1632[wm]);
	//	adder += 1;	
	//}


	//Displayer.IIC_SetPos(0 , 0);
	//for(wm = 0;wm < 8;wm++)
	//{
	//	Displayer.WriteData(*((unsigned char*)(&ASCII1632[wm/2])));

	//	printf("*((unsigned char*)(&ASCII1632[wm/2])) = %x \r\n",*((unsigned char*)(&ASCII1632[wm/2])));
	//}


	//Displayer.IIC_SetPos(0,0 + 1);
	//for(wm = 0;wm < 8;wm++)
	//{
	//	Displayer.WriteData(*(((unsigned char*)(&ASCII1632[wm/2]))+1));
	//			printf("*((unsigned char*)(&ASCII1632[wm/2]+1)) = %x \r\n",(*(((unsigned char*)(&ASCII1632[wm/2]))+1)));
	//}

	//Displayer.IIC_SetPos(1 , 0);
	//for(wm = 0;wm < 8;wm++)
	//{
	//	Displayer.WriteData(*(((unsigned char*)(&ASCII1632[wm+8/2]))));
	//}


	//Displayer.IIC_SetPos(1,0 + 1);
	//for(wm = 0;wm < 8;wm++)
	//{
	//	Displayer.WriteData(*(((unsigned char*)(&ASCII1632[wm+8/2]))+1));
	//}


}

void loop()
{

	SecondsSinceStartTask();
	TenthSecondsSinceStartTask();
	nRFTask();
	CheckTime_task();
	ChHopTask();
	Buzz_task();


} // Loop

void CheckTime_task()
{



	if (!Auto)
	{
		return;
	}

	if ((SecondsSinceStart - LastHomeGetTime < TIME_OUT_HOME) && (LastHomeGetTime != 0))
	{
		Home = true;
	}
	else
	{
		Home = false;
	}


	if (LastOn)
	{
		if ((SecondsSinceStart - LastTagGetTime == TIME_OUT_TURN_OFF_BIKE/2) && (LastTagGetTime != 0)&&(Alarm == 0))
		{
			Alarm = 3;
			Alarmed = true;
#ifdef DEGBUG_OUTPUT
			printf("Power off warning \r\n");
#endif
		}

		if ((SecondsSinceStart - LastTagGetTime < TIME_OUT_TURN_OFF_BIKE/2) && (LastTagGetTime != 0)&&(Alarm == 0)&&Alarmed)
		{
			Alarm = 1;
			Alarmed = false;
#ifdef DEGBUG_OUTPUT
			printf("Power off warning clear \r\n");
#endif
		}

		if ((SecondsSinceStart - LastTagGetTime == TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0)&&(Alarm == 0))
		{
			Alarm = 2;
#ifdef DEGBUG_OUTPUT
			printf("Power off soon \r\n");
#endif
		}

		if ((SecondsSinceStart - LastTagGetTime > TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0))
		{

			digitalWrite(RELAY, LOW);
			Displayer.Fill_Screen(0x00,0x00);
			ScreenOn = false;
			UnitShowed = false;
			LastOn = false;
#ifdef DEGBUG_OUTPUT
			printf("Turn OFF \r\n");
#endif
		}
	}
	else // last OFF
	{
		if ((SecondsSinceStart - LastTagGetTime < TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0))
		{
			Alarm = 1;

#ifdef DEGBUG_OUTPUT
			printf("Turn ON \r\n");
#endif
			digitalWrite(RELAY, HIGH);
			ScreenOn = true;
			LastOn = true;
			Alarmed = false;
		}
	}
}

void OnKeyPress()
{

	if (LastOn)
	{
		Alarm = 3;
		digitalWrite(RELAY, LOW);
		UnitShowed = false;
		Displayer.Fill_Screen(0x00,0x00);
		ScreenOn = false;
		LastOn = false;
		Auto = false;

#ifdef DEGBUG_OUTPUT
		printf("Turn OFF manually \r\n");
#endif
	}
	else
	{
		Alarm = 1;
		digitalWrite(RELAY, HIGH);
		ScreenOn = true;
		LastOn = true;
		Auto = true;

#ifdef DEGBUG_OUTPUT
		printf("Turn ON manually \r\n");
#endif
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

			LastTagGetTime = SecondsSinceStart;
			//if (Alarmed)
			//{
			//	Alarm = 1;
			//}

		}
		else if (GotData[0] == 1)
		{
#ifdef DEGBUG_OUTPUT
			printf("LastHomeGetTime offset = %d \r\n", SecondsSinceStart - LastHomeGetTime);
#endif
			LastHomeGetTime = SecondsSinceStart;
		}
		else if (GotData[0] == 2)
		{
			LastTagGetTime = SecondsSinceStart;
			if (millis() - LastKeyGetTime > 500)
			{
				LastKeyGetTime = millis();
				OnKeyPress();
			}
		}

#ifdef DEGBUG_OUTPUT
		//Serial.print(PackageCounter);
		//Serial.print(" ");
		//Serial.print(F("Get data "));
		//for (char i = 0; i < DATA_LENGTH; i++)
		//{
		//    printf("%d,", GotData[i]);
		//}
		//printf("Volt:%d ", Volt);
		//printf("CH:%d\r\n", CurrCH);
#endif
	}

}
void ChHopTask()
{
	if (SecondsSinceStart - LastChangeCHTime > 0)             //RF_HOP every seconds
	{
		CurrCH++;
		if (CurrCH > 2)
		{
			CurrCH = 0;
		}
		LastChangeCHTime = SecondsSinceStart;
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
	if (abs(CurrentMillis - LastMillis) > 1000)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;


		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
		//printf("LastTagGetTime = %d \r\n",LastTagGetTime);
	}
}

unsigned long LastMillis_Tenth = 0;
void TenthSecondsSinceStartTask()
{
	unsigned long CurrentMillis_Tenth = millis();
	if (abs(CurrentMillis_Tenth - LastMillis_Tenth) > 99)
	{
		//printf("TenthSecondsSinceStartTask = %d \r\n",CurrentMillis_Tenth - LastMillis_Tenth);
		LastMillis_Tenth = CurrentMillis_Tenth-(CurrentMillis_Tenth - LastMillis_Tenth -100);

		if (ScreenOn)
		{
			ShowState();
			ShowChargeInfo();
		}

		GetMeasurementData();

		//TenthSecondsSinceStart++;
		//printf("TenthSecondsSinceStartTask = %d \r\n",SecondsSinceStart);
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
				if (!Home)
				{
					digitalWrite(BUZZ, HIGH);
				}
			}
		}
	}
	else
	{
	}
}

void ShowState()
{

	//static bool UnitShowed = false;
	static bool DataStored = false;
	static unsigned int CellVolt = 0;
	static unsigned char ShowIndex = 0;

	if (!UnitShowed)
	{
		Displayer.ShowASCII1632(32,4,12);
		Displayer.ShowASCII1632(16*5,4,10);
		Displayer.ShowASCII1632(16*7,4,11);
		Displayer.ShowASCII816(0,0,13);//C
		Displayer.ShowASCII816(0,2,14);//D
		Displayer.ShowASCII816(3*8,0,15);//.
		Displayer.ShowASCII816(3*8+56-16,0,15);//.
		Displayer.ShowASCII816(3*8,2,15);//.
		Displayer.ShowASCII816(3*8+56-16,2,15);//.
		Displayer.ShowASCII816(9*8+56-24,0,15);//.
		Displayer.ShowASCII816(9*8+56-24+16,0,12);//V
		Displayer.ShowASCII816(9*8+56-24,2,15);//.
		Displayer.ShowASCII816(9*8+56-24+16,2,16);//%


		UnitShowed = true;
	}


	if (!DataStored)
	{
		CellVolt = FullVolt/13;
		DataStored = true;
		ShowIndex++;
	}


	switch(ShowIndex)
	{
	case 1:
		Displayer.ShowASCII1632(0,4,CellVolt/100%10);
		break;
	case 2:
		Displayer.ShowASCII1632(16,4,CellVolt/10%10);
		break;
	case 3:
		Displayer.ShowASCII1632(16*3,4,Current/10000%10);
		break;
	case 4:
		Displayer.ShowASCII1632(16*4,4,Current/1000%10);
		break;
	case 5:
		Displayer.ShowASCII1632(16*6,4,Current/100%10);
		break;
	}

	ShowIndex++;

	if (ShowIndex>=6)
	{
		ShowIndex = 0;
		DataStored = false;
	}



}

void ShowChargeInfo()
{


	//static bool UnitShowed = false;
	static bool DataStored = false;
	static unsigned int DisplayerCharge;
	static unsigned int DisplayerDisCharge;
	static unsigned int DisplayerLastCharge;
	static unsigned int DisplayerLastDisCharge;
	static unsigned int DisplayerFullVolt;
	static unsigned int DisplayerRemain;
	static unsigned char ShowIndex = 0;



	if (!DataStored)
	{
		DisplayerCharge =Charge/360000;//3600; //Charge/36000; //mah
		DisplayerDisCharge =DisCharge/360000;//3600; //DisCharge/36000; //mah
		DisplayerLastCharge = LastCharge/360000;
		DisplayerLastDisCharge = LastDisCharge/360000;
		DisplayerFullVolt = FullVolt;
		if (Capability >= DisCharge)
		{
			DisplayerRemain = (Capability - DisCharge)/(Capability/1000);
		} 
		else
		{
			DisplayerRemain = 0;
		}
		DataStored = true;
		ShowIndex++;
	}







	switch(ShowIndex)
	{
	case 1:
		Displayer.ShowASCII816(8,0,DisplayerCharge/100%10);
		Displayer.ShowASCII816(8+56-16,0,DisplayerLastCharge/100%10);
		break;
	case 2:
		Displayer.ShowASCII816(2*8,0,DisplayerCharge/10%10);
		Displayer.ShowASCII816(2*8+56-16,0,DisplayerLastCharge/10%10);
		break;
	case 3:

		break;
	case 4:
		Displayer.ShowASCII816(4*8,0,DisplayerCharge/1%10);
		Displayer.ShowASCII816(4*8+56-16,0,DisplayerLastCharge/1%10);
		break;
	case 5:
		//Displayer.ShowASCII816(5*8,0,DisplayerCharge/1%10);
		//Displayer.ShowASCII816(5*8+56,0,DisplayerLastCharge/1%10);
		break;
	case 6:
		Displayer.ShowASCII816(1*8,2,DisplayerDisCharge/100%10);
		Displayer.ShowASCII816(1*8+56-16,2,DisplayerLastDisCharge/100%10);
		break;
	case 7:
		Displayer.ShowASCII816(2*8,2,DisplayerDisCharge/10%10);
		Displayer.ShowASCII816(2*8+56-16,2,DisplayerLastDisCharge/10%10);
		break;
	case 8:

		break;
	case 9:
		Displayer.ShowASCII816(4*8,2,DisplayerDisCharge/1%10);
		Displayer.ShowASCII816(4*8+56-16,2,DisplayerLastDisCharge/1%10);
		break;
	case 10:
		//Displayer.ShowASCII816(5*8,2,DisplayerDisCharge/1%10);
		//Displayer.ShowASCII816(5*8+56,2,DisplayerLastDisCharge/1%10);
		break;
	case 11:
		Displayer.ShowASCII816(7*8+56-24,0,DisplayerFullVolt/10000%10);
		Displayer.ShowASCII816(8*8+56-24,0,DisplayerFullVolt/1000%10);
		Displayer.ShowASCII816(10*8+56-24,0,DisplayerFullVolt/100%10);
		break;
	case 12:
		if (DisplayerRemain>=999)
		{
			DisplayerRemain = 999;
		}
		Displayer.ShowASCII816(7*8+56-24,2,DisplayerRemain/100%10);
		Displayer.ShowASCII816(8*8+56-24,2,DisplayerRemain/10%10);
		Displayer.ShowASCII816(10*8+56-24,2,DisplayerRemain/1%10);
		break;


	}



	ShowIndex++;

	if (ShowIndex>=13)
	{
		ShowIndex = 0;
		DataStored = false;
	}

}

void GetMeasurementData()
{
	static unsigned char AnalogReadCounter = 0;
	static unsigned int AnalogReadVolt = 0;
	static unsigned int AnalogReadCurrent = 0;
	static unsigned char ChargeStateCounter = 0;

	AnalogReadVolt += analogRead(VOLT_INPUT);
	AnalogReadCurrent += analogRead(CURRENT_INPUT);
	AnalogReadCounter ++;



	if (AnalogReadCounter >=10)
	{
		AnalogReadCounter = 0;

		FullVolt = AnalogReadVolt/10;
		Current = AnalogReadCurrent/10;

		//printf("FullVolt  %d \r\n",FullVolt);
		


		FullVolt = FullVolt*VOLT_RATE;


		//standing by discharge
		DisCharge = DisCharge + (ScreenOn ? 80 : 35);



		if (Current > (CURRENT_MIDDLE + CURRENT_DEAD))//Charge
		{
			//printf("analogRead(CURRENT_INPUT)  %d \r\n",Current);
			Current = Current - CURRENT_MIDDLE;
			Current = Current*CURRENT_RATE;          //2.5/512*1000/66*1000 ma

			printf("Charge Current =  %d \r\n",Current);


			Charge = Charge + Current;


			if ((OnCharge == false)&&(Current > CHARGE_MODE_AMP))
			{
				ChargeStateCounter++;
				printf("ChargeStateCounter++  to %d \r\n",ChargeStateCounter);
				if (ChargeStateCounter > 4)
				{
					OnCharge = true;
					ChargeStateCounter = 0;
					LastCharge = Charge;
					Charge = 0;

					printf("ChargeState changed to charge \r\n");
				}
			}


		} 
		else if (Current < (CURRENT_MIDDLE - CURRENT_DEAD))//DisCharge
		{
			//printf("analogRead(CURRENT_INPUT)  %d \r\n",Current);
			Current =  CURRENT_MIDDLE - Current;
			Current = Current*CURRENT_RATE;          //2.5/512*1000/66*1000 ma
			DisCharge = DisCharge + Current;

			printf("DisCharge Current =  %d \r\n",Current);
			printf("DisCharge mah %d \r\n",DisCharge/3600);


			if ((OnCharge == true)&&(Current > DISCHARGE_MODE_AMP))
			{
				ChargeStateCounter++;
				printf("ChargeStateCounter++  to %d \r\n",ChargeStateCounter);
				if (ChargeStateCounter > 4)
				{
					OnCharge = false;
					ChargeStateCounter = 0;

					LastDisCharge = DisCharge;
					DisCharge = 0;
					printf("ChargeState changed to DisCharge \r\n");
				}
			}
		}
		else
		{
			Current = 0;
		}


		AnalogReadCurrent = 0;
		AnalogReadVolt = 0;
		AnalogReadCounter = 0;
	}


}
