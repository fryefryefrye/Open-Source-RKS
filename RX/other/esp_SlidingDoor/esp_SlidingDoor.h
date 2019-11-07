




//D0 = GPIO16; 
//D1 = GPIO5; 
//D2 = GPIO4;	LED on esp8266
//D3 = GPIO0;can not download when connected to low
//D4 = GPIO2;	
//D5 = GPIO14;  
//D6 = GPIO12;
//D7 = GPIO13;
//D8 = GPIO15;  can not start when high input
//D9 = GPIO3; UART RX
//D10 = GPIO1; UART TX
//LED_BUILTIN = GPIO16 (auxiliary constant for the board LED, not a board pin);






#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8


const char* ssid = "frye_iot";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 23;
tSlidingDoorData SlidingDoorData;
//tSlidingDoorCommand SlidingDoorCommand;


unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();


void MyPrintf(const char *fmt, ...);

#define POSTION_OPEN		D5
#define POSTION_CLOSE		D4
#define RELAY1				D8
#define RELAY2				D7


//RF
#define RF_IN				D6
void DecodeRf_INT();
unsigned char RcCommand[3] = {0,0,0};
bool DecodeFrameOK = false;
void CheckRf();
void CheckRfCommand(unsigned char * RfCommand);

#define RF_COMMAND_LEN 3
#define RF_COMMAND_KEY_COUNTER 5
#define RF_COMMAND_FUNCTION_COUNTER 4
unsigned char PreSetRfCommand[RF_COMMAND_FUNCTION_COUNTER][RF_COMMAND_KEY_COUNTER][RF_COMMAND_LEN]
={

	//关门
	{{0x00, 0x00, 0x00},{0x13, 0x06, 0x64},{0x39, 0x92, 0x24},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	//开门
	,{{0x00, 0x00, 0x00},{0x13, 0x06, 0x62},{0x39, 0x92, 0x23},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
};


//door
bool OpeningDoor = false;
bool ClosingDoor = false;
bool Stop = false;
void OpenDoor();
void CloseDoor();
void OpenRelay(bool on);
void CloseRelay(bool on);
#define PAUSE_KEY_PRESS 4




void setup() 
{       

	pinMode(RELAY1, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY1, LOW);

	pinMode(RELAY2, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY2, LOW);

	pinMode(POSTION_OPEN, INPUT_PULLUP);
	pinMode(POSTION_CLOSE, INPUT_PULLUP);


	SlidingDoorData.DataType = 15;
	SlidingDoorData.DisableRf = false;


	delay(50);                      
	Serial.begin(115200);


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	Serial.print("Is connection routing, please wait");  
	WiFi.begin(ssid, password); //Wifi接入到网络
	Serial.println("\nConnecting to WiFi");
	//如果Wifi状态不是WL_CONNECTED，则表示连接失败
	while (WiFi.status() != WL_CONNECTED) {  
		Serial.print("."); 
		delay(1000);    //延时等待接入网络
	}



	//设置时间格式以及时间服务器的网址
	configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
	Serial.println("\nWaiting for time");
	while (!time(nullptr)) {
		Serial.print(".");
		delay(1000);    
	}
	Serial.println("");




	byte mac[6];
	WiFi.softAPmacAddress(mac);
	//printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	for (byte i=0;i<6;i++)
	{
		SlidingDoorData.Mac[i] = mac[i];
	}

	//for (unsigned char i = 0;i<ROOM_NUMBER;i++)
	//{
	//	if (memcmp(&RoomData.Mac[0],&RoomMacAddress[i][0],sizeof(unsigned long)*6) == 0)
	//	{
	//		MyPrintf("room ID=%d \r\n",i);
	//		RoomIndex = i;
	//		break;
	//	}
	//}

	SlidingDoorData.DoorId = 0xFF;

	for (unsigned char i = 0;i<SLIDING_DOOR_NUMBER;i++)
	{
		if (memcmp(&SlidingDoorData.Mac[0],&SlidingDoorMacAddress[i][0],sizeof(unsigned long)*6) == 0)
		{
			MyPrintf("door ID=%d \r\n",i);
			SlidingDoorData.DoorId = i;

			break;
		}
	}



	m_WiFiUDP.begin(5050); 

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else { // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			Serial.println("Auth Failed");
		} else if (error == OTA_BEGIN_ERROR) {
			Serial.println("Begin Failed");
		} else if (error == OTA_CONNECT_ERROR) {
			Serial.println("Connect Failed");
		} else if (error == OTA_RECEIVE_ERROR) {
			Serial.println("Receive Failed");
		} else if (error == OTA_END_ERROR) {
			Serial.println("End Failed");
		}
	});
	ArduinoOTA.begin();
	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	pinMode(RF_IN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(RF_IN), DecodeRf_INT, CHANGE);


}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();

	CheckRf();

	OpenDoor();
	CloseDoor();


	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tSlidingDoorCommand))
	{
		//MyPrintf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		tSlidingDoorCommand tempSlidingDoorCommand;
		m_WiFiUDP.read((char *)&tempSlidingDoorCommand,sizeof(tSlidingDoorCommand));

		if (tempSlidingDoorCommand.DisableRfTriger)
		{
			printf("DisableRf to %d! \r\n",tempSlidingDoorCommand.DisableRf);
			SlidingDoorData.DisableRf = tempSlidingDoorCommand.DisableRf;
		}

		if (tempSlidingDoorCommand.DoorCommandTriger)
		{
			if (tempSlidingDoorCommand.DoorCommand == DOOR_OP_OPEN)
			{
				printf("OpenDoor from wifi! \r\n");

				if (ClosingDoor)
				{
					Stop = true;
					printf("Stop! \r\n");
				} 
				else
				{
					OpeningDoor = true;
				}
			}
			if (tempSlidingDoorCommand.DoorCommand == DOOR_OP_CLOSE)
			{
				printf("CloseDoor from wifi! \r\n");

				if (OpeningDoor)
				{
					Stop = true;
					printf("Stop! \r\n");
				} 
				else
				{
					ClosingDoor = true;
				}
			}
			if (tempSlidingDoorCommand.DoorCommand == DOOR_OP_OPEN_CLOSE)
			{
				printf("OpenCloseDoor from wifi! \r\n");

				if (!digitalRead(POSTION_OPEN))
				{
					ClosingDoor = true;
				}
				else if (!digitalRead(POSTION_CLOSE))
				{
					OpeningDoor = true;
				}
			}
		}
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
		//MyPrintf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
		//MyPrintf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
	}
}

