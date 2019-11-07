
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx


#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 2000				//set lenght of the buzz 2000
#define BUZZOFF 2000			//set interval of the buzz 5000
#define DEGBUG_OUTPUT


#define TIME_OUT_KEY 100  //0.1s
#define TIME_OUT_READY 300  //0.1s

#define FIX_DISTANCE_IN 800
#define FIX_DISTANCE_OUT 1051
#define DISTANCE_DIFF 200

#include "Z:\bt\web\datastruct.h"


#include <SoftwareSerial.h>
SoftwareSerial SoftSerialOut(5, 4); //rx ,tx 
SoftwareSerial SoftSerialIn(3, 2); //rx ,tx 

//D6 
//D11 D12 free

//D7,D8 for RF24
#define BUZZ			9
#define CLOSED			10

#define RELAY_OPEN		A2
#define RELAY_CLOSE		A0
#define RELAY_UNLOCK	A1
#define RELAY_LOCK		A3
//#define KEY_OUT1		A4
//#define KEY_OUT2		A5
#define KEY_OUT1		11
#define KEY_OUT2		12
#define KEY_IN			A6
#define KEY_xx			A7

//A6 A7 are input only

unsigned int DistanceIn = 0;
unsigned int DistanceOut = 0;


/*****************************************************/


#include <SPI.h>
#include "RF24.h"
#include <printf.h>

#include <Wire.h>


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

unsigned char Buzz = 0;
bool Buzzed = false;

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;



unsigned long LastReadyOutGetTime = 0;
static unsigned long LastTagGetTime[MAX_TAG_NUMBER];


unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long TenthSecondsSinceStart;




unsigned char IicSlaveSend[IIC_SLAVE_SEND_BYTE];
unsigned char IicSlaveRecv[IIC_SLAVE_RECV_BYTE];
unsigned char IicSlaveRecvCounter;
unsigned long IicRecvTimeOutTenthSecond = 0;

bool isKeyPressed[4];
bool isDoorClosed = false;
bool isBodyExistIn = false;
bool isBodyExistOut = false;



void SecondsSinceStartTask();
void nRFTask();
void ChHopTask();
void RF_task();
void Buzz_task();
void CheckTag_task();
void OnTenthSecond();
void GetDistance_task();
void CheckKey_task();

bool KeyState(unsigned char KeyIndex);

void UnlockRelay(bool on);
void LockRelay(bool on);
void OpenRelay(bool on);
void CloseRelay(bool on);
bool IsDoorClosed();
void CheckDoor_task();

void receiveEvent(int howMany);
void requestEvent();


void setup()
{

	Wire.begin(8);                /* join i2c bus with address 8 */
	Wire.onReceive(receiveEvent); /* register receive event */
	Wire.onRequest(requestEvent); /* register request event */

	pinMode(RELAY_LOCK, OUTPUT);
	pinMode(RELAY_UNLOCK, OUTPUT);
	pinMode(RELAY_OPEN, OUTPUT);
	pinMode(RELAY_CLOSE, OUTPUT);
	pinMode(BUZZ, OUTPUT);

	digitalWrite(RELAY_LOCK,HIGH);
	digitalWrite(RELAY_UNLOCK,HIGH);
	digitalWrite(RELAY_OPEN,HIGH);
	digitalWrite(RELAY_CLOSE,HIGH);
	digitalWrite(BUZZ,HIGH);

	SoftSerialOut.begin(9600);
	SoftSerialIn.begin(9600);


	//digitalWrite(RELAY_LOCK,LOW);
	//digitalWrite(RELAY_UNLOCK,LOW);
	//digitalWrite(RELAY_OPEN,LOW);
	//digitalWrite(RELAY_CLOSE,LOW);




	//pinMode(KEY_OUT1, INPUT_PULLUP);
	//pinMode(KEY_OUT2, INPUT_PULLUP);
	pinMode(KEY_OUT1, INPUT);
	pinMode(KEY_OUT2, INPUT);
	pinMode(KEY_IN, INPUT);
	pinMode(KEY_xx, INPUT);





#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_AutoDoor_IO_Board"));
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
	SecondsSinceStartTask();
	nRFTask();
	ChHopTask();
	GetDistance_task();
} // Loop



void receiveEvent(int howMany) 
{
	unsigned char RecvIndex = 0;
	while (0 <Wire.available()) 
	{
		if (RecvIndex<IIC_SLAVE_RECV_BYTE)
		{
			IicSlaveRecv[RecvIndex] = Wire.read(); 
			RecvIndex++;
			IicRecvTimeOutTenthSecond = 0;
			IicSlaveRecvCounter = RecvIndex;
		}
	}
}

void requestEvent() 
{
	Wire.write(IicSlaveSend,IIC_SLAVE_SEND_BYTE);
	IicSlaveSend[IIC_SLAVE_SEND_BYTE-1]++;
}


