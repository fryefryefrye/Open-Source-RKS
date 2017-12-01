
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
//#define TIME_OUT_TURN_OFF_BIKE 90		//s
#define TIME_OUT_LOCK 30  //s
//#define WAIT_KEY_IN_HOME_INTERVAL 10
#define TIME_OUT_HOME 60
#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 1000				//set lenght of the buzz
#define BUZZOFF 30000			//set interval of the buzz
#define DEGBUG_OUTPUT
#define PARK_SWITCH_TIME 3  //s
/*****************************************************/

#include <SPI.h>
#include "RF24.h"
#include <printf.h>
//#include <avr/wdt.h>


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;

unsigned long LastTagGetTime = 0;   //unit: s
unsigned long LastHomeGetTime = 0;  //unit: s
unsigned long LastKeyGetTime = 0;   //unit: ms

unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long SecondsSinceStart;


bool LastOn = false;
bool Locked = false;
bool Auto = true;
bool NeedLock = false;
//unsigned long TrunOffTime = 0;
bool Park = true;
bool Home = false;



#define RF_315 6
#define PARK_SWITCH 5
#define RF_LENGTH 11
//unsigned char  RfCommand[3][RF_LENGTH]={
//	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB2 ,0xDB ,0x7F},
//	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB6 ,0x5B ,0x7F},
//	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB6 ,0xCB ,0x7F}
//};


//录音后反转，记录高低。
unsigned char RfCommand[3][RF_LENGTH] = {
	{ 0x00, 0x9A, 0x49, 0x36, 0xDA, 0x4D, 0xA4, 0x9A, 0x6D, 0x24, 0x80 },   //L
	{ 0x00, 0x9A, 0x49, 0x36, 0xDA, 0x4D, 0xA4, 0x9A, 0x69, 0xA4, 0x80 },   //U
	{ 0x00, 0x9A, 0x49, 0x36, 0xDA, 0x4D, 0xA4, 0x9A, 0x69, 0x26, 0x80 }   //P
};

#define RF_COMMAND_LOCK 0
#define RF_COMMAND_POWER_OFF 1
#define RF_COMMAND_POWER_ON 2


void RF_Command(unsigned char command, unsigned char repeat);
void SecondsSinceStartTask();
void OnSecond();
void nRFInit();
void nRFTask();
void ChHopTask();
void RF_task();
//void OnKeyPress();

void setup()
{
	pinMode(RF_315, OUTPUT);
	pinMode(PARK_SWITCH, INPUT_PULLUP);



#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_315_ID_Read"));
	printf_begin();
#endif

	//wdt_enable(WDTO_2S);

	nRFInit();


}

