




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

#define FULL_AH 8

#define IIC_DAT				D2
#define IIC_CLK				D1

const char* ssid = "frye_iot3";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;

char *time_str;   

#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 20;
//tPowerBankData PowerBankData;

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

unsigned int RemainVolt[100]=
{
	4046,
	4024,
	4012,
	4002,
	3992,
	3986,
	3979,
	3975,
	3973,
	3968,
	3966,
	3963,
	3958,
	3956,
	3954,
	3954,
	3950,
	3949,
	3946,
	3944,
	3942,
	3940,
	3938,
	3936,
	3934,
	3932,
	3929,
	3927,
	3924,
	3920,
	3916,
	3914,
	3911,
	3908,
	3905,
	3900,
	3899,
	3894,
	3890,
	3888,
	3884,
	3881,
	3878,
	3875,
	3871,
	3866,
	3863,
	3862,
	3857,
	3853,
	3850,
	3846,
	3844,
	3840,
	3834,
	3833,
	3830,
	3826,
	3824,
	3818,
	3816,
	3814,
	3810,
	3804,
	3804,
	3802,
	3798,
	3795,
	3791,
	3790,
	3783,
	3781,
	3774,
	3775,
	3766,
	3766,
	3760,
	3757,
	3750,
	3747,
	3740,
	3737,
	3727,
	3726,
	3716,
	3714,
	3706,
	3701,
	3694,
	3688,
	3679,
	3670,
	3662,
	3651,
	3638,
	3621,
	3602,
	3578,
	3543,
	3478
};

struct {
	uint32_t crc32;
	unsigned long SecondsSincePowerOn;
	unsigned long LastWifiTimeBasePowerOn;
	//unsigned long GmtTimeDiffToPowerOn;
	tPowerBankCommand PowerBankCommand;
	tPowerBankData PowerBankData;
} rtcData;

unsigned long SecondsSinceStart = 0;
unsigned long TenthSecondsSinceStart = 0;
unsigned long CpuSleepTime = 10;
unsigned long WiFiSleepTime = 600;

unsigned char CurrentLine[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned char UpdateLine[16] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };

//bool Charging = false;

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

