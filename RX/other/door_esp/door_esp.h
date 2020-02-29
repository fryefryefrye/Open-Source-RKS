




//D0 = GPIO16; 
//D1 = GPIO5; 
//D2 = GPIO4;	LED on esp8266
//D3 = GPIO0;can not download when connected to low
//D4 = GPIO2;	
//D5 = GPIO14;  
//D6 = GPIO12;
//D7 = GPIO13;
//D8 = GPIO15;  can not start when high input
//D9 = GPIO3; UART RX
//D10 = GPIO1; UART TX
//LED_BUILTIN = GPIO16 (auxiliary constant for the board LED, not a board pin);


#define RELAY_OPEN	D0
#define RELAY_CLOSE	D4
#define SDA		D1
#define SCL		D2
#define INT1	D5
#define INT2	D7

#define RF_IN				D6
ICACHE_RAM_ATTR void DecodeRf_INT();

unsigned char RcCommand[3] = {0,0,0};
bool DecodeFrameOK = false;
void CheckRf();
void CheckRfCommand(unsigned char * RfCommand);
bool RfInitialized = false;

#define RF_COMMAND_LEN 3
#define RF_COMMAND_KEY_COUNTER 5
#define RF_COMMAND_FUNCTION_COUNTER 4
unsigned char PreSetRfCommand[RF_COMMAND_FUNCTION_COUNTER][RF_COMMAND_KEY_COUNTER][RF_COMMAND_LEN]
={

	{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	//key Out
	,{{0xD3, 0x73, 0x30},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	//In
	,{{0xD3, 0x73, 0x0C},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
};



#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8


#include <Wire.h>

const char* ssid = "frye_iot";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 24;
tKeyLessData KeyLessData;


#define DOOR_POSRION_NUMBER 50
bool isIntDataProcessing = false;
unsigned long Int1_LastMicros = 0;
unsigned long Int2_LastMicros = 0;
unsigned long Int1_DiffMicros = 0;
unsigned long Int2_DiffMicros = 0;
unsigned int Speed = 0;
int Postion = 0;
bool Direction; //True for open
unsigned int OpenSpeed[DOOR_POSRION_NUMBER];
unsigned int CloseSpeed[DOOR_POSRION_NUMBER];
unsigned int MaxSpeed;
unsigned int MinSpeed;
unsigned int OpenSpeedRef[DOOR_POSRION_NUMBER] = 
{
	357
	,317
	,286
	,266
	,250
	,234
	,210
	,211
	,204
	,196
	,192
	,185
	,181
	,179
	,165
	,166
	,165
	,166
	,163
	,169
	,178
	,196
	,227
	,278
	,387
	,775
	,775
	,619
	,544
	,486
	,453
	,432
	,420
	,419
	,411
	,407
	,424
	,457
	,589
	,929

};
unsigned int CloseSpeedRef[DOOR_POSRION_NUMBER] = 
{
	1256
	,359+20
	,272+30
	,227+40
	,199+50
	,181+50
	,166+50
	,152+50
	,138+50
	,139+50
	,132
	,130
	,129
	,132
	,133
	,134
	,137
	,141
	,146
	,147
	,151
	,155
	,162
	,165
	,168
	,176
	,184
	,190
	,202
	,213
	,220
	,241
	,249
	,276
	,309
	,323
	,357
	,412
	,471
	,574
};
//
//unsigned int CloseSpeedRef[DOOR_POSRION_NUMBER] = 
//{
//	1256
//	,359
//	,272
//	,227
//	,199
//	,181
//	,166
//	,152
//	,138
//	,139
//	,132
//	,130
//	,129
//	,132
//	,133
//	,134
//	,137
//	,141
//	,146
//	,147
//	,151
//	,155
//	,162
//	,165
//	,168
//	,176
//	,184
//	,190
//	,202
//	,213
//	,220
//	,241
//	,249
//	,276
//	,309
//	,323
//	,357
//	,412
//	,471
//	,574
//};

unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
ICACHE_RAM_ATTR void Encoder_INT1();
ICACHE_RAM_ATTR void Encoder_INT2();

void MyPrintf(const char *fmt, ...);

void UnlockRelay(bool on);
void LockRelay(bool on);
void OpenRelay(bool on);
void CloseRelay(bool on);
void BuzzOutput(bool on);

////////////////IIC data///////////////////////////////////////////////
unsigned char IicMasterSend[IIC_MASTER_SEND_BYTE];
unsigned char IicMasterRecv[IIC_MASTER_RECV_BYTE];
unsigned char IicMasterRecvCounter;
void IicExchange();
unsigned long IicRecvTimeOutTenthSecond;

bool AnyTagExist;
bool TagExist[MAX_TAG_NUMBER];
bool isKeyPressed[4];
bool isBodyExistOut;
bool isBodyExistIn;
bool isDoorClosed;
bool DoorClosedConfirm = false;


////////////////Door control/////////////////////////////////////////////////
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

bool bLastDoorClosed = false;

bool CheckBodyLeft(From eWhichBody);
bool CheckBodyCome(From eWhichBody);
//void CheckKey_task();
void KeyPressed(unsigned char KeyIndex);
void CheckDoorClose_task();
void DoorOpen_Task();
void DoorClose_Task();
void DoorOpen_Speed_Task();
void DoorClose_Speed_Task();
void StopAllTask();



void DoorLock_Task();
void DoorUnlock_Task();
void AutoClose_task();
void Buzz_task();
unsigned char Buzz = 0;

#define BUZZON 1				//set lenght of the buzz 0.1s
#define BUZZOFF 1			//set interval of the buzz 0.1s
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
				BuzzOutput(true);
			}
		}
		else
		{
			BuzzOff++;
			if (BuzzOff > BUZZOFF)
			{
				BuzzOff = 0;
				BuzzHigh = true;
				BuzzOutput(false);
				Buzz--;
			}
		}
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
			StopCloseing = false;
			Step = 0;
			DoorCloseing = false;
			printf("DoorCloseing canceled.\r\n");
		}
	} 
}