void nRFInit()
{
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

void OnSecond()
{



	static unsigned char UnChangeCounter;
	bool SwitchState;

	SwitchState = !(digitalRead(PARK_SWITCH));


	if (Park != SwitchState) //down postion
	{
		UnChangeCounter++;
		if (UnChangeCounter > PARK_SWITCH_TIME)
		{
			Park = SwitchState;
			UnChangeCounter = 0;
#ifdef DEGBUG_OUTPUT
			printf("Park State change to  %d \r\n",Park);
#endif

		} 

	} 
	else
	{
		UnChangeCounter = 0;
	}


#ifdef DEGBUG_OUTPUT
	//printf("SwitchState = %d ,PARK = %d, UnChangeCounter = %d \r\n",SwitchState,Park,UnChangeCounter);
#endif








	if (SecondsSinceStart%5 == 0)
	{
		nRFInit();
#ifdef DEGBUG_OUTPUT
		printf("re init nRF \r\n");
#endif

	}
}

void loop()
{

	SecondsSinceStartTask();
	nRFTask();
	ChHopTask();
	RF_task();










	//RF_Control test
	//RF_Command(RF_COMMAND_POWER_OFF,10);
	//delay(2000);

	//RF_Command(RF_COMMAND_LOCK,10);
	//delay(2000);

	//RF_Command(RF_COMMAND_POWER_ON,10);
	//delay(300);
	//RF_Command(RF_COMMAND_POWER_ON,10);
	//delay(15000);

} // Loop

//void RF_task()//simple trun on and off,then lock
//{
//	static bool LastOn = false;
//
//
//
//
//	if (LastOn)
//	{
//		if ((SecondsSinceStart - LastTagGetTime >TIME_OUT_TURN_OFF_BIKE)&&(LastTagGetTime != 0))
//		{
//			RF_Command(RF_COMMAND_POWER_OFF,10);
//			LastOn = false;
//			NeedSendLock = true;
//			TrunOffTime = SecondsSinceStart;
//#ifdef DEGBUG_OUTPUT
//			printf("Turn OFF \r\n");
//#endif
//		} 
//	} 
//	else// last OFF
//	{
//		if ((SecondsSinceStart - LastTagGetTime <TIME_OUT_TURN_OFF_BIKE)&&(LastTagGetTime != 0))
//		{
//#ifdef DEGBUG_OUTPUT
//			printf("Turn ON \r\n");
//#endif
//			RF_Command(RF_COMMAND_POWER_ON,10);
//			delay(300);
//			RF_Command(RF_COMMAND_POWER_ON,10);
//			LastOn = true;
//			NeedSendLock = false;
//		}
//	}
//
//	if ((NeedSendLock)&&(SecondsSinceStart - TrunOffTime >TIME_OUT_LOCK_WAIT_HOME)&&(TrunOffTime != 0))
//	{
//
//		RF_Command(RF_COMMAND_LOCK,10);
//#ifdef DEGBUG_OUTPUT
//		printf("Lock \r\n");
//#endif
//		NeedSendLock = false;
//	}
//}



//void RF_task() // do not lock at home,  wait key for a little long at home
//{
//
//	static bool Home = false;
//	static bool HomeFirstKeyGet = false;
//	static unsigned long HomeFirstKeyGetTime = 0;
//
//	if(!Auto)
//	{
//		return;
//	}
//
//	if ((SecondsSinceStart - LastHomeGetTime < TIME_OUT_LOCK_WAIT_HOME) && (LastHomeGetTime != 0))
//	{
//		Home = true;
//	}
//	else
//	{
//		if((Home)&&(LastOn))
//		{
//			RF_Command(RF_COMMAND_LOCK, 10);
//			Locked = true;
//#ifdef DEGBUG_OUTPUT
//			printf("Lock! leaving home without power on \r\n");
//#endif
//		}
//		Home = false;
//	}
//
//	if (LastOn)
//	{
//		if ((SecondsSinceStart - LastTagGetTime > TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0))
//		{
//			RF_Command(RF_COMMAND_POWER_OFF, 10);
//			LastOn = false;
//			Locked = false;
//			NeedSendLock = true;
//			TrunOffTime = SecondsSinceStart;
//#ifdef DEGBUG_OUTPUT
//			printf("Turn OFF \r\n");
//#endif
//		}
//	}
//	else // last OFF
//	{
//		if ((SecondsSinceStart - LastTagGetTime < WAIT_KEY_IN_HOME_INTERVAL) && (LastTagGetTime != 0))
//		{
//			if (!Home) //No home,bike Off, key in
//			{
//#ifdef DEGBUG_OUTPUT
//				printf("Turn ON \r\n");
//#endif
//				RF_Command(RF_COMMAND_POWER_ON, 10);
//				delay(300);
//				RF_Command(RF_COMMAND_POWER_ON, 10);
//				LastOn = true;
//				Locked = false;
//				NeedSendLock = false;
//			}
//			else //home,bike Off, key in
//			{
//				if (!HomeFirstKeyGet)
//				{
//					HomeFirstKeyGetTime = SecondsSinceStart;
//					HomeFirstKeyGet = true;
//				}
//				else
//				{
//					if (SecondsSinceStart - HomeFirstKeyGetTime > WAIT_KEY_IN_HOME)
//					{
//#ifdef DEGBUG_OUTPUT
//						printf("Turn ON, in home \r\n");
//#endif
//						RF_Command(RF_COMMAND_POWER_ON, 10);
//						delay(300);
//						RF_Command(RF_COMMAND_POWER_ON, 10);
//						LastOn = true;
//						Locked = false;
//						NeedSendLock = false;
//						HomeFirstKeyGet = false;
//						HomeFirstKeyGetTime = 0;
//					}
//				}
//
//			}
//
//		}
//		else // Off,key out
//		{
//			HomeFirstKeyGet = false;
//			HomeFirstKeyGetTime = 0;
//		}
//	}
//
//	if ((NeedSendLock) && (SecondsSinceStart - TrunOffTime > TIME_OUT_LOCK_WAIT_HOME) && (TrunOffTime != 0))
//	{
//		if (LastHomeGetTime < TrunOffTime)
//		{
//			RF_Command(RF_COMMAND_LOCK, 10);
//			Locked = true;
//#ifdef DEGBUG_OUTPUT
//			printf("Lock \r\n");
//#endif
//		}
//		else
//		{
//#ifdef DEGBUG_OUTPUT
//			printf("Not Lock \r\n");
//#endif
//		}
//		NeedSendLock = false;
//	}
//}



void RF_task() // alarm on/off by tag, do not lock at home,  on/off depend on park, 
{









	if (Park)
	{

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
			RF_Command(RF_COMMAND_POWER_OFF, 10);
			LastOn = false;
			Locked = false;
			NeedLock = true;
			printf("Turn OFF when park \r\n");
		}
		else
		{
			if ((SecondsSinceStart - LastTagGetTime > TIME_OUT_LOCK) 
				&& (LastTagGetTime != 0)
				&&(!Locked)
				&&NeedLock
				)
			{
				if (!Home)
				{
					delay(1000);
					RF_Command(RF_COMMAND_LOCK, 10);
					Locked = true;
					printf("Lock not at home \r\n");
				} 
				else
				{
					NeedLock = false;
					printf("Do not Lock at home \r\n");
				}

			}

			if ((SecondsSinceStart - LastTagGetTime < TIME_OUT_LOCK) && (LastTagGetTime != 0)&&Locked)
			{
				RF_Command(RF_COMMAND_POWER_OFF, 10);
				NeedLock = true;
				Locked = false;
				printf("Unlock \r\n");
			}
		}
	} 
	else
	{
		if((SecondsSinceStart - LastTagGetTime < TIME_OUT_LOCK) && (LastTagGetTime != 0)&&(!LastOn))
		{
			printf("Turn ON \r\n");
			RF_Command(RF_COMMAND_POWER_ON, 10);
			delay(300);
			RF_Command(RF_COMMAND_POWER_ON, 10);
			LastOn = true;
			Locked = false;
		}


		if ((SecondsSinceStart - LastHomeGetTime < TIME_OUT_LOCK) && (LastHomeGetTime != 0))
		{
			Home = true;
		}
		else
		{
			if((Home)&&(!LastOn)&&(!Locked))
			{
				RF_Command(RF_COMMAND_LOCK, 10);
				Locked = true;
#ifdef DEGBUG_OUTPUT
				printf("Lock! leaving home without power on \r\n");
#endif
			}
			Home = false;
		}
	}
}

