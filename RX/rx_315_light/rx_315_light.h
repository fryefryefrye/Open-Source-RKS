
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


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;

unsigned long LastCommandSendTime = 0;   //unit: s


unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long SecondsSinceStart;





#define RF_315 6
#define RF_LENGTH 11


unsigned char RfCommand[1][RF_LENGTH] = {
	{ 0x00, 0x9A, 0x49, 0xA4, 0x92, 0x69, 0x26, 0x92, 0x4D, 0xA4, 0x80 }
};




void RF_Command(unsigned char command, unsigned char repeat);
void SecondsSinceStartTask();
void OnSecond();
void nRFInit();
void nRFTask();
void ChHopTask();
void RF_task();


void setup()
{
	pinMode(RF_315, OUTPUT);

#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_315_ID_Read"));
	printf_begin();
#endif

	//wdt_enable(WDTO_2S);

	nRFInit();


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

void OnSecond()
{
	//wdt_reset();


	if (SecondsSinceStart%5 == 0)
	{
		nRFInit();
#ifdef DEGBUG_OUTPUT
		printf("re init nRF \r\n");
#endif

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




void RF_task() // do not lock at home,  wait key for a little long at home
{


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
			if (SecondsSinceStart - LastCommandSendTime > RF_COMMAND_INTERVAL)
			{
				LastCommandSendTime = SecondsSinceStart;
#ifdef DEGBUG_OUTPUT
				printf("sending light command \r\n");
#endif
				RF_Command(0,10);
#ifdef DEGBUG_OUTPUT
				printf("sent light command \r\n");
#endif
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
	if (SecondsSinceStart - LastChangeCHTime > 0)             //RF_HOP every seconds
	{
		CurrCH++;
		if (CurrCH > 2)
		{
			CurrCH = 0;
		}
		LastChangeCHTime = SecondsSinceStart;
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
					digitalWrite(RF_315, HIGH );
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
					digitalWrite(RF_315, LOW);
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
	if (abs(CurrentMillis - LastMillis) > 1000)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;
		OnSecond();
		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
	}
}