void DoorOpen_Speed_Task()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (DoorOpening)
	{
		if (!StopOpening)
		{
			switch(Step)
			{
			case 0:
				printf("DoorOpening start.\r\n");
				printf("Unlocking.\r\n");
				Step = 1;
				StartTime = TenthSecondsSinceStart;
				UnlockRelay(true);
				break;
			case 1:
				if(TenthSecondsSinceStart -StartTime > 3)
				{
					OpenRelay(true);
					Step ++;
				}
				break;
			case 2:
				if(TenthSecondsSinceStart -StartTime > 9)
				{
					OpenRelay(false);
					Step ++;
				}
				break;
			case 3:
				if((Postion > 39)||((TenthSecondsSinceStart -StartTime > 50)))
				{
					Step ++;
				}
				if ((Postion>=0)&&(Postion<DOOR_POSRION_NUMBER)&&(!isIntDataProcessing))
				{
					MaxSpeed = OpenSpeedRef[Postion]+OpenSpeedRef[Postion]/5;
					MinSpeed = OpenSpeedRef[Postion]-OpenSpeedRef[Postion]/5;
					if (Speed < MinSpeed)//open speed too high
					{
						OpenRelay(false);
						//CloseRelay(true);
					}
					if (Speed > MaxSpeed)//open speed too low
					{
						OpenRelay(true);
						//CloseRelay(false);
					}
					if ((Speed <= MaxSpeed)&&(Speed >= MinSpeed))
					{
						OpenRelay(false);
						//CloseRelay(false);
					}
				}

				break;

			default:

				OpenRelay(false);
				CloseRelay(false);
				UnlockRelay(false);
				Step = 0;
				DoorOpening = false;
				printf("DoorOpening finished\r\n");
				break;
			}
		}
		else
		{
			StopOpening = false;
			Step = 0;
			DoorOpening = false;
			printf("DoorOpening canceled.\r\n");
		}
	} 
}


