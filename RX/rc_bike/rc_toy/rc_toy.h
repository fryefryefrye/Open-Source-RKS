
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx

#define DATA_LENGTH 4					//use fixed data length 1-32
#define DEGBUG_OUTPUT

#define LOST_CONTROL  3//Tenth s

#define THROTLE_MIDDLE 128
#define RUDDER_MIDDLE 128
#define THROTLE_DEAD_ZONE 70


/*****************************************************/

#include <SPI.h>
#include "RF24.h"
#include <printf.h>
#include <Servo.h> 
//#include <avr/wdt.h>


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/



unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;




unsigned long LastRcGetTime = 0;  //unit: Tenth s
unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long TenthSecondsSinceStart;


bool RcControled = false;

unsigned char RcCode;
//unsigned char RcThrottle;
int RcThrottle;
int RcRudder;

int LeftSpeed;
int RightSpeed;



//#define THROTTLE_OUTPUT			2
//#define RUDDER_OUTPUT			3
//#define REMOTE_LOCAL			4
//#define PARK_SWITCH				5
//#define RF_315					6
//RF24							7
//RF24							8


//RF24							11
//RF24							12


#define LR	A0
#define LF	A1
#define RR	A2
#define RF	A3

#define LEFT_PWM 9
#define RIGHT_PWM 10

void TenthSecondsSinceStartTask();
void OnTenthSecond();
void nRFInit();
void nRFTask();
void ChHopTask();
void RC_out();

void setup()
{
	//pinMode(RF_315, OUTPUT);

	pinMode(LF, OUTPUT);
	pinMode(LR, OUTPUT);
	pinMode(RF, OUTPUT);
	pinMode(RR, OUTPUT);


#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RC_toy_rx"));
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

void OnTenthSecond()
{



	//If lost control
	if ((RcControled)&&(TenthSecondsSinceStart - LastRcGetTime > LOST_CONTROL))
	{
		RcControled = false;

		printf("lost control \r\n");
	}





	if (TenthSecondsSinceStart%50 == 0)
	{
		nRFInit();
		printf("re init nRF \r\n");
	}
}

void loop()
{

	TenthSecondsSinceStartTask();
	nRFTask();
	ChHopTask();


	RC_out();




} // Loop




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



		//Get RC data
		if (GotData[0] == 4)
		{
			RcCode = GotData[1];
			RcThrottle = GotData[2];
			//RcRudder = GotData[3];

			if(RcThrottle < (THROTLE_MIDDLE - THROTLE_DEAD_ZONE))

			{
				RcRudder = 255 - GotData[3];

			}
			else
			{
				RcRudder =  GotData[3];
			}


			LastRcGetTime = TenthSecondsSinceStart;
			if (!RcControled)
			{

				RcControled = true;
			}
		}





#ifdef DEGBUG_OUTPUT
		//Serial.print(PackageCounter);
		//Serial.print(" ");

		//if (GotData[0] == 4)
		//{
		//	Serial.print(F("Get RC data "));

		//	printf("code = %d,", GotData[1]);
		//	printf("THROTTLE = %d,", GotData[2]);
		//	printf("RUDDER = %d,", GotData[3]);
		//	printf("CH:%d\r\n", CurrCH);
		//} 
		//else
		//{
		//	Serial.print(F("Get data "));
		//	for (char i = 0; i < DATA_LENGTH; i++)
		//	{
		//		printf("%d,", GotData[i]);
		//	}
		//	printf("Volt:%d ", Volt);
		//	printf("CH:%d\r\n", CurrCH);
		//}



#endif
	}

}
void ChHopTask()
{
	if (TenthSecondsSinceStart - LastChangeCHTime > 2)             //RF_HOP every seconds, or 0.1seconds
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
		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
	}


}



void RC_out()
{
	if (RcControled)
	{
		//if (RcThrottle<(THROTLE_MIDDLE - THROTLE_DEAD_ZONE))
		//{
		//	digitalWrite(LF,HIGH);
		//	digitalWrite(LR,LOW);
		//	digitalWrite(RF,HIGH); 
		//	digitalWrite(RR,LOW);
		//} 
		//else if (RcThrottle>(THROTLE_MIDDLE + THROTLE_DEAD_ZONE))
		//{
		//	digitalWrite(LR,HIGH);
		//	digitalWrite(LF,LOW);
		//	digitalWrite(RR,HIGH); 
		//	digitalWrite(RF,LOW);
		//}
		//else
		//{
		//	digitalWrite(LR,LOW);
		//	digitalWrite(LF,LOW);
		//	digitalWrite(RR,LOW);
		//	digitalWrite(RF,LOW); 
		//}



		//analogWrite(LEFT_PWM, 255);
		//analogWrite(RIGHT_PWM, 255);






		//speed control


		LeftSpeed = THROTLE_MIDDLE - RcThrottle;
		RightSpeed = LeftSpeed;

		LeftSpeed = LeftSpeed + (RcRudder-RUDDER_MIDDLE);
		RightSpeed = RightSpeed - (RcRudder-RUDDER_MIDDLE);

		//if ((LeftSpeed < 0)&&(RightSpeed < 0))// exange speed when revese



		//if(RcThrottle > THROTLE_MIDDLE + THROTLE_DEAD_ZONE)
		//{
		//	analogWrite(RIGHT_PWM , min(abs(LeftSpeed)*2,255));//0-255
		//	analogWrite(LEFT_PWM, min(abs(RightSpeed)*2,255));//0-255

		//} 
		//else
		//{
		//	analogWrite(LEFT_PWM, min(abs(LeftSpeed)*2,255));//0-255
		//	analogWrite(RIGHT_PWM, min(abs(RightSpeed)*2,255));//0-255
		//}

		analogWrite(LEFT_PWM, min(abs(LeftSpeed)*2,255));//0-255
		analogWrite(RIGHT_PWM, min(abs(RightSpeed)*2,255));//0-255



		if (LeftSpeed > 0)
		{
			digitalWrite(LF,HIGH);
			digitalWrite(LR,LOW);
		} 
		else
		{
			digitalWrite(LR,HIGH);
			digitalWrite(LF,LOW);
		}

		if (RightSpeed > 0)
		{
			digitalWrite(RF,HIGH);
			digitalWrite(RR,LOW);
		} 
		else
		{
			digitalWrite(RR,HIGH);
			digitalWrite(RF,LOW);
		}


		printf("Throttle:%d ,Rudder:%d ",RcThrottle,RcRudder);
		printf("L_PWM:%d ,R_PWM:%d ",min(abs(LeftSpeed)*2,255),min(abs(RightSpeed)*2,255));
		printf("LSpeed:%d ,RSpeed:%d \r\n",LeftSpeed,RightSpeed);


	}
	else
	{
		digitalWrite(LR,LOW); 
		digitalWrite(LF,LOW); 
		digitalWrite(RR,LOW); 
		digitalWrite(RF,LOW);
		analogWrite(LEFT_PWM, 0);
		analogWrite(RIGHT_PWM, 0);

	}




}

