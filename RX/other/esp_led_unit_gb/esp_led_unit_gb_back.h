




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

#include "LedUnitChat.h""


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


//64*32
//up half 64*16 pix  8*16 data.
//whole data8*32
unsigned char ASCII816Buf[16] = {0};
unsigned char DispBuf[8][32] = {0};
//unsigned char FirstLine[8]= {32,32,32,32,32,32,32,32};
//unsigned char FirstLine[8]={'A','B','C','D','H','I','O','P'};
unsigned char FirstLine[8]={'0','1','2','3','4','5','6','7'};
//unsigned char SecondLine[8]= {32,32,32,32,32,32,32,32};//{'A','B','C','D','H','I','O','P'};
unsigned char DateLine[10]= {32,32,32,32,32,32,32,32,32,32};//{'1','2','3','4','5','6','7','8','8',' '};//{'A','B','C','D','H','I','O','P'};
unsigned char DateLine2[10] = {32,32,32,32,32,32,32,32,32,32};//{'w','e','a','t','h','e','r','n','o','w'};

const char* WeekStr[7] = {"Sunday"
							,"Monday"
							,"Tuesday"
							,"Wednesday"
							,"Thursday"
							,"Friday"
							,"Saturday"};

#define RowA D8    		//行信号,驱动138  
#define RowB D7
#define RowC D6
#define RowD D5
#define STB D2         //595 刷新显示  SS  
#define CLK D1         //时钟    SCK  
#define OE D0 			//  使能  
#define R1 D4          //上半屏列信号输出    
#define R2 D3          //下半屏列信号输出    


