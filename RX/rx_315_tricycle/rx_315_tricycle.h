
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
//#define TIME_OUT_TURN_OFF_BIKE 90		//s
#define TIME_OUT_LOCK 300  //s
//#define WAIT_KEY_IN_HOME_INTERVAL 10
//#define TIME_OUT_HOME 60
#define DATA_LENGTH 4					//use fixed data length 1-32
//#define BUZZON 1000				//set lenght of the buzz
//#define BUZZOFF 30000			//set interval of the buzz
#define DEGBUG_OUTPUT
#define SWITCH_TIME 10  //tenth s
#define LOCK_MOTO_MAX_TIME 25  //tenth s
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
unsigned long TenthSecondsSinceStart;


static bool LastLockMotoSwitch;
static bool LastLockStateSwitch;


bool LastOn = false;
bool Locked = false;
//bool Auto = true;
bool NeedLock = false;
//unsigned long TrunOffTime = 0;
bool MainOnOff = false;
//bool Home = false;

#define LOCK_STATE 3
#define LOCK_MOTO 4
#define SWITCH 5
#define RF_315 6
//D7,D8 for RF24
#define BUZZ 9
#define RELAY 10





#define RF_LENGTH 11
unsigned char  RfCommand[3][RF_LENGTH]={
	//这组记录未反转，需要输出脚高低反转
	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB2 ,0xDB ,0x7F},
	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB6 ,0x5B ,0x7F},
	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB6 ,0xCB ,0x7F}
};


//录音后反转，记录高低。
//unsigned char RfCommand[3][RF_LENGTH] = {
//	{ 0x00, 0x9A, 0x49, 0x36, 0xDA, 0x4D, 0xA4, 0x9A, 0x6D, 0x24, 0x80 },   //L
//	{ 0x00, 0x9A, 0x49, 0x36, 0xDA, 0x4D, 0xA4, 0x9A, 0x69, 0xA4, 0x80 },   //U
//	{ 0x00, 0x9A, 0x49, 0x36, 0xDA, 0x4D, 0xA4, 0x9A, 0x69, 0x26, 0x80 }   //P
//};

#define RF_COMMAND_LOCK 0
#define RF_COMMAND_POWER_OFF 1
#define RF_COMMAND_POWER_ON 2


void RF_Command(unsigned char command, unsigned char repeat);
void SecondsSinceStartTask();
void OnTenthSecond();
void nRFInit();
void nRFTask();
void ChHopTask();
void RF_task();
void ShowLockState_task();
void InitLock();
void UnLock();
bool IsLock();
void LockMotoOn();
void LockMotoOff();
bool GetSwitchState(unsigned char Switch);
void CheckLock();
void CheckPush();
//void OnKeyPress();