void DoorClose_Speed_Task()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	static unsigned char StartPostion = 0;
	if (DoorCloseing)
	{
		if (!StopCloseing)
		{
			switch(Step)
			{
			case 0:
				printf("DoorCloseing start.\r\n");
				Step = 2;
				StartTime = TenthSecondsSinceStart;
				StartPostion = Postion;
				CloseRelay(true);
			//case 1:
			//	if(TenthSecondsSinceStart -StartTime > 10)
			//	{
			//		CloseRelay(false);
			//		Step ++;
			//	}
			//	break;


			//case 1:
			//	if(StartPostion != Postion)
			//	{
			//		Step ++;
			//	}
			//	if(TenthSecondsSinceStart -StartTime > 10)
			//	{
			//		Step = 0xFF;
			//	}
			//	break;


			case 2:
				if (TenthSecondsSinceStart - StartTime > 50)
				{
					Step = 0xFF;
					MyPrintf("DoorCloseing timeout 5s.\r\n");
				}
				if((Postion < 2)||(DoorClosedConfirm))
				{
					Step ++;
				}
				if ((Postion>=0)&&(Postion<DOOR_POSRION_NUMBER)&&(!isIntDataProcessing))
				{
					MaxSpeed = CloseSpeedRef[Postion]+CloseSpeedRef[Postion]/5;
					MinSpeed = CloseSpeedRef[Postion]-CloseSpeedRef[Postion]/5;
					if (!Direction&&(Speed < MinSpeed))//close speed too high
					{
						OpenRelay(true);
						CloseRelay(false);
					}
					if ((Speed > MaxSpeed)||(Direction))//close speed too low, or wrong Direction
					{
						OpenRelay(false);
						CloseRelay(true);
					}
					if (!Direction&&(Speed <= MaxSpeed)&&(Speed >= MinSpeed))
					{
						OpenRelay(false);
						CloseRelay(false);
					}
				}

				break;
			case 3:
				OpenRelay(false);
				CloseRelay(true);
				StartTime = TenthSecondsSinceStart;
				Step ++;
				break;
			case 4:
				if(TenthSecondsSinceStart - StartTime > 10)
				{
					CloseRelay(false);
					Step ++;
				}
				break;
			default:
					OpenRelay(false);
					CloseRelay(false);
					Step = 0;
					DoorCloseing = false;
					printf("DoorCloseing finished\r\n");
				break;

			}

		}
		else
		{
			StopCloseing = false;
			Step = 0;
			DoorCloseing = false;
			printf("DoorCloseing canceled.\r\n");
		}
	} 
}

void StopAllTask()
{
	if (DoorCloseing)
	{
		StopCloseing = true;
	}
	if (DoorOpening)
	{
		StopOpening = true;
	}
	if (DoorLocking)
	{
		StopLocking = true;
	}
	if (DoorUnlocking)
	{
		StopUnlocking = true;
	}
	OpenRelay(false);
	CloseRelay(false);
	UnlockRelay(false);
	LockRelay(false);
	
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
		if (!StopLocking)
		{
			if (isDoorClosed)
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
				Step = 0;
				DoorLocking = false;
				printf("DoorLocking canceled because door not closed.\r\n");
			}
		}
		else
		{
			StopLocking = false;
			Step = 0;
			DoorLocking = false;
			printf("DoorLocking canceled.\r\n");
		}
	} 
}


void CheckDoorClose_task()
{
	if (isDoorClosed!=bLastDoorClosed)
	{
		bLastDoorClosed = isDoorClosed;

		if (!bLastDoorClosed)
		{
			DoorClosedConfirm = false;
		}

		if ((bLastDoorClosed)&&(TenthSecondsSinceStart > 100))
		{
			//DoorLocking = true;
			DoorClosedConfirm = true;



			Postion = 0;

			//char AllStrBuf[1024];
			//AllStrBuf[0]=0;
			//char OneStrBuf[20];
			//OneStrBuf[0]=0;
			//for (char i = 0; i < DOOR_POSRION_NUMBER; i++)
			//{
			//	sprintf(OneStrBuf,"%d:	%d	%d\r\n",i,OpenSpeed[i],CloseSpeed[i]);
			//	strcat(AllStrBuf,OneStrBuf);
			//	OpenSpeed[i] = 0;
			//	CloseSpeed[i] = 0;
			//}
			//MyPrintf("Door speed:Open Close\r\n %s ",AllStrBuf);

		}
		MyPrintf("Door close change to %d \r\n",bLastDoorClosed);
	}
}

void AutoClose_task()
{

	if (bWaitAutoClose)
	{
		if (eOpenFrom == OUT)
		{
			if(isBodyExistIn)
			{
				StopAllTask();
				DoorCloseing = true;
				bWaitAutoClose = false;
				MyPrintf("Auto Door close.From out, Body IN  \r\n");
			}
		}

		if (eOpenFrom == IN)
		{
			if(isBodyExistOut)
			{
				StopAllTask();
				DoorCloseing = true;
				bWaitAutoClose = false;
				MyPrintf("Auto Door close.From IN, Body OUT  \r\n");
			}
		}
	} 
}

