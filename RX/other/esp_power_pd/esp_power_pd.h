




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
#include<time.h>
#define timezone 8

#define IIC_DAT				D2
#define IIC_CLK				D1




const char* ssid = "frye_iot3";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;
char TimeString[9];


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 20;
tPowerBankData PowerBankData;

#include <Wire.h>



//#include <U8g2lib.h>
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, IIC_CLK, IIC_DAT); //配置构造函数


#include "CN_SSD1306_Wire.h"
CN_SSD1306_Wire Displayer;//HardWare I2C

#include "Adafruit_INA219.h"
Adafruit_INA219 ina219(INA219_ADDRESS);
#define CURRENT_LOW_LIMIT 10			//mA
#define CURRENT_LOW_LIMIT_SECONDS 10	//Seconds

#define CURRENT_UP_LIMIT 20			//mA
//#define CURRENT_UP_LIMIT_SECONDS 10	//Seconds

struct {
	uint32_t crc32;
	unsigned long SecondsSincePowerOn;
	unsigned long LastWifiTimeBasePowerOn;
	//unsigned long GmtTimeDiffToPowerOn;
	tPowerBankCommand PowerBankCommand;
} rtcData;

unsigned long SecondsSinceStart = 0;
unsigned long TenthSecondsSinceStart = 0;
unsigned long CpuSleepTime = 10;
unsigned long WiFiSleepTime = 600;


void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
void StartWifi();
void StartOTA();
void GetPowerState();
void UpdateOled();
void GotoSleep();


void MyPrintf(const char *fmt, ...);
uint32_t calculateCRC32(const uint8_t* data, size_t length);

void setup() 
{       


	//delay(50);                      
	Serial.begin(115200);

	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	//delay(1);


	//Read data after sleep
	if (ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData))) {
		//Serial.println("Read: ");
		//Serial.println();
		uint32_t crcOfData = calculateCRC32(((uint8_t*)&rtcData.crc32)+4, sizeof(rtcData)-4);
		//printf("SecondsSincePowerOn read from RTC = %d \r\n", rtcData.SecondsSincePowerOn);
		//Serial.print("CRC32 read from RTC: ");
		//Serial.println(rtcData.crc32, HEX);
		if (crcOfData != rtcData.crc32) {
			Serial.println("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably Not OK!");
			rtcData.SecondsSincePowerOn = 0;
			rtcData.LastWifiTimeBasePowerOn = 0;
			//rtcData.GmtTimeDiffToPowerOn = 0;
			rtcData.PowerBankCommand.WiFiAlwaysOn = false;
		}
		else {
			Serial.println("CRC32 check ok, data is probably OK.");
			rtcData.SecondsSincePowerOn = rtcData.SecondsSincePowerOn + CpuSleepTime;
		}
	}

	//启动后，先检查电流状态
	Wire.begin(IIC_DAT,IIC_CLK);
	ina219.begin();
	GetPowerState();

	//有电流的话，立即亮屏，否则等待WiFi时间太长
	if (abs(PowerBankData.Current) > CURRENT_UP_LIMIT)
	{
		UpdateOled();
	}
		




	//根据各项条件判断是否需要打开WiFi，否则休眠。
	if ((rtcData.SecondsSincePowerOn == 0)
		||(rtcData.SecondsSincePowerOn - rtcData.LastWifiTimeBasePowerOn > WiFiSleepTime)
		|| (abs(PowerBankData.Current) > CURRENT_UP_LIMIT)
		)
	{
		StartWifi();
		Displayer.Initial(); // 首次通电肯定启动WiFi
		delay(10);
		Displayer.Fill_Screen(0x00);//首次通电清屏 否则首次通电花屏。
	}
	else 
	{
		printf("GotoSleep for no need wifi and no Current \r\n");
		GotoSleep();
	}


	if ((abs(PowerBankData.Current) > CURRENT_UP_LIMIT)
		||(rtcData.PowerBankCommand.WiFiAlwaysOn)
		)
	{
		//电流大，或者要求WiFi常开
		//进入运行状态，判断WiFi是否成功连接,否则关闭WiFi省电
		if (WiFi.status() == WL_CONNECTED)
		{
			printf("StartOTA \r\n");
			StartOTA();
		}
		else
		{

			printf("Turn off wifi，because wifi connection failed\r\n");
			WiFi.disconnect();
			WiFi.mode(WIFI_OFF);
			WiFi.forceSleepBegin();
			delay(1);
		}

	}
	else//没有要求常开WiFi，并且无电流的话，休眠。
	{
		printf("GotoSleep for no WiFiAlwaysOn, and no Current  \r\n");
		GotoSleep();
	}




}


void loop()
{

	TenthSecondsSinceStartTask();

	if (WiFi.status() == WL_CONNECTED)
	{
		ArduinoOTA.handle();

		m_WiFiUDP.parsePacket();
		unsigned int UdpAvailable = m_WiFiUDP.available();
		if (UdpAvailable == sizeof(tPowerBankCommand))
		{
			//printf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
			m_WiFiUDP.read((char*)&rtcData.PowerBankCommand, sizeof(tPowerBankCommand));
		}
	}
}

