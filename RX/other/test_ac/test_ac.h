

#include <printf.h>
void blink();


//#include "IRremote\IRremote.h"                  // 引用 IRRemote 函式库
//IRsend irsend;                          // 定义 IRsend 物件来发射红外线讯号

#include "ir_Midea.h""

IRMideaAC m_IRMideaAC;





unsigned int changes = 0;


//unsigned long time[SAMPLE_NUMBER];


#define  SAMPLE_NUMBER 200
byte IrData[6] = {0,0,0,0,0,0};
bool isComamndStart = false;
bool FrameOK = false;

unsigned char Temperature;
unsigned char Mode;




void SecondsSinceStartTask();
unsigned char acTemperature(unsigned char code);
void DecodeAc();
void OnTenthSecond();

void SendIr();



void setup()
{
	Serial.begin(115200);
	Serial.println(F("test_ac"));
	printf_begin();




	//used for RX

	pinMode(2, INPUT_PULLUP);
	attachInterrupt(0, blink, RISING);



	//used for TX
	//pinMode(3, OUTPUT); 
	//digitalWrite(3, HIGH);








	//detachInterrupt(0);
	//pinMode(2, OUTPUT);
	//digitalWrite(2, HIGH);

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
	if (TenthSecondsSinceStart%50 == 0)
	{
		//printf("send command \r\n");
		//m_IRMideaAC.setMode(kMideaACCool);
		//m_IRMideaAC.setFan(kMideaACFanAuto);
		//m_IRMideaAC.send();
		//delay(5000);

	}
}

void DecodeAc()
{
	if (FrameOK)
	{
		printf("Get IR data:");
		for (unsigned char i = 0; i<6 ; i++)
		{
			printf("0x%02X ",IrData[i]);
		}
		printf("\r\n");

		if(IrData[0] == 0xb2)
		{
			printf("Media AC checked!\r\n");
		}


		if((IrData[0] == (byte)(~(IrData[1])))&&(IrData[2] == (byte)(~(IrData[3])))&&(IrData[4] == (byte)(~(IrData[5]))))
		{
			printf("Data checked!\r\n");


			if ((IrData[2] == 0x7B)&&(IrData[4] == 0xE0))
			{
				printf("Mode = 关机 \r\n");
			}
			else

			{


				Temperature = acTemperature(IrData[4]>>4);
				printf("Temperature = %d \r\n",Temperature);

				Mode = (IrData[4]>>2)&3;//0冷 1风湿 2自 3热 
				switch(Mode)
				{
				case 0:
					printf("Mode = 制冷 \r\n");
					break;
				case 1:
					if (Temperature == 0)
					{
						printf("Mode = 送风 \r\n");
					} 
					else
					{
						printf("Mode = 抽湿 \r\n");
					}
					break;
				case 2:
					printf("Mode = 自动 \r\n");
					break;
				case 3:
					printf("Mode = 制热 \r\n");
					break;
				}
			}
		}
		else
		{
			printf("Data checke failed!\r\n");
		}

		printf("\r\n");


		for (unsigned char i = 0; i<6 ; i++)
		{
			IrData[i] = 0;
		}
		FrameOK = false;
		isComamndStart = false;
	}
}

void loop()
{

	SecondsSinceStartTask();

	DecodeAc();

	//delay(1000);
}

unsigned char acTemperature(unsigned char code)
{
	switch(code)
	{
	case 0:
		return 17; 
		break;
	case 1:
		return 18; 
		break;
	case 3:
		return 19; 
		break;
	case 2:
		return 20; 
		break;
	case 6:
		return 21; 
		break;
	case 7:
		return 22; 
		break;
	case 5:
		return 23; 
		break;
	case 4:
		return 24; 
		break;
	case 12:
		return 25; 
		break;
	case 13:
		return 26; 
		break;
	case 9:
		return 27; 
		break;
	case 8:
		return 28; 
		break;
	case 10:
		return 29; 
		break;
	case 11:
		return 30; 
		break;
	case 14:
		return 0; 
		break;
	}
}



void blink()//中断函数
{
	unsigned long ThisTime;
	unsigned long DiffTime;
	unsigned int PulseTime;
	unsigned int IdleTime;

	static unsigned long PulseStartTime = 0;
	static bool isLastLong = true;
	static unsigned char BitCounter = 0;
	static unsigned long LastTime = 0;


	if (BitCounter<SAMPLE_NUMBER)
	{
		if (LastTime == 0)
		{
			LastTime = micros();
		} 
		else
		{
			ThisTime = micros();
			DiffTime = ThisTime-LastTime;
			if (DiffTime < 100)
			{
				if (isLastLong)
				{
					PulseStartTime = ThisTime;
					isLastLong = false;
				}
			}
			else
			{
				PulseTime = LastTime - PulseStartTime;
				IdleTime = DiffTime;

				if (isComamndStart)
				{
					if (IdleTime > 3800)
					{
						FrameOK = true;
						BitCounter = 0;
						LastTime = 0;
					}
					else if(!FrameOK)
					{
						if (IdleTime > 1000)//bit 1
						{
							IrData[BitCounter/8] += 1<<(7-BitCounter%8);
						}
						else//bit 0
						{
						}
						BitCounter++;
					}
				}
				else
				{
					if ((PulseTime > 3800)&&(IdleTime > 3800))
					{
						isComamndStart = true;
					}
				}
				isLastLong = true;
			}
			LastTime = ThisTime;
		}
	}
}

//void blink3()//中断函数
//{
//	unsigned long ThisTime;
//	unsigned long DiffTime;
//	bool lastLongTime;
//
//	if (counter<100)
//	{
//		if (LastTime == 0)
//		{
//			LastTime = micros();
//		} 
//		else
//		{
//			ThisTime = micros();
//			DiffTime = ThisTime-LastTime;
//			if (DiffTime < 100)
//			{
//				
//				if (lastLongTime)
//				{
//					counter++;
//				}
//				lastLongTime = false;
//				time[counter] = time[counter] + DiffTime;
//				
//			}
//			else
//			{
//				counter++;
//				time[counter] = DiffTime;
//				lastLongTime = true;
//
//
//			}
//			LastTime = ThisTime;
//		}
//	}
//}

//void blink()//中断函数
//{
//	printf("int\r\n");
//}