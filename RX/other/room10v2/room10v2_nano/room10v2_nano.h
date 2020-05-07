
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define RF_COMMAND_INTERVAL 10   //s
#define DATA_LENGTH 4					//use fixed data length 1-32
#define DEGBUG_OUTPUT
#define MAX_TAG_NUMBER 8
#define TIME_OUT_KEY 100  //0.1s
/*****************************************************/

#include <SPI.h>
#include "RF24.h"
#include <printf.h>


#include <Wire.h>
unsigned long IicRecvTimeOutTenthSecond = 0;

#include "D:\GitHub\Open-Source-RKS\RX\other\room10v2\struct.h"
tIicCommand IicCommand;
tIicData IicData;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/
//#define xxx 2//				INT 0
//#define xxx 3//				INT 1
#define LOCAL_1 4//Loacal1
#define LOCAL_2 5//Loacal2
#define LOCAL_3 6//Loacal3
////D7,D8 for RF24
//#define xxx 9
//#define xxx 10
////D11,D12,D13 for RF24
#define RELAY_4 A0//relay1
#define RELAY_3 A1//relay2
#define RELAY_2 A2//relay3
#define RELAY_1 A3//relay4
//#define xxx A4//IIC_D
//#define xxx A5//IIC_C
//#define xxx A6//             input only
//#define xxx A7//             input only

#define RELAY_NUMBER 4
#define LOCAL_NUMBER 3
unsigned char RelayArray[RELAY_NUMBER];
unsigned char LocalArray[LOCAL_NUMBER];

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;
unsigned long LastTagGetTime[MAX_TAG_NUMBER];

unsigned long LastCommandSendTime = 0;   //unit: s


unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long TenthSecondsSinceStart;

bool LastLocalControl[3];


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


void SetRelay(unsigned char index, bool On);


void setup()
{

	pinMode(RELAY_1, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY_1, LOW);


	pinMode(RELAY_2, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY_2, LOW);
	pinMode(RELAY_3, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY_3, LOW);
	pinMode(RELAY_4, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY_4, LOW);

	pinMode(LOCAL_1, INPUT_PULLUP);
	LastLocalControl[0] = digitalRead(LOCAL_1);
	pinMode(LOCAL_2, INPUT_PULLUP);
	LastLocalControl[1] = digitalRead(LOCAL_2);
	pinMode(LOCAL_3, INPUT_PULLUP);
	LastLocalControl[2] = digitalRead(LOCAL_3);

	LocalArray[0] = LOCAL_1;
	LocalArray[1] = LOCAL_2;
	LocalArray[2] = LOCAL_3;


	//pinMode(RELAY_1, OUTPUT);//set the pin to be OUTPUT pin.
	//digitalWrite(RELAY_1, HIGH);
	//pinMode(RELAY_2, OUTPUT);//set the pin to be OUTPUT pin.
	//digitalWrite(RELAY_2, HIGH);
	//pinMode(RELAY_3, OUTPUT);//set the pin to be OUTPUT pin.
	//digitalWrite(RELAY_3, HIGH);
	//pinMode(RELAY_4, OUTPUT);//set the pin to be OUTPUT pin.
	//digitalWrite(RELAY_4, HIGH);



	RelayArray[0] = RELAY_1;
	RelayArray[1] = RELAY_2;
	RelayArray[2] = RELAY_3;
	RelayArray[3] = RELAY_4;

	IicData.RelayState = 0;

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
	//nRFTask();
	//ChHopTask();
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
		return;
	}

	for (unsigned char i = 0;i<4;i++)
	{
		//if a Relay Need to be Set
		//if ((IicCommand.RelayNeedSet>>i)&1>0)
		if (GetBit(IicCommand.RelayNeedSet,i))
		{
			//SetRelayData(i,(IicCommand.RelayState>>i)&1);
			SetRelay(i,GetBit(IicCommand.RelayState,i));
		}
	}
}

void requestEvent() 
{

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
			if (GotData[1]<MAX_TAG_NUMBER)
			{
				LastTagGetTime[GotData[1]] = TenthSecondsSinceStart;
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
	unsigned char TagState = 0;
	for (unsigned char i = 0; i < MAX_TAG_NUMBER; i++)
	{
		if (TenthSecondsSinceStart - LastTagGetTime[i] < TIME_OUT_KEY)
		{
			TagState = TagState + (0x01<<i);
		}
	}
	IicData.TagState = TagState;

	//printf("OnSecond \r\n");
}
void OnTenthSecond()
{
	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}

	for (byte i=0;i<LOCAL_NUMBER;i++)
	{
		bool LocalControl = digitalRead(LocalArray[i]);
		if (LastLocalControl[i] != LocalControl)
		{
			LastLocalControl[i] = LocalControl;
			printf("LocalControl[%d] to %d \r\n",i,LocalControl);
			//SetBit(&IicData.RelayState,i,!GetBit(IicData.RelayState,i));
			SetRelay(i,!GetBit(IicData.RelayState,i));
		} 
	}
}

void SetRelay(unsigned char index, bool On)
{
	SetBit(&IicData.RelayState,index,On);

	if (On)
	{
		printf("set relay:%d to HIGH \r\n",index);
		digitalWrite(RelayArray[index],HIGH);
	} 
	else
	{
		printf("set relay:%d to LOW \r\n",index);
		digitalWrite(RelayArray[index],LOW);
	}

	//for (byte i=0;i<4;i++)
	//{
	//	
		//if (GetBit(IicData.RelayState,i))
		//{
		//	printf("set relay:%d to HIGH \r\n",i);
		//	digitalWrite(RelayArray[i],HIGH);
		//} 
		//else
		//{
		//	printf("set relay:%d to LOW \r\n",i);
		//	digitalWrite(RelayArray[i],LOW);
		//}
	//	
	//}
}

