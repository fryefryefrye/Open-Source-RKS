/*
Demo code for Relay Shield 
by Catalex
catalex.taobao.com
*/
#define RELAY1 5
#define RELAY2 16
//#define RELAY3 D5
//#define RELAY4 D4


#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266WiFiMulti.h>
#include <time.h>
#define timezone 8

#include <Wire.h>
#include "Adafruit_INA219.h"
Adafruit_INA219 ina219_1(INA219_ADDRESS);


#include <SoftwareSerial.h>
//#define BAUD_RATE 115200
SoftwareSerial swSer(4, 4, false, 256);
//0==nodeD3 
//4==nodeD2

//D0 = GPIO16;
//D1 = GPIO5;
//D2 = GPIO4;	无线接收
//D3 = GPIO0;	连接时无法下载
//D4 = GPIO2;	DHT
//D5 = GPIO14;  IIC
//D6 = GPIO12;
//D7 = GPIO13;
//D8 = GPIO15;  IIC
//D9 = GPIO3;
//D10 = GPIO1;
//LED_BUILTIN = GPIO16 (auxiliary constant for the board LED, not a board pin);


#include "DHT.h"
#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);


//#include "Adafruit_NeoPixel.h"
//#define PIN 6
//#define MAX_LED 20
//uint32_t color_array[MAX_LED];
//Adafruit_NeoPixel strip = Adafruit_NeoPixel( MAX_LED, PIN, NEO_RGB + NEO_KHZ800 );


const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;



unsigned long MS_TIMER = 0;
unsigned long timer,lasttime=0;
char *time_str;   
char H1,H2,M1,M2,S1,S2;

#define RF_COMMAND_DELAY 4
#define RF_COMMAND_LEN 3

#define RF_COMMAND_COUNTER 5
#define RF_COMMAND_LIGHT 2
unsigned char RelayPin[RF_COMMAND_LIGHT] = {RELAY1,RELAY2};
unsigned char PreSetRfCommand[RF_COMMAND_LIGHT][RF_COMMAND_COUNTER][RF_COMMAND_LEN] 
		={
			{{0xC3, 0x7C, 0x68},{0xC3, 0x7C, 0x64},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
			,{{0xC3, 0x7C, 0x61},{0xC3, 0x7C, 0x62},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
		};
unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void GetRfCommand(unsigned char RfData);
void CheckRfCommand(unsigned char * RfCommand);


#include "Z:\bt\web\datastruct.h"
tRoomData RoomData;
//unsigned char Humidity;
//unsigned char Temperature;


//#include <LCD4Bit_mod.h>
//LCD4Bit_mod lcd = LCD4Bit_mod(2);

void setup() 
{       
	digitalWrite(RELAY1, HIGH);
	digitalWrite(RELAY2, HIGH);

	pinMode(RELAY1, OUTPUT);//set the pin to be OUTPUT pin.
	pinMode(RELAY2, OUTPUT);//
	//pinMode(RELAY3, OUTPUT);//
	//pinMode(RELAY4, OUTPUT);//
	digitalWrite(RELAY1, HIGH);
	digitalWrite(RELAY2, HIGH);



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
	printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	for (byte i=0;i<6;i++)
	{
		RoomData.Mac[i] = mac[i];
	}

	m_WiFiUDP.begin(5050); 
	



	//ina219_1.begin();



	swSer.begin(9600);
	//Serial.println("\nSoftware serial test started");
	//for (char ch = ' '; ch <= 'z'; ch++) {
	//	swSer.write(ch);
	//}
	//swSer.println("");


	dht.begin();




	// 初始化库
	//strip.begin();
	//// 发送数据，默认每个点的颜色为0，所以初始的每个点都是不亮的
	//strip.show();




	//Serial.begin(BAUD_RATE);
	//swSer.begin(BAUD_RATE);

	//Serial.println("\nSoftware serial test started");

	//for (char ch = ' '; ch <= 'z'; ch++) {
	//	swSer.write(ch);
	//}
	//swSer.println("");


	RoomData.DataType = 2;




}

void loop() 
{

	while (swSer.available() > 0) {
		unsigned char recvdata = swSer.read();
		GetRfCommand(recvdata);
		yield();
	}

	TenthSecondsSinceStartTask();


	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tRoomCommand))
	{
		//printf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		tRoomCommand tempRoomCommand;
		m_WiFiUDP.read((char *)&tempRoomCommand,sizeof(tRoomCommand));

		if (tempRoomCommand.ChangeNotice == true)
		{
			for (byte i=0;i<RF_COMMAND_LIGHT;i++)
			{
				RoomData.Light[i] = tempRoomCommand.Light[i];
				printf("get from control %d %d\r\n",i,RoomData.Light[i]);
				digitalWrite(RelayPin[i], !RoomData.Light[i]);
			}
		}




	}

	//UDP.read().

	//	//模拟量读取
	//	int Light = 100-(analogRead(A0)/11);
	//	Serial.printf("Light = %d \n",  Light);

	//	//IIC读取
	//	unsigned int Current = ina219_1.getCurrent_mA();
	//	Serial.printf("Current  =  %d mA \r\n",Current);
	//	unsigned int testvolt = ina219_1.getBusVoltage_V()*1000;
	//	Serial.printf("test volt  =  %d mv \r\n",testvolt);
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
		//printf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
	}
}

