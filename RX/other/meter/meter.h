




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

#include "SoftwareSerialEven.h"
SoftwareSerialEven swSer(SOFT_SERIAL_RX, SOFT_SERIAL_TX, false, 256);



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


#include "Z:\bt\web\datastruct.h"
tMeterData MeterData;
unsigned char DebugLogIndex = 23;

//表号001503120116
#define HEAD_LEN 14
#define REQUEST_LEN 6
#define REQUEST_NUMBER 6
unsigned char RequestHead[HEAD_LEN] = {0xFE,0xFE,0xFE,0xFE,0x68,0x16,0x01,0x12,0x03,0x15,0x00,0x68,0x11,0x04};
unsigned char Request[REQUEST_NUMBER][REQUEST_LEN] = {
	{0x33,0x33,0x36,0x35,0xF7,0x16}
	,{0x33,0x33,0x33,0x33,0xF2,0x16}
	,{0x33,0x33,0x3E,0x33,0xFD,0x16}
	,{0x34,0x33,0x3E,0x33,0xFE,0x16}
	,{0x33,0x34,0x35,0x35,0xF7,0x16}
	,{0x33,0x34,0x34,0x35,0xF6,0x16}
};

#define MAX_RESPONE 24

unsigned char Respone[MAX_RESPONE];
unsigned char ResponeCounter = 0;
bool isResponeHead = false;
unsigned char ResponeLen;
unsigned char RequestIndex = 0;

unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
unsigned long  BCDToDec(const unsigned char *bcd, int length);
void ProcessData();


void MyPrintf(const char *fmt, ...);


void setup() 
{       



	swSer.begin(2400);

	MeterData.DataType = 5;



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


	while (swSer.available() > 0) {
		unsigned char recvdata = swSer.read();

		//printf("%02X",recvdata);

		if (isResponeHead)
		{
			Respone[ResponeCounter] = recvdata;
			if (ResponeCounter == 13)
			{
				ResponeLen = BCDToDec(Respone+13,1);
				//printf("ResponeLen = %d \n",ResponeLen);
			}
			ResponeCounter ++;

			if (ResponeCounter>13+ResponeLen)
			{
				isResponeHead = false;
				ProcessData();
				ResponeCounter = 0;

				//printf("data = %d \n",BCDToDec(Respone+18,ResponeLen-4));
			}
			if (ResponeCounter>=MAX_RESPONE)
			{
				isResponeHead = false;
				ResponeCounter = 0;
			}
		} 
		else
		{
			if (recvdata == 0xFE)
			{
				Respone[ResponeCounter] = recvdata;
				ResponeCounter ++;
				if (ResponeCounter == 4)
				{
					isResponeHead = true;
				}
			}
			else
			{
				ResponeCounter = 0;
			}
		}


		yield();
	}

	//m_WiFiUDP.parsePacket(); 
	//unsigned int UdpAvailable = m_WiFiUDP.available();
	//if (UdpAvailable == sizeof(tCompressorCommand))
	//{
	//	//MyPrintf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
	//	tCompressorCommand tempCompressorCommand;
	//	m_WiFiUDP.read((char *)&tempCompressorCommand,sizeof(tRoomCommand));

	//	if (tempCompressorCommand.Triger == true)
	//	{
	//		RunningCounter = 60;
	//		MyPrintf("get Compressor trig from control\r\n");
	//	}
	//}
}

void ProcessData()
{
	unsigned long * pDataType = (unsigned long *)(Respone+14);
	for (unsigned char i = 0; i< ResponeLen ; i++)
	{
		Respone[14+i] = Respone[14+i] - 0x33;
	}
	//printf("pDataType = %d \n",*pDataType);
	//printf("data = %d \n",BCDToDec(Respone+18,ResponeLen-4));

	switch(*pDataType)
	{
	case 0:
		MeterData.TotalEnergy = BCDToDec(Respone+18,ResponeLen-4);
		//MyPrintf("TotalEnergy = %.2f kWh  \n",(float)MeterData.TotalEnergy/100);
		break;
	case 720896:
		MeterData.ThisEnergy = BCDToDec(Respone+18,ResponeLen-4);
		//MyPrintf("ThisEnergy = %.2f kWh  \n",(float)MeterData.ThisEnergy/100);
		break;
	case 720897:
		MeterData.LastEnergy = BCDToDec(Respone+18,ResponeLen-4);
		//MyPrintf("LastEnergy = %.2f kWh  \n",(float)MeterData.LastEnergy/100);
		break;
	case 33751040:
		MeterData.Power = BCDToDec(Respone+18,ResponeLen-4);
		//MyPrintf("Power = %.4f kW  \n",(float)MeterData.Power/10000);
		break;
	case 33685760:
		MeterData.Current = BCDToDec(Respone+18,ResponeLen-4);
		//MyPrintf("Current = %.3f A  \n",(float)MeterData.Current/1000);
		break;
	case 33620224:
		MeterData.volt = BCDToDec(Respone+18,ResponeLen-4);
		//MyPrintf("volt = %.1f v  \n",(float)MeterData.volt/10);
		break;
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

	if (TenthSecondsSinceStart%10 == 0)
	{
		for (unsigned char i = 0; i<HEAD_LEN ; i++)
		{
			swSer.write(RequestHead[i]);
			yield();
		}
		for (unsigned char i = 0; i<REQUEST_LEN ; i++)
		{
			swSer.write(Request[RequestIndex][i]);
			yield();
		}
		RequestIndex++;
		if (RequestIndex>=REQUEST_NUMBER)
		{
			RequestIndex = 1;
		}

	}

	if (TenthSecondsSinceStart%10 == 5)
	{
		for (unsigned char i = 0; i<HEAD_LEN ; i++)
		{
			swSer.write(RequestHead[i]);
			yield();
		}
		for (unsigned char i = 0; i<REQUEST_LEN ; i++)
		{
			swSer.write(Request[0][i]);
			yield();
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



unsigned long  BCDToDec(const unsigned char *bcd, int length) 
{ 
	int tmp; 
	unsigned long dec = 0; 

	for(int i = 0; i < length; i++) 
	{ 
		tmp = ((bcd[i] >> 4) & 0x0F) * 10 + (bcd[i] & 0x0F);    
		dec += tmp * pow(100, i);           
	} 

	return dec; 
}

//unsigned long  BCDToDec(const unsigned char *bcd, int length) 
//{ 
//	int tmp; 
//	unsigned long dec = 0; 
//
//	for(int i = 0; i < length; i++) 
//	{ 
//		tmp = ((bcd[i] >> 4) & 0x0F) * 10 + (bcd[i] & 0x0F);    
//		dec += tmp * pow(100, length - 1 - i);           
//	} 
//
//	return dec; 
//}
