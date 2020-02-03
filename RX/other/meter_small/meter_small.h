




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

#define SERVER_ADDRESS "fryefryefrye.myds.me"

#define SOFT_SERIAL_RX		D6
#define SOFT_SERIAL_TX		D7

#include <SoftwareSerial.h>
SoftwareSerial swSer(SOFT_SERIAL_RX, SOFT_SERIAL_TX, false, 256);



#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8


const char* ssid = "frye_iot2";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
tMeterData MeterData;
unsigned char DebugLogIndex = 28;
unsigned long LastServerUpdate;



unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();


//Modbus AC Power
#define MODBUS_REQUEST_LEN 8
unsigned char ModbusRequest[MODBUS_REQUEST_LEN] = {0x01,0x04,0x00,0x00,0x00,0x0a,0x70,0x0d};
void CheckModebus(unsigned char k);


void MyPrintf(const char *fmt, ...);


void setup() 
{       



	swSer.begin(9600);

	MeterData.DataType = 5;



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
	unsigned char recvdata;

	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();





	while (swSer.available() > 0) 
	{
		recvdata = swSer.read();
		//printf("%02X",recvdata);
		CheckModebus(recvdata);
	}

	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tMeterCommand))
	{
		//MyPrintf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		//printf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		tMeterCommand tempMeterCommand;
		m_WiFiUDP.read((char *)&tempMeterCommand,sizeof(tMeterCommand));

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

	LastServerUpdate++;
	if (LastServerUpdate > 30)
	{
		printf("Re connection routing!\n");  
		WiFi.disconnect();
		WiFi.mode(WIFI_STA);//设置模式为STA
		WiFi.begin(ssid, password); //Wifi接入到网络
		LastServerUpdate = 0;
	}


	//send modbus request to read power
	for (unsigned char i = 0; i<MODBUS_REQUEST_LEN ; i++)
	{
		swSer.write(ModbusRequest[i]);
	}




	//every minute
	if (now%60 == 0)
	{

	}







	m_WiFiUDP.beginPacket(SERVER_ADDRESS, 5050);
	m_WiFiUDP.write((const char*)&MeterData, sizeof(tMeterData));
	m_WiFiUDP.endPacket(); 

}

void OnTenthSecond()
{

	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}
}

#define MODBUS_REPLY_START_LEN 3
#define MODBUS_REPLY_CONTENT_LEN 20
unsigned char ModbusReplyStart[MODBUS_REPLY_START_LEN] =  {0x01,0x04,0x14};
void CheckModebus(unsigned char k)
{
	static unsigned char PowerByte[4];
	static unsigned char EnergyByte[4];
	static bool IsModbusReplyStart = false;
	static unsigned char ModbusReplyStartIndex = 0;
	static unsigned char ModbusReplyIndex = 0;

	if (IsModbusReplyStart)
	{
		switch (ModbusReplyIndex)
		{
		case 6:
			PowerByte[1] = k;
			break;
		case 7:
			PowerByte[0] = k;
			break;
		case 8:
			PowerByte[3] = k;
			break;
		case 9:
			PowerByte[2] = k;
			break;

		case 10:
			EnergyByte[1] = k;
			break;
		case 11:
			EnergyByte[0] = k;
			break;
		case 12:
			EnergyByte[3] = k;
			break;
		case 13:
			EnergyByte[2] = k;
			break;
		}


		ModbusReplyIndex++;


		if (ModbusReplyIndex >= MODBUS_REPLY_CONTENT_LEN)
		{
			memcpy(&(MeterData.Power),&PowerByte,4);
			//MeterData.Power = MeterData.Power/10;
			//MyPrintf("Power = %d  *0.1W\n",MeterData.Power);

			memcpy(&(MeterData.TotalEnergy),&EnergyByte,4);
			MeterData.TotalEnergy = MeterData.TotalEnergy/10;
			//MyPrintf("TotalEnergy = %d  *1Wh\n",MeterData.TotalEnergy);

			ModbusReplyStartIndex = 0	;
			IsModbusReplyStart = false;
			ModbusReplyIndex = 0;
		}
	} 
	else
	{
		if (ModbusReplyStart[ModbusReplyStartIndex] == k)
		{
			ModbusReplyStartIndex++;
			if (ModbusReplyStartIndex >= MODBUS_REPLY_START_LEN)
			{
				IsModbusReplyStart =true;
			}
		} 
		else
		{
			ModbusReplyStartIndex=0;
		}
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

	m_WiFiUDP.beginPacket(SERVER_ADDRESS, 5050);
	m_WiFiUDP.write((const char*)send_buf, sizeof(tDebugData)+n);
	m_WiFiUDP.endPacket(); 


}