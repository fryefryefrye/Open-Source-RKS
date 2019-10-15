
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx


#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 2000				//set lenght of the buzz 2000
#define BUZZOFF 2000			//set interval of the buzz 5000
#define DEGBUG_OUTPUT


#define TIME_OUT_KEY 100  //0.1s
#define TIME_OUT_READY 300  //0.1s

#define FIX_DISTANCE_IN 800
#define FIX_DISTANCE_OUT 1051
#define DISTANCE_DIFF 200




#include <SoftwareSerial.h>
SoftwareSerial SoftSerialOut(5, 4); //rx ,tx 
SoftwareSerial SoftSerialIn(3, 2); //rx ,tx 

//D7,D8 for RF24
#define BUZZ			9
#define CLOSED			10

#define RELAY_OPEN		A2
#define RELAY_CLOSE		A0
#define RELAY_UNLOCK	A1
#define RELAY_LOCK		A3
#define KEY_OUT1		A4
#define KEY_OUT2		A5
#define KEY_IN			A6
#define KEY_xx			A7

//A6 A7 are input only

unsigned int DistanceIn = 0;
unsigned int DistanceOut = 0;
bool isBodyExistIn = false;
bool isBodyExistOut = false;

bool DoorOpening = false;
bool DoorCloseing = false;
bool DoorLocking = false;
bool DoorUnlocking = false;

bool StopOpening = false;
bool StopCloseing = false;
bool StopLocking = false;
bool StopUnlocking = false;


bool bWaitAutoClose = false;
unsigned long DoorOpenedTime;
#define AUTO_CLOSE_MAX_WAIT 300

enum From{IN,OUT};
From eOpenFrom;


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

unsigned long LastReadyOutGetTime = 0;
unsigned long LastTagGetTime = 0;
bool Tag = false;
bool TagHouXiang = false;
unsigned long LastTagHouXiangGetTime = 0;
bool ReadyOut = false;


unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long TenthSecondsSinceStart;

bool bLastDoorClosed;






void SecondsSinceStartTask();
void nRFTask();
void ChHopTask();
void RF_task();
void Buzz_task();
void CheckTag_task();
void OnTenthSecond();
void GetDistance_task();
bool CheckBodyLeft(From eWhichBody);
bool CheckBodyCome(From eWhichBody);
void CheckKey_task();
void KeyPressed(unsigned char KeyIndex);
bool KeyState(unsigned char KeyIndex);



void UnlockRelay(bool on);
void LockRelay(bool on);
void OpenRelay(bool on);
void CloseRelay(bool on);
bool IsDoorClosed();


void CheckDoor_task();
void DoorOpen_Task();
void DoorClose_Task();
void DoorLock_Task();
void DoorUnlock_Task();
void AutoClose_task();

void setup()
{

	pinMode(RELAY_LOCK, OUTPUT);
	pinMode(RELAY_UNLOCK, OUTPUT);
	pinMode(RELAY_OPEN, OUTPUT);
	pinMode(RELAY_CLOSE, OUTPUT);


	pinMode(BUZZ, OUTPUT);

	digitalWrite(RELAY_LOCK,HIGH);
	digitalWrite(RELAY_UNLOCK,HIGH);
	digitalWrite(RELAY_OPEN,HIGH);
	digitalWrite(RELAY_CLOSE,HIGH);

	SoftSerialOut.begin(9600);
	SoftSerialIn.begin(9600);


	//digitalWrite(RELAY_LOCK,LOW);
	//digitalWrite(RELAY_UNLOCK,LOW);
	//digitalWrite(RELAY_OPEN,LOW);
	//digitalWrite(RELAY_CLOSE,LOW);

	digitalWrite(BUZZ,HIGH);


	//pinMode(KEY_OUT1, INPUT_PULLUP);
	//pinMode(KEY_OUT2, INPUT_PULLUP);
	pinMode(KEY_OUT1, INPUT);
	pinMode(KEY_OUT2, INPUT);
	pinMode(KEY_IN, INPUT);
	pinMode(KEY_xx, INPUT);





#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_AutoDoor"));
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
	ChHopTask();
	Buzz_task();
	GetDistance_task();
} // Loop




