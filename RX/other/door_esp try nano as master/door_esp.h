




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


//#define RELAY				D0





#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8


#include <Wire.h>

const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
tCompressorData CompressorData;
unsigned char RoomIndex = 20;
bool FullReached = false;
unsigned long RunningCounter = 0;


unsigned long Int1_LastMicros = 0;
unsigned long Int2_LastMicros = 0;
unsigned long Int1_DiffMicros = 0;
unsigned long Int2_DiffMicros = 0;
unsigned long Speed = 0;
int Postion = 0;


unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
void IR_DEconde_INT1();
void IR_DEconde_INT2();

void MyPrintf(const char *fmt, ...);

unsigned char IicEspSend[IIC_ESP_SEND_BYTE];
unsigned char IicEspRecv[IIC_ESP_RECV_BYTE];
unsigned char IicRecvCounter;

unsigned long IicRecvTimeOutTenthSecond;

void receiveEvent(int howMany);
void requestEvent();

void receiveEvent(int howMany) 
{
	unsigned char RecvIndex = 0;
	while (0 <Wire.available()) 
	{
		if (RecvIndex<IIC_ESP_RECV_BYTE)
		{
			IicEspRecv[RecvIndex] = Wire.read(); 
			RecvIndex++;
			IicRecvTimeOutTenthSecond = 0;
			IicRecvCounter = RecvIndex;
		}
	}
}

void requestEvent() 
{
	Wire.write(IicEspSend,IIC_ESP_SEND_BYTE);
	IicEspSend[IIC_ESP_SEND_BYTE-1]++;
}



void setup() 
{       

	//pinMode(RELAY, OUTPUT);//set the pin to be OUTPUT pin.
	//digitalWrite(RELAY, LOW);

	//pinMode(D2, INPUT_PULLUP);


	//pinMode(D7, INPUT);
	//attachInterrupt(digitalPinToInterrupt(D7), IR_DEconde_INT1, RISING);
	//pinMode(D6, INPUT);
	//attachInterrupt(digitalPinToInterrupt(D6), IR_DEconde_INT2, RISING);

	CompressorData.DataType = 4;
	CompressorData.isOn = false;

	Wire.begin(D1,D2,0x08); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
	Wire.onReceive(receiveEvent); /* register receive event */
	Wire.onRequest(requestEvent); /* register request event */


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
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X AP:%s\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],ssid);
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


		Speed = micros() - Int1_LastMicros;
		unsigned long CurrentUnInt1Diff = micros() - Int1_LastMicros;
		if (CurrentUnInt1Diff > Speed)
		{
			Speed = CurrentUnInt1Diff;
		}


		//printf("Postio: %d Speed: %d\r\n"
		//	,Postion
		//	,Speed
		//	);
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

	

	//printf("IIC_recv %d bytes ",IicMasterRecvCounter);
	//for (char i = 0; i < IIC_MASTER_RECV_BYTE; i++)
	//{
	//	printf("0x%02X ",IicMasterRecv[i]);
	//}
	//printf("%s","\n");





	//printf("IIC timeout:%d %dbytes  0x%02X 0x%02X 0x%02X 0x%02X \r\n"
	//	,IicRecvTimeOut
	//	,IicMasterRecv
	//	,IicMasterRecv[0]
	//	,IicMasterRecv[1]
	//	,IicMasterRecv[2]
	//	,IicMasterRecv[3]
	//	);



	//IicRecvTimeOutTenthSecond++;
	//printf("IIC timeout = %d",IicRecvTimeOutTenthSecond);
	//printf(" %d bytes ",IicMasterRecvCounter);
	//printf("data: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
	//	,IicMasterRecv[0]
	//,IicMasterRecv[1]
	//,IicMasterRecv[2]
	//,IicMasterRecv[3]
	//,IicMasterRecv[4]);




	//printf("IIC timeout:%d . %d bytes  ",IicRecvTimeOut,IicMasterRecvCounter);
	
	//for (char i = 0; i < IIC_MASTER_RECV_BYTE; i++)
	//{
	//	printf("0x%02X ",IicMasterRecv[i]);
	//}
	//printf("\r\n");





	//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	//m_WiFiUDP.write((const char*)&CompressorData, sizeof(tCompressorData));
	//m_WiFiUDP.endPacket(); 

}

void OnTenthSecond()
{

	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}

	

	//IicExchange();

	
	printf("IIC timeout = %d",IicRecvTimeOutTenthSecond);
	printf(" %d bytes ",IicRecvCounter);
	printf("data: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n"
		,IicEspRecv[0]
	,IicEspRecv[1]
	,IicEspRecv[2]
	,IicEspRecv[3]
	,IicEspRecv[4]);

	IicRecvTimeOutTenthSecond++;

	

}



void IR_DEconde_INT1()//中断函数
{
	static unsigned int Counter = 0;

	Counter ++;
	Int1_DiffMicros = micros() - Int2_LastMicros;

	
	//printf("Counter1 = %d MicrosDiff = %d OtherDiff = %d Director: %s \r\n"
	//	,Counter
	//	,micros() - Int1_LastMicros
	//	,Int1_DiffMicros
	//	,(Int1_DiffMicros>Int2_DiffMicros?"Up":"Down")
	//	);

	bool Director = Int1_DiffMicros>Int2_DiffMicros;

	if (Director)
	{
		Postion++;
	} 
	else
	{
		Postion--;
	}

	Speed = micros() - Int1_LastMicros;

	printf("Director: %s Postion %d Speed: %d\r\n"
		,(Director?"Up":"Down")
		,Postion
		,Speed
		);

	Int1_LastMicros = micros();

}

void IR_DEconde_INT2()//中断函数
{
	static unsigned int Counter = 0;
	static unsigned long LastMicros = 0;
	Counter ++;
	Int2_DiffMicros = micros() - Int1_LastMicros;
	//printf("Counter2 = %d MicrosDiff = %d OtherDiff = %d \r\n"
	//	,Counter
	//	,micros() - Int2_LastMicros
	//	,Int2_DiffMicros
	//	);

	//printf("Speed: %d \r\n"
	//	,micros() - Int2_LastMicros
	//	);
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
	pDebugData->RoomId = RoomIndex;
	pDebugData->Length = n;

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)send_buf, sizeof(tDebugData)+n);
	m_WiFiUDP.endPacket(); 


}