void OnTenthSecond()
{


	//if (TenthSecondsSinceStart%10 == 0)
	//{


		if (IicRecvTimeOutTenthSecond>3)
		{
			printf("IIC restart\r\n");
			Wire.end();
			Wire.begin(8);
		}

		printf("IIC timeout = %d",IicRecvTimeOutTenthSecond);
		printf(" %d bytes ",IicSlaveRecvCounter);
		//printf("data: 0x%02X 0x%02X \r\n",IicSlaveRecv[0],IicSlaveRecv[1]);
		
		for (char i = 0; i < IIC_SLAVE_RECV_BYTE; i++)
		{
			printf("0x%02X ",IicSlaveRecv[i]);
		}
		printf("\r\n");

		IicRecvTimeOutTenthSecond++;
	//}


	if (IicSlaveRecv[0] == (unsigned char)(~IicSlaveRecv[1]))
	{
		digitalWrite(RELAY_LOCK		,((IicSlaveRecv[0])&1));
		digitalWrite(RELAY_UNLOCK	,((IicSlaveRecv[0]>>2)&1));
		digitalWrite(RELAY_OPEN		,((IicSlaveRecv[0]>>3)&1));
		digitalWrite(RELAY_CLOSE	,((IicSlaveRecv[0]>>4)&1));
		digitalWrite(BUZZ			,((IicSlaveRecv[0]>>5)&1));
	}
	else
	{
		//printf("IIC data check failed 0x%02X 0x%02X\r\n"
		//	,IicSlaveRecv[0]
		//	,IicSlaveRecv[1]);
	}

	CheckKey_task();
	CheckDoor_task();

	unsigned char TagState = 0;
	for (unsigned char i = 0; i < MAX_TAG_NUMBER; i++)
	{
		if (TenthSecondsSinceStart - LastTagGetTime[i] < TIME_OUT_KEY)
		{
			TagState = TagState + 0x01<<i;
		}
	}
	IicSlaveSend[0] = TagState;
	IicSlaveSend[1] = (unsigned char)(~TagState);



	unsigned char TempState = 0;
	TempState = TempState + isDoorClosed;
	TempState = TempState<<1 + isBodyExistIn;
	TempState = TempState<<1 + isBodyExistOut;
	TempState = TempState<<1 + isKeyPressed[3];
	TempState = TempState<<1 + isKeyPressed[2];
	TempState = TempState<<1 + isKeyPressed[1];
	TempState = TempState<<1 + isKeyPressed[0];

	IicSlaveSend[2] = TempState;
	IicSlaveSend[3] = (unsigned char)(~TempState);
}


void CheckDoor_task()
{
	isDoorClosed = IsDoorClosed();
}


void CheckTag_task()
{


}


void GetDistance_task()
{
	static unsigned char Step = 1;
	static unsigned long LastCheckMillis;

	static unsigned char BodyDisappearCounterIn = 0;
	static unsigned char BodyDisappearCounterOut = 0;



	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastCheckMillis) > 25)
	{
		LastCheckMillis = CurrentMillis;

		switch(Step)
		{
		case 0:
			SoftSerialOut.listen();
			SoftSerialOut.write(0x55);
			Step++;
			break;
		case 1:

			if (SoftSerialOut.read() == 0xFF)
			{
				DistanceOut = SoftSerialOut.read()<<8;
				DistanceOut = DistanceOut + SoftSerialOut.read();
				SoftSerialOut.read();
				//printf("DistanceOut = %d mm. \r\n",DistanceOut);
			}
			if((DistanceOut < FIX_DISTANCE_OUT - DISTANCE_DIFF)||(DistanceOut > FIX_DISTANCE_OUT + DISTANCE_DIFF))
			{
				isBodyExistOut = true;
				BodyDisappearCounterOut = 0;
				//printf("DistanceOut = %d mm. \r\n",DistanceOut);
			}
			else
			{
				if (BodyDisappearCounterOut>3)
				{
					isBodyExistOut = false;
				}
				else
				{
					BodyDisappearCounterOut++;
				}
			}
			Step++;
			break;
		case 2:
			SoftSerialIn.listen();
			SoftSerialIn.write(0x55);
			Step++;
			break;
		case 3:
			if (SoftSerialIn.read() == 0xFF)
			{
				DistanceIn = SoftSerialIn.read()<<8;
				DistanceIn = DistanceIn + SoftSerialIn.read();
				SoftSerialIn.read();
			}
			Step = 0;
			if((DistanceIn < FIX_DISTANCE_IN - DISTANCE_DIFF)||(DistanceIn > FIX_DISTANCE_IN + DISTANCE_DIFF))
			{
				isBodyExistIn = true;
				BodyDisappearCounterIn = 0;
				//printf("DistanceIn = %d mm. \r\n",DistanceIn);
			}
			else
			{
				if (BodyDisappearCounterIn>10)
				{
					isBodyExistIn = false;
				}
				else
				{
					BodyDisappearCounterIn++;
				}
			}

			//printf("DistanceOut = %d mm.DistanceIn = %d mm. \r\n",DistanceOut,DistanceIn);

			break;

		default:

			Step = 0;
			break;
		}
	}
}


void CheckKey_task()
{
	isKeyPressed[0] = KeyState(0);
	isKeyPressed[1] = KeyState(1);
	isKeyPressed[2] = KeyState(2);
	isKeyPressed[3] = KeyState(3);
	
}


bool KeyState(unsigned char KeyIndex)
{
	switch(KeyIndex)
	{
	case 0:
		return digitalRead(KEY_OUT1);
		break;
	case 1:
		return digitalRead(KEY_OUT2);
		break;
	case 2:
		return (analogRead(KEY_IN)>900);
		break;
	case 3:
		return (analogRead(KEY_xx)>900);
		break;
	}
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
		//Serial.print(PackageCounter);
		//Serial.print(" ");
		//Serial.print(F("Get data "));
		//for (char i = 0; i < DATA_LENGTH; i++)
		//{
		//	printf("%d,", GotData[i]);
		//}
		//printf("Volt:%d ", (int)Volt);
		//printf("CH:%d\r\n", CurrCH);
#endif
	}

}
void ChHopTask()
{
	if (TenthSecondsSinceStart - LastChangeCHTime > 0)             //RF_HOP every seconds
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

		//printf("CH change \r\n");
	}
}

unsigned long LastMillis = 0;
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

bool IsDoorClosed()
{
	return !digitalRead(CLOSED);
}
