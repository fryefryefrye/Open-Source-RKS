

/*************************user modify settings****************************/
byte addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char  HopCH[3] = {105,76,108};//Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define TIME_OUT_CLOSE_DOOR 5000		//ms
#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 1000				//set lenght of the buzz
#define BUZZOFF 30000			//set interval of the buzz

#define DEGBUG_OUTPUT
/*****************************************************/



#include <SPI.h>
#include "RF24.h"
#include <printf.h>


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/



unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;

unsigned long LastKeyGetTime = 0;
unsigned long LastHomeGetTime = 0;

unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long SecondsSinceStart;



#define RF_315 6
#define RF_LENGTH 11
unsigned char  RfCommand[3][RF_LENGTH]={//Lock,Unlock,Power
	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB2 ,0xDB ,0x7F},
	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB6 ,0x5B ,0x7F},
	{0xFF ,0x25 ,0xB6 ,0x4B ,0x64 ,0x96 ,0xD9 ,0x2C ,0xB6 ,0xCB ,0x7F}
};



void RF_Command(unsigned char command,unsigned char repeat);
void SecondsSinceStartTask();
void nRFTask();
void ChHopTask();




void setup()
{
	pinMode(RF_315, OUTPUT);

#ifdef DEGBUG_OUTPUT
	Serial.begin(9600);
	Serial.println(F("RF24_WiFi_ID_Read"));
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
	radio.openReadingPipe(0,addresses);
	radio.closeReadingPipe(1);
	radio.closeReadingPipe(2);
	radio.closeReadingPipe(3);
	radio.closeReadingPipe(4);
	radio.closeReadingPipe(5);
	radio.setAutoAck(1,false);
	radio.setAutoAck(2,false);
	radio.setAutoAck(3,false);
	radio.setAutoAck(4,false);
	radio.setAutoAck(5,false);
	radio.setAutoAck(0,false);
	//Start the radio listening for data
	radio.startListening();
}

void loop()
{
	SecondsSinceStartTask();
	nRFTask();
	ChHopTask();

	//RF_task();

	RF_Command(2,10);
	delay(300);
	RF_Command(2,10);
	delay(10000);
	RF_Command(1,10);
		delay(10000);
} // Loop


void nRFTask()
{
	if( radio.available())
	{
		// Variable for the received timestamp
		while (radio.available())                                     // While there is data ready
		{
			radio.read( GotData, DATA_LENGTH );             // Get the payload
		}

		PackageCounter++;

		Volt=1.2*(GotData[DATA_LENGTH-2]*256+GotData[DATA_LENGTH-1])*3*1000/4096;

		if (GotData[0] == 0)
		{
			LastKeyGetTime = SecondsSinceStart;
		}
		else if (GotData[0] == 1)
		{
			LastHomeGetTime = SecondsSinceStart;
		}



#ifdef DEGBUG_OUTPUT
		Serial.print(PackageCounter);
		Serial.print(" ");
		Serial.print(F("Get data "));
		for(char i=0; i<DATA_LENGTH ; i++)
		{
			printf("%d,",GotData[i]);
		}
		printf("Volt:%d ",Volt);
		printf("CH:%d\r\n",CurrCH);
#endif
	}

}
void ChHopTask()
{
	if (SecondsSinceStart - LastChangeCHTime>0)//RF_HOP every seconds
	{
		CurrCH++;
		if (CurrCH>2)
		{
			CurrCH = 0;
		}
		LastChangeCHTime = SecondsSinceStart;
		radio.stopListening();
		radio.setChannel(HopCH[CurrCH]);
		radio.startListening();
	}
}


void RF_Command(unsigned char command,unsigned char repeat)
{
	unsigned char k;//bit
	unsigned char j;//repeat
	unsigned char i;//byte

	bool LastBit;

	for(j = 0;j < repeat;j++)
	{
		for(i = 0;i < RF_LENGTH;i++)
		{
			for(k = 0;k < 8;k++)
			{
				if((RfCommand[command][i]>>(7-k))&1)
				{
					digitalWrite(RF_315, LOW);
					//Serial.print(1);
					if (LastBit)
					{
						delayMicroseconds(385*2);
					} 
					else
					{
						delayMicroseconds(385);
					}

					LastBit = true;
				}
				else
				{
					digitalWrite(RF_315, HIGH);
					//Serial.print(0);
					if (LastBit)
					{
						delayMicroseconds(385);
					} 
					else
					{
						delayMicroseconds(385*2);
					}

					LastBit = false;
				}
				//delayMicroseconds(514);
			}
		}
		delay(12);
		printf("\r\n");
	}

	digitalWrite(RF_315, LOW);

}


unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if(abs(CurrentMillis-LastMillis)> 1000)
	{
		LastMillis = (CurrentMillis/1000)*1000;
		SecondsSinceStart++;

		printf("SecondsSinceStart = %d ; LastMillis = %d\r\n",SecondsSinceStart,LastMillis);

	}
}