

#include <printf.h>
void Dump_433();
void Dump_315();
void DumpDecode_315();
void DumpDecode_433();
void Dump_315_Lead();
void DecodeRf_INT();


#include "RCSwitch.h"
RCSwitch mySwitch = RCSwitch();


unsigned long LastTime[2] = {0,0};
unsigned int Counter[2] = {0,0};
unsigned int Base[2];
bool FrameOK[2] = {false,false};

bool DecodeFrameOK = false;
bool StartHigh = false;




unsigned char RcCommand[3] = {0,0,0};

void CheckRf();





void SecondsSinceStartTask();
void DecodeRf(unsigned char index);
void OnTenthSecond();

//void SendRf();

 // #define DUMP_LONG
//  #define DUMP_DECODE
  #define DECODE_2622


#ifdef DUMP_LONG
	#define  SAMPLE_NUMBER 300
#else
	#define  SAMPLE_NUMBER 100
#endif

unsigned int TimeArray[2][SAMPLE_NUMBER];



void setup()
{
	Serial.begin(115200);
	Serial.println(F("test_rf"));
	printf_begin();


	//used for RX

	//pinMode(2, INPUT_PULLUP);
	pinMode(2, INPUT);
	pinMode(3, INPUT);

	mySwitch.enableTransmit(4);
	mySwitch.setRepeatTransmit(10);

#ifdef DUMP_LONG
	attachInterrupt(0, Dump_315, CHANGE);
	attachInterrupt(1, Dump_433, CHANGE);
#endif

#ifdef DUMP_DECODE
	attachInterrupt(0, DumpDecode_315, CHANGE);
	//attachInterrupt(1, DumpDecode_433, CHANGE);

#endif

#ifdef DECODE_2622
	attachInterrupt(0, DecodeRf_INT, CHANGE);
#endif




}

void loop()
{

	SecondsSinceStartTask();

#ifdef DUMP_LONG

	for (unsigned char i = 0;i<2;i++)
	{
		if (FrameOK[i])
		{
			DecodeRf(i);

			FrameOK[i] = false;
			LastTime[i] = 0;
			Counter[i] = 0;
		}
	}

#endif

#ifdef DUMP_DECODE
	for (unsigned char i = 0;i<2;i++)
	{
		if (FrameOK[i])
		{
			DecodeRf(i);

			FrameOK[i] = false;
			LastTime[i] = 0;
			Counter[i] = 0;
		}
	}
#endif

#ifdef DECODE_2622
	CheckRf();
#endif




	//delay(1000);
}


unsigned long LastMillis = 0;
unsigned long TenthSecondsSinceStart = 0;
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

void OnTenthSecond()
{
	//if (TenthSecondsSinceStart%10 == 0)
	//{
	//	printf("send \r\n");

	//	mySwitch.send(12811368, 24);

	//}
}