void OnSecond()
{
	time_t now = time(nullptr); //获取当前时间
	time_str = ctime(&now);
	H1 = time_str[11];
	H2 = time_str[12];
	M1 = time_str[14];
	M2 = time_str[15];
	S1 = time_str[17];
	S2 = time_str[18];
	//printf("%c%c:%c%c:%c%c\n",H1,H2,M1,M2,S1,S2);
	//Serial.printf(time_str);

	struct   tm     *timenow;
	timenow   =   localtime(&now);
	unsigned char Hour = timenow->tm_hour;
	unsigned char Minute = timenow->tm_min;

	SlidingDoorData.isClosed = !digitalRead(POSTION_CLOSE);
	SlidingDoorData.isOpened = !digitalRead(POSTION_OPEN);

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&SlidingDoorData, sizeof(tSlidingDoorData));
	m_WiFiUDP.endPacket(); 

}

void OnTenthSecond()
{


	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}
	
}

void CheckRfCommand(unsigned char * RfCommand)
{

	static unsigned char PauseKeyCounter = PAUSE_KEY_PRESS;

	if (PauseKeyCounter>0)
	{
		PauseKeyCounter--;
		return;
	} 



	static unsigned long LastTrigerTimer;
	for (byte j=0;j<RF_COMMAND_FUNCTION_COUNTER;j++)
	{
		for (byte i=0;i<RF_COMMAND_KEY_COUNTER;i++)
		{
			if(memcmp(RfCommand,PreSetRfCommand[j][i],RF_COMMAND_LEN)==0)
			{

				//printf("rc Command %d \r\n",j);
				PauseKeyCounter = PAUSE_KEY_PRESS;

				if (j == 0)
				{
					if (ClosingDoor)
					{
						Stop = true;
						printf("Stop! \r\n");
					} 
					else
					{
						if (digitalRead(POSTION_OPEN))
						{

							OpeningDoor = true;
							printf("OpenDoor! Key press!  \r\n");
						}
						else
						{
							ClosingDoor = true;
							printf("OpenDoor Key press! But already opened. So, cloes it \r\n");
						}
					}
				} 
				else if (j == 1)
				{
					if (OpeningDoor)
					{
						Stop = true;
						printf("Stop! \r\n");
					} 
					else
					{
						if (digitalRead(POSTION_CLOSE))
						{
							ClosingDoor = true;
							printf("CloseDoor Key press! \r\n");
						}
						else
						{
							OpeningDoor = true;
							printf("CloseDoor Key press! But already closed. So Open it\r\n");
						}
					}
				}
			}

		}
	}
}



