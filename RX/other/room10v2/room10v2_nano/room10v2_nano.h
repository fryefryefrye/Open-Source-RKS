
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define RF_COMMAND_INTERVAL 10   //s
#define DATA_LENGTH 4					//use fixed data length 1-32
#define DEGBUG_OUTPUT
/*****************************************************/

#include <SPI.h>
#include "RF24.h"
#include <printf.h>
//#include <avr/wdt.h>

#include <Wire.h>
//#define IIC_BYTE 8
//unsigned char IicSlaveSend[IIC_BYTE];
//unsigned char IicSlaveRecv[IIC_BYTE];
//unsigned char IicSlaveRecvCounter;
unsigned long IicRecvTimeOutTenthSecond = 0;

#include "D:\GitHub\Open-Source-RKS\RX\other\room10v2\struct.h"
tIicCommand IicCommand;
tIicData IicData;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/
//#define xxx 2//				INT 0
//#define xxx 3//				INT 1
//#define xxx 4//Loacal1
//#define xxx 5//Loacal2
//#define xxx 6//Loacal3
////D7,D8 for RF24
//#define xxx 9
//#define xxx 10
////D11,D12,D13 for RF24
//#define xxx A0//relay1
//#define xxx A1//relay2
//#define xxx A2//relay3
//#define xxx A3//relay4
//#define xxx A4//IIC_D
//#define xxx A5//IIC_C
//#define xxx A6//             input only
//#define xxx A7//             input only


unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;

unsigned long LastCommandSendTime = 0;   //unit: s


unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long TenthSecondsSinceStart;


//void DecodeRf_INT();
//unsigned char RcCommand[3] = {0,0,0};
//bool DecodeFrameOK = false;
//void CheckRf();
void TenthSecondsSinceStartTask();
void SecondsSinceStartTask();
void OnSecond();
void nRFInit();
void OnTenthSecond();
void nRFTask();
void ChHopTask();
void RF_task();
void receiveEvent(int howMany);
void requestEvent();

void setup()
{

	Wire.begin(8);                /* join i2c bus with address 8 */
	Wire.onReceive(receiveEvent); /* register receive event */
	Wire.onRequest(requestEvent); /* register request event */


#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_IIC_Test"));
	printf_begin();
#endif
	nRFInit();
	//attachInterrupt(0, DecodeRf_INT, CHANGE); //pin2

	printf("sizeof(tIicCommand) = %d sizeof(tIicCommand) = %d \r\n",sizeof(tIicCommand),sizeof(tIicCommand));
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


void loop()
{

	TenthSecondsSinceStartTask();
	nRFTask();
	ChHopTask();


	//CheckRf();



} // Loop

void receiveEvent(int howMany) 
{
	unsigned char RecvIndex = 0;
	while (0 <Wire.available()) 
	{
		if (RecvIndex<sizeof(tIicCommand))
		{
			((byte*)(&IicCommand))[RecvIndex] = Wire.read(); 
			RecvIndex++;
			IicRecvTimeOutTenthSecond = 0;
		}
	}

	//printf("IIC recv");
	//for (byte i=0;i<sizeof(tIicCommand);i++)
	//{
	//	printf(" 0x%02X",((byte*)(&IicCommand))[i]);
	//}
	//printf("\r\n");

	unsigned long crc_should = cal_crc((byte *)&IicCommand,sizeof(tIicCommand)-4);
	if (IicCommand.Sum == crc_should)
	{
		//printf("crc check passed. \n");
	}
	else
	{
		printf("crc check failed. should:0x%04X recv:0x%04X \n",crc_should,IicCommand.Sum);
	}

	//attachInterrupt(0, DecodeRf_INT, CHANGE); //pin2
	//printf("IIC slave get %d bytes %d,%d,%d,%d,%d,%d,%d,%d, \r\n"
	//	,IicSlaveRecvCounter
	//	,IicSlaveRecv[0]
	//,IicSlaveRecv[1]
	//,IicSlaveRecv[2]
	//,IicSlaveRecv[3]
	//,IicSlaveRecv[4]
	//,IicSlaveRecv[5]
	//,IicSlaveRecv[6]
	//,IicSlaveRecv[7]
	//);
}

void requestEvent() 
{
	IicData.RelayState = 0x55;
	IicData.Sum = cal_crc((byte *)&IicData,sizeof(tIicData)-4);

	//printf("IIC send");
	//for (byte i=0;i<sizeof(tIicData);i++)
	//{
	//	printf(" 0x%02X",((byte*)(&IicData))[i]);
	//}
	//printf("\r\n");

	Wire.write((byte*)(&IicData),sizeof(tIicData));

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


unsigned long LastMillis = 0;
void TenthSecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 100)
	{
		LastMillis = CurrentMillis;
		TenthSecondsSinceStart++;
		OnTenthSecond();
		//printf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
	}
}
void OnSecond()
{
	//printf("OnSecond \r\n");
}
void OnTenthSecond()
{
	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}


	//printf("OnTenthSecond \r\n");
	//wdt_reset();
}