void setup()
{
	pinMode(RF_315, OUTPUT);
	pinMode(RELAY, OUTPUT);
	pinMode(BUZZ, OUTPUT);


	LockMotoOff();
	digitalWrite(RF_315, LOW);

	pinMode(LOCK_STATE, INPUT_PULLUP);
	pinMode(LOCK_MOTO, INPUT_PULLUP);
	pinMode(SWITCH, INPUT_PULLUP);






	LastLockMotoSwitch = digitalRead(LOCK_MOTO);
	LastLockStateSwitch = digitalRead(LOCK_STATE);


#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_315_TRICYCLE_ID_Read"));
	printf_begin();
#endif

	//wdt_enable(WDTO_2S);

	nRFInit();



	//InitLock();
	//UnLock();



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

void OnTenthSecond()
{
	CheckPush();
	CheckLock();



#ifdef DEGBUG_OUTPUT
	//printf("SwitchState = %d ,main = %d, UnChangeCounter = %d \r\n",SwitchState,MainOnOff,UnChangeCounter);
#endif




	if (TenthSecondsSinceStart%50 == 0)
	{
		nRFInit();
#ifdef DEGBUG_OUTPUT
		//printf("re init nRF \r\n");
#endif

	}

}

void CheckLock()
{
	static unsigned char UnChangeCounter;
	bool Locked;

	if (LastOn)
	{
		if (IsLock()) //push postion
		//if (digitalRead(LOCK_STATE)) 
		{
			UnChangeCounter++;

			if (UnChangeCounter%2 == 0)
			{
				digitalWrite(BUZZ, HIGH);
			} 
			if (UnChangeCounter%2 == 1)
			{
				digitalWrite(BUZZ, LOW);
			} 
			if (UnChangeCounter >= SWITCH_TIME)
			{

				MainOnOff = false;
				digitalWrite(BUZZ, HIGH);
				delay(500);
				digitalWrite(BUZZ, LOW);
				delay(200);
				digitalWrite(BUZZ, HIGH);
				delay(500);
				digitalWrite(BUZZ, LOW);
				delay(200);
				digitalWrite(BUZZ, HIGH);
				delay(500);
				digitalWrite(BUZZ, LOW);
				printf("Lock Confirm, to off \r\n",MainOnOff);
				UnChangeCounter = 0;
			} 
		} 
		else
		{
			UnChangeCounter = 0;
			//digitalWrite(BUZZ, LOW);
		}

	}



}

void CheckPush()
{
	static unsigned char UnChangeCounter;
	bool SwitchState;
	//static bool LastSwitchState;

	SwitchState = !(digitalRead(SWITCH));

	if (SwitchState) //push postion
	{
		UnChangeCounter++;

		if (UnChangeCounter%2 == 0)
		{
			digitalWrite(BUZZ, HIGH);
		} 
		if (UnChangeCounter%2 == 1)
		{
			digitalWrite(BUZZ, LOW);
		} 
		if (UnChangeCounter >= SWITCH_TIME)
		{
			if((TenthSecondsSinceStart - LastTagGetTime < TIME_OUT_LOCK) && (LastTagGetTime != 0)&&(!MainOnOff))
			{
				MainOnOff = true;
				digitalWrite(BUZZ, HIGH);
				delay(1500);
				digitalWrite(BUZZ, LOW);
			}
			else if (MainOnOff)
			{
				MainOnOff = false;
				digitalWrite(BUZZ, HIGH);
				delay(500);
				digitalWrite(BUZZ, LOW);
				delay(200);
				digitalWrite(BUZZ, HIGH);
				delay(500);
				digitalWrite(BUZZ, LOW);
				delay(200);
				digitalWrite(BUZZ, HIGH);
				delay(500);
				digitalWrite(BUZZ, LOW);
			}
			printf("Switch Confirm,  MainOnOff to %d \r\n",MainOnOff);
			UnChangeCounter = 0;
		} 
	} 
	else
	{
		UnChangeCounter = 0;
		digitalWrite(BUZZ, LOW);
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




void RF_task() // alarm on/off by tag, do not lock at home,  on/off depend on park, 
{
	if (!MainOnOff)//to off
	{
		if (LastOn)
		{
			RF_Command(RF_COMMAND_POWER_OFF, 20);
			delay(300);
			RF_Command(RF_COMMAND_POWER_OFF, 20);
			LastOn = false;
			Locked = false;
			NeedLock = true;
			printf("Turn OFF  \r\n");
		}
		else
		{
			if ((TenthSecondsSinceStart - LastTagGetTime > TIME_OUT_LOCK) 
				&& (LastTagGetTime != 0)
				&&(!Locked)
				&&NeedLock
				)
			{

				delay(1000);
				RF_Command(RF_COMMAND_LOCK, 20);
				Locked = true;
				printf("Lock \r\n");


			}

			if ((TenthSecondsSinceStart - LastTagGetTime < TIME_OUT_LOCK) && (LastTagGetTime != 0)&&Locked)
			{
				RF_Command(RF_COMMAND_POWER_OFF, 20);
				delay(300);
				RF_Command(RF_COMMAND_POWER_OFF, 20);
				NeedLock = true;
				Locked = false;
				printf("Unlock \r\n");
			}
		}
	} 
	else//to on
	{
		if(!LastOn)
		{
			printf("Turn ON \r\n");

			UnLock();

			RF_Command(RF_COMMAND_POWER_ON, 20);
			delay(300);
			RF_Command(RF_COMMAND_POWER_ON, 20);
			delay(300);
			RF_Command(RF_COMMAND_POWER_ON, 20);
			delay(300);
			RF_Command(RF_COMMAND_POWER_ON, 20);
			LastOn = true;
			Locked = false;
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
					digitalWrite(RF_315, LOW);
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
					digitalWrite(RF_315,HIGH );
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
	if (abs(CurrentMillis - LastMillis) > 100)
	{
		LastMillis = CurrentMillis;
		TenthSecondsSinceStart++;
		OnTenthSecond();
		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
	}
}

void ShowLockState_task()
{
	bool LockMotoSwitch;
	bool LockStateSwitch;
	static unsigned char i = 0;


	LockMotoSwitch = GetSwitchState(LOCK_MOTO);
	if (LastLockMotoSwitch != LockMotoSwitch)
	{
		LastLockMotoSwitch = LockMotoSwitch;
		i++;
		printf("%d LockMotoSwitch changed to: %d \r\n",i,LastLockMotoSwitch);
	} 

	LockStateSwitch = GetSwitchState(LOCK_STATE);
	if (LastLockStateSwitch != LockStateSwitch)
	{
		LastLockStateSwitch = LockStateSwitch;
		i++;
		printf("%d LockStateSwitch changed to: %d \r\n",i,LastLockStateSwitch);
	} 
}


void InitLock()
{
	//if (!digitalRead(LOCK_MOTO))
	//{
	//	return;
	//}
	LockMotoOn();
	delay(300);
	while (!digitalRead(LOCK_MOTO))
	{
	}
	LockMotoOff();
}
void UnLock()
{
	unsigned long MotoOnTenthSeconds = TenthSecondsSinceStart;
	if(IsLock())
	{
		LockMotoOn();
		while (GetSwitchState(LOCK_STATE))
		{
			SecondsSinceStartTask();
			if(TenthSecondsSinceStart - MotoOnTenthSeconds>LOCK_MOTO_MAX_TIME) break;
		}
		printf("UnLock OK  \r\n");

		while (!GetSwitchState(LOCK_MOTO))
		{
			SecondsSinceStartTask();
			if(TenthSecondsSinceStart - MotoOnTenthSeconds>LOCK_MOTO_MAX_TIME) break;
		}
		printf("reset moto 1  \r\n");

		while (GetSwitchState(LOCK_MOTO))
		{
			SecondsSinceStartTask();
			if(TenthSecondsSinceStart - MotoOnTenthSeconds>LOCK_MOTO_MAX_TIME) break;
		}
		printf("reset moto 2  \r\n");


		while (!GetSwitchState(LOCK_MOTO))
		{
			SecondsSinceStartTask();
			if(TenthSecondsSinceStart - MotoOnTenthSeconds>LOCK_MOTO_MAX_TIME) break;
		}
		printf("reset moto 3  \r\n");

		printf("Unlock time = %d  \r\n",TenthSecondsSinceStart - MotoOnTenthSeconds);

		LockMotoOff();
	}
	else
	{
		printf("Already unlocked  \r\n");
	}


}
bool IsLock()
{
	return (GetSwitchState(LOCK_STATE));
}

void LockMotoOff()
{
	digitalWrite(RELAY, LOW);
}
void LockMotoOn()
{
	digitalWrite(RELAY, HIGH);
}

bool GetSwitchState(unsigned char Switch)
{
#define AVG 100
	unsigned char i;
	unsigned char Key = 0;
	//bool State;

	for (i=0;i<AVG;i++)
	{
		//State = digitalRead(Switch);
		//printf("key type = %d  State = %d  \r\n",Switch,State);
		if (digitalRead(Switch))
		{
			Key ++;
		}

	}

	//printf("key type = %d  value = %d  \r\n",Switch,Key);

	if (Key > (AVG/2))
	{
		return true;
	} 
	else
	{
		return false;
	}
}