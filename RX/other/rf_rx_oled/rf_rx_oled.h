
#include <printf.h>
#include <Wire.h>
#include "CN_SSD1306_Wire\CN_SSD1306_Wire.h"
#include <EEPROM.h>

CN_SSD1306_Wire Displayer(8);//HardWare I2C


#include "RCSwitch.h"
RCSwitch mySwitch = RCSwitch();


void ShowState();
void DecodeRf_315_INT();
void DecodeRf_433_INT();
void CheckRf();
void SecondsSinceStartTask();

bool DecodeFrameOK_315 = false;
unsigned char RcCommand_315[3] = {0,0,0};

bool DecodeFrameOK_433 = false;
unsigned char RcCommand_433[3] = {0,0,0};

unsigned char RcCommand[3] = {0,0,0};

unsigned int LastBase;

char str_buf[8];

void setup()
{

	Displayer.Initial();
	delay(10);

	Displayer.Fill_Screen(0x00,0x00);



	Serial.begin(115200);
	Serial.println(F("rf_rx_oled"));
	printf_begin();

	mySwitch.enableTransmit(4);
	mySwitch.setRepeatTransmit(10);

	if(EEPROM.read(0) == 3)
	{
		printf("Last 315 Mhz \r\n");


		RcCommand[2] = EEPROM.read(1);
		RcCommand[1] = EEPROM.read(2);
		RcCommand[0] = EEPROM.read(3);
		printf("read:0x%02X 0x%02X 0x%02X\r\n"
			,RcCommand[0],RcCommand[1],RcCommand[2]);

		mySwitch.setPulseLength(EEPROM.read(4)*10);
		mySwitch.send(RcCommand, 24);
	}

	Displayer.ShowCN3232(0,0,2);
	Displayer.ShowCN3232(32,0,3);

	sprintf(str_buf,"%dG%02X%02X%02X",EEPROM.read(0),RcCommand[2],RcCommand[1],RcCommand[0]);

	for (byte i=0;i<8;i++)
	{
		Displayer.ShowASCII1632((i)*16,4,str_buf[i]-0x30);
	}

	sprintf(str_buf,"%d",EEPROM.read(4)*10);
	Displayer.ShowASCII1632(64,0,str_buf[0]-0x30);
	Displayer.ShowASCII1632(64+16,0,str_buf[1]-0x30);
	Displayer.ShowASCII1632(64+16*2,0,str_buf[2]-0x30);


	delay(3000);



	Displayer.Fill_Screen(0x00,0x00);

	attachInterrupt(0, DecodeRf_315_INT, CHANGE);//pin2
	//attachInterrupt(1, DecodeRf_433_INT, CHANGE);//pin3

	Displayer.ShowCN3232(0,0,0);
	Displayer.ShowCN3232(32,0,1);




}

void loop()
{
	CheckRf();
	ShowState();

	SecondsSinceStartTask();

} // Loop

unsigned long LastMillis = 0;
unsigned long SecondsSinceStart = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 1000)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;
		//OnSecond();
		printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
	}

}
void ShowState()
{






		//Displayer.ShowASCII1632(32,4,12);
		//Displayer.ShowASCII1632(16*5,4,10);
		//Displayer.ShowASCII1632(16*7,4,11);
		//Displayer.ShowASCII816(0,0,13);//C
		//Displayer.ShowASCII816(0,2,14);//D
		//Displayer.ShowASCII816(3*8,0,15);//.
		//Displayer.ShowASCII816(3*8+56-16,0,15);//.
		//Displayer.ShowASCII816(3*8,2,15);//.
		//Displayer.ShowASCII816(3*8+56-16,2,15);//.
		//Displayer.ShowASCII816(9*8+56-24,0,15);//.
		//Displayer.ShowASCII816(9*8+56-24+16,0,12);//V
		//Displayer.ShowASCII816(9*8+56-24,2,15);//.
		//Displayer.ShowASCII816(9*8+56-24+16,2,16);//%



}


