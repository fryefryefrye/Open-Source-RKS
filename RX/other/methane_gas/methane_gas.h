




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


#define RELAY_CLOSE			D5
#define RELAY_OPEN			D6
#define BUZZ				D7
#define KEY					D2

#define GAS_LIMIT			12






#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8


const char* ssid = "frye_iot";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 24;
tMethaneGasData MethaneGasData;


unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();

void CheckGasTask();
void KeyTask();
void RelayBuzzTask();
void CloseVavle();
void OpenVavle();
//bool isCloseVavle = false;
//bool isOpenVavle = false;
unsigned long VavleStartTime = 0;
bool isVavleOpened = true;



void MyPrintf(const char *fmt, ...);


void setup() 
{       

	pinMode(RELAY_CLOSE, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY_CLOSE, LOW);
	pinMode(RELAY_OPEN, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY_OPEN, LOW);
	pinMode(BUZZ, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(BUZZ, LOW);

	pinMode(KEY,INPUT_PULLUP);

	MethaneGasData.DataType = 16;



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
	for (byte i=0;i<6;i++)
	{
		//UsbChargeData.Mac[i] = mac[i];
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


}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();
	//RelayTask();





	//m_WiFiUDP.parsePacket(); 
	//unsigned int UdpAvailable = m_WiFiUDP.available();
	//if (UdpAvailable == sizeof(tUsbChargeCommand))
	//{
	//	//MyPrintf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
	//	tUsbChargeCommand tempUsbChargeCommand;
	//	m_WiFiUDP.read((char *)&UsbChargeCommand,sizeof(tUsbChargeCommand));
	//}
}

void RelayBuzzTask()
{
	if (VavleStartTime!=0)
	{
		if (TenthSecondsSinceStart - VavleStartTime > 70)
		{
			digitalWrite(RELAY_CLOSE, LOW);
			digitalWrite(RELAY_OPEN, LOW);
			VavleStartTime = 0;
		}


		if (TenthSecondsSinceStart%10 == 0)
		{
			digitalWrite(BUZZ, HIGH);
		}


		if (TenthSecondsSinceStart%10 == 2)
		{
			digitalWrite(BUZZ, LOW);
		}
	}
	else
	{
		//beep when off
		if (!isVavleOpened)
		{
			if (TenthSecondsSinceStart%30 == 0)
			{
				digitalWrite(BUZZ, HIGH);
			}


			if (TenthSecondsSinceStart%30 == (MethaneGasData.Percentage > GAS_LIMIT ? 10:0))
			{
				digitalWrite(BUZZ, LOW);
			}
		}
		else
		{
			digitalWrite(BUZZ, LOW);
		}

	}
}

void CloseVavle()
{
	isVavleOpened = false;
	digitalWrite(RELAY_CLOSE, HIGH);
	VavleStartTime = TenthSecondsSinceStart;
}
void OpenVavle()
{
	isVavleOpened = true;
	digitalWrite(RELAY_OPEN, HIGH);
	VavleStartTime = TenthSecondsSinceStart;
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


	unsigned long AnalogValue= analogRead(A0);
	MethaneGasData.Percentage = AnalogValue*100/1024;
	if (MethaneGasData.Percentage > 10)
	{
		//MyPrintf("AnalogValue = %d Percentage = %d  TenthSecondsSinceStart = %d \r\n",AnalogValue,MethaneGasData.Percentage,TenthSecondsSinceStart);
	}


	//MyPrintf("digitalRead(KEY) = %d  \r\n",digitalRead(KEY));



	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&MethaneGasData, sizeof(tMethaneGasData));
	m_WiFiUDP.endPacket(); 

}

void KeyTask()
{

#define  KEY_PRESS_MIN 2
#define PAUSE_KEY_PRESS 4


	static unsigned char KeyCounter = 0;
	static unsigned char PauseKeyCounter = PAUSE_KEY_PRESS;

	// key process
	if (PauseKeyCounter>0)
	{
		PauseKeyCounter--;
	} 
	else 
	{
		if (!(digitalRead(KEY)))
		{

			KeyCounter++;
			if (KeyCounter > KEY_PRESS_MIN)
			{
				PauseKeyCounter = PAUSE_KEY_PRESS;
				if (isVavleOpened)
				{
					CloseVavle();
				} 
				else
				{
					OpenVavle();
				}

				KeyCounter = 0;
			}
		}
		else
		{
			KeyCounter = 0;
		}
	}

}

void CheckGasTask()
{
	if ((MethaneGasData.Percentage > GAS_LIMIT)
		&&(TenthSecondsSinceStart>1800)
		&&isVavleOpened)
	{
		MyPrintf("Methane over limit. CloseVavle. \n");
		CloseVavle();
	}
}
void OnTenthSecond()
{

	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}


	RelayBuzzTask();
	KeyTask();
	CheckGasTask();
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
