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


const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;



unsigned long MS_TIMER = 0;
unsigned long timer,lasttime=0;
char *time_str;   
char H1,H2,M1,M2,S1,S2;
unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();


void setup() 
{       




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



	ina219_1.begin();


	swSer.begin(9600);


	Serial.println("\esp iic test");






}

void loop() 
{

	//while (swSer.available() > 0) {
	//	unsigned char recvdata = swSer.read();
	//	GetRfCommand(recvdata);
	//	yield();
	//}


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
		Serial.printf(time_str);


		//模拟量读取
		int Light = 100-(analogRead(A0)/11);
		Serial.printf("Light = %d \n",  Light);

		//IIC读取
		unsigned int Current = ina219_1.getCurrent_mA();
		Serial.printf("Current  =  %d mA \r\n",Current);
		unsigned int testvolt = ina219_1.getBusVoltage_V()*1000;
		Serial.printf("test volt  =  %d mv \r\n",testvolt);

	}



}