void OnTenthSecond()
{
	CheckKey_task();
	CheckTag_task();
	CheckDoor_task();
	DoorOpen_Task();
	DoorClose_Task();
	DoorLock_Task();
	DoorUnlock_Task();
	AutoClose_task();



}
void AutoClose_task()
{

	if (bWaitAutoClose)
	{

		if (eOpenFrom == OUT)
		{
			if(CheckBodyCome(IN))
			{
				DoorCloseing = true;
				bWaitAutoClose = false;
				printf("Auto Door close.From out, Body IN  \r\n");
			}
		}

		if (eOpenFrom == IN)
		{
			if(CheckBodyCome(OUT))
			{
				DoorCloseing = true;
				bWaitAutoClose = false;
				printf("Auto Door close.From IN, Body OUT  \r\n");
			}
		}
		
		//if (TenthSecondsSinceStart - DoorOpenedTime < AUTO_CLOSE_MAX_WAIT)
		//{
		//	if (eOpenFrom == IN)
		//	{
		//		if (isBodyExistOut))
		//		{
		//			printf("Auto Door close \r\n");
		//			DoorLocking = true;
		//			bWaitAutoClose= false;
		//		}
		//	} 

		//} 
		//else
		//{
		//	bWaitAutoClose= false;
		//}
	} 

}

void CheckDoor_task()
{


	if (IsDoorClosed()!=bLastDoorClosed)
	{
		bLastDoorClosed = IsDoorClosed();
		//Buzz++;
		printf("Door close change to %d \r\n",bLastDoorClosed);

		if (bLastDoorClosed)
		{
			//DoorLocking = true;
		}
	}

	//printf("Door close change to %d \r\n",bLastDoorClosed);

}

void CheckTag_task()
{

	if (TenthSecondsSinceStart - LastTagGetTime < TIME_OUT_KEY)
	{
		if (Tag == false)
		{
			printf("Get key \r\n");
		}
		Tag = true;
	} 
	else
	{
		if (Tag == true)
		{
			printf("Lost key \r\n");
		}
		Tag = false;
	}

	if (TenthSecondsSinceStart - LastTagHouXiangGetTime < TIME_OUT_KEY)
	{
		if (TagHouXiang == false)
		{
			printf("Get TagHouXiang \r\n");
		}
		TagHouXiang = true;
	} 
	else
	{
		if (TagHouXiang == true)
		{
			printf("Lost TagHouXiang \r\n");
		}
		TagHouXiang = false;
	}



	if (TenthSecondsSinceStart - LastReadyOutGetTime < TIME_OUT_READY)
	{
		if (ReadyOut == false)
		{
			printf("Get ReadyOut \r\n");
		}
		ReadyOut = true;
	} 
	else
	{
		if (ReadyOut == true)
		{
			printf("Lost ReadyOut \r\n");
		}
		ReadyOut = false;
	}
}
bool CheckBodyCome(From eWhichBody)
{
	static bool bLastBodyCheckIn;
	static bool bLastBodyCheckOut;
	if (eWhichBody == IN)
	{
		if (isBodyExistIn != bLastBodyCheckIn)
		{
			bLastBodyCheckIn = isBodyExistIn;
			printf("Body check in change to %d. timestamp = %d \r\n",isBodyExistIn,TenthSecondsSinceStart);
			if (isBodyExistIn)
			{
				return true;
			}

		}
	}


	if (eWhichBody == OUT)
	{
		if (isBodyExistOut != bLastBodyCheckOut)
		{
			bLastBodyCheckOut = isBodyExistOut;
			printf("Body check Out change to %d .timestamp = %d\r\n",isBodyExistOut,TenthSecondsSinceStart);
			if (isBodyExistOut)
			{
				return true;
			}
		}
	}
	return false;
}