void CheckRf()
{
	static unsigned char LastRf[3];
	if (DecodeFrameOK)
	{


		if (memcmp(LastRf,RcCommand,3) == 0)
		{
			//MyPrintf("0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2]);
			CheckRfCommand(RcCommand);
		} 
		else
		{
			memcpy(LastRf,RcCommand,3);
		}
		DecodeFrameOK = false;
	}
}

void DecodeRf_INT()
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


	if (SlidingDoorData.DisableRf)
	{
		return;
	}

	if (DecodeFrameOK)
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
							RcCommand[CommandIndex] = RcCommand[CommandIndex]<<1;
							if ((FirstTime>Base)&&(DiffTime<Base))//bit 1
							{
								RcCommand[CommandIndex]++;
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
									RcCommand[0] = 0;
									RcCommand[1] = 0;
									RcCommand[2] = 0;
								}
							}
						}
						PulseIndex++;
						if (PulseIndex >= PULSE_NUMBER)//收集到48个位
						{
							DecodeFrameOK = true;
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
						RcCommand[0] = 0;
						RcCommand[1] = 0;
						RcCommand[2] = 0;
					}
				} 
				else
				{
					if (DiffTime > LEAD_LEN)//收到引导码
					{
						Base = DiffTime/16;
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
						RcCommand[0] = 0;
						RcCommand[1] = 0;
						RcCommand[2] = 0;
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


void MyPrintf(const char *fmt, ...)
{


	static char send_buf[1024];
	char * sprint_buf = send_buf+sizeof(tDebugData);
	tDebugData * pDebugData = (tDebugData*)send_buf;

	int n;
	va_list args;

	va_start(args,fmt);
	n = vsprintf(sprint_buf, fmt, args);
	va_end(args);

	printf(sprint_buf);

	pDebugData->DataType = 3;
	pDebugData->RoomId = DebugLogIndex;
	pDebugData->Length = n;

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)send_buf, sizeof(tDebugData)+n);
	m_WiFiUDP.endPacket(); 


}



void OpenDoor()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (OpeningDoor)
	{
		if (!Stop)
		{
			if (Step == 0)
			{
				Step = 1;
				StartTime = TenthSecondsSinceStart;
				OpenRelay(true);
			}
			if (Step == 1)
			{
				if(TenthSecondsSinceStart -StartTime > 25)
				{
					OpenRelay(false);
					Step = 2;

					Step = 0;
					OpeningDoor = false;
					printf("Open door finished.\r\n");
				}
			}
		}
		else
		{
			OpenRelay(false);
			Stop = false;
			Step = 0;
			OpeningDoor = false;
			printf("Open door canceled.\r\n");
		}
	} 

}

void CloseDoor()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (ClosingDoor)
	{
		if (!Stop)
		{
			if (Step == 0)
			{
				Step = 1;
				StartTime = TenthSecondsSinceStart;
				CloseRelay(true);
			}
			if (Step == 1)
			{
				if(TenthSecondsSinceStart -StartTime > 25)
				{
					CloseRelay(false);
					Step = 2;

					Step = 0;
					ClosingDoor = false;
					printf("Close door finished.\r\n");
				}
			}
		}
		else
		{
			CloseRelay(false);
			Stop = false;
			Step = 0;
			ClosingDoor = false;
			printf("Close door canceled.\r\n");
		}
	} 


}

void OpenRelay(bool on)
{
	digitalWrite(RELAY1,on);
	//digitalWrite(BUZZ,on);
	printf("Open Relay to %d .\r\n",on);
}
void CloseRelay(bool on)
{
	digitalWrite(RELAY2,on);
	//digitalWrite(BUZZ,on);
	printf("Close Relay to %d .\r\n",on);
}