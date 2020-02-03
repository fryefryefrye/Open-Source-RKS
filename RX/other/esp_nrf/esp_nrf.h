




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


#define RELAY				D4

#include <SPI.h>
#include "RF24.h"
RF24 radio(2,4);//D5 D6 D7






#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8

#define DATA_LENGTH 4
unsigned char addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
unsigned char GotData[DATA_LENGTH];
unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;
unsigned long LastTagGetTime = 0;
unsigned long Volt;   //unit: mV,


const char* ssid = "frye_iot3";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 22;
tKeyLessData KeyLessData;
unsigned long LastServerUpdate;



unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
void nRFTask();
void ChHopTask();


void MyPrintf(const char *fmt, ...);


void setup() 
{       

	pinMode(RELAY, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY, LOW);

	KeyLessData.Triger = false;



	delay(50);                      
	Serial.begin(115200);


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
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
	if (WiFi.status() == WL_CONNECTED)
	{
		configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
		Serial.println("\nWaiting for time");
		while (!time(nullptr)) {
			Serial.print(".");
			delay(1000);    
		}
		Serial.println("");
	}





	byte mac[6];
	WiFi.softAPmacAddress(mac);
	//printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	for (byte i=0;i<6;i++)
	{
		KeyLessData.Mac[i] = mac[i];
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


	radio.begin();
	radio.setPALevel(RF24_PA_MIN);
	radio.setAddressWidth(5);
	radio.setPayloadSize(4);
	radio.setDataRate(RF24_2MBPS); //RF24_250KBPS  //RF24_2MBPS     //RF24_1MBPS
	radio.setChannel(105);
	radio.setCRCLength(RF24_CRC_8); //RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit

	//Open a writing and reading pipe on each radio, with opposite addresses
	radio.openWritingPipe(addresses);
	radio.openReadingPipe(0,addresses);
	radio.closeReadingPipe(1);
	radio.closeReadingPipe(2);
	radio.closeReadingPipe(3);
	radio.closeReadingPipe(4);
	radio.closeReadingPipe(5);
	radio.setAutoAck(1,false);
	radio.setAutoAck(2,false);
	radio.setAutoAck(3,false);
	radio.setAutoAck(4,false);
	radio.setAutoAck(5,false);
	radio.setAutoAck(0,false);
	//Start the radio listening for data
	radio.startListening();


}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();
	nRFTask();
	ChHopTask();


	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tKeyLessCommand))
	{
		//MyPrintf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		//printf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		tKeyLessCommand tempKeyLessCommand;
		m_WiFiUDP.read((char *)&tempKeyLessCommand,sizeof(tKeyLessCommand));

		LastServerUpdate = 0;
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

			LastTagGetTime = TenthSecondsSinceStart;

		}
		else if (GotData[0] == 2)
		{
			LastTagGetTime = TenthSecondsSinceStart;

		}

		KeyLessData.DataType = 14;
		KeyLessData.KeyLessData = GotData[0];
		KeyLessData.KeyLessData = KeyLessData.KeyLessData<<8;
		KeyLessData.KeyLessData = KeyLessData.KeyLessData + GotData[1];
		KeyLessData.KeyLessData = KeyLessData.KeyLessData<<8;
		KeyLessData.KeyLessData = KeyLessData.KeyLessData + GotData[2];
		KeyLessData.KeyLessData = KeyLessData.KeyLessData<<8;
		KeyLessData.KeyLessData = KeyLessData.KeyLessData + GotData[3];

		KeyLessData.Triger = true;


		m_WiFiUDP.beginPacket("192.168.0.17", 5050);
		m_WiFiUDP.write((const char*)&KeyLessData, sizeof(tKeyLessData));
		m_WiFiUDP.endPacket(); 

		//printf("PackageCounter:%d Data:%d,%d,%d,%d Volt:%d CH:%d\r\n"
		//,PackageCounter
		//,GotData[0]
		//,GotData[1]
		//,GotData[2]
		//,GotData[3]
		//,Volt
		//,CurrCH);

	}

}
void ChHopTask()
{
	if (TenthSecondsSinceStart - LastChangeCHTime > 10)             //RF_HOP every seconds
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

	printf("LastServerUpdate = %d\r\n",LastServerUpdate);


	LastServerUpdate++;
	if (LastServerUpdate > 30)
	{
		printf("Re connection routing!\n");  
		WiFi.disconnect();
		WiFi.mode(WIFI_STA);//设置模式为STA
		WiFi.begin(ssid, password); //Wifi接入到网络
		LastServerUpdate = 0;
	}

	KeyLessData.Triger = false;

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&KeyLessData, sizeof(tKeyLessData));
	m_WiFiUDP.endPacket(); 
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