void DecodeRf(unsigned char index)
{
	unsigned char RcCommand[5] = {0,0,0,0,0};
	bool CheckOK = true;


		printf("Base = %d  ",Base[index]);
		printf("counter = %d  ",Counter[index]);
		if (index==0)
		{
			printf("FR = 315Mhz   ");
		} 
		else
		{
			printf("FR = 433Mhz   ");
		}

		printf("\r\n");


#ifdef DUMP_LONG

		unsigned int LastLong = 0;
		for (unsigned int i = 0;i<Counter[index];i++)
		{
			printf("%d ",TimeArray[index][i]);

			if (((i-LastLong)%2==1)&&((i-LastLong)<80))
			{
				RcCommand[(i-LastLong)/16] = RcCommand[(i-LastLong)/16]<<1;
				if (TimeArray[index][i-1]> TimeArray[index][i])
				{
					RcCommand[(i-LastLong)/16]++;
				} 
			}

			if (TimeArray[index][i] > 1000)
			{
				printf("\r\nAbove line. Pulse No.%02d end with %d  0x%02X 0x%02X 0x%02X 0x%02X 0x%02X|   ", i-LastLong-1,(i%2==0?StartHigh:!StartHigh),RcCommand[0],RcCommand[1],RcCommand[2],RcCommand[3],RcCommand[4]);
				LastLong = i+1;
				RcCommand[0] = 0;
				RcCommand[1] = 0;
				RcCommand[2] = 0;
				RcCommand[3] = 0;
				RcCommand[4] = 0;
				
			}
		}
		printf("end\r\n");

#endif

#ifdef DUMP_DECODE

		for (unsigned int i = 0;i<Counter[index];i++)
		{
			printf("%d ",TimeArray[index][i]);
			if (i%4 == 3)
			//if (TimeArray[index][i] > 5000)
			{
				printf("\r\n");
			}
		}
		printf("end\r\n");


		if (Counter[index] == 49)
		{
			for (unsigned char i = 0;i<24;i++)
			{
				RcCommand[i/8] = RcCommand[i/8]<<1;
				if ((TimeArray[index][i*2]>Base[index])&&(TimeArray[index][i*2+1]<Base[index]))
				{
					RcCommand[i/8]++;
				} 
				else
				{
					if ((TimeArray[index][i*2]<Base[index])&&(TimeArray[index][i*2+1]>Base[index]))
					{

					}
					else
					{
						CheckOK = false;
						break;
					}
				}
			}
		} 
		else if (Counter[index] == 79)
		{
			for (unsigned char i = 0;i<38;i++)
			{
				RcCommand[i/8] = RcCommand[i/8]<<1;
				if ((TimeArray[index][i*2]>Base[index])&&(TimeArray[index][i*2+1]<Base[index]))
				{
					RcCommand[i/8]++;
				} 
				else
				{
					if ((TimeArray[index][i*2]<Base[index])&&(TimeArray[index][i*2+1]>Base[index]))
					{

					}
					else
					{
						CheckOK = false;
						break;
					}
				}
			}
		}
		else
		{
			CheckOK = false;
		}


		if (CheckOK)
		{
			printf("check OK   ");
			printf("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2],RcCommand[3],RcCommand[4]);
		}
		else
		{
			printf("check failed\r\n");
		}


#endif





}


void CheckRf()
{
	static unsigned char LastRf[3];
	if (DecodeFrameOK)
	{


		if (memcmp(LastRf,RcCommand,3) == 0)
		{
			printf("0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2]);
		} 
		else
		{
			memcpy(LastRf,RcCommand,3);
		}
		DecodeFrameOK = false;
	}
}

//void Dump_315_Lead()
//{
//	static bool FrameStarted = false;
//
//	unsigned long ThisTime;
//	unsigned long DiffTime;
//	static unsigned long LastTime = 0;
//	static unsigned long LastDiff = 0;
//
//	if (FrameOK[0])
//	{
//		return;
//	}
//
//	if (Counter[0]==SAMPLE_NUMBER)
//	{
//		FrameOK[0] = true;
//		LastTime = 0;
//		LastDiff = 0; 
//		return;
//	}
//
//	if (LastTime == 0)
//	{
//		LastTime = micros();
//		return;
//	} 
//
//
//
//	ThisTime = micros();
//	DiffTime = ThisTime-LastTime;
//	LastTime = ThisTime;
//
//	if (LastDiff == 0)
//	{
//		LastDiff = DiffTime;
//		return;
//	}
//
//	if (FrameStarted)
//	{
//		if ((DiffTime>7000)&&(LastDiff<(DiffTime/15))&&(LastDiff>(DiffTime/46)))
//		{
//			printf("found head, %ld,%ld ,%ld ,%ld\r\n",LastDiff,DiffTime,DiffTime/15,DiffTime/46);
//
//			FrameOK[0] = true;
//			FrameStarted = false;
//			LastTime = 0;
//			LastDiff = 0;
//		}
//		else
//		{
//			TimeArray[0][Counter[0]] = DiffTime;
//			Counter[0]++;
//		}
//	} 
//	else
//	{
//		if ((DiffTime>7000)&&(LastDiff<(DiffTime/15))&&(LastDiff>(DiffTime/46)))
//		{
//			//printf("found head, %ld,%ld ,%ld ,%ld\r\n",LastDiff,DiffTime,DiffTime/25,DiffTime/37);
//			Base[0] = DiffTime/16;
//			FrameStarted = true;
//			Counter[0] = 0;
//		}
//	}
//	LastDiff = DiffTime;
//
//}


void Dump_433()//中断函数   //debug for rf data simply
{
	unsigned long ThisTime;
	unsigned long DiffTime;
	static bool FrameStarted = false;
	static bool RfOn = false;

	if (FrameOK[1])
	{
		return;
	}

	if (Counter[1]<SAMPLE_NUMBER)
	{
		if (LastTime[1] == 0)
		{
			LastTime[1] = micros();
		} 
		else
		{
			ThisTime = micros();
			DiffTime = ThisTime-LastTime[1];
			LastTime[1] = ThisTime;

			if (RfOn)
			{
				TimeArray[1][Counter[1]] = DiffTime;
				Counter[1]++;
			}
			else
			{
				if ((DiffTime > 100)&&(DiffTime < 2000))
				{
					Counter[1]++;
					if (Counter[1] > 32)
					{
						RfOn = true;
						Counter[1] = 0;
						LastTime[1] = 0;
					}
				}
				else
				{	
					LastTime[1] = 0;
					Counter[1] = 0;
				}
			}
		}
	}
	else
	{
		FrameOK[1] = true;
		FrameStarted = false;
		RfOn = false;
	}
}


void Dump_315()//中断函数   //debug for rf data simply
{
	unsigned long ThisTime;
	unsigned long DiffTime;
	static bool FrameStarted = false;
	static bool RfOn = false;

	if (FrameOK[0])
	{
		return;
	}

	if (Counter[0]<SAMPLE_NUMBER)
	{
		if (LastTime[0] == 0)
		{
			LastTime[0] = micros();
		} 
		else
		{
			ThisTime = micros();
			DiffTime = ThisTime-LastTime[0];
			LastTime[0] = ThisTime;

			if (RfOn)
			{
				TimeArray[0][Counter[0]] = DiffTime;
				Counter[0]++;
			}
			else
			{
				if ((DiffTime > 100)&&(DiffTime < 2000))
				{
					Counter[0]++;
					if (Counter[0] > 32)
					{
						StartHigh = digitalRead(2);
						RfOn = true;
						Counter[0] = 0;
						LastTime[0] = 0;
					}
				}
				else
				{	
					LastTime[0] = 0;
					Counter[0] = 0;
				}
			}
		}
	}
	else
	{
		FrameOK[0] = true;
		FrameStarted = false;
		RfOn = false;
	}
}

void DumpDecode_315()
{
	unsigned long ThisTime;
	unsigned long DiffTime;
	static bool FrameStarted = false;
	static bool RfOn = false;
	static unsigned int Min_Base;
	static unsigned int Max_Base;

	if (FrameOK[0])
	{
		return;
	}

	if (Counter[0]<SAMPLE_NUMBER)
	{
		if (LastTime[0] == 0)
		{
			LastTime[0] = micros();
		} 
		else
		{
			ThisTime = micros();
			DiffTime = ThisTime-LastTime[0];
			LastTime[0] = ThisTime;

			if (RfOn)
			{
				if (FrameStarted)
				{

					if ((DiffTime > Min_Base)&&(DiffTime < Max_Base))
					{
						TimeArray[0][Counter[0]] = DiffTime;
						Counter[0]++;
					}
					else
					{
						if (DiffTime > 7000)
						{
							FrameOK[0] = true;
							FrameStarted = false;
							RfOn = false;
						}
						else
						{
							//printf("Wrong time len\r\n");
							LastTime[0] = 0;
							//counter = 0;
							FrameStarted = false;
							RfOn = false;
						}

					}
				} 
				else
				{
					if (DiffTime > 7000)
					{
						//time[counter] = DiffTime;
						Base[0] = DiffTime/16;
						Min_Base = DiffTime/120;
						Max_Base = DiffTime/4;
						//counter++;
						FrameStarted = true;
					}
				}

			}
			else
			{
				if ((DiffTime > 100)&&(DiffTime < 2000))
				{
					Counter[0]++;
					if (Counter[0] > 32)
					{
						RfOn = true;
						Counter[0] = 0;
						LastTime[0] = 0;
					}
				}
				else
				{	
					LastTime[0] = 0;
					Counter[0] = 0;
				}

			}


		}
	}
	else
	{
		FrameOK[0] = true;
		FrameStarted = false;
		RfOn = false;
	}
}



void DumpDecode_433()
{
	unsigned long ThisTime;
	unsigned long DiffTime;
	static bool FrameStarted = false;
	static bool RfOn = false;
	static unsigned int Min_Base;
	static unsigned int Max_Base;

	if (FrameOK[1])
	{
		return;
	}

	if (Counter[1]<SAMPLE_NUMBER)
	{
		if (LastTime[1] == 0)
		{
			LastTime[1] = micros();
		} 
		else
		{
			ThisTime = micros();
			DiffTime = ThisTime-LastTime[1];
			LastTime[1] = ThisTime;

			if (RfOn)
			{
				if (FrameStarted)
				{

					if ((DiffTime > Min_Base)&&(DiffTime < Max_Base))
					{
						TimeArray[1][Counter[1]] = DiffTime;
						Counter[1]++;
					}
					else
					{
						if (DiffTime > 7000)
						{
							FrameOK[1] = true;
							FrameStarted = false;
							RfOn = false;
						}
						else
						{
							//LastTime = 0;
							////counter = 0;
							//FrameStarted = false;
							//RfOn = false;
						}

					}
				} 
				else
				{
					if (DiffTime > 7000)
					{
						//time[counter] = DiffTime;
						Base[1] = DiffTime/16;
						Min_Base = DiffTime/120;
						Max_Base = DiffTime/4;
						//counter++;
						FrameStarted = true;
					}
				}

			}
			else
			{
				if ((DiffTime > 100)&&(DiffTime < 2000))
				{
					Counter[1]++;
					if (Counter[1] > 32)
					{
						RfOn = true;
						Counter[1] = 0;
						LastTime[1] = 0;
					}
				}
				else
				{	
					LastTime[1] = 0;
					Counter[1] = 0;
				}

			}


		}
	}
	else
	{
		FrameOK[1] = true;
		FrameStarted = false;
		RfOn = false;
	}
}


void DecodeRf_INT()
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


//void blink()
//{
//	printf("blink\r\n");
//}