void KeyPressed(unsigned char KeyIndex)
{
	switch(KeyIndex)
	{
	case 0:
		//LastReadyOutGetTime = TenthSecondsSinceStart;
		break;
	case 1:
		printf("Key Out! \r\n");
		//if (Tag&&ReadyOut&&IsDoorClosed())
		//*if (Tag&&bBodyCheckOut&&IsDoorClosed())*/

		if ((TagExist[3]||TagExist[2])&&isDoorClosed)
		{
			Buzz = 1;
			StopAllTask();
			DoorOpening = true;
			bWaitAutoClose = true;
			eOpenFrom = OUT;
			printf("open from Out! \r\n");
		}
		else if (!isDoorClosed)
		{
			Buzz = 1;
			StopAllTask();
			DoorCloseing = true;
			printf("close from Out! \r\n");
		}
		else if (AnyTagExist&&isDoorClosed)
		{
			Buzz = 1;
			StopAllTask();
			DoorUnlocking = true;
			eOpenFrom = OUT;
			printf("unlock from Out! \r\n");
		}
		else
		{
			Buzz = 3;
		}



		break;
	case 2:
		printf("Key In! \r\n");
		if (isDoorClosed)
		{
			printf("open from In! \r\n");
			StopAllTask();
			DoorOpening = true;
			eOpenFrom = IN;
			bWaitAutoClose = true;
			Buzz = 10;
		}
		else
		{
			printf("close from In! \r\n");
			StopAllTask();
			DoorCloseing = true;
		}
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

//#define KEY_PRESS_MIN 0
//#define PAUSE_KEY_PRESS 20
//void CheckKey_task()
//{
//	static unsigned char KeyCounter[4] = {0,0,0,0};
//	static unsigned char PauseKeyCounter = PAUSE_KEY_PRESS;
//
//
//	if (PauseKeyCounter>0)
//	{
//		PauseKeyCounter--;
//		return;
//	} 
//
//	for (char i = 0; i < 4; i++)
//	{
//		if (isKeyPressed[i])
//		{
//
//			KeyCounter[i]++;
//			if (KeyCounter[i] > KEY_PRESS_MIN)
//			{
//				KeyCounter[i] = 0;
//				PauseKeyCounter = PAUSE_KEY_PRESS;
//				//Buzz = i+1;
//				printf("Key %d press!  \r\n",i);
//				KeyPressed(i);
//			}
//		}
//		else
//		{
//			KeyCounter[i] = 0;
//		}
//	}	
//}

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
/////////////////////////////////////////////////////////////


void setup() 
{       

	KeyLessData.DataType = 14;

	pinMode(RELAY_OPEN, OUTPUT);
	digitalWrite(RELAY_OPEN, HIGH);
	pinMode(RELAY_CLOSE, OUTPUT);
	digitalWrite(RELAY_CLOSE, HIGH);


	pinMode(INT1, INPUT);
	attachInterrupt(digitalPinToInterrupt(INT1), Encoder_INT1, RISING);
	pinMode(INT2, INPUT);
	attachInterrupt(digitalPinToInterrupt(INT2), Encoder_INT2, RISING);




	Wire.begin(SDA, SCL); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */


	delay(50);                      
	Serial.begin(115200);


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	Serial.print("Is connection routing, please wait");  
	WiFi.begin(ssid, password); //Wifi接入到网络
	Serial.println("\nConnecting to WiFi");
	//如果Wifi状态不是WL_CONNECTED，则表示连接失败
	while (WiFi.status() != WL_CONNECTED) {  
		Serial.print("."); 
		delay(1000);    //延时等待接入网络
	}



	//设置时间格式以及时间服务器的网址
	configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
	Serial.println("\nWaiting for time");
	while (!time(nullptr)) {
		Serial.print(".");
		delay(1000);    
	}
	Serial.println("");




	byte mac[6];
	WiFi.softAPmacAddress(mac);
	for (byte i=0;i<6;i++)
	{
		KeyLessData.Mac[i] = mac[i];
	}
	//printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X AP:%s\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],ssid);



	m_WiFiUDP.begin(5050); 

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else { // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			Serial.println("Auth Failed");
		} else if (error == OTA_BEGIN_ERROR) {
			Serial.println("Begin Failed");
		} else if (error == OTA_CONNECT_ERROR) {
			Serial.println("Connect Failed");
		} else if (error == OTA_RECEIVE_ERROR) {
			Serial.println("Receive Failed");
		} else if (error == OTA_END_ERROR) {
			Serial.println("End Failed");
		}
	});
	ArduinoOTA.begin();
	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());


}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();

	CheckRf();

	DoorOpen_Speed_Task();
	DoorClose_Speed_Task();
}


