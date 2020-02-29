




//D0 = GPIO16; 
//D1 = GPIO5; 
//D2 = GPIO4;	LED on esp8266
//D3 = GPIO0;can not download when connected to low
//D4 = GPIO2;	
//D5 = GPIO14;  
//D6 = GPIO12;//only port Confirm can be used for RF_IN
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


const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;

#define ROOM_10
#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 0xFF;
tRoom10Data Room10Data;
tRoom10Command Room10Command;
unsigned long LastAndroidBatteryUpdate;
unsigned char ScreenOffCounter = 0;

#include <Wire.h>     //The DHT12 uses I2C comunication.
#include "VL53L0X.h"
#include "DHT12.h"
DHT12 dht12;          //Preset scale CELSIUS and ID 0x5c.
VL53L0X sensor;


unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();


void MyPrintf(const char *fmt, ...);


#define RELAY1				D0
#define RELAY2				D1
#define USB_CHARGE			D4
#define IIC_DAT				D2
#define IIC_CLK				D7
#define BUZZ				D8


//RF
#define RF_IN				D6//D3//D6


ICACHE_RAM_ATTR void DecodeRf_INT();

unsigned char RcCommand[3] = {0,0,0};
bool DecodeFrameOK = false;
void CheckRf();
void CheckRfCommand(unsigned char * RfCommand);
bool RfInitialized = false;

#define RF_COMMAND_LEN 3
#define RF_COMMAND_KEY_COUNTER 5
#define RF_COMMAND_FUNCTION_COUNTER 4
unsigned char PreSetRfCommand[RF_COMMAND_FUNCTION_COUNTER][RF_COMMAND_KEY_COUNTER][RF_COMMAND_LEN]
={

	//
	//{{0xC3, 0x7c, 0x68},{0x13, 0x06, 0x64},{0x39, 0x92, 0x24},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	//
	//,{{0x00, 0x00, 0x00},{0x13, 0x06, 0x62},{0x39, 0x92, 0x23},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
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

	pinMode(USB_CHARGE, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(USB_CHARGE, LOW);

	pinMode(BUZZ, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(BUZZ, HIGH);


	Wire.begin(IIC_DAT,IIC_CLK);

	sensor.setTimeout(500);
	if (!sensor.init())
	{
		printf("Failed to detect and initialize sensor!\r\n");
	}
	sensor.startContinuous();

	

	Room10Data.DataType = 19;
	Room10Data.DisableRf = false;
	Room10Data.UsbChargeOn = false;


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

	for (byte i=0;i<6;i++)
	{
		Room10Data.Mac[i] = mac[i];
	}

	Room10Data.RoomId = 0xFF;
	for (unsigned char i = 0;i<ROOM_10_NUMBER;i++)
	{
		if (memcmp(&Room10Data.Mac[0],&Room10MacAddress[i][0],sizeof(unsigned long)*6) == 0)
		{
			Room10Data.RoomId = i;
			DebugLogIndex = i + 10;
			MyPrintf("room10 ID=%d \r\n",i);
			break;
		}
	}

	//printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X AP:%s\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],ssid);


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


	//pinMode(RF_IN, INPUT);
	//attachInterrupt(digitalPinToInterrupt(RF_IN), DecodeRf_INT, CHANGE);


}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();

	CheckRf();



	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tRoom10Command))
	{
		//MyPrintf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		m_WiFiUDP.read((char *)&Room10Command,sizeof(tRoom10Command));
		Room10Data.DisableRf = Room10Command.DisableRf;
		LastAndroidBatteryUpdate = 0;
		//printf("DisableRf:%d! Percentage:%d Timeout:%d\r\n"
		//	,Room10Command.DisableRf
		//	,Room10Command.BatteryPercentage
		//	,Room10Command.AndroidTimeout
		//	);
	}
	//else
	//{
	//	m_WiFiUDP.read((char *)&tempRoom10Command,sizeof(tRoom10Command));
	//	printf("Wrong tRoom10Command len.got:%d except:%d\r\n",UdpAvailable,sizeof(tRoom10Command));
	//}
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


	static bool LastUsbChargeOn = false;


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

	//delay 10s for Initialize 315_RF input
	if ((!RfInitialized)&&(TenthSecondsSinceStart > 100))
	{
		pinMode(RF_IN, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(RF_IN), DecodeRf_INT, CHANGE);
		RfInitialized = true;
	}


	//Usb chager update
	LastAndroidBatteryUpdate++;

	//if ((LastAndroidBatteryUpdate>30)||(Room10Command.AndroidTimeout>30))
	//{
	//	if (now%3600 <= 40*60)
	//	{
	//		if (!Room10Data.UsbChargeOn)
	//		{
	//			MyPrintf("Room10 Usb chager offline ON \r\n");
	//			Room10Data.UsbChargeOn = true;
	//		}	
	//	}
	//	else
	//	{
	//		if (Room10Data.UsbChargeOn)
	//		{
	//			Room10Data.UsbChargeOn = false;
	//			MyPrintf("Room10 Usb chager offline OFF \r\n");
	//		}
	//	}
	//} 
	//if (Room10Command.AndroidTimeout>2)
	//{
	//	if (!Room10Data.UsbChargeOn)
	//	{
	//		MyPrintf("Room10 Usb chager ON for active\r\n");
	//		Room10Data.UsbChargeOn = true;
	//	}
	//}

	//if (Room10Command.AndroidTimeout>10)
	//{
	//	digitalWrite(BUZZ, LOW);
	//}
	//else
	//{
	//	digitalWrite(BUZZ, HIGH);
	//}
	
	if (false)
	{
	}
	else
	{
		if ((!Room10Data.UsbChargeOn)&&(Room10Command.BatteryPercentage<60))
		{
			MyPrintf("Room10 Usb chager online ON \r\n");
			Room10Data.UsbChargeOn = true;
		}	
		else if ((Room10Data.UsbChargeOn)&&(Room10Command.BatteryPercentage>70))
		{
			MyPrintf("Room10 Usb chager online OFF \r\n");
			Room10Data.UsbChargeOn = false;
		}	
	}

	if (Room10Data.UsbChargeOn)
	{
		digitalWrite(USB_CHARGE,HIGH);
	}
	else
	{
		digitalWrite(USB_CHARGE,LOW);
	}

	//sync screen on when charge on/off. Then the screen can be off.
	if (LastUsbChargeOn != Room10Data.UsbChargeOn)
	{
		LastUsbChargeOn = Room10Data.UsbChargeOn;
		Room10Data.ScreenOn = true;
		ScreenOffCounter = 0;
	}
	


	if(dht12.read() == 0)		
	{
		Room10Data.Humidity = dht12.LastHumidity*10;
		Room10Data.RealTemperature = dht12.LastTemperature*10;
	}
	Room10Data.Brightness = 100-(analogRead(A0)*100/1024);

	//printf("Humidity:%d %% Temperature:%d *C Brightness:%d%%\r\n"
		//,Room10Data.Humidity
		//,Room10Data.RealTemperature
		//,Room10Data.Brightness
		//);



	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&Room10Data, sizeof(tRoom10Data));
	m_WiFiUDP.endPacket(); 

}