unsigned char GetRemain(unsigned int volt);
unsigned char LastRemain = 100;


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
			rtcData.PowerBankCommand.WiFiAlwaysOn = false;
			rtcData.PowerBankData.ThisCharge = 0;
			rtcData.PowerBankData.LastCharge = 0;
			rtcData.PowerBankData.ThisDisCharge = 0;
			rtcData.PowerBankData.LastDisCharge = 0;
			rtcData.PowerBankData.Charging = false;
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
	if (abs(rtcData.PowerBankData.Current) > CURRENT_UP_LIMIT)
	{
		UpdateOled();
	}
		
	//根据各项条件判断是否需要打开WiFi，否则休眠。
	if ((rtcData.SecondsSincePowerOn == 0)
		||(rtcData.SecondsSincePowerOn - rtcData.LastWifiTimeBasePowerOn > WiFiSleepTime)
		|| (abs(rtcData.PowerBankData.Current) > CURRENT_UP_LIMIT)
		)
	{
		StartWifi();
		Displayer.Initial(); // 首次通电肯定启动WiFi
		delay(10);
		Displayer.Fill_Screen(0x00);//首次通电清屏 否则首次通电花屏。
		memset(CurrentLine,0,16);
	}
	else 
	{
		printf("GotoSleep for no need wifi and no Current \r\n");
		GotoSleep();
	}

	if ((abs(rtcData.PowerBankData.Current) > CURRENT_UP_LIMIT)
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
		m_WiFiUDP.beginPacket("fryefryefrye.myds.me", 5050);
		m_WiFiUDP.write((const char*)&rtcData.PowerBankData, sizeof(tPowerBankData));
		m_WiFiUDP.endPacket();
	}

	if ((WiFi.status() != WL_CONNECTED)
		&& (rtcData.SecondsSincePowerOn - rtcData.LastWifiTimeBasePowerOn > WiFiSleepTime))
	{
		printf("Start wifi when running offline ,but need wifi time is up \r\n");
		StartWifi();
	}

	if (abs(rtcData.PowerBankData.Current) < CURRENT_LOW_LIMIT)
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

	//充电放电状态切换
	if((rtcData.PowerBankData.Current>200)&&(rtcData.PowerBankData.Charging == false))
	{
		rtcData.PowerBankData.Charging = true;
		rtcData.PowerBankData.LastCharge = rtcData.PowerBankData.ThisCharge;
		rtcData.PowerBankData.ThisCharge = 0;
		MyPrintf("PowerBank Start Charging!\r\n");
	}
	//一旦停止充电，就认为充电完成，重设电量计数器
	else if((rtcData.PowerBankData.Current<=0)&&(rtcData.PowerBankData.Charging == true))
	{
		rtcData.PowerBankData.Charging = false;
		rtcData.PowerBankData.LastDisCharge = rtcData.PowerBankData.ThisDisCharge;
		rtcData.PowerBankData.ThisDisCharge = 0;
		MyPrintf("PowerBank Start Discharging!\r\n");
	}

	//电量本地统计
	if (rtcData.PowerBankData.Charging)
	{
		rtcData.PowerBankData.ThisCharge = rtcData.PowerBankData.ThisCharge + rtcData.PowerBankData.Current;
	} 
	else
	{
		rtcData.PowerBankData.ThisDisCharge = rtcData.PowerBankData.ThisDisCharge + (rtcData.PowerBankData.Current*(-1));
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
		rtcData.PowerBankData.volt = BusVoltage * 1000;
	}

	if (abs(Current)<(float)5000)
	{
		rtcData.PowerBankData.Current = Current*(-1);
	}


	//unsigned char ThisRemain = GetRemain(rtcData.PowerBankData.volt/6);
	//if (ThisRemain < LastRemain)
	//{
	//	MyPrintf("Remain from:%d to:%d \r\n"
	//		,LastRemain
	//		,ThisRemain
	//		);
	//	rtcData.PowerBankData.Percentage = ThisRemain;
	//	LastRemain = ThisRemain;
	//}

	rtcData.PowerBankData.Percentage = GetRemain(rtcData.PowerBankData.volt/6);


	//MyPrintf("Current:%d BusVoltage:%d Percentage:%d \r\n"
	//	,rtcData.PowerBankData.Current
	//	,rtcData.PowerBankData.volt
	//	,rtcData.PowerBankData.Percentage
	//	);
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

	rtcData.PowerBankData.DataType = 24;
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

	//首次联网后，与服务器交换一次信息，主要目的是为了获取是否有常开WiFi的要求。
	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&rtcData.PowerBankData, sizeof(tPowerBankData));
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

	//time_t now = time(nullptr); //获取当前时间
	//time_str = ctime(&now);
	////printf("%s\n", time_str);
	//UpdateLine[0] = time_str[11];
	//UpdateLine[1] = time_str[12];
	//UpdateLine[2] = ':';
	//UpdateLine[3] = time_str[14];
	//UpdateLine[4] = time_str[15];
	//UpdateLine[5] = ':';
	//UpdateLine[6] = time_str[17];
	//UpdateLine[7] = time_str[18];

	char buf[8];
	sprintf(buf,"%.2f%.2f",(float)rtcData.PowerBankData.ThisCharge / 3600000,(float)rtcData.PowerBankData.ThisDisCharge / 3600000);


	UpdateLine[0] = buf[0];
	UpdateLine[1] = '.';
	UpdateLine[2] = buf[2];
	UpdateLine[3] = buf[3];
	UpdateLine[4] = buf[4];
	UpdateLine[5] = '.';
	UpdateLine[6] = buf[6];
	UpdateLine[7] = buf[7];

	UpdateLine[0 + 8] = rtcData.PowerBankData.volt / 6 / 1000 % 10 + 0x30;
	UpdateLine[1 + 8] = rtcData.PowerBankData.volt / 6 / 100 % 10 + 0x30;
	UpdateLine[2 + 8] = rtcData.PowerBankData.volt / 6 / 10 % 10 + 0x30;

	if (rtcData.PowerBankData.Current < 0)
	{
		UpdateLine[3 + 8] = '-';
	}
	else
	{
		UpdateLine[3 + 8] = '+';
	}
	UpdateLine[4 + 8] = abs(rtcData.PowerBankData.Current) / 1000 % 10 + 0x30;
	UpdateLine[5 + 8] = abs(rtcData.PowerBankData.Current) / 100 % 10 + 0x30;
	UpdateLine[6 + 8] = abs(rtcData.PowerBankData.Current) / 10 % 10 + 0x30;
	UpdateLine[7 + 8] = abs(rtcData.PowerBankData.Current) / 1 % 10 + 0x30;

	//unsigned long MillisBeforeOled = millis();
	for (byte i = 0; i < 16; i++)
	{
		if (CurrentLine[i] != UpdateLine[i])
		{
			Displayer.ShowASCII1632(16 * (i%8), 4 * (i/8), UpdateLine[i]);
			CurrentLine[i] = UpdateLine[i];
		}
	}
	//MyPrintf("Update:%dms PowerOn:%ds; Start:%ds \r\n", millis() - MillisBeforeOled,rtcData.SecondsSincePowerOn, SecondsSinceStart);	
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

unsigned char GetRemain(unsigned int volt)
{
	//MyPrintf("GetRemain input Voltage:%d \r\n"
	//	,volt
	//	);
	//for (byte i = 0; i < 100; i++)
	//{
	//	if (volt > RemainVolt[i])
	//	{
	//		return 100 -i;
	//	}
	//}
	//return 0;

	if (rtcData.PowerBankData.Charging)
	{
		if (rtcData.PowerBankData.ThisDisCharge > rtcData.PowerBankData.ThisCharge)
		{
			return 100 - (rtcData.PowerBankData.ThisDisCharge - rtcData.PowerBankData.ThisCharge) / (36000*FULL_AH);//10Ah
		} 
		else
		{
			return 100;
		}
	}
	else
	{
		return 100 - rtcData.PowerBankData.ThisDisCharge / (36000*FULL_AH);//10Ah
	}


	
}