void CheckRf()
{
	


	if (DecodeFrameOK_315)
	{
		printf("315Mhz:0x%02X 0x%02X 0x%02X Base = %d\r\n",RcCommand_315[0],RcCommand_315[1],RcCommand_315[2],LastBase);
		sprintf(str_buf,"%dG%02X%02X%02X",3,RcCommand_315[0],RcCommand_315[1],RcCommand_315[2]);

		for (byte i=0;i<8;i++)
		{
			Displayer.ShowASCII1632((i)*16,4,str_buf[i]-0x30);
		}

		sprintf(str_buf,"%d",LastBase);
		Displayer.ShowASCII1632(64,0,str_buf[0]-0x30);
		Displayer.ShowASCII1632(64+16,0,str_buf[1]-0x30);
		Displayer.ShowASCII1632(64+16*2,0,str_buf[2]-0x30);

		EEPROM.write(0,3);
		EEPROM.write(1,RcCommand_315[0]);
		EEPROM.write(2,RcCommand_315[1]);
		EEPROM.write(3,RcCommand_315[2]);

		EEPROM.write(4,LastBase/10);

		DecodeFrameOK_315 = false;
	}

	if (DecodeFrameOK_433)
	{
		printf("433Mhz:0x%02X 0x%02X 0x%02X Base = %d\r\n",RcCommand_433[0],RcCommand_433[1],RcCommand_433[2],LastBase);
		sprintf(str_buf,"%dG%02X%02X%02X",4,RcCommand_433[0],RcCommand_433[1],RcCommand_433[2]);

		for (byte i=0;i<8;i++)
		{
			Displayer.ShowASCII1632((i)*16,4,str_buf[i]-0x30);
		}

		sprintf(str_buf,"%d",LastBase);
		Displayer.ShowASCII1632(64,0,str_buf[0]-0x30);
		Displayer.ShowASCII1632(64+16,0,str_buf[1]-0x30);
		Displayer.ShowASCII1632(64+16*2,0,str_buf[2]-0x30);

		DecodeFrameOK_433 = false;
	}
}



void DecodeRf_315_INT()
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


	if (DecodeFrameOK_315)
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
							RcCommand_315[CommandIndex] = RcCommand_315[CommandIndex]<<1;
							if ((FirstTime>Base)&&(DiffTime<Base))//bit 1
							{
								RcCommand_315[CommandIndex]++;
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
									RcCommand_315[0] = 0;
									RcCommand_315[1] = 0;
									RcCommand_315[2] = 0;
								}
							}
						}
						PulseIndex++;
						if (PulseIndex >= PULSE_NUMBER)//收集到48个位
						{
							DecodeFrameOK_315 = true;
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
						RcCommand_315[0] = 0;
						RcCommand_315[1] = 0;
						RcCommand_315[2] = 0;
					}
				} 
				else
				{
					if (DiffTime > LEAD_LEN)//收到引导码
					{
						Base = DiffTime/16;
						LastBase = Base;
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
						RcCommand_315[0] = 0;
						RcCommand_315[1] = 0;
						RcCommand_315[2] = 0;
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



void DecodeRf_433_INT()
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


	if (DecodeFrameOK_433)
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
							RcCommand_433[CommandIndex] = RcCommand_433[CommandIndex]<<1;
							if ((FirstTime>Base)&&(DiffTime<Base))//bit 1
							{
								RcCommand_433[CommandIndex]++;
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
									RcCommand_433[0] = 0;
									RcCommand_433[1] = 0;
									RcCommand_433[2] = 0;
								}
							}
						}
						PulseIndex++;
						if (PulseIndex >= PULSE_NUMBER)//收集到48个位
						{
							DecodeFrameOK_433 = true;
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
						RcCommand_433[0] = 0;
						RcCommand_433[1] = 0;
						RcCommand_433[2] = 0;
					}
				} 
				else
				{
					if (DiffTime > LEAD_LEN)//收到引导码
					{
						Base = DiffTime/16;
						LastBase = Base;
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
						RcCommand_433[0] = 0;
						RcCommand_433[1] = 0;
						RcCommand_433[2] = 0;
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