unsigned long LastMillis = 0;
void TenthSecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 100)
	{
		LastMillis = CurrentMillis;
		TenthSecondsSinceStart++;
		OnTenthSecond();
		//MyPrintf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);



	}
}

void OnSecond()
{
	time_t now = time(nullptr); //获取当前时间
	time_str = ctime(&now);
	H1 = time_str[11];
	H2 = time_str[12];
	M1 = time_str[14];
	M2 = time_str[15];
	S1 = time_str[17];
	S2 = time_str[18];
	//printf("%c%c:%c%c:%c%c\n",H1,H2,M1,M2,S1,S2);
	//Serial.printf(time_str);

	struct   tm     *timenow;
	timenow   =   localtime(&now);
	unsigned char Hour = timenow->tm_hour;
	unsigned char Minute = timenow->tm_min;

	if ((!RfInitialized)&&(TenthSecondsSinceStart > 100))
	{
		pinMode(RF_IN, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(RF_IN), DecodeRf_INT, CHANGE);
		RfInitialized = true;
	}

	//if (now%10==0)
	//{
	//	MyPrintf("Tag Any:%d  %d%d%d%d%d%d%d%d out:%d in:%d Closed:%d \r\n"
	//		,AnyTagExist
	//		,(TagExist[0])
	//		,(TagExist[1])
	//		,(TagExist[2])
	//		,(TagExist[3])
	//		,(TagExist[4])
	//		,(TagExist[5])
	//		,(TagExist[6])
	//		,(TagExist[7])

	//		,isBodyExistOut
	//		,isBodyExistIn
	//		,isDoorClosed
	//		);
	//}


	//if (now%5==0)
	//{
	//	BuzzOutput(true);
	//}

	//if (now%5==1)
	//{
	//	BuzzOutput(false);
	//}

	



	if (IicRecvTimeOutTenthSecond > 5)
	{
		//printf("IIC timeout = %d\r\n",IicRecvTimeOutTenthSecond);
	}
	//printf("IIC timeout = %d",IicRecvTimeOutTenthSecond);
	//printf(" %d bytes ",IicMasterRecvCounter);
	//printf("data: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
	//	,IicMasterRecv[0]
	//,IicMasterRecv[1]
	//,IicMasterRecv[2]
	//,IicMasterRecv[3]
	//,IicMasterRecv[4]);


	KeyLessData.Triger = false;
	static bool LastTagExist[MAX_TAG_NUMBER];
	for(unsigned char i = 0; i<MAX_TAG_NUMBER; i++)
	{
		if (LastTagExist[i] != TagExist[i])
		{
			LastTagExist[i] = TagExist[i];
		
			if (LastTagExist[i])
			{

				KeyLessData.KeyLessData = i;
				KeyLessData.KeyLessData = KeyLessData.KeyLessData<<16;
				KeyLessData.Triger = true;

				m_WiFiUDP.beginPacket("192.168.0.17", 5050);
				m_WiFiUDP.write((const char*)&KeyLessData, sizeof(tKeyLessData));
				m_WiFiUDP.endPacket(); 
			}
		}
	}
	if (KeyLessData.Triger == false)
	{
		m_WiFiUDP.beginPacket("192.168.0.17", 5050);
		m_WiFiUDP.write((const char*)&KeyLessData, sizeof(tKeyLessData));
		m_WiFiUDP.endPacket(); 
	}



}

