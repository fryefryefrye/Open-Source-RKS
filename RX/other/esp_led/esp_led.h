




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


//#define RELAY				D4



#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8




const char* ssid = "frye_iot3";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 20;
tLedTimeData LedTimeData;
tLedTimeCommand LedTimeCommand;
unsigned long LastServerUpdate;



unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
unsigned long cal_crc(unsigned char *ptr, unsigned char len);
void MyPrintf(const char *fmt, ...);


#include <FastLED.h>
void SetRGB(unsigned long ColorIndex,unsigned long Brightness);
void setColor(unsigned char r,unsigned char g,unsigned char b);

CRGB Color = {255,255,255};

#define LED_PIN     8     //我的LED信号线IN接在8引脚
#define NUM_LEDS    15    
CRGB leds[NUM_LEDS];

//#define MAX_LIGHT 20

//0-9数字所对应数组
unsigned char DIGITAL_DISPLAY[10][7] = { 
	{ 1,1,1,1,1,1,0 }, // = 0
	{ 1,1,0,0,0,0,0 }, // = 1
	{ 1,0,1,1,0,1,1 }, // = 2
	{ 1,1,1,0,0,1,1 }, // = 3
	{ 1,1,0,0,1,0,1 }, // = 4
	{ 0,1,1,0,1,1,1 }, // = 5
	{ 0,1,1,1,1,1,1 }, // = 6
	{ 1,1,0,0,0,1,0 }, // = 7
	{ 1,1,1,1,1,1,1 }, // = 8
	{ 1,1,1,0,1,1,1 }, // = 9
};



void setup() 
{       


	delay(50);                      
	Serial.begin(115200);

	LedTimeData.DataType = 9;


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	byte mac[6];
	WiFi.softAPmacAddress(mac);
	printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	//for (byte i=0;i<6;i++)
	//{
	//	LedTimeData.Mac[i] = mac[i];
	//}

	//LedTimeData.Id = 0xFF;

	//for (unsigned char i = 0;i<ROOM_NUMBER;i++)
	//{
	//	if (memcmp(&RoomData.Mac[0],&RoomMacAddress[i][0],sizeof(unsigned long)*6) == 0)
	//	{
	//		LedTimeData.Id = i;
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



	FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
	FastLED.setBrightness(20);

}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();

	////渐变色,单色亮灭。
	static uint8_t mode = 0;
	static uint8_t r=0; 
	static uint8_t g=0; 
	static uint8_t b=0;
	switch (mode)
	{
	case 0:
		r++;
		if (r == LedTimeCommand.Brightness)
		{
			mode = 1;
		}
		break;
	case 1:
		r--;
		if (r == 0)
		{
			mode = 2;
		}
		break;
	case 2:
		g++;
		if (g == LedTimeCommand.Brightness)
		{
			mode = 3;
		}
		break;
	case 3:
		g--;
		if (g == 0)
		{
			mode = 4;
		}
		break;
	case 4:
		b++;
		if (b == LedTimeCommand.Brightness)
		{
			mode = 5;
		}
		break;
	case 5:
		b--;
		if (b == 0)
		{
			mode = 6;
		}
		break;
	case 6:
		r++;
		g++;
		if (r == LedTimeCommand.Brightness)
		{
			mode = 7;
		}
		break;
	case 7:
		r--;
		g--;
		if (r == 0)
		{
			mode = 8;
		}
		break;
	case 8:
		g++;
		b++;
		if (g == LedTimeCommand.Brightness)
		{
			mode = 9;
		}
		break;
	case 9:
		g--;
		b--;
		if (g == 0)
		{
			mode = 10;
		}
		break;
	case 10:
		b++;
		r++;
		if (b == LedTimeCommand.Brightness)
		{
			mode = 11;
		}
		break;
	case 11:
		b--;
		r--;
		if (b == 0)
		{
			mode = 12;
		}
		break;
	case 12:
		b++;
		r++;
		g++;
		if (b == LedTimeCommand.Brightness)
		{
			mode = 13;
		}
		break;
	case 13:
		b--;
		r--;
		g--;
		if (b == 0)
		{
			mode = 0;
		}
		break;
	}
	setColor(r, g, b);
	for (byte i=0;i<NUM_LEDS;i++)
	{
		leds[i] = Color;
	}
	FastLED.show();
	delay(20);



	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tLedTimeCommand))
	{
		//MyPrintf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		//printf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		//tLedTimeCommand tempLedTimeCommand;
		m_WiFiUDP.read((char *)&LedTimeCommand,sizeof(tLedTimeCommand));
		SetRGB(LedTimeCommand.ColorIndex,LedTimeCommand.Brightness);
		LedTimeData.ColorIndex = LedTimeCommand.ColorIndex;
		LedTimeData.Brightness = LedTimeCommand.Brightness;
		//printf("ColorIndex = %d Brightness = %d \r\n",tempLedTimeCommand.ColorIndex,tempLedTimeCommand.Brightness);
		LastServerUpdate = 0;
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
	unsigned char Numbers[6];

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

	LedTimeData.RunningDateTime = now-3600*8;

	Numbers[0] = S2-0x30;
	Numbers[1] = S1-0x30;
	Numbers[2] = M2-0x30;
	Numbers[3] = M1-0x30;



	//for(int i = 0; i < 6;i++)
	//{
	//	printf("Numbers = %d\r\n",Numbers[i]);
	//}
	//for(int i = 0; i < NUM_LEDS;i++)
	//{
	//	if(DIGITAL_DISPLAY[Numbers[i/7]][i%7] == 1)
	//	{
	//		leds[i] = Color;
	//	}
	//	else
	//	{
	//		leds[i] = CRGB::Black;
	//	}
	//}
	//for(int i = 14; i < MAX_LED;i++)
	//{

	//	strip.setPixelColor(i, 0);
	//}
	//strip.show();


	//if (TenthSecondsSinceStart%20==0)
	//{
	//	
	//	for (byte i=0;i<NUM_LEDS;i++)
	//	{
	//		leds[i] = Color;
	//	}
	//	
	//}
	//if (TenthSecondsSinceStart%20==10)
	//{
	//	for (byte i=0;i<NUM_LEDS;i++)
	//	{
	//		leds[i] = CRGB::Black;
	//	}
	//}

		//for (byte i=0;i<NUM_LEDS;i++)
		//{
		//	leds[i] = Color;
		//}


	//FastLED.show();






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




	LedTimeData.Sum = cal_crc((unsigned char *)&LedTimeData,sizeof(tLedTimeData)-4);

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&LedTimeData, sizeof(tLedTimeData));
	m_WiFiUDP.endPacket(); 
}

