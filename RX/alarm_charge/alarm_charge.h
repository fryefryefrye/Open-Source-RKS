
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
//#define TIME_OUT_TURN_OFF_BIKE 30		//s
//#define TIME_OUT_HOME 60

#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 700				//set lenght of the buzz 2000
#define BUZZOFF 1000			//set interval of the buzz 5000
#define DEGBUG_OUTPUT


#define TIME_OUT_KEY 100  //0.1s
#define BIKE_CHECK_TIME 10  //0.1s
#define PRE_ALARM_TIME 200  //0.1s
#define ALARM_AUDIO_LIMIT 600  //0.1s
#define ALARM_FLASH_LIMIT 6000  //0.1s
#define DATA_UPDATE_INTERVAL 10 //0.1s
#define WIFI_COMMAND_LEN 5


//analogWrite() on pins 3, 5, 6, 9, 10, and 11

//D2,D3 for SoftwareSerial
#define BAT_CHK_1 4
#define BAT_CHK_2 5
#define  AUDIO 6
//D7,D8 for RF24
#define BUZZ 9
#define CHARGE_SWITCH 10
//D11,D12,D13 for RF24



#define LIGHT A0
#define FLASH A1
#define POWER A2
#define ALARM A3
//IIC A4 SDA A5 SCL
//#define   A6 //analog in only
//#define   A7 //analog in only



/*****************************************************/


#include <SPI.h>
#include "RF24.h"
#include <printf.h>

#include <SoftwareSerial.h>
SoftwareSerial SoftSerial(2, 3); //rx 2,tx 3

#include <Wire.h>
#include "Adafruit_INA219.h"

Adafruit_INA219 ina219_1(INA219_ADDRESS);
Adafruit_INA219 ina219_2(INA219_ADDRESS+1);


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

unsigned char Buzz = 0;
bool Buzzed = false;

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;

unsigned long LastTagGetTime = 0;
unsigned long LastHomeGetTime = 0;


unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long TenthSecondsSinceStart;


bool Tag = false;
bool AlarmOn = false;
unsigned long AlarmOnCounter = 0;
bool BikeChecked[2] = {false,false};
unsigned long * BikeState;
unsigned char UnChangeCounter[2];
unsigned char BickCheckCounter = 0;
bool WifiOK = false;
//bool Home = false;
unsigned char RecvData[WIFI_COMMAND_LEN];
bool RecvDataOK = false;
unsigned int WaitForTimeOut = 300;
bool RecvStater = false;

#include "Z:\bt\web\datastruct.h"
//struct tUpdateData
//{
//	unsigned long volt_1 ;			//0.1v
//	unsigned long volt_2 ;			//0.1v
//	unsigned long volt_3 ;			//0.1v
//	unsigned long Current ;		//mA
//	unsigned long  Connected_1 ;
//	unsigned long  Connected_2 ;
//	unsigned long  Connected_3 ;
//	unsigned long Sum;
//};

tUpdateData UpdateData;

void SecondsSinceStartTask();
void nRFTask();
void ChHopTask();
void RF_task();
void Buzz_task();
void CheckTime_task();
void WiFi_task();
void OnKeyPress();
void OnTenthSecond();
void ESP8266_Check();

void NonStopTask();
bool _esp8266_waitFor(const char *string);
bool _esp8266_getch(char * RetData);


void setup()
{
	pinMode(BUZZ, OUTPUT);


	pinMode(POWER, OUTPUT);
	pinMode(ALARM, OUTPUT);
	pinMode(LIGHT, OUTPUT);
	pinMode(AUDIO, OUTPUT);
	pinMode(FLASH, OUTPUT);

		digitalWrite(LIGHT, HIGH);
		digitalWrite(ALARM, HIGH);
		digitalWrite(FLASH, HIGH);
		digitalWrite(POWER, HIGH);


		//digitalWrite(AUDIO, HIGH);
		


	pinMode(CHARGE_SWITCH, OUTPUT);



	pinMode(BAT_CHK_1, INPUT_PULLUP);
	pinMode(BAT_CHK_2, INPUT_PULLUP);



	SoftSerial.begin(115200);

	ina219_1.begin();
	ina219_2.begin();

#ifdef DEGBUG_OUTPUT
	Serial.begin(115200);
	Serial.println(F("RF24_alarm_charge"));
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

	SoftSerial.print(F("AT+RST\r\n"));
	WaitForTimeOut = 300;


	UpdateData.volt_1 = 0;
	UpdateData.volt_2 = 0;

	UpdateData.volt_1_updateTime = 0;
	UpdateData.volt_2_updateTime = 0;



	UpdateData.Current_1  = 0;		//mA
	UpdateData.Current_2  = 0;		//mA
	UpdateData.Connected_1 = false;
	UpdateData.Connected_2 = false;
	UpdateData.ChargeON  = true;

	BikeState = &(UpdateData.Connected_1);

}

