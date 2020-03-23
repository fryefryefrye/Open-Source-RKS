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
#include<ESP8266WiFiGeneric.h>
#include<time.h>

#define timezone 8

const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;

char *time_str;   
char H1,H2,M1,M2,S1,S2;

#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 20;
tLedUnitGbData LedUnitGbData;
tLedUnitGbCommand LedUnitGbCommand;
unsigned long LastServerUpdate;

unsigned long SecondsSinceStart = 0;
void SecondsSinceStartTask();
void OnSecond();
void NonStopTask();
void MyPrintf(const char *fmt, ...);

#define RowA D8    		//行信号,驱动138  
#define RowB D7
#define RowC D6
#define RowD D5
#define STB D2         //595 刷新显示  SS  
#define CLK D1         //时钟    SCK  
#define OE D0 			//  使能  
#define R1 D4          //上半屏列信号输出    
#define R2 D3          //下半屏列信号输出    

void setup() 
{       


	delay(50);                      
	Serial.begin(115200);

	pinMode(RowA, OUTPUT);
	pinMode(RowB, OUTPUT);
	pinMode(RowC, OUTPUT);
	pinMode(RowD, OUTPUT); //138片选
	pinMode(OE, OUTPUT); //138 使能
	pinMode(R1, OUTPUT);//595 数据
	pinMode(R2, OUTPUT);//595 数据
	pinMode(CLK, OUTPUT); //595 时钟
	pinMode(STB, OUTPUT); //595 使能


	LedUnitGbData.DataType = 23;
	LedUnitGbData.Cycle = 1000;
	LedUnitGbData.DisplayLight = 50;


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	byte mac[6];
	WiFi.softAPmacAddress(mac);
	printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	for (byte i=0;i<6;i++)
	{
		//LedUnitGbData.Mac[i] = mac[i];
	}

	//LedUnitGbData.Id = 0xFF;

	//for (unsigned char i = 0;i<ROOM_NUMBER;i++)
	//{
	//	if (memcmp(&RoomData.Mac[0],&RoomMacAddress[i][0],sizeof(unsigned long)*6) == 0)
	//	{
	//		LedUnitGbData.Id = i;
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

	m_WiFiUDP.begin(5050); 
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X AP:%s\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],ssid);

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


void hc595senddata(byte data,byte data2)//发送上下半屏，各一行数据。
{
	for (byte i=0; i<8; i++)
	{
		digitalWrite(CLK,0);
		digitalWrite(R1, ((data>>(7-i))&1));//发送上半屏
		digitalWrite(R2, ((data2>>(7-i))&1));//发送下半屏
		digitalWrite(CLK,1);
	}
}

void hc138sacn(byte r)   //输出行线状态ABCD （A低,D高)
{
	digitalWrite(RowA,(r & 0x01));
	digitalWrite(RowB,(r & 0x02));
	digitalWrite(RowC,(r & 0x04));
	digitalWrite(RowD,(r & 0x08));
}

void DisplayOneLine()
{
	//显示缓存扫描
	static unsigned char row=0;
	for (int i=0; i<64; i++)
	{
		digitalWrite(CLK,0);
		digitalWrite(R1, ((LedUnitGbCommand.DispBuf[i%64+(row/8)*64]>>(row%8))&1));//发送上半屏
		digitalWrite(R2, ((LedUnitGbCommand.DispBuf[i%64+(row/8)*64+128]>>(row%8))&1));//发送下半屏
		digitalWrite(CLK,1);
	}

	hc138sacn(row);            //选行
	digitalWrite(STB, 1);      //数据确认
	digitalWrite(STB, 0);
	if (LedUnitGbData.DisplayLight>0)
	{
		digitalWrite(OE, 0);  //开启显示
	}
	delayMicroseconds(LedUnitGbData.DisplayLight) ;  //亮度调节
	digitalWrite(OE, 1);  //关闭显示
	row++;
	if (row>15)
	{
		row = 0;
		NonStopTask();
	}
}

unsigned long CurrentDisplayTime;
unsigned long LastDisplayTime;
void DisplayTimerTask()
{

	static bool ScreenOn = false;
	CurrentDisplayTime = micros();
	if (abs(CurrentDisplayTime - LastDisplayTime) > LedUnitGbData.Cycle)
	{
		LastDisplayTime = CurrentDisplayTime;
		DisplayOneLine();
		ScreenOn = true;
	}
}


void loop() 
{
	DisplayTimerTask();
}

void NonStopTask() 
{
	SecondsSinceStartTask();
	ArduinoOTA.handle();
	delay(0);
	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tLedUnitGbCommand))
	{
		//printf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		m_WiFiUDP.read((char *)&LedUnitGbCommand,sizeof(tLedUnitGbCommand));
		if (LedUnitGbCommand.Triger == true)
		{
			LedUnitGbData.Cycle = LedUnitGbCommand.Cycle;
			LedUnitGbData.DisplayLight = LedUnitGbCommand.Light;
		}
		LastServerUpdate = 0;
	}
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

void OnSecond()
{
	static bool FirstStart = true;

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
	unsigned char Second = timenow->tm_sec;


	//printf("LastServerUpdate = %d\r\n",LastServerUpdate);


	//LastServerUpdate++;
	//if (LastServerUpdate > 30)
	//{
	//	printf("Re connection routing!\n");  
	//	WiFi.disconnect();
	//	WiFi.mode(WIFI_STA);//设置模式为STA
	//	WiFi.begin(ssid, password); //Wifi接入到网络

	//	//printf("reset!\n");  
	//	//ESP.reset();

	//	LastServerUpdate = 0;
	//}

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&LedUnitGbData, sizeof(tLedUnitGbData));
	m_WiFiUDP.endPacket(); 
}

void OnTenthSecond()
{
	if (SecondsSinceStart%10 == 0)
	{
		OnSecond();
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