void OnSecond()
{
	static unsigned char CurrentUnderCounter = 0;
	rtcData.SecondsSincePowerOn++;
	SecondsSinceStart++;
	GetPowerState();
	UpdateOled();

	//printf("PowerOn = %d; Start = %d \r\n", rtcData.SecondsSincePowerOn, SecondsSinceStart);


	if (WiFi.status() == WL_CONNECTED)
	{
		m_WiFiUDP.beginPacket("192.168.0.17", 5050);
		m_WiFiUDP.write((const char*)&PowerBankData, sizeof(tPowerBankData));
		m_WiFiUDP.endPacket();
	}

	if ((WiFi.status() != WL_CONNECTED)
		&& (rtcData.SecondsSincePowerOn - rtcData.LastWifiTimeBasePowerOn > WiFiSleepTime))
	{
		printf("Start wifi when running offline ,but need wifi time is up \r\n");
		StartWifi();
		//Displayer.Fill_Screen(0x00);
	}

	if (abs(PowerBankData.Current) < CURRENT_LOW_LIMIT)
	{
		CurrentUnderCounter++;
	}
	else
	{
		CurrentUnderCounter = 0;
	}
	
	printf("CurrentUnderCounter = %d \r\n", CurrentUnderCounter);



	//没有要求常开WiFi，并且电流消失的话，休眠。
	if (
		(CurrentUnderCounter > CURRENT_LOW_LIMIT_SECONDS)
		&& (!rtcData.PowerBankCommand.WiFiAlwaysOn)
		)
	{
		printf("GotoSleep for no WiFiAlwaysOn, and  Current under for long time  \r\n");
		if (WiFi.status() == WL_CONNECTED)
		{
			rtcData.LastWifiTimeBasePowerOn = rtcData.SecondsSincePowerOn;
		}
		Displayer.Fill_Screen(0x00);
		GotoSleep();
	}



}

void GetPowerState()
{
	float Current;
	float BusVoltage;

	Current = ina219.getCurrent_mA();
	BusVoltage = ina219.getBusVoltage_V();


	if ((BusVoltage >= 0)&&(BusVoltage<(float)30))
	{
		PowerBankData.volt = BusVoltage * 1000;
	}

	if (abs(Current)<(float)5000)
	{
		PowerBankData.Current = Current;

	}

	printf("Current:%d BusVoltage:%d \r\n",PowerBankData.Current,PowerBankData.volt);

	//PowerBankData.Current = PowerBankData.Current - 820;

}


void StartOTA()
{

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		}
		else { // U_SPIFFS
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
		}
		else if (error == OTA_BEGIN_ERROR) {
			Serial.println("Begin Failed");
		}
		else if (error == OTA_CONNECT_ERROR) {
			Serial.println("Connect Failed");
		}
		else if (error == OTA_RECEIVE_ERROR) {
			Serial.println("Receive Failed");
		}
		else if (error == OTA_END_ERROR) {
			Serial.println("End Failed");
		}
		});
	ArduinoOTA.begin();
	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

}

void StartWifi()
{

	PowerBankData.DataType = 24;
	rtcData.LastWifiTimeBasePowerOn = rtcData.SecondsSincePowerOn;

	//Static IP address configuration
	IPAddress staticIP(192, 168, 0, 19); //ESP static ip
	IPAddress gateway(192, 168, 0, 1);   //IP Address of your WiFi Router (Gateway)
	IPAddress subnet(255, 255, 255, 0);  //Subnet mask
	IPAddress dns(192, 168, 0, 1);  //DNS


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	byte mac[6];
	WiFi.softAPmacAddress(mac);
	printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	for (byte i = 0; i < 6; i++)
	{
		//XXXXXXXXData.Mac[i] = mac[i];
	}


	Serial.print("Is connection routing, please wait");
	WiFi.config(staticIP, gateway, subnet, dns);
	WiFi.begin(ssid, password); //Wifi接入到网络
	Serial.println("\nConnecting to WiFi");
	//如果Wifi状态不是WL_CONNECTED，则表示连接失败
	unsigned char WiFiTimeOut = 0;
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(1000);    //延时等待接入网络
		WiFiTimeOut++;
		if (WiFiTimeOut > 10)
		{
			break;
			Serial.println("\nConnecting to WiFi Failed");
			return;
		}
	}
	Serial.println("");

	m_WiFiUDP.begin(5050);
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X AP:%s\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ssid);



	//设置时间格式以及时间服务器的网址
	configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
	//Serial.println("\nWaiting for time");
	////while (!time(nullptr)) {
	////	Serial.print(".");
	////	delay(1000);    
	////}
	//time_t now;
	//while (true)
	//{
	//	now = time(nullptr); //获取当前时间
	//	time_str = ctime(&now);
	//	if (time_str[20] != '2')
	//	{
	//		Serial.print(".");
	//		delay(1000);
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}
	//Serial.println("Got Time");


	//rtcData.GmtTimeDiffToPowerOn = now - rtcData.SecondsSincePowerOn;

	//printf("LastWifiTimeBasePowerOn = %d \r\n", rtcData.LastWifiTimeBasePowerOn);
	//printf("GmtTimeDiffToPowerOn = %d \r\n", rtcData.GmtTimeDiffToPowerOn);


	//首次联网后，与服务器交换一次信息，主要目的是为了获取是否有常开WiFi的要求。
	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&PowerBankData, sizeof(tPowerBankData));
	m_WiFiUDP.endPacket(); 

	delay(1000);

	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tPowerBankCommand))
	{
		printf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		m_WiFiUDP.read((char *)&rtcData.PowerBankCommand,sizeof(tPowerBankCommand));
	}
}