void OnTenthSecond()
{

	if (TenthSecondsSinceStart%10==0)
	{
		time_t now = time(nullptr); //获取当前时间
		time_str = ctime(&now);
		H1 = time_str[11];
		H2 = time_str[12];
		M1 = time_str[14];
		M2 = time_str[15];
		S1 = time_str[17];
		S2 = time_str[18];
		//Serial.printf("%c%c:%c%c:%c%c\n",H1,H2,M1,M2,S1,S2);
		//Serial.printf(time_str);


		//UDP.parsePacket(); 
		//WiFiUDP.available()
		//UDP.read().

		m_WiFiUDP.beginPacket("192.168.0.17", 5050);
		m_WiFiUDP.write((const char*)&RoomData, sizeof(tRoomData));
		m_WiFiUDP.endPacket(); 
	}


	if (TenthSecondsSinceStart%200==0)
	{

		// Reading temperature or humidity takes about 250 milliseconds!
		// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
		float h = dht.readHumidity();
		RoomData.Humidity = h;
		// Read temperature as Celsius (the default)
		float t = dht.readTemperature();
		RoomData.Temperature = t;

		// Check if any reads failed.
		if (!isnan(h) && !isnan(t)) 
		{
			printf("Humidity: %d %%	Temperature: %d *C \r\n",RoomData.Humidity,RoomData.Temperature);
		}
		else
		{
			Serial.println("Read Temperature failed");
		}
	}
}

void GetRfCommand(unsigned char RfData)
{
	static unsigned char Index = 0;
	static unsigned char GetCommand[4];
	static bool start = false;

	if ((start == false)&&(RfData == 0xFD))
	{
		start = true;
		Index = 0;
		return;
	}

	if (start == true)
	{
		GetCommand[Index] = RfData;
		Index++;
		if (Index >=3)
		{
			CheckRfCommand(GetCommand);
			Index = 0;
			start = false;
		}
	}


	/*  0xFD 0xC3 0x7C 0x68 0x55 0xDF */

}

void CheckRfCommand(unsigned char * RfCommand)
{
	static unsigned long LastTrigerTimer;
	printf("CheckRfCommand 0x%02X 0x%02X 0x%02X\r\n",RfCommand[0],RfCommand[1],RfCommand[2]);
	for (byte j=0;j<RF_COMMAND_LIGHT;j++)
	{
		for (byte i=0;i<RF_COMMAND_COUNTER;i++)
		{
			//printf("Check with 0x%02X 0x%02X 0x%02X\r\n",PreSetRfCommand[i][0],PreSetRfCommand[i][1],PreSetRfCommand[i][2]);
			if(memcmp(RfCommand,PreSetRfCommand[j][i],RF_COMMAND_LEN)==0)
			{
				printf("Found Command %d \r\n",j);
				if (TenthSecondsSinceStart - LastTrigerTimer > RF_COMMAND_DELAY)
				{
					LastTrigerTimer = TenthSecondsSinceStart;
					RoomData.Light[j] = !RoomData.Light[j] ;
					digitalWrite(RelayPin[j], !RoomData.Light[j]);
				}
			}
		}
	}




}