bool CheckBodyLeft(From eWhichBody)
{
	static bool bLastBodyCheckIn;
	static bool bLastBodyCheckOut;
	if (eWhichBody == IN)
	{
		if (isBodyExistIn != bLastBodyCheckIn)
		{
			bLastBodyCheckIn = isBodyExistIn;
			printf("Body check in change to %d. timestamp = %d \r\n",isBodyExistIn,TenthSecondsSinceStart);
			if (!isBodyExistIn)
			{
				return true;
			}

		}
	}


	if (eWhichBody == OUT)
	{
		if (isBodyExistOut != bLastBodyCheckOut)
		{
			bLastBodyCheckOut = isBodyExistOut;
			printf("Body check Out change to %d .timestamp = %d\r\n",isBodyExistOut,TenthSecondsSinceStart);
			if (!isBodyExistOut)
			{
				return true;
			}
		}
	}
	return false;
}
void GetDistance_task()
{
	static unsigned char Step = 1;
	static unsigned long LastCheckMillis;

	static unsigned char BodyDisappearCounterIn = 0;
	static unsigned char BodyDisappearCounterOut = 0;



	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastCheckMillis) > 25)
	{
		LastCheckMillis = CurrentMillis;

		switch(Step)
		{
		case 0:
			SoftSerialOut.listen();
			SoftSerialOut.write(0x55);
			Step++;
			break;
		case 1:

			if (SoftSerialOut.read() == 0xFF)
			{
				DistanceOut = SoftSerialOut.read()<<8;
				DistanceOut = DistanceOut + SoftSerialOut.read();
				SoftSerialOut.read();
				//printf("DistanceOut = %d mm. \r\n",DistanceOut);
			}
			if((DistanceOut < FIX_DISTANCE_OUT - DISTANCE_DIFF)||(DistanceOut > FIX_DISTANCE_OUT + DISTANCE_DIFF))
			{
				isBodyExistOut = true;
				BodyDisappearCounterOut = 0;
				printf("DistanceOut = %d mm. \r\n",DistanceOut);
			}
			else
			{
				if (BodyDisappearCounterOut>3)
				{
					isBodyExistOut = false;
				}
				else
				{
					BodyDisappearCounterOut++;
				}
			}
			Step++;
			break;
		case 2:
			SoftSerialIn.listen();
			SoftSerialIn.write(0x55);
			Step++;
			break;
		case 3:
			if (SoftSerialIn.read() == 0xFF)
			{
				DistanceIn = SoftSerialIn.read()<<8;
				DistanceIn = DistanceIn + SoftSerialIn.read();
				SoftSerialIn.read();
			}
			Step = 0;
			if((DistanceIn < FIX_DISTANCE_IN - DISTANCE_DIFF)||(DistanceIn > FIX_DISTANCE_IN + DISTANCE_DIFF))
			{
				isBodyExistIn = true;
				BodyDisappearCounterIn = 0;
				printf("DistanceIn = %d mm. \r\n",DistanceIn);
			}
			else
			{
				if (BodyDisappearCounterIn>10)
				{
					isBodyExistIn = false;
				}
				else
				{
					BodyDisappearCounterIn++;
				}
			}

			//printf("DistanceOut = %d mm.DistanceIn = %d mm. \r\n",DistanceOut,DistanceIn);

			break;

		default:

			Step = 0;
			break;
		}
	}
}

#define KEY_PRESS_MIN 2
#define PAUSE_KEY_PRESS 4
void CheckKey_task()
{
	static unsigned char KeyCounter[4] = {0,0,0,0};
	static unsigned char PauseKeyCounter = PAUSE_KEY_PRESS;


	if (PauseKeyCounter>0)
	{
		PauseKeyCounter--;
		return;
	} 

	for (char i = 0; i < 4; i++)
	{
		if (KeyState(i))
		{

			KeyCounter[i]++;
			if (KeyCounter[i] > KEY_PRESS_MIN)
			{
				KeyCounter[i] = 0;
				PauseKeyCounter = PAUSE_KEY_PRESS;
				//Buzz = i+1;
				printf("Key %d press!  \r\n",i);
				KeyPressed(i);
			}
		}
		else
		{
			KeyCounter[i] = 0;
		}
	}	
}

