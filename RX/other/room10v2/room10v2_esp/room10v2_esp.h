




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


#define IIC_CLK				D1
#define IIC_DAT				D2
							//D3
#define USB_CHARGE			D4
							//D5
#define RF_IN				D6
							//D7
#define BUZZ				D8
//Light						A0


#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8


#include "D:\GitHub\Open-Source-RKS\RX\other\room10v2\struct.h"
#include <Wire.h>

void IicExchange();
tIicCommand IicCommand;
tIicData IicData;


ICACHE_RAM_ATTR void DecodeRf_INT();
unsigned char RcCommand[3] = {0,0,0};
bool DecodeFrameOK = false;
void CheckRf();
void CheckRfCommand(unsigned char * RfCommand);
#define RF_COMMAND_LEN 3
#define RF_COMMAND_KEY_COUNTER 5
#define RF_COMMAND_FUNCTION_COUNTER 4
unsigned char PreSetRfCommand[RF_COMMAND_FUNCTION_COUNTER][RF_COMMAND_KEY_COUNTER][RF_COMMAND_LEN]
={

	//
	{{0xbe, 0xA2, 0x24},{0x13, 0x06, 0x64},{0x39, 0x92, 0x24},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	//
	,{{0x00, 0x00, 0x00},{0x13, 0x06, 0x62},{0x39, 0x92, 0x23},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
};


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


void setup() 
{       

	pinMode(USB_CHARGE, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(USB_CHARGE, LOW);


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


	printf("sizeof(tIicCommand) = %d sizeof(tIicCommand) = %d \r\n",sizeof(tIicCommand),sizeof(tIicCommand));

	//Wire.begin(SDA, SCL); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */



	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	//byte mac[6];
	//WiFi.softAPmacAddress(mac);
	//printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	//for (byte i=0;i<6;i++)
	//{
	//	Room10Data.Mac[i] = mac[i];
	//}

	//XXXXXXXXData.Id = 0xFF;

	//for (unsigned char i = 0;i<ROOM_NUMBER;i++)
	//{
	//	if (memcmp(&RoomData.Mac[0],&RoomMacAddress[i][0],sizeof(unsigned long)*6) == 0)
	//	{
	//		XXXXXXXXData.Id = i;
	//		switch(i)
	//		{
	//		case ROOM_SOUTH:
	//			ssid = "frye_iot2";  //Wifi名称
	//			break;
	//		case ROOM_SMALL:
	//			ssid = "frye_iot2";  //Wifi名称
	//			break;
	//		case ROOM_CANTEEN:
	//			ssid = "frye_iot";  //Wifi名称
	//			break;
	//		case ROOM_BALCONY:
	//			ssid = "frye_iot2";  //Wifi名称
	//			break;
	//		}
	//		break;
	//	}
	//}


	Serial.print("Is connection routing, please wait");  
	WiFi.begin(ssid, password); //Wifi接入到网络
	Serial.println("\nConnecting to WiFi");
	//如果Wifi状态不是WL_CONNECTED，则表示连接失败
	unsigned char WiFiTimeOut = 0;
	while (WiFi.status() != WL_CONNECTED) {  
		Serial.print("."); 
		delay(1000);    //延时等待接入网络
		WiFiTimeOut++;
		if (WiFiTimeOut>10)
		{
			break;
			Serial.println("\nConnecting to WiFi Failed");
		}
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


	static bool RfInitialized = false;
	if ((!RfInitialized)&&(TenthSecondsSinceStart > 50))
	{
		printf("RfInitialized!!!\r\n");
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

	//if ((Hour<23)&&(Hour>7))//8:00:00~22:59:59
	//{
	//	if ((!Room10Data.UsbChargeOn)&&(Room10Command.BatteryPercentage<60))
	//	{

	//		MyPrintf("Room10 Usb chager online ON \r\n");
	//		Room10Data.UsbChargeOn = true;

	//	}	
	//	else if ((Room10Data.UsbChargeOn)&&(Room10Command.BatteryPercentage>70))
	//	{
	//		MyPrintf("Room10 Usb chager online OFF \r\n");
	//		Room10Data.UsbChargeOn = false;
	//	}	
	//}
	//else
	//{
	//	MyPrintf("Room10 Usb chager OFF after 23:00:00 \r\n");
	//	Room10Data.UsbChargeOn = false;
	//}



	static long DarkCounter = 0;
	Room10Data.Brightness = 100-(analogRead(A0)*100/1024);

	if (Room10Data.Brightness<5)
	{
		//if (Room10Data.UsbChargeOn)
		//{
		//	Room10Data.UsbChargeOn = false;
		//}
		DarkCounter++;
	}
	else
	{
		DarkCounter = 0;
	}


	if ((DarkCounter == 0)||(DarkCounter>3600))
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
	else
	{
		if (Room10Data.UsbChargeOn)
		{
			MyPrintf("Room10 Usb chager OFF when dark in 1 hour \r\n");
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
#define DISTANCE_HIGH 700
#define DISTANCE_LOW  200
#define SCREEN_TIMEOUT  50 //0.1s
	if (Room10Data.ScreenOn)//current on
	{
		//if (Room10Data.Brightness > 50)
		if ((Room10Data.Distance > DISTANCE_HIGH)||(Room10Data.Distance < DISTANCE_LOW))
		{
			ScreenOffCounter++;
			printf("ScreenOffCounter = %d \r\n",ScreenOffCounter);
			if (ScreenOffCounter > SCREEN_TIMEOUT)
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
		if ((Room10Data.Distance < DISTANCE_HIGH)&&(Room10Data.Distance > DISTANCE_LOW))
		{
			Room10Data.ScreenOn = true;
			printf("ScreenOn to true;\r\n");
			m_WiFiUDP.beginPacket("192.168.0.17", 5050);
			m_WiFiUDP.write((const char*)&Room10Data, sizeof(tRoom10Data));
			m_WiFiUDP.endPacket(); 
		}
	}


	IicExchange();


}

void IicExchange()
{
	//printf("IIC send");
	for (byte i=0;i<(sizeof(tIicCommand)-4);i++)
	{
		((byte *)(&IicCommand))[i] = rand();
		//printf(" 0x%02X",((byte*)(&IicCommand))[i]);
	}


	IicCommand.Sum = cal_crc((byte *)&IicCommand,sizeof(tIicCommand)-4);

	//printf("IIC send 0x%02X 0x%02X 0x%02X 0x%02X  \r\n"
	//		,((byte*)(&IicCommand))[0]
	//		,((byte*)(&IicCommand))[1]
	//		,((byte*)(&IicCommand))[2]
	//		,((byte*)(&IicCommand))[3]
	////		,((byte*)(&IicCommand))[4]
	////		,((byte*)(&IicCommand))[5]
	////		,((byte*)(&IicCommand))[6]
	////		,((byte*)(&IicCommand))[7]
	//		);

	Wire.beginTransmission(8); /* begin with device address 8 */
	Wire.write((byte*)(&IicCommand),sizeof(tIicCommand));
	Wire.endTransmission();    /* stop transmitting */

	Wire.requestFrom(8, sizeof(tIicData)); /* request & read data of size 13 from slave */
	unsigned char RecvIndex = 0;
	while(Wire.available() > 0)
	{
		if (RecvIndex<sizeof(tIicData))
		{
			((byte*)(&IicData))[RecvIndex] = Wire.read();
			RecvIndex++;
		}
	}

	//printf("IIC recv 0x%02X 0x%02X 0x%02X 0x%02X \r\n"
	//	,((byte*)(&IicData))[0]
	//,((byte*)(&IicData))[1]
	//,((byte*)(&IicData))[2]
	//,((byte*)(&IicData))[3]
	////		,((byte*)(&IicData))[4]
	////		,((byte*)(&IicData))[5]
	////		,((byte*)(&IicData))[6]
	////		,((byte*)(&IicData))[7]
	//);

	unsigned long crc_should = cal_crc((byte *)&IicData,sizeof(tIicData)-4);
	if (IicData.Sum == crc_should)
	{
		//printf("crc check passed. \n");
	}
	else
	{
		printf("crc check failed. should:0x%04X recv:0x%04X \n",crc_should,IicData.Sum);
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






void CheckRf()
{
	//static unsigned char LastRf[3];
	if (DecodeFrameOK)
	{

		printf("0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2]);
		CheckRfCommand(RcCommand);

		//if (memcmp(LastRf,RcCommand,3) == 0)
		//{
		//	//MyPrintf("0x%02X 0x%02X 0x%02X\r\n",RcCommand[0],RcCommand[1],RcCommand[2]);
		//CheckRfCommand(RcCommand);
		//} 
		//else
		//{
		//	memcpy(LastRf,RcCommand,3);
		//}
		DecodeFrameOK = false;
	}
}

#define PAUSE_RF_KEY_PRESS 5  //0.1s
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

				printf("rc Command %d \r\n",j);
				LastKeyTime = TenthSecondsSinceStart;
			}

		}
	}
}


ICACHE_RAM_ATTR void DecodeRf_INT()
{
#define PULSE_NUMBER 48
#define MIN_LEN 100
#define MAX_LEN 2000
#define LEAD_LEN 7000

	static unsigned long ThisTime;
	static unsigned long DiffTime;
	static unsigned long FirstTime;
	static unsigned long LastRfTime = 0;
	static bool FrameStarted = false;
	static bool RfOn = false;
	static unsigned char PulseIndex = 0;
	static unsigned char CommandIndex;
	static unsigned int Base;
	static unsigned int Min_Base;
	static unsigned int Max_Base;

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
							CommandIndex = (PulseIndex-1)/2/8;
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