//void OnKeyPress()
//{
//	if (LastOn)
//	{
//		RF_Command(RF_COMMAND_POWER_OFF,10);//Unlock, powerOFF
//		LastOn = false;
//		Locked = false;
//		Auto = false;
//
//#ifdef DEGBUG_OUTPUT
//		printf("Turn OFF manually \r\n");
//#endif
//	}
//	else
//	{
//		//powerON
//		RF_Command(RF_COMMAND_POWER_ON,10);
//		delay(300);
//		RF_Command(RF_COMMAND_POWER_ON,10);
//		LastOn = true;
//		Locked = false;
//		Auto = true;
//
//#ifdef DEGBUG_OUTPUT
//		printf("Turn ON manually \r\n");
//#endif
//	}
//}

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
			if (millis() - LastKeyGetTime > 2000)
			{
				LastKeyGetTime = millis();
				//OnKeyPress();
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
	}
}

void RF_Command(unsigned char command, unsigned char repeat)
{
	unsigned char k;             //bit
	unsigned char j;             //repeat
	unsigned char i;             //byte

	bool LastBit;

	for (j = 0; j < repeat; j++)
	{
		for (i = 0; i < RF_LENGTH; i++)
		{
			for (k = 0; k < 8; k++)
			{
				if ((RfCommand[command][i] >> (7 - k)) & 1)
				{
					digitalWrite(RF_315, HIGH);
					if (LastBit)
					{
						delayMicroseconds(385 * 2);
					}
					else
					{
						delayMicroseconds(385);
					}

					LastBit = true;
				}
				else
				{
					digitalWrite(RF_315, LOW);
					if (LastBit)
					{
						delayMicroseconds(385);
					}
					else
					{
						delayMicroseconds(385 * 2);
					}

					LastBit = false;
				}
			}
		}
		delay(12);
	}
	digitalWrite(RF_315, LOW);

}

unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 1000)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;
		OnSecond();
		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
	}
}
