




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


#define RELAY				D0
#define POSTION_OPEN		D1
#define SOFT_SERIAL_RX		D4
#define SOFT_SERIAL_TX		D5





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

#include <SoftwareSerial.h>
SoftwareSerial swSer(SOFT_SERIAL_RX, SOFT_SERIAL_TX, false, 256);


#include "Z:\bt\web\datastruct.h"
tLogisticsUpdate LogisticsUpdate;
tLogisticsData LogisticsData;
tLogisticsCommand LogisticsCommand;
unsigned long LastSentUpdateSeq;

unsigned char DebugLogIndex = 25;



unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();


void MyPrintf(const char *fmt, ...);

void OpenDoor();
bool ProcessBarCode();


void setup() 
{       

	pinMode(RELAY, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY, HIGH);

	pinMode(POSTION_OPEN, INPUT_PULLUP);

	LogisticsUpdate.DataType = 18;
	LogisticsUpdate.NeedIdList = true;
	LogisticsUpdate.JustScaned = false;

	strcpy((char*)LogisticsUpdate.LogisticsID,"test");



	delay(50);                      
	Serial.begin(115200);

	swSer.begin(9600);


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
	//for (byte i=0;i<6;i++)
	//{
	//	RoomData.Mac[i] = mac[i];
	//}

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


}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();


	static unsigned char swSerlLen = 0;
	while (swSer.available() > 0) 
	{
		unsigned char ReadValue = swSer.read();
		if ((ReadValue>32)&&(ReadValue<127))//ACSII chart
		{
			LogisticsUpdate.LogisticsID[swSerlLen] = ReadValue;
			swSerlLen++;
		}
		else if (ReadValue == 13)
		{
			swSer.read();//clear \n
			LogisticsUpdate.LogisticsID[swSerlLen] = '\0';


			MyPrintf("Got Bar code len = %d %s\r\n",swSerlLen,LogisticsUpdate.LogisticsID);

			if (swSerlLen>11)
			{
				MyPrintf("swSerlLen  > 11 \r\n");
				LogisticsUpdate.ScanOpened = ProcessBarCode();
				MyPrintf("check bar code = %d \r\n",LogisticsUpdate.ScanOpened);
				LogisticsUpdate.JustScaned = true;

				if(LogisticsUpdate.ScanOpened)
				{
					OpenDoor();
				}
			}
			else
			{
				MyPrintf("swSerlLen not > 11 \r\n");
			}


			swSerlLen = 0;

		}
		else
		{
			MyPrintf("Got none ACSII char %d \r\n",ReadValue);
			swSerlLen = 0;
		}
		yield();
	}


	//static unsigned char swSerlLen = 0;
	//while (Serial.available() > 0) {
	//	unsigned char ReadValue = Serial.read();
	//	LogisticsUpdate.LogisticsID[swSerlLen] = ReadValue;
	//	if (LogisticsUpdate.LogisticsID[swSerlLen] == 13)
	//	{
	//		LogisticsUpdate.LogisticsID[swSerlLen] = '\0';
	//		printf("Got code len = %d  %s\r\n",swSerlLen,LogisticsUpdate.LogisticsID);
	//		Serial.read();//clear \n
	//		swSerlLen = 0;
	//		LogisticsUpdate.ScanOpened = ProcessBarCode();
	//		LogisticsUpdate.JustScaned = true;

	//		if(LogisticsUpdate.ScanOpened)
	//		{
	//			OpenDoor();
	//		}
	//	}
	//	else
	//	{
	//		swSerlLen++;
	//	}
	//}



	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tLogisticsData))
	{
		m_WiFiUDP.read((char *)&LogisticsData,sizeof(tLogisticsData));
		LogisticsUpdate.NeedIdList = false;
		LogisticsUpdate.LastGetLogisticsIDListVersion = LogisticsData.LogisticsIDListVersion;

		for(unsigned int i = 0; i<MAX_LOGISTICS_NUMBER; i++)
		{
			printf("%s\r\n", LogisticsData.LogisticsID[i]);
		}

	}
	if (UdpAvailable == sizeof(tLogisticsCommand))
	{
		m_WiFiUDP.read((char *)&LogisticsCommand,sizeof(tLogisticsCommand));
		//LogisticsUpdate.LastGetCommandSeq = LogisticsCommand.Seq;

		if(LogisticsCommand.LastGetUpdateSeq == LastSentUpdateSeq)
		{
			LogisticsUpdate.JustScaned = false;

		}

		if (LogisticsCommand.ManualTriger == true)
		{
			OpenDoor();
			MyPrintf("get open trig from control\r\n");
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
	}
}