void SetRGB(unsigned long ColorIndex,unsigned long Brightness)
{
	//{"黑","红","绿","蓝","黄","紫","青","白"};

	FastLED.setBrightness(Brightness*2);  //可以设置全局亮度，

	switch(ColorIndex)
	{
	case 0:
		//黑
		Color.r = 0;
		Color.g = 0;
		Color.b = 0;
		break;
	case 1:
		//红
		Color.r = 255;
		Color.g = 0;
		Color.b = 0;
		break;
	case 2:
		//绿色
		Color.r = 0;
		Color.g = 0;
		Color.b = 255;
		break;
	case 3:
		//蓝
		Color.r = 0;
		Color.g = 255;
		Color.b = 0;
		break;
	case 4:
		//黄色
		Color.r = 255;
		Color.g = 0;
		Color.b = 255;
		break;
	case 5:
		//紫色
		Color.r = 255;
		Color.g = 255;
		Color.b = 0;
		break;
	case 6:
		//青色
		Color.r = 0;
		Color.g = 255;
		Color.b = 255;
		break;
	case 7:
		Color.r = 255;
		Color.g = 255;
		Color.b = 255;
		break;
	default:
		Color = 0;
		break;
	}


}

void setColor(unsigned char r,unsigned char g,unsigned char b)
{

	Color.r = r;
	Color.g = g;
	Color.b = b;

}
void OnTenthSecond()
{


	if (TenthSecondsSinceStart%10 == 0)
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

#define crc_mul 0x1021  //生成多项式
unsigned long cal_crc(unsigned char *ptr, unsigned char len)
{
	unsigned char i;
	unsigned long crc=0;
	while(len-- != 0)
	{
		for(i=0x80; i!=0; i>>=1)
		{
			if((crc&0x8000)!=0)
			{
				crc<<=1;
				crc^=(crc_mul);
			}else{
				crc<<=1;
			}
			if((*ptr&i)!=0)
			{
				crc ^= (crc_mul);
			}
		}
		ptr ++;
	}
	return (crc);
}