void OnTenthSecond()
{

	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}

	Buzz_task();


	IicExchange();
		//MyPrintf("Tag Any:%d  %d%d%d%d%d%d%d%d out:%d in:%d Closed:%d \r\n"
		//	,AnyTagExist
		//	,(TagExist[0])
		//	,(TagExist[1])
		//	,(TagExist[2])
		//	,(TagExist[3])
		//	,(TagExist[4])
		//	,(TagExist[5])
		//	,(TagExist[6])
		//	,(TagExist[7])

		//	,isBodyExistOut
		//	,isBodyExistIn
		//	,isDoorClosed
		//	);

	CheckDoorClose_task();
	//DoorOpen_Task();
	//DoorClose_Task();
	DoorLock_Task();
	DoorUnlock_Task();
	AutoClose_task();

	
	//千万不要删下面这个函数！！！Never delete
	//CheckKey_task();

	//yield();

	//printf("IIC timeout = %d",IicRecvTimeOutTenthSecond);
	//printf(" %d bytes ",IicMasterRecvCounter);
	//printf("data: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
	//	,IicMasterRecv[0]
	//,IicMasterRecv[1]
	//,IicMasterRecv[2]
	//,IicMasterRecv[3]
	//,IicMasterRecv[4]);

	IicRecvTimeOutTenthSecond++;

	//Decoder data from IO board

	if ((IicMasterRecv[0] == (unsigned char)(~IicMasterRecv[1]))
		&&(IicMasterRecv[2] == (unsigned char)(~IicMasterRecv[3])))
	{

		
		if (IicMasterRecv[0] > 0)
		{
			AnyTagExist = true;
		}
		else
		{
			AnyTagExist = false;
		}
		for (unsigned char i = 0; i < MAX_TAG_NUMBER; i++)
		{
			TagExist[i] = (IicMasterRecv[0]>>i)&1;
		}

		//for (unsigned char i = 0; i < 4; i++)
		//{
		//	isKeyPressed[i] = IicMasterRecv[2]>>i&1;
		//}

		isBodyExistOut = IicMasterRecv[2]>>4&1;
		isBodyExistIn = IicMasterRecv[2]>>5&1;
		isDoorClosed = IicMasterRecv[2]>>6&1;
	}
	else
	{
		//printf("IIC data check failed 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
		//	,IicMasterRecv[0]
		//	,IicMasterRecv[1]
		//	,IicMasterRecv[2]
		//	,IicMasterRecv[3]);
	}


	//Encoder


	//Speed = micros() - Int1_LastMicros;
	//unsigned long CurrentUnInt1Diff = micros() - Int1_LastMicros;
	//if (CurrentUnInt1Diff > Speed)
	//{
	//	Speed = CurrentUnInt1Diff;
	//}


	//printf("Postio: %d Speed: %d\r\n"
	//	,Postion
	//	,Speed
	//	);

	

}

void IicExchange()
{
	IicMasterSend[1] = (unsigned char)(~IicMasterSend[0]);
	//for (char i = 0; i < IIC_MASTER_SEND_BYTE-1; i++)
	//{
	//	IicMasterSend[IIC_MASTER_SEND_BYTE-1] = IicMasterSend[IIC_MASTER_SEND_BYTE-1] + IicMasterSend[i];
	//}
	Wire.beginTransmission(8); /* begin with device address 8 */
	Wire.write(IicMasterSend,IIC_MASTER_SEND_BYTE);
	Wire.endTransmission();    /* stop transmitting */

	Wire.requestFrom(8, IIC_MASTER_RECV_BYTE); /* request & read data of size 13 from slave */
	unsigned char RecvIndex = 0;
	while(Wire.available()){
		IicMasterRecv[RecvIndex] = Wire.read();
		RecvIndex++;
		IicMasterRecvCounter = RecvIndex;
		IicRecvTimeOutTenthSecond = 0;
	}
	


	//printf("IIC_recv %d bytes\r\n",IicMasterRecvCounter);
	//for (char i = 0; i < IIC_MASTER_RECV_BYTE; i++)
	//{
	//	printf("0x%02X ",IicMasterRecv[i]);
	//}
	//printf("\r\n");


	//test data
	//for (char i = 0; i < IIC_MASTER_SEND_BYTE; i++)
	//{
	//	IicMasterSend[i]++;
	//}

	IicMasterSend[IIC_MASTER_SEND_BYTE-1]++;

}

ICACHE_RAM_ATTR void Encoder_INT1()//中断函数
{
	//static unsigned int Counter = 0;

	//Counter ++;
	Int1_DiffMicros = micros() - Int2_LastMicros;

	Direction = Int1_DiffMicros<Int2_DiffMicros;


	if (!isDoorClosed||DoorOpening)
	{
		isIntDataProcessing = true;
		if (Direction)
		{
			Postion++;
		} 
		else
		{
			Postion--;
		}

		unsigned long CurrentSpeed = (micros() - Int1_LastMicros)/0xFF;
		if (CurrentSpeed > 0xFFFF)
		{
			CurrentSpeed = 0xFFFF;
		}
		Speed = CurrentSpeed;
		isIntDataProcessing = false;

		if ((Postion>=0)&&(Postion<DOOR_POSRION_NUMBER))
		{
			if (Direction)
			{
				OpenSpeed[Postion] = Speed;
			} 
			else
			{
				CloseSpeed[Postion] = Speed;
			}
		}

	}
	else
	{
		Postion = 0;
	}



	//MyPrintf("Director: %s Postion %d Speed: %d\r\n"
	//	,(Director?"Open":"Close")
	//	,Postion
	//	,Speed
	//	);

	Int1_LastMicros = micros();

}