void KeyPressed(unsigned char KeyIndex)
{
	switch(KeyIndex)
	{
	case 0:
		LastReadyOutGetTime = TenthSecondsSinceStart;
		break;
	case 1:
		printf("Key Out! \r\n");
		//if (Tag&&ReadyOut&&IsDoorClosed())
		//*if (Tag&&bBodyCheckOut&&IsDoorClosed())*/

		if (TagHouXiang&&IsDoorClosed())
		{
			Buzz = 1;
			DoorOpening = true;
			bWaitAutoClose = true;
			eOpenFrom = OUT;
			printf("open from Out! \r\n");
		}
		else if (Tag&&IsDoorClosed())
		{
			Buzz = 1;
			DoorUnlocking = true;
			//eOpenFrom = OUT;
			printf("unlock from Out! \r\n");
		}
		else
		{
			Buzz = 3;
		}



		break;
	case 2:
		printf("Key In! \r\n");
		if (IsDoorClosed())
		{
			printf("open from In! \r\n");
			DoorOpening = true;
			eOpenFrom = IN;
			bWaitAutoClose = true;
			Buzz = 10;
		}
		else
		{
			printf("close from In! \r\n");
			DoorCloseing = true;
		}
		//else
		//{	
		//	if (DoorOpening)
		//	{
		//		StopOpening = true;
		//	}
		//	DoorLocking = true;
		//}

		//OpenRelay(true);
		//delay(3000);
		//OpenRelay(false);
		//delay(500);
		//CloseRelay(true);
		//delay(500);
		//CloseRelay(false);
		//delay(500);


		break;
	case 3:
		//if (DoorOpening)
		//{
		//	printf("STOP when opening \r\n");
		//	StopOpening = true;
		//}
		//DoorLocking = true;


		break;
	}
}

bool KeyState(unsigned char KeyIndex)
{
	switch(KeyIndex)
	{
	case 0:
		return digitalRead(KEY_OUT1);
		break;
	case 1:
		return digitalRead(KEY_OUT2);
		break;
	case 2:
		return (analogRead(KEY_IN)>900);
		break;
	case 3:
		return (analogRead(KEY_xx)>900);
		break;
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

			if (GotData[1] == 3)
			{
				LastTagHouXiangGetTime = TenthSecondsSinceStart;
			}
		}
		//		else if (GotData[0] == 1)
		//		{
		//#ifdef DEGBUG_OUTPUT
		//			printf("LastHomeGetTime offset = %d \r\n", TenthSecondsSinceStart - LastHomeGetTime);
		//#endif
		//			LastHomeGetTime = TenthSecondsSinceStart;
		//		}
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
		printf("Volt:%d ", (int)Volt);
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
				digitalWrite(BUZZ, HIGH);
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
				digitalWrite(BUZZ, LOW);
			}
		}
	}
	else
	{
		//if (TagHouXiang&&(isBodyExistOut||isBodyExistIn))
		//{
		//	digitalWrite(BUZZ, LOW);
		//}
		//else
		//{
		//	digitalWrite(BUZZ, HIGH);
		//}
	}
}


void DoorOpen_Task()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (DoorOpening)
	{
		if (!StopOpening)
		{
			if (Step == 0)
			{
				printf("DoorOpening start.\r\n");
				printf("Unlocking.\r\n");
				Step = 1;
				StartTime = TenthSecondsSinceStart;
				UnlockRelay(true);
			}
			if (Step == 1)
			{
				if(TenthSecondsSinceStart -StartTime > 3)//Unlock for 200ms
				{
					OpenRelay(true);
					Step ++;
					StartTime = TenthSecondsSinceStart;
				}
			}
			//if (Step == 2)
			//{
			//	if(TenthSecondsSinceStart -StartTime > 6)//open for 200ms
			//	{
			//		OpenRelay(false);
			//		Step ++;
			//		StartTime = TenthSecondsSinceStart;
			//	}
			//}
			//if (Step == 3)
			//{
			//	if(TenthSecondsSinceStart -StartTime > 0)//pause open for 200ms
			//	{
			//		OpenRelay(true);
			//		Step ++;
			//		StartTime = TenthSecondsSinceStart;
			//	}
			//}
			if (Step == 2)
			{
				if(TenthSecondsSinceStart -StartTime > 6)//open for 200ms
				{
					OpenRelay(false);
					UnlockRelay(false);
					Step = 0;
					DoorOpening = false;
					printf("DoorOpening finished\r\n");
				}
			}

		}
		else
		{
			OpenRelay(false);
			UnlockRelay(false);
			StopOpening = false;
			Step = 0;
			DoorOpening = false;
			printf("DoorOpening canceled.\r\n");
		}
	} 
}