void OnTenthSecond()
{


	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}

	Room10Data.Distance = sensor.readRangeContinuousMillimeters();
	//printf("distance = %d\r\n",Room10Data.Distance);


	if (Room10Data.ScreenOn)//current on
	{
		//if (Room10Data.Brightness > 50)
		if ((Room10Data.Distance > 1000)||(Room10Data.Distance < 200))
		{
			ScreenOffCounter++;
			printf("ScreenOffCounter = %d \r\n",ScreenOffCounter);
			if (ScreenOffCounter > 50)
			{
				Room10Data.ScreenOn = false;
				printf("ScreenOn to false;\r\n");
				ScreenOffCounter = 0;
			}
		}
		else
		{
			ScreenOffCounter = 0;
			printf("ScreenOffCounter to 0  \r\n");
		}
	}
	else//current off
	{
		//if (Room10Data.Brightness < 50)
		if ((Room10Data.Distance < 1000)&&(Room10Data.Distance > 200))
		{
			Room10Data.ScreenOn = true;
			printf("ScreenOn to true;\r\n");
			m_WiFiUDP.beginPacket("192.168.0.17", 5050);
			m_WiFiUDP.write((const char*)&Room10Data, sizeof(tRoom10Data));
			m_WiFiUDP.endPacket(); 
		}
	}



	
}

#define PAUSE_RF_KEY_PRESS 20  //0.1s
void CheckRfCommand(unsigned char * RfCommand)
{

	static unsigned long LastKeyTime = PAUSE_RF_KEY_PRESS;

	if (TenthSecondsSinceStart - LastKeyTime < PAUSE_RF_KEY_PRESS)
	{
		return;
	} 


	static unsigned long LastTrigerTimer;
	for (byte j=0;j<RF_COMMAND_FUNCTION_COUNTER;j++)
	{
		for (byte i=0;i<RF_COMMAND_KEY_COUNTER;i++)
		{
			if(memcmp(RfCommand,PreSetRfCommand[j][i],RF_COMMAND_LEN)==0)
			{

				MyPrintf("rc Command %d \r\n",j);
				LastKeyTime = TenthSecondsSinceStart;

				if (j == 0)
				{
					Room10Data.ScreenOn = !Room10Data.ScreenOn;

				} 
				else if (j == 1)
				{

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
			printf("0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2]);
			CheckRfCommand(RcCommand);
		} 
		else
		{
			memcpy(LastRf,RcCommand,3);
		}
		DecodeFrameOK = false;
	}
}

ICACHE_RAM_ATTR void DecodeRf_INT()
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

	//static unsigned int temp;
	//temp++;
	//printf("DecodeRf_INT %d \r\n",temp);

	if (Room10Data.DisableRf)
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
							//printf("DecodeFrameOK \r\n");
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