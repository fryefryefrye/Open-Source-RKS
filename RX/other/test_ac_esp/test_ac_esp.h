

ICACHE_RAM_ATTR void blink();



#include "ir_Midea.h"

IRMideaAC m_IRMideaAC(D7);



#define  SAMPLE_NUMBER 200
unsigned long counter;

unsigned int changes = 0;
unsigned long timeLen[SAMPLE_NUMBER];
unsigned char BitCounter = 0;
unsigned long LastTime = 0;
bool isComamndStart = false;

byte IrData[6] = {0,0,0,0,0,0};

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





	//used for RX

	pinMode(D6, INPUT_PULLUP);//NodeMCU D6 GPIO12
	attachInterrupt(digitalPinToInterrupt(D6), blink, RISING);

	m_IRMideaAC.begin();



	//used for TX
	//pinMode(IrLed, OUTPUT); 
	//digitalWrite(IrLed, HIGH);








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
	//if (TenthSecondsSinceStart%200 == 100)
	//{
	//	pinMode(2, INPUT);
	//	attachInterrupt(0, blink, RISING);
	//	printf("Set IR Rx \r\n");

	//	FrameOK = false;
	//	isComamndStart = false;
	//	BitCounter = 0;
	//	time[0] = 0;
	//	LastTime = 0;
	//}

	//if (TenthSecondsSinceStart%200 == 0)
	//{
	//	detachInterrupt(0);
	//	pinMode(2, OUTPUT);
	//	digitalWrite(2, HIGH);
	//	SendIr();
	//	printf("Set IR Tx \r\n");
	//}

	if (TenthSecondsSinceStart%50 == 0)
	{
		//printf("send command \r\n");
		//m_IRMideaAC.setMode(kMideaACCool);
		//m_IRMideaAC.setFan(kMideaACFanAuto);
		//m_IRMideaAC.send();
		//delay(5000);

		//m_IRMideaAC.setFan(kMideaACFanHigh);
		//m_IRMideaAC.send();
		//delay(5000);

		//m_IRMideaAC.setFan(kMideaACFanMed);
		//m_IRMideaAC.send();
		//delay(5000);

		//m_IRMideaAC.setFan(kMideaACFanLow);
		//m_IRMideaAC.send();
		//delay(5000);


	}

	if (TenthSecondsSinceStart%10 == 0)
	{
		printf("send command \r\n");
		m_IRMideaAC.send();
		DecodeAc();
	}

}



void SendIr()
{

		//10110010 01001101 00011111 11100000 11011000 00100111
		//Get IR data:0xB2 0x4D 0x1F 0xE0 0xD8 0x27 
		//Temperature = 26 
		//Mode = 自动 




	// Set pin 10’s PWM frequency to 31 Hz (31250/1024 = 31)

	delay(100);


}


void DecodeAc()
{
	if (FrameOK)
	{
		printf("%d bits \r\n",BitCounter);
		for (unsigned char i = 0; i<BitCounter ; i++)
		{
			if (i % 8 == 0)
			{
				printf(" ");
			}
			printf("%lu",timeLen[i]);
		}
		printf("\r\n");




		if (BitCounter == 48)
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
					printf("Mode = OFF \r\n");
				}
				else

				{


					Temperature = acTemperature(IrData[4]>>4);
					printf("Temperature = %d \r\n",Temperature);

					Mode = (IrData[4]>>2)&3;//0冷 1风湿 2自 3热 
					switch(Mode)
					{
					case 0:
						printf("Mode = Cold \r\n");
						break;
					case 1:
						if (Temperature == 0)
						{
							printf("Mode = Fan \r\n");
						} 
						else
						{
							printf("Mode = Dry \r\n");
						}
						break;
					case 2:
						printf("Mode = Auto \r\n");
						break;
					case 3:
						printf("Mode = Heat \r\n");
						break;
					}
				}
			}
			else
			{
				printf("Data checke failed!\r\n");
			}

		}



		for (unsigned char i = 0; i<6 ; i++)
		{
			IrData[i] = 0;
		}


		FrameOK = false;
		isComamndStart = false;

		printf("\r\n");
		BitCounter = 0;
		timeLen[0] = 0;
		LastTime = 0;
	}


}

void loop()
{

	SecondsSinceStartTask();

	//for (unsigned char i = 0; i<SAMPLE_NUMBER ; i++)
	//{
	//	printf("%lu ",timeLen[i]);
	//}
	//printf("\r\n");
	//counter = 0;

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


ICACHE_RAM_ATTR void blink()//中断函数
{
	unsigned long ThisTime;
	unsigned long DiffTime;
	static unsigned long PulseStartTime = 0;
	static bool isLastLong = true;




	unsigned int PulseTime;
	unsigned int IdleTime;

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
					//counter++;
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

					}
					else if(!FrameOK)
					{
						if (IdleTime > 1000)//bit 1
						{
							IrData[BitCounter/8] += 1<<(7-BitCounter%8);
							timeLen[BitCounter] = 1;

						}
						else//bit 0
						{
							timeLen[BitCounter] = 0;
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





				//counter++;
				//time[counter] = PulseTime;
				//counter++;
				//time[counter] = IdleTime;


				isLastLong = true;


			}
			LastTime = ThisTime;
		}
	}
}

void blink2()//中断函数
{


	Serial.print("Stamp(ms): ");
	Serial.println(millis());


	//unsigned long ThisTime;
	//unsigned long DiffTime;
	//bool lastLongTime;

	//if (counter<SAMPLE_NUMBER)
	//{
	//	timeLen[counter] = micros();
	//	counter++;
	//	//if (LastTime == 0)
	//	//{
	//	//	LastTime = micros();
	//	//} 
	//	//else
	//	//{
	//	//	ThisTime = micros();
	//	//	DiffTime = ThisTime-LastTime;
	//	//	if (DiffTime < 100)
	//	//	{
	//	//		
	//	//		if (lastLongTime)
	//	//		{
	//	//			counter++;
	//	//		}
	//	//		lastLongTime = false;
	//	//		timeLen[counter] = timeLen[counter] + DiffTime;
	//	//		
	//	//	}
	//	//	else
	//	//	{
	//	//		counter++;
	//	//		timeLen[counter] = DiffTime;
	//	//		lastLongTime = true;


	//	//	}
	//	//	LastTime = ThisTime;
	//	//}
	//}
}