//
//void CheckRf()
//{
//	//static unsigned char LastRf[3];
//	//if (DecodeFrameOK)
//	//{
//
//	//	printf("0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2]);
//
//	//	//if (memcmp(LastRf,RcCommand,3) == 0)
//	//	//{
//	//	//	//MyPrintf("0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2]);
//	//	//CheckRfCommand(RcCommand);
//	//	//} 
//	//	//else
//	//	//{
//	//	//	memcpy(LastRf,RcCommand,3);
//	//	//}
//	//	memset(RcCommand,0,3);
//	//	DecodeFrameOK = false;
//	//}
//}
//
//void DecodeRf_INT()
//{
//#define PULSE_NUMBER 48
//#define MIN_LEN 100
//#define MAX_LEN 2000
//#define LEAD_LEN 7000
//
//	unsigned long ThisTime;
//	unsigned long DiffTime;
//	static unsigned long FirstTime;
//	static unsigned long LastRfTime = 0;
//	static bool FrameStarted = false;
//	static bool RfOn = false;
//	static unsigned char PulseIndex = 0;
//	static unsigned int Base;
//	static unsigned int Min_Base;
//	static unsigned int Max_Base;
//
//	if (DecodeFrameOK)
//	{
//		return;
//	}
//
//	if (PulseIndex<PULSE_NUMBER)
//	{
//		if (LastRfTime == 0)
//		{
//			LastRfTime = micros();
//		} 
//		else
//		{
//			ThisTime = micros();
//			DiffTime = ThisTime-LastRfTime;
//			LastRfTime = ThisTime;
//
//			if (RfOn)
//			{
//				if (FrameStarted)
//				{
//					if ((DiffTime > Min_Base)&&(DiffTime < Max_Base))
//					{
//						if (PulseIndex%2==0)
//						{
//							FirstTime = DiffTime;
//						} 
//						else
//						{
//							unsigned char CommandIndex = (PulseIndex-1)/2/8;
//							RcCommand[CommandIndex] = RcCommand[CommandIndex]<<1;
//							if ((FirstTime>Base)&&(DiffTime<Base))//bit 1
//							{
//								RcCommand[CommandIndex]++;
//							} 
//							else
//							{
//								if ((FirstTime<Base)&&(DiffTime>Base))//bit 0
//								{
//
//								}
//								else//如果编码规则出错
//								{
//									LastRfTime = 0;
//									PulseIndex = 0;
//									FrameStarted = false;
//									RfOn = false;
//									RcCommand[0] = 0;
//									RcCommand[1] = 0;
//									RcCommand[2] = 0;
//								}
//							}
//						}
//						PulseIndex++;
//						if (PulseIndex >= PULSE_NUMBER)//收集到48个位
//						{
//							DecodeFrameOK = true;
//							LastRfTime = 0;
//							PulseIndex = 0;
//							FrameStarted = false;
//							RfOn = false;
//						}
//					}
//					else//如果时间长度出错
//					{	
//						LastRfTime = 0;
//						PulseIndex = 0;
//						FrameStarted = false;
//						RfOn = false;
//						RcCommand[0] = 0;
//						RcCommand[1] = 0;
//						RcCommand[2] = 0;
//					}
//				} 
//				else
//				{
//					if (DiffTime > LEAD_LEN)//收到引导码
//					{
//						Base = DiffTime/16;
//						Min_Base = DiffTime/62;
//						Max_Base = DiffTime/8;
//						FrameStarted = true;
//					}
//				}
//			}
//			else
//			{
//				if ((DiffTime > MIN_LEN)&&(DiffTime < MAX_LEN))
//				{
//					PulseIndex++;
//					if (PulseIndex > 32)//连续正确的时间长度
//					{
//						RfOn = true;
//						RcCommand[0] = 0;
//						RcCommand[1] = 0;
//						RcCommand[2] = 0;
//						PulseIndex = 0;
//						LastRfTime = 0;
//					}
//				}
//				else//时间长度异常
//				{	
//					LastRfTime = 0;
//					PulseIndex = 0;
//				}
//			}
//		}
//	}
//	else
//	{
//		LastRfTime = 0;
//		PulseIndex = 0;
//		FrameStarted = false;
//		RfOn = false;
//	}
//}