void NonStopTask()
{
	SecondsSinceStartTask();
	nRFTask();
	CheckTime_task();
	ChHopTask();
	Buzz_task();

}
void loop()
{

	//if (!RecvStater)
	//{
	NonStopTask();
	WiFi_task();
	//}
	ESP8266_Check();
} // Loop



void CheckTime_task()
{

	if (TenthSecondsSinceStart - LastTagGetTime < TIME_OUT_KEY)
	{
		if (Tag == false)
		{
			printf("Get key \r\n");
			digitalWrite(LIGHT, LOW);
			AlarmOn = false;
		}
		Tag = true;
	} 
	else
	{
		if (Tag == true)
		{
			printf("Lost key \r\n");
			digitalWrite(LIGHT, HIGH);
		}
		Tag = false;
	}
}


void OnTenthSecond()
{


	if (RecvDataOK)
	{
		printf("Recv command = %d %d %d %d \r\n",RecvData[0],RecvData[1],RecvData[2],RecvData[3],RecvData[4]);
		UpdateData.ChargeON = RecvData[0];
		//UpdateData.ChargeON_2 = RecvData[1];
		RecvDataOK = false;
	}



	float Current;
	if (TenthSecondsSinceStart%10==0)
	{
		float Current = ina219_1.getCurrent_mA();
		if (Current < 0)
		{
			Current = 0;
		}
		UpdateData.Current_1 = Current;
		//UpdateData.Current_2 = ina219_2.getCurrent_mA();


		//printf("current  =  %d \r\n",UpdateData.Current);

		unsigned int testvolt = ina219_1.getBusVoltage_V()*1000;

		printf("test volt  =  %d \r\n",testvolt);

		//UpdateData.volt_1 = analogRead(CHARGE_VOLT_1);
		//printf("volt  =  %d \r\n",UpdateData.volt_1);

	}

	if (TenthSecondsSinceStart%10==5)
	{
		//UpdateData.Current_1 = ina219_1.getCurrent_mA();
		float Current = ina219_2.getCurrent_mA();
		if (Current < 0)
		{
			Current = 0;
		}
		UpdateData.Current_2 = Current;


		//printf("current  =  %d \r\n",UpdateData.Current);

		//UpdateData.volt_1 = analogRead(CHARGE_VOLT_1);
		//printf("volt  =  %d \r\n",UpdateData.volt_1);

	}

	if (TenthSecondsSinceStart%1==0)
	{

		digitalWrite(CHARGE_SWITCH, LOW);
		delay(1);
		UpdateData.Connected_1 = !digitalRead(BAT_CHK_1);
		UpdateData.Connected_2 = !digitalRead(BAT_CHK_2);
		//printf("INPUT_TEST 2  =  %s \r\n",(UpdateData.Connected_1?"Yes":"No"));
		if (UpdateData.ChargeON)
		{
			digitalWrite(CHARGE_SWITCH, HIGH);
		}

	}


	if (TenthSecondsSinceStart%10==0)
	{
		UpdateData.volt_1_updateTime++;
		UpdateData.volt_2_updateTime++;
	}

	//if (TenthSecondsSinceStart%20==0)
	//{

	//	digitalWrite(LIGHT, HIGH);
	//	digitalWrite(ALARM, HIGH);
	//	digitalWrite(FLASH, HIGH);
	//	digitalWrite(POWER, HIGH);
	//	printf("test relay High \r\n");

	//}

	//if (TenthSecondsSinceStart%20==10)
	//{

	//	digitalWrite(LIGHT, LOW);
	//	digitalWrite(ALARM, LOW);
	//	digitalWrite(FLASH, LOW);
	//	digitalWrite(POWER, LOW);
	//	printf("test relay Low \r\n");

	//}




	for (unsigned char i = 0; i < 2; i++)
	{



		if (BikeChecked[i] != BikeState[i]) 
		{
			UnChangeCounter[i]++;

			if (UnChangeCounter[i] > BIKE_CHECK_TIME)
			{
				BikeChecked[i] = BikeState[i];
				UnChangeCounter[i] = 0;
				printf("Bike checked[%d] =  %d \r\n",i,BikeChecked[i]);

				if (BikeChecked[i])
				{
					AlarmOn = false;
					Buzz = 2;
				} 
				else//disconnect
				{
					Buzz = 1;
					if (Tag == false)
					{
						printf("Bike %d disconnect without Tag \r\n",i);
						AlarmOn = true;
						AlarmOnCounter = 0;
					}
				}
			} 
		} 
		else
		{
			UnChangeCounter[i] = 0;
		}
	}

	static bool PreAlarm = false;
	static bool AlarmAudio = false;
	static bool AlarmFlash = false;

	if (AlarmOn)
	{
		AlarmOnCounter++;


		if ((AlarmOnCounter>0)&&(AlarmOnCounter<PRE_ALARM_TIME))
		{
			if (!PreAlarm)
			{
				digitalWrite(LIGHT, LOW);
				digitalWrite(AUDIO, HIGH);
				PreAlarm = true;
				AlarmAudio = false;
				AlarmFlash = false;
				printf("Pre_Alarm \r\n");
			} 
		}

		if ((AlarmOnCounter>PRE_ALARM_TIME)&&(AlarmOnCounter<ALARM_AUDIO_LIMIT))
		{
			if (!AlarmAudio)
			{
				digitalWrite(LIGHT, HIGH);
				digitalWrite(ALARM, LOW);
				digitalWrite(FLASH, LOW);
				printf("Alarm_Audio_Flash \r\n");

				PreAlarm = false;
				AlarmAudio = true;
				AlarmFlash = false;

			}
		}

		if ((AlarmOnCounter>ALARM_AUDIO_LIMIT)&&(AlarmOnCounter<ALARM_FLASH_LIMIT))
		{
			if (!AlarmFlash)
			{
				digitalWrite(AUDIO, LOW);
				printf("Alarm_Flash only \r\n");
				PreAlarm = false;
				AlarmAudio = false;
				AlarmFlash = true;
			}
		}

		if (AlarmOnCounter>ALARM_FLASH_LIMIT)
		{
			printf("Alarm_Over \r\n");
			AlarmOn = false;
		}

	}
	else
	{
		if (AlarmOnCounter != 0)
		{
			AlarmOnCounter = 0;
			if (Tag == false)
			{
				digitalWrite(LIGHT, HIGH);
			}
			digitalWrite(FLASH, HIGH);
			digitalWrite(ALARM, HIGH);
			digitalWrite(AUDIO, LOW);
			PreAlarm = false;
			AlarmAudio = false;
			AlarmFlash = false;
		}
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


		//printf("%d Get data:%d,%d,%d,%d CurrCH=%d \r\n", PackageCounter,GotData[0],GotData[1],GotData[2],GotData[3],CurrCH );

		printf("Get data:%d,%d,%d,%d \r\n", GotData[0],GotData[1],GotData[2],GotData[3]);


		if (GotData[0] == 0)
		{
			Volt = 1.2 * (GotData[DATA_LENGTH - 2] * 256 + GotData[DATA_LENGTH - 1]) * 3 * 1000 / 4096;
			LastTagGetTime = TenthSecondsSinceStart;
			Serial.print(F("Get Key \r\n"));


		}
		//		else if (GotData[0] == 1)
		//		{
		//#ifdef DEGBUG_OUTPUT
		//			printf("LastHomeGetTime offset = %d \r\n", TenthSecondsSinceStart - LastHomeGetTime);
		//#endif
		//			LastHomeGetTime = TenthSecondsSinceStart;
		//		}
		else if (GotData[0] == 2)
		{
			LastTagGetTime = TenthSecondsSinceStart;
			//if (millis() - LastKeyGetTime > 500)
			//{
			//    LastKeyGetTime = millis();
			//    OnKeyPress();
			//}
		}
		else if (GotData[0] == 5)
		{
			UpdateData.volt_1_updateTime = 0;
			UpdateData.volt_1 = 0;
			UpdateData.volt_1 = UpdateData.volt_1 + GotData[1];
			UpdateData.volt_1 = UpdateData.volt_1 << 8;
			UpdateData.volt_1 = UpdateData.volt_1 + GotData[2];
			UpdateData.volt_1 = UpdateData.volt_1 << 8;
			UpdateData.volt_1 = UpdateData.volt_1 + GotData[3];
			printf("Get volt  = %d V \r\n", UpdateData.volt_1/1000);

			printf("Get test data  = %d V \r\n", UpdateData.volt_1);
		}



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

void Buzz_task()
{

	static signed int BuzzOn;
	static signed int BuzzOff;
	static bool BuzzHigh;

	if (Buzz > 0)
	{
		if (BuzzHigh)
		{
			BuzzOn++;
			if (BuzzOn > BUZZON)
			{
				BuzzOn = 0;
				BuzzHigh = false;
				digitalWrite(BUZZ, LOW);
				Buzz--;
			}
		}
		else
		{
			BuzzOff++;
			if (BuzzOff > BUZZOFF)
			{
				BuzzOff = 0;
				BuzzHigh = true;
				digitalWrite(BUZZ, HIGH);
			}
		}
	}
	else
	{
	}
}

void ESP8266_Check()
{
	unsigned char a;
	unsigned char b;


	static bool StoreData = false;
	static unsigned char RecvLen;
	static unsigned char RecvCounter;
	static unsigned char Step = 0;
	unsigned char temp;

	if (SoftSerial.available())
	{
		a = SoftSerial.read();
		switch(Step)
		{
		case 0:
			if (!_esp8266_waitFor("IPD,")) return;
			RecvStater = true;
			Step++;
			return;
			break;
		case 1:
			RecvLen = a - 0x30;
			if(RecvLen != WIFI_COMMAND_LEN)
			{
				printf("Recv wrong len \r\n");
				RecvStater = false;
				Step = 0;
				return;
			}
			else
			{
				Step++;
			}
			return;
			break;
		case 2:
			if (a ==':')
			{
				Step++;
				return;
			}
			else
			{
				printf("Recv wrong len, >=10 \r\n");
				RecvStater = false;
				RecvLen = 0;
				Step = 0;
				return;
			}
			break;
		case 3:
			RecvData[RecvCounter] = a;
			RecvCounter++;
			if (RecvCounter >= WIFI_COMMAND_LEN)
			{

				for(unsigned char i = 0; i<WIFI_COMMAND_LEN-1 ; i++)
				{
					temp = temp + RecvData[i];
				}
				if (temp == RecvData[WIFI_COMMAND_LEN-1])
				{
					RecvDataOK = true;
				}
				else
				{
					printf("Recv data check failed \r\n");
				}
				RecvStater = false;
				StoreData = false;
				RecvLen = 0;
				RecvCounter = 0;
				Step = 0;
				//printf("Recv data = %c%c%c%c%c \r\n",RecvData[0],RecvData[1],RecvData[2],RecvData[3],RecvData[4]);
			}
			break;
		}
		//a = SoftSerial.read();
		//if ((!RecvStater)&&(a == '+'))
		//{
		//	if (!_esp8266_waitFor("IPD,")) return;

		//	RecvStater = true;
		//	return;
		//}

		//if ((RecvStater)&&(RecvLen == 0))
		//{
		//	RecvLen = a - 0x30;
		//	if(RecvLen != WIFI_COMMAND_LEN)
		//	{
		//		printf("Recv wrong len \r\n");
		//		RecvStater = false;
		//		RecvLen = 0;
		//		return;
		//	}
		//	return;
		//	//printf("RecvLen = %d \r\n",RecvLen);
		//}

		//if ((RecvStater)&&(RecvLen != 0)&&(!StoreData))
		//{
		//	if (a ==':')
		//	{
		//		StoreData = true;
		//		return;
		//	}
		//	else
		//	{
		//		printf("Recv wrong len, >=10 \r\n");
		//		RecvStater = false;
		//		RecvLen = 0;
		//		return;
		//	}

		//}

		//if (StoreData)
		//{

		//	RecvData[RecvCounter] = a;
		//	RecvCounter++;
		//	if (RecvCounter >= WIFI_COMMAND_LEN)
		//	{
		//		RecvDataOK = true;
		//		RecvStater = false;
		//		StoreData = false;
		//		RecvLen = 0;
		//		RecvCounter = 0;
		//		//printf("Recv data = %c%c%c%c%c \r\n",RecvData[0],RecvData[1],RecvData[2],RecvData[3],RecvData[4]);
		//	}

		//}
	}

	//if (Serial.available()) 
	//{
	//	a = Serial.read();
	//	SoftSerial.write(a);
	//}

}

//bool CheckResponse(unsigned char GetData,const char * ExpectResponse)
//{
//	static  char CheckedBytePostion = -1;
//	if (GetData == ExpectResponse[CheckedBytePostion+1])
//	{
//		CheckedBytePostion++;
//		if(ExpectResponse[CheckedBytePostion+1] == 0)
//		{
//			return true;
//		}
//	}
//	else
//	{
//		CheckedBytePostion = -1;
//	}
//
//	return false;
//}

void WiFi_task()
{
	static unsigned long LastSendTime;
	unsigned char data;
	//unsigned char Sum;
	unsigned char Len = sizeof(tUpdateData);
	if (!WifiOK)
	{
		//if (_esp8266_waitFor("WIFI GOT IP"))
		if (_esp8266_waitFor("GOT"))
		{
			printf("Wifi is connected \r\n");
			SoftSerial.print(F("AT+CIPSTART=\"UDP\","));
			SoftSerial.print(F("\"192.168.0.17\""));
			SoftSerial.print(F(",5050\r\n"));
			WifiOK = true;
			if (!_esp8266_waitFor("OK")) return;

			WaitForTimeOut = 10;
		}
		else
		{
			printf("wait IP time out, reset wifi \r\n");
			SoftSerial.print(F("AT+RST\r\n"));
			WaitForTimeOut = 300;
		}
	}
	else
	{
		if (TenthSecondsSinceStart - LastSendTime > DATA_UPDATE_INTERVAL)
		{
			printf("Start Data Update.\r\n");

			SoftSerial.print(F("AT+CIPSEND="));
			SoftSerial.print(Len);
			SoftSerial.print(F("\r\n"));

			//if (!_esp8266_waitFor(">")) return;
			//printf("Sending data \r\n");
			delay(10);

			UpdateData.Sum = 0;
			for(unsigned char i = 0; i<(sizeof(tUpdateData)-4) ; i++)
			{
				data = *(((char*)(&UpdateData))+i);
				UpdateData.Sum += data;
				SoftSerial.write(data);
			}

			for(unsigned char i = 0; i<4 ; i++)
			{
				SoftSerial.write( *(((char*)(&UpdateData))+(sizeof(tUpdateData)-4)+i));
			}
			//SoftSerial.write(Sum);


			if (!_esp8266_waitFor("OK")) return;
			printf("Data Update OK \r\n");

			LastSendTime = TenthSecondsSinceStart;


		}

	}


}

bool _esp8266_waitFor(const char *string) {
	char so_far = 0;
	char received;
	int counter = 0;
	do {
		//received = _esp8266_getch();
		if (!_esp8266_getch(&received))
		{
			return false;
		}
		counter++;
		if (received == string[so_far]) {
			so_far++;
		} else {
			so_far = 0;
		}
	} while (string[so_far] != 0);
	return true;
}

bool _esp8266_getch(char * RetData)   
{
	unsigned long RecvStartTime = TenthSecondsSinceStart;
	while (1)
	{
		if (SoftSerial.available() > 0)
		{
			*RetData = SoftSerial.read();
			//Serial.write(*RetData);
			return true;
		}
		NonStopTask();
		if (TenthSecondsSinceStart - RecvStartTime > WaitForTimeOut)
		{
			return false;
		}
	}
}