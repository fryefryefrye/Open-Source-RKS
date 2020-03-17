




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



#define INT1	D2
#define INT2	D3


const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 20;
//tXXXXXXXXData XXXXXXXXData;
unsigned long LastServerUpdate;



unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();


unsigned long Int1_LastMicros = 0;
unsigned long Int2_LastMicros = 0;
unsigned long Int1_DiffMicros = 0;
unsigned long Int2_DiffMicros = 0;
unsigned int Speed = 0;
int Postion = 0;
bool Direction; //True for open
ICACHE_RAM_ATTR void Encoder_INT1();
ICACHE_RAM_ATTR void Encoder_INT2();



void MyPrintf(const char *fmt, ...);


void setup() 
{       


	delay(50);                      
	Serial.begin(115200);

	//XXXXXXXXData.DataType = 0;


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	byte mac[6];
	WiFi.softAPmacAddress(mac);
	printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	for (byte i=0;i<6;i++)
	{
		//XXXXXXXXData.Mac[i] = mac[i];
	}

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

	pinMode(INT1, INPUT);
	attachInterrupt(digitalPinToInterrupt(INT1), Encoder_INT1, RISING);
	pinMode(INT2, INPUT);
	attachInterrupt(digitalPinToInterrupt(INT2), Encoder_INT2, RISING);
}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();



	//m_WiFiUDP.parsePacket(); 
	//unsigned int UdpAvailable = m_WiFiUDP.available();
	//if (UdpAvailable == sizeof(tXXXXXXXXCommand))
	//{
	//	//MyPrintf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
	//	//printf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
	//	tXXXXXXXXCommand tempXXXXXXXXCommand;
	//	m_WiFiUDP.read((char *)&tempXXXXXXXXCommand,sizeof(tXXXXXXXXCommand));
	//	LastServerUpdate = 0;
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

	//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	//m_WiFiUDP.write((const char*)&XXXXXXXXData, sizeof(tXXXXXXXXData));
	//m_WiFiUDP.endPacket(); 
}

void OnTenthSecond()
{


	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}
	
}

ICACHE_RAM_ATTR void Encoder_INT1()//中断函数
{
	//static unsigned int Counter = 0;

	//Counter ++;
	Int1_DiffMicros = micros() - Int2_LastMicros;

	Direction = Int1_DiffMicros<Int2_DiffMicros;


	//if (!isDoorClosed||DoorOpening)
	//{
	//	isIntDataProcessing = true;
		if (Direction)
		{
			Postion++;
		} 
		else
		{
			Postion--;
		}

	//	unsigned long CurrentSpeed = (micros() - Int1_LastMicros)/0xFF;
	//	if (CurrentSpeed > 0xFFFF)
	//	{
	//		CurrentSpeed = 0xFFFF;
	//	}
	//	Speed = CurrentSpeed;
	//	isIntDataProcessing = false;

	//	if ((Postion>=0)&&(Postion<DOOR_POSRION_NUMBER))
	//	{
	//		if (Direction)
	//		{
	//			OpenSpeed[Postion] = Speed;
	//		} 
	//		else
	//		{
	//			CloseSpeed[Postion] = Speed;
	//		}
	//	}

	//}
	//else
	//{
	//	Postion = 0;
	//}



	printf("Direction: %s Postion %d Speed: %d\r\n"
		,(Direction?"Open":"Close")
		,Postion
		,Speed
		);

	Int1_LastMicros = micros();

}

ICACHE_RAM_ATTR void Encoder_INT2()//中断函数
{
	static unsigned long LastMicros = 0;
	Int2_DiffMicros = micros() - Int1_LastMicros;
	Int2_LastMicros = micros();
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
