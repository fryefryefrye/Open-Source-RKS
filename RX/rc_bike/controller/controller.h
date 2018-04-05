
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define DATA_LENGTH 4					//use fixed data length 1-32
#define DEGBUG_OUTPUT



#define PRESS_INPUT 5
#define THROTTLE_INPUT 7
#define RUDDER_INPUT 6


/*****************************************************/

#include <SPI.h>
#include "RF24.h"
#include <printf.h>
//#include <avr/wdt.h>


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/



unsigned char DataBuf[DATA_LENGTH];
unsigned char CurrCH = 0;


void nRFInit();
void SendData();
void ChHop();

void setup()
{




#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_rc_bike_controller"));
	printf_begin();
#endif

	//wdt_enable(WDTO_2S);

	nRFInit();


	DataBuf[0] = 4;
	DataBuf[1] = 0;
	DataBuf[2] = 127;
	DataBuf[3] = 127;

	pinMode(PRESS_INPUT, INPUT_PULLUP);


}

void loop()
{
	DataBuf[1] = digitalRead(PRESS_INPUT);
	DataBuf[2] =  analogRead(THROTTLE_INPUT)/4;
	DataBuf[3] =  analogRead(RUDDER_INPUT)/4;
	delay(5);
	SendData();
	delay(5);




	ChHop();



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
	//radio.startListening();


}




void SendData()
{
	radio.write(DataBuf, 4);
}


void ChHop()
{
		CurrCH++;
		if (CurrCH > 2)
		{
			CurrCH = 0;
		}
		//radio.stopListening();
		radio.setChannel(HopCH[CurrCH]);
		//radio.startListening();
}