ICACHE_RAM_ATTR void Encoder_INT2()//中断函数
{
	//static unsigned int Counter = 0;
	static unsigned long LastMicros = 0;
	//Counter ++;
	Int2_DiffMicros = micros() - Int1_LastMicros;
	Int2_LastMicros = micros();
}


void MyPrintf(const char *fmt, ...)
{


	static char send_buf[1024];
	char * sprint_buf = send_buf+sizeof(tDebugData);
	tDebugData * pDebugData = (tDebugData*)send_buf;

	int n;
	va_list args;

	va_start(args,fmt);
	n = vsprintf(sprint_buf, fmt, args);
	va_end(args);

	printf(sprint_buf);

	pDebugData->DataType = 3;
	pDebugData->RoomId = DebugLogIndex;
	pDebugData->Length = n;

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)send_buf, sizeof(tDebugData)+n);
	m_WiFiUDP.endPacket(); 


}

void UnlockRelay(bool on)
{	
	static bool LastOn;

	if (LastOn != on)
	{
		LastOn = on;
		if (on)
		{
			IicMasterSend[0] = IicMasterSend[0]|(1<<0);
		} 
		else
		{
			IicMasterSend[0] = IicMasterSend[0]&(~(1<<0));
		}
		IicMasterSend[1] = (unsigned char)(~IicMasterSend[0]);
		MyPrintf("Unlock Relay to %d .\r\n",on);
	}
}
void LockRelay(bool on)
{
	static bool LastOn;

	if (LastOn != on)
	{
		LastOn = on;
		if (on)
		{
			IicMasterSend[0] = IicMasterSend[0]|(1<<1);
		} 
		else
		{
			IicMasterSend[0] = IicMasterSend[0]&(~(1<<1));
		}
		IicMasterSend[1] = (unsigned char)(~IicMasterSend[0]);
		MyPrintf("Lock Relay to %d .\r\n",on);
	}
}

void OpenRelay(bool on)
{
	//if (on)
	//{
	//	IicMasterSend[0] = IicMasterSend[0]|(1<<2);
	//} 
	//else
	//{
	//	IicMasterSend[0] = IicMasterSend[0]&(~(1<<2));
	//}
	//IicMasterSend[1] = (unsigned char)(~IicMasterSend[0]);


	static bool LastOn;

	if (LastOn != on)
	{
		LastOn = on;
		if (on)
		{
			digitalWrite(RELAY_OPEN, LOW);
		} 
		else
		{
			digitalWrite(RELAY_OPEN, HIGH);
		}

		MyPrintf("Open  Relay to %d Postion:%d time:%d Speed:%d Max:%d Min:%d\r\n"
			,on
			,Postion
			,TenthSecondsSinceStart
			,Speed
			,MaxSpeed
			,MinSpeed);
	}
}
void CloseRelay(bool on)
{
	//if (on)
	//{
	//	IicMasterSend[0] = IicMasterSend[0]|(1<<3);
	//} 
	//else
	//{
	//	IicMasterSend[0] = IicMasterSend[0]&(~(1<<3));
	//}
	//IicMasterSend[1] = (unsigned char)(~IicMasterSend[0]);

	static bool LastOn;

	if (LastOn != on)
	{
		LastOn = on;
		if (on)
		{
			digitalWrite(RELAY_CLOSE, LOW);
		} 
		else
		{
			digitalWrite(RELAY_CLOSE, HIGH);
		}
		MyPrintf("Close Relay to %d Postion:%d time:%d Speed:%d Max:%d Min:%d\r\n"
			,on
			,Postion
			,TenthSecondsSinceStart
			,Speed
			,MaxSpeed
			,MinSpeed);
	} 




}

void BuzzOutput(bool on)
{
	if (on)
	{
		IicMasterSend[0] = IicMasterSend[0]|(1<<4);
	} 
	else
	{
		IicMasterSend[0] = IicMasterSend[0]&(~(1<<4));
	}
	IicMasterSend[1] = (unsigned char)(~IicMasterSend[0]);

}