void DoorClose_Task()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (DoorCloseing)
	{
		if (!StopCloseing)
		{
			//if (bLastDoorClosed)
			//{
			//	StopCloseing = true;
			//}

			if (Step == 0)
			{
				printf("DoorCloseing start.\r\n");
				Step = 1;
				StartTime = TenthSecondsSinceStart;
				CloseRelay(true);
			}
			if (Step == 1)
			{
				if(TenthSecondsSinceStart -StartTime > 10)//Close for n*100ms
				{
					CloseRelay(false);
					Step ++;
					StartTime = TenthSecondsSinceStart;
				}
			}
			if (Step == 2)
			{
				if(TenthSecondsSinceStart -StartTime > 20)//pause for n*100ms
				{
					CloseRelay(true);
					Step ++;
					StartTime = TenthSecondsSinceStart;
				}
			}			
			//if (Step == 3)
			//{
			//	if(TenthSecondsSinceStart -StartTime > 0)//Close for n*100ms
			//	{
			//		CloseRelay(false);
			//		Step ++;
			//		StartTime = TenthSecondsSinceStart;
			//	}
			//}
			//if (Step == 4)
			//{
			//	if(TenthSecondsSinceStart -StartTime > 0)//pause for n*100ms
			//	{
			//		CloseRelay(true);
			//		Step ++;
			//		StartTime = TenthSecondsSinceStart;
			//	}
			//}
			//if (Step == 5)
			//{
			//	if(TenthSecondsSinceStart -StartTime > 0)//Close for n*100ms
			//	{
			//		CloseRelay(false);
			//		Step ++;
			//		StartTime = TenthSecondsSinceStart;
			//	}
			//}
			//if (Step == 6)
			//{
			//	if(TenthSecondsSinceStart -StartTime > 0)//pause for n*100ms
			//	{
			//		CloseRelay(true);
			//		Step ++;
			//		StartTime = TenthSecondsSinceStart;
			//	}
			//}
			if (Step == 3)
			{
				if(TenthSecondsSinceStart -StartTime > 5)//Close for n*100ms
				{
					CloseRelay(false);
					Step = 0;
					DoorCloseing = false;
					printf("DoorCloseing finished\r\n");
				}
			}

		}
		else
		{
			CloseRelay(false);
			StopCloseing = false;
			Step = 0;
			DoorCloseing = false;
			printf("DoorCloseing canceled.\r\n");
		}
	} 
}

void DoorUnlock_Task()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (DoorUnlocking)
	{
		if (!StopUnlocking)
		{
			if (Step == 0)
			{
				printf("DoorUnlocking start.\r\n");
				Step = 1;
				StartTime = TenthSecondsSinceStart;
				UnlockRelay(true);
			}
			if (Step == 1)
			{
				if(TenthSecondsSinceStart -StartTime > 30)
				{
					UnlockRelay(false);
					Step = 0;
					DoorUnlocking = false;
					printf("Door unlock finished.\r\n");
				}
			}
		}
		else
		{
			UnlockRelay(false);
			StopUnlocking = false;
			Step = 0;
			DoorUnlocking = false;
			printf("DoorUnlocking canceled.\r\n");
		}
	} 
}

void DoorLock_Task()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (DoorLocking)
	{
		if (IsDoorClosed())
		{
			if (Step == 0)
			{
				printf("DoorLocking start.\r\n");
				Step = 1;
				StartTime = TenthSecondsSinceStart;
			}
			if (Step == 1)
			{
				if(TenthSecondsSinceStart -StartTime > 20)
				{
					LockRelay(true);
					Step = 2;
					StartTime = TenthSecondsSinceStart;
					printf("Locking.\r\n");
				}
			}
			if (Step == 2)
			{
				if(TenthSecondsSinceStart -StartTime > 5)
				{
					LockRelay(false);
					Step = 0;
					DoorLocking = false;
					printf("DoorLocking finished.\r\n");
				}
			}

		}
		else
		{
			LockRelay(false);
			StopLocking = false;
			Step = 0;
			DoorLocking = false;
			printf("DoorLocking canceled.\r\n");
		}
	} 
}


void UnlockRelay(bool on)
{
	digitalWrite(RELAY_UNLOCK,!on);
	printf("Unlock Relay to %d .\r\n",on);
}
void LockRelay(bool on)
{
	digitalWrite(RELAY_LOCK,!on);
	printf("Lock Relay to %d .\r\n",on);
}

void OpenRelay(bool on)
{
	digitalWrite(RELAY_OPEN,!on);
	printf("Open Relay to %d .\r\n",on);
}
void CloseRelay(bool on)
{
	digitalWrite(RELAY_CLOSE,!on);
	printf("Close Relay to %d .\r\n",on);
}

bool IsDoorClosed()
{
	return !digitalRead(CLOSED);
}