void OnSecond()
{
	time_t now = time(nullptr); //获取当前北京时间
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

	if (now%(3600*24) == 3600+1200) //running at 1:20 AM every day
	{
		LogisticsUpdate.NeedIdList = true;
	}

	//if (now%2 == 0)
	//{
	//	digitalWrite(RELAY,HIGH);
	//}
	//else
	//{
	//	digitalWrite(RELAY,LOW);
	//}


	if (LogisticsUpdate.JustScaned)
	{
		LastSentUpdateSeq++;
	}
				

	LogisticsUpdate.Open = digitalRead(POSTION_OPEN);
	LogisticsUpdate.Seq = LastSentUpdateSeq;
	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&LogisticsUpdate, sizeof(tLogisticsUpdate));
	m_WiFiUDP.endPacket(); 

}

void OnTenthSecond()
{

	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}
	

}

void OpenDoor()
{
	yield();
	digitalWrite(RELAY,LOW);
	delay(500);
	yield();
	digitalWrite(RELAY,HIGH);
	delay(500);
	if (digitalRead(POSTION_OPEN))
	{
		return;
	}
	yield();
	digitalWrite(RELAY,LOW);
	delay(500);
	yield();
	digitalWrite(RELAY,HIGH);
	if (digitalRead(POSTION_OPEN))
	{
		return;
	}
	yield();
	digitalWrite(RELAY,LOW);
	delay(500);
	yield();
	digitalWrite(RELAY,HIGH);
}
bool ProcessBarCode()
{


	if (LogisticsUpdate.NeedIdList)
	{
		return true;
	}

	if (LogisticsCommand.IfNeedCheckLogisticsID)
	{

		//MyPrintf("strlen((char*)LogisticsUpdate.LogisticsID = %d\r\n", strlen((const char*)LogisticsUpdate.LogisticsID));
		//do not compare last 4 digits,Tmall more ID for one order

		char a[MAX_LOGISTICS_ID_LEN];
		unsigned char aLen;
		char b[MAX_LOGISTICS_ID_LEN];
		unsigned char bLen;

		bLen = strlen((const char*)LogisticsUpdate.LogisticsID);
		if (bLen<5)
		{
			return false;
		}
		strncpy(b,(const char*)LogisticsUpdate.LogisticsID,bLen-4);
		b[bLen-4] = 0;
		//MyPrintf("b = %s \r\n", b);

		unsigned int i;
		for(i = 0; i<MAX_LOGISTICS_NUMBER; i++)
		{
			//MyPrintf("before compare %s \r\n", LogisticsData.LogisticsID[i]);
			aLen = strlen((const char*)LogisticsData.LogisticsID[i]);
			if (aLen<5)
			{
				//MyPrintf("compare taobao %s failed,Len<5 \r\n", LogisticsData.LogisticsID[i]);
				continue;
			}
			strncpy(a,(const char*)LogisticsData.LogisticsID[i],aLen-4);
			a[aLen-4] = 0;
			//MyPrintf("a = %s \r\n", a);
			if(strcmp((const char*)a,(const char*)b) == 0)
			{
				MyPrintf("compare taobao %s OK \r\n", LogisticsData.LogisticsID[i]);
				return true;
			}
			else
			{
				//MyPrintf("compare %s failed \r\n", LogisticsData.LogisticsID[i]);
			}
		}

		//MyPrintf("finish taobao compare i = %d \r\n",i);


		for( i = 0; i<MAX_LOGISTICS_NUMBER; i++)
		{
			if(strcmp((char*)LogisticsData.TempLogisticsIDList[i],(char*)LogisticsUpdate.LogisticsID) == 0)
			{
				MyPrintf("compare temp %s OK \r\n", LogisticsData.TempLogisticsIDList[i]);
				return true;
			}
			else
			{
				//printf("compare %s failed \r\n", LogisticsData.LogisticsID[i]);
			}
		}
	}
	else
	{
		return true;
	}

	return false;
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