ICACHE_RAM_ATTR void DecodeRf_INT()
{
#define PULSE_NUMBER 48
#define MIN_LEN 100
#define MAX_LEN 2000
#define LEAD_LEN 7000

	unsigned long ThisTime;
	unsigned long DiffTime;
	static unsigned long FirstTime;
	static unsigned long LastRfTime = 0;
	static bool FrameStarted = false;
	static bool RfOn = false;
	static unsigned char PulseIndex = 0;
	static unsigned int Base;
	static unsigned int Min_Base;
	static unsigned int Max_Base;

	if (DecodeFrameOK)
	{
		return;
	}

	if (PulseIndex<PULSE_NUMBER)
	{
		if (LastRfTime == 0)
		{
			LastRfTime = micros();
		} 
		else
		{
			ThisTime = micros();
			DiffTime = ThisTime-LastRfTime;
			LastRfTime = ThisTime;

			if (RfOn)
			{
				if (FrameStarted)
				{
					if ((DiffTime > Min_Base)&&(DiffTime < Max_Base))
					{
						if (PulseIndex%2==0)
						{
							FirstTime = DiffTime;
						} 
						else
						{
							unsigned char CommandIndex = (PulseIndex-1)/2/8;
							RcCommand[CommandIndex] = RcCommand[CommandIndex]<<1;
							if ((FirstTime>Base)&&(DiffTime<Base))//bit 1
							{
								RcCommand[CommandIndex]++;
							} 
							else
							{
								if ((FirstTime<Base)&&(DiffTime>Base))//bit 0
								{

								}
								else//如果编码规则出错
								{
									LastRfTime = 0;
									PulseIndex = 0;
									FrameStarted = false;
									RfOn = false;
									RcCommand[0] = 0;
									RcCommand[1] = 0;
									RcCommand[2] = 0;
								}
							}
						}
						PulseIndex++;
						if (PulseIndex >= PULSE_NUMBER)//收集到48个位
						{
							DecodeFrameOK = true;
							LastRfTime = 0;
							PulseIndex = 0;
							FrameStarted = false;
							RfOn = false;
						}
					}
					else//如果时间长度出错
					{	
						LastRfTime = 0;
						PulseIndex = 0;
						FrameStarted = false;
						RfOn = false;
						RcCommand[0] = 0;
						RcCommand[1] = 0;
						RcCommand[2] = 0;
					}
				} 
				else
				{
					if (DiffTime > LEAD_LEN)//收到引导码
					{
						Base = DiffTime/16;
						Min_Base = DiffTime/62;
						Max_Base = DiffTime/8;
						FrameStarted = true;
					}
				}
			}
			else
			{
				if ((DiffTime > MIN_LEN)&&(DiffTime < MAX_LEN))
				{
					PulseIndex++;
					if (PulseIndex > 32)//连续正确的时间长度
					{
						RfOn = true;
						RcCommand[0] = 0;
						RcCommand[1] = 0;
						RcCommand[2] = 0;
						PulseIndex = 0;
						LastRfTime = 0;
					}
				}
				else//时间长度异常
				{	
					LastRfTime = 0;
					PulseIndex = 0;
				}
			}
		}
	}
	else
	{
		LastRfTime = 0;
		PulseIndex = 0;
		FrameStarted = false;
		RfOn = false;
	}
}

void CheckRf()
{
	static unsigned char LastRf[3];
	if (DecodeFrameOK)
	{


		if (memcmp(LastRf,RcCommand,3) == 0)
		{
			//MyPrintf("0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2]);
			CheckRfCommand(RcCommand);
		} 
		else
		{
			memcpy(LastRf,RcCommand,3);
		}
		DecodeFrameOK = false;
	}
}

#define PAUSE_RF_KEY_PRESS 20  //0.1s
void CheckRfCommand(unsigned char * RfCommand)
{

	static unsigned long LastKeyTime = PAUSE_RF_KEY_PRESS;

	if (TenthSecondsSinceStart - LastKeyTime < PAUSE_RF_KEY_PRESS)
	{
		return;
	} 




	for (byte j=0;j<RF_COMMAND_FUNCTION_COUNTER;j++)
	{
		for (byte i=0;i<RF_COMMAND_KEY_COUNTER;i++)
		{
			yield();
			if(memcmp(RfCommand,PreSetRfCommand[j][i],RF_COMMAND_LEN)==0)
			{

				
				MyPrintf("rc Command %d \r\n",j);
				LastKeyTime = TenthSecondsSinceStart;

				if (j < 4)
				{
					KeyPressed(j);
				}
			}

		}
	}
}