void UpdateOled()
{

	//time_t now = rtcData.SecondsSincePowerOn + rtcData.GmtTimeDiffToPowerOn;
	time_t now = time(nullptr); //获取当前时间
	time_str = ctime(&now);
	H1 = time_str[11];
	H2 = time_str[12];
	M1 = time_str[14];
	M2 = time_str[15];
	S1 = time_str[17];
	S2 = time_str[18];

	//printf("%c%c:%c%c:%c%c\n", H1, H2, M1, M2, S1, S2);

	//Displayer.ShowASCII1632(0,0,H1);
	//Displayer.ShowASCII1632(16,0,H2);
	//Displayer.ShowASCII1632(16*2,0,':');
	//Displayer.ShowASCII1632(16*3,0,M1);
	//Displayer.ShowASCII1632(16*4,0,M2);
	//Displayer.ShowASCII1632(16*5,0, ':');
	//Displayer.ShowASCII1632(16*6,0,S1);
	//Displayer.ShowASCII1632(16*7,0,S2);


	printf("SecondsSincePowerOn = %d; SecondsSinceStart = %d \r\n", rtcData.SecondsSincePowerOn, SecondsSinceStart);


	//Displayer.ShowASCII1632(0,0,rtcData.SecondsSincePowerOn/10000000%10+ 0x30);
	//Displayer.ShowASCII1632(16,0,rtcData.SecondsSincePowerOn/1000000%10+ 0x30);
	//Displayer.ShowASCII1632(16*2,0,rtcData.SecondsSincePowerOn/100000%10+ 0x30);
	//Displayer.ShowASCII1632(16*3,0,rtcData.SecondsSincePowerOn/10000%10+ 0x30);
	//Displayer.ShowASCII1632(16*4,0,rtcData.SecondsSincePowerOn/1000%100+ 0x30);
	//Displayer.ShowASCII1632(16*5,0,rtcData.SecondsSincePowerOn/100%10+ 0x30);
	//Displayer.ShowASCII1632(16*6,0,rtcData.SecondsSincePowerOn/10%10+ 0x30);
	//Displayer.ShowASCII1632(16*7,0,rtcData.SecondsSincePowerOn/1%10+ 0x30);


	Displayer.ShowASCII1632(0,4   ,PowerBankData.volt/6/1000%10+0x30);
	Displayer.ShowASCII1632(16,4  ,PowerBankData.volt/6/100%10 + 0x30);
	Displayer.ShowASCII1632(16*2,4,PowerBankData.volt/6/10%10 + 0x30);
	//Displayer.ShowASCII1632(16*3,4,PowerBankData.volt/3/10%10);

	if(PowerBankData.Current>0)
	{
		Displayer.ShowASCII1632(16 * 3, 4, '+');
	}
	else
	{
		Displayer.ShowASCII1632(16 * 3, 4, '-');
	}

	
	
	Displayer.ShowASCII1632(16*4,4,abs(PowerBankData.Current)/1000%100 + 0x30);
	Displayer.ShowASCII1632(16*5,4, abs(PowerBankData.Current)/100%10 + 0x30);
	Displayer.ShowASCII1632(16*6,4, abs(PowerBankData.Current)/10%10 + 0x30);
	Displayer.ShowASCII1632(16*7,4, abs(PowerBankData.Current)/1%10 + 0x30);
}

void GotoSleep()
{
	// Update CRC32 of data
	rtcData.crc32 = calculateCRC32(((uint8_t*)&rtcData.crc32) + 4, sizeof(rtcData)-4);
	// Write struct to RTC memory
	ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
	ESP.deepSleep(CpuSleepTime * 1e6);
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



void OnTenthSecond()
{
	static long LastTenthSecondsForOnSecond = 0;
	if (TenthSecondsSinceStart - LastTenthSecondsForOnSecond >9)
	{
		OnSecond();
		LastTenthSecondsForOnSecond = TenthSecondsSinceStart;
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

uint32_t calculateCRC32(const uint8_t* data, size_t length)
{
	uint32_t crc = 0xffffffff;
	while (length--) {
		uint8_t c = *data++;
		for (uint32_t i = 0x80; i > 0; i >>= 1) {
			bool bit = crc & 0x80000000;
			if (c & i) {
				bit = !bit;
			}
			crc <<= 1;
			if (bit) {
				crc ^= 0x04c11db7;
			}
		}
	}
	return crc;
}