//const PROGMEM unsigned char ACSII57[95][5]=
const unsigned char PROGMEM  ACSII57[95][5]=
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00 },  // sp
	{ 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
	{ 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
	{ 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
	{ 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
	{ 0xc4, 0xc8, 0x10, 0x26, 0x46 },   // %
	{ 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
	{ 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
	{ 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
	{ 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
	{ 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
	{ 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
	{ 0x00, 0x00, 0x50, 0x30, 0x00 },   // ,
	{ 0x10, 0x10, 0x10, 0x10, 0x10 },   // -
	{ 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
	{ 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
	{ 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0
	{ 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
	{ 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
	{ 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
	{ 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
	{ 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
	{ 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
	{ 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
	{ 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
	{ 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
	{ 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
	{ 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
	{ 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
	{ 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
	{ 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
	{ 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
	{ 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
	{ 0x7E, 0x11, 0x11, 0x11, 0x7E },   // A
	{ 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
	{ 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
	{ 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
	{ 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
	{ 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
	{ 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
	{ 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
	{ 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
	{ 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
	{ 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
	{ 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
	{ 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
	{ 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
	{ 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
	{ 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
	{ 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
	{ 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
	{ 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
	{ 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
	{ 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
	{ 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
	{ 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
	{ 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
	{ 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
	{ 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
	{ 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
	{ 0x55, 0x2A, 0x55, 0x2A, 0x55 },   // 55
	{ 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ]
	{ 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
	{ 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
	{ 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
	{ 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
	{ 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
	{ 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
	{ 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
	{ 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
	{ 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
	{ 0x0C, 0x52, 0x52, 0x52, 0x3E },   // g
	{ 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
	{ 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
	{ 0x20, 0x40, 0x44, 0x3D, 0x00 },   // j
	{ 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
	{ 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
	{ 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
	{ 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
	{ 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
	{ 0x7C, 0x14, 0x14, 0x14, 0x08 },   // p
	{ 0x08, 0x14, 0x14, 0x18, 0x7C },   // q
	{ 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
	{ 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
	{ 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
	{ 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
	{ 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
	{ 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
	{ 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
	{ 0x0C, 0x50, 0x50, 0x50, 0x3C },   // y
	{ 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z
	{ 0x00, 0x08, 0x36, 0x41, 0x00 },   // {
	{ 0x00, 0x00, 0x77, 0x00, 0x00 },   // |
	{ 0x00, 0x41, 0x36, 0x08, 0x00 },   // }
	{ 0x04, 0x02, 0x04, 0x08, 0x04 }   // ~
};


//void Hdot()
//{
//	//字库取模改成横向
//	unsigned char pix = 0;
//	for (int k=0; k<96; k++)
//	{
//		for (int i=0; i<16; i++)//暂存
//		{
//			ASCII816Buf[i]=ASCII816[k][i];
//		}
//
//		for(int j=0; j<8; j++)//转换上半部分8*8
//		{
//			pix = 0;
//			for(int i=0; i<8; i++)
//			{
//				pix = pix<<1;
//				pix=pix+((ASCII816Buf[7-i]>>j)&1);
//			}
//			ASCII816[k][j] = pix;
//		}
//
//		for(int j=8; j<16; j++)//转换下半部分8*8
//		{
//			pix = 0;
//			for(int i=8; i<16; i++)
//			{
//				pix = pix<<1;
//				pix=pix+((ASCII816Buf[16+7-i]>>(j-8))&1);
//			}
//			ASCII816[k][j] = pix;
//		}
//	}
//}



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

	//Hdot();//字库取模改成横向


	LedUnitGbData.DataType = 23;
	LedUnitGbData.Cycle = 500;
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

	printf("LedUnitGbCommand size = %d\r\n",sizeof(struct tLedUnitGbCommand));

	delay(5000);





	//四条横线
	//for (int j=0; j<8; j++)
	//{
	//	DispBuf[j][0] = 0xff;
	//	DispBuf[j][15] = 0xff;
	//	DispBuf[j][16] = 0xff;
	//	DispBuf[j][31] = 0xff;
	//}

	////间隔横线
	//for (int j=0; j<8; j++)
	//{
	//	for (int i=0; i<32; i=i+2)
	//	{
	//		DispBuf[j][i] = 0xff;
	//	}
	//}

	////方框
	//for (int j=0; j<8; j++)
	//{
	//	DispBuf[j][0] = 0xff;
	//	DispBuf[j][31] = 0xff;
	//}
	//for (int i=1; i<31; i++)
	//{
	//	DispBuf[0][i] = 1;
	//	DispBuf[7][i] = 128;
	//}


	//FirstLine[0] = '0';
	//FirstLine[2] = '2';
	//SecondLine[2] = '2';

	char * test = "两位同学快睡觉";
	//test = "两";
	ShowChineseOnLedUnit(0,0,test,LedUnitGbCommand.DispBuf);

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
	//for(row=0; row<16; row++)
	//{

	static unsigned char row=0;
	for (int i=0; i<8; i++)
	{
		hc595senddata(DispBuf[i][row],DispBuf[i][row+16]);//发送列数据，上16行与下16行同时发送。
	}
	//digitalWrite(OE, 1);  //关闭显示
	hc138sacn(row);            //选行
	digitalWrite(STB, 1);      //数据确认
	digitalWrite(STB, 0);
	//delayMicroseconds(50) ;   //节电用,
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
	//}

}

unsigned long CurrentDisplayTime;
unsigned long LastDisplayTime;
void DisplayTimerTask()
{

	static bool ScreenOn = false;
	//micros()
	//millis()
	CurrentDisplayTime = micros();
	if (abs(CurrentDisplayTime - LastDisplayTime) > LedUnitGbData.Cycle)
	{
		LastDisplayTime = CurrentDisplayTime;
		DisplayOneLine();
		ScreenOn = true;
	}

	//if ((ScreenOn)&&(abs(CurrentDisplayTime - LastDisplayTime) >= LedUnitGbData.Light))
	//{
	//	digitalWrite(OE, 1);  //关闭显示
	//	ScreenOn = false;
	//}

	//if ((ScreenOn))
	//{
	//	digitalWrite(OE, 1);  //关闭显示
	//	ScreenOn = false;
	//}

	

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

			//for (int j=0; j<8; j++)
			//{
			//	for (int i=0; i<32; i++)
			//	{
			//		DispBuf[j][i] = LedUnitGbCommand.DispBuf[j][i];
			//	}
			//}

			printf("Update Cycle = %d  Light = %d\r\n"
				,LedUnitGbData.Cycle
				,LedUnitGbData.DisplayLight);
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
		//MyPrintf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
		//MyPrintf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
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
