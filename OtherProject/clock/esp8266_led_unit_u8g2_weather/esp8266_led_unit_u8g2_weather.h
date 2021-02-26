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
#include<ESP8266WiFiGeneric.h>
#include<time.h>

#include <U8g2lib.h>
u8g2_t u8g2;

//Include the client
//#include <WiFiClientSecure.h>
#include <WiFiClient.h>

//Add a client
WiFiClient client;

#define timezone 8

unsigned long Cycle = 1000;//刷新间隔，微秒
unsigned long DisplayLight = 50;//亮度

//一定要修改这里，使用自己的wifi信息
#include "D:\GitHub\Private\wifi_config.h"
//const char* ssid = "wang";  //Wifi名称
//const char* password = "12345678";  //Wifi密码

//我自己用的key保存在配置文件里了，不公开了。
//要改用下面那一行URL，并把xxxxxxxxxxxxxxxxxxx改成你在“心知天气”申请到的key。
#include "D:\GitHub\Private\weather_config.h"
//String URL = "/v3/weather/daily.json?key=xxxxxxxxxxxxxxxxxxx&location=nanjing&language=zh-Hans&unit=c&start=0&days=3";


char *time_str;   
char H1,H2,M1,M2,S1,S2;

unsigned long SecondsSinceStart = 0;
unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnSecond();
void OnHour();
void NonStopTask();

unsigned long TimeOut = 30;
bool ssl_getValue(const char* string, unsigned char* Value, unsigned char* Len, unsigned char Start, unsigned char Max);
bool ssl_waitFor(const char* string);
bool ssl_getch(char* RetData);
char Weather[3][7];//"中雨"
char Date[3][6];//"02-28"
char HighTemperature[3][4];//-15
char LowTemperature[3][4];


#define RowA D8    		//行信号,驱动138  
#define RowB D7
#define RowC D6
#define RowD D5
#define STB D2         //595 刷新显示  SS  
#define CLK D1         //时钟    SCK  
#define OE D0 			//  使能  
#define R1 D4          //上半屏列信号输出    
#define R2 D3          //下半屏列信号输出    

void setup() 
{       
	delay(50);                      
	Serial.begin(115200);

	pinMode(RowA, OUTPUT);
	pinMode(RowB, OUTPUT);
	pinMode(RowC, OUTPUT);
	pinMode(RowD, OUTPUT); //138片选
	pinMode(OE, OUTPUT); //138 使能
	pinMode(R1, OUTPUT);//595 数据
	pinMode(R2, OUTPUT);//595 数据
	pinMode(CLK, OUTPUT); //595 时钟
	pinMode(STB, OUTPUT); //595 使能

	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	byte mac[6];
	WiFi.softAPmacAddress(mac);
	printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

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


	//开启OTA功能。除了第一次需要用USB下载，以后就可以使用WiFi下载程序了。
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

	u8g2_Setup_ssd1306_64x32_noname_f(&u8g2, U8G2_R0, NULL, NULL);  // init u8g2 structure

}

void hc138sacn(byte r)   //输出行线状态ABCD （A低,D高)
{
	digitalWrite(RowA,(r & 0x01));
	digitalWrite(RowB,(r & 0x02));
	digitalWrite(RowC,(r & 0x04));
	digitalWrite(RowD,(r & 0x08));
}

void DisplayOneLine()
{
	//显示缓存扫描
	static unsigned char row=0;
	for (int i=0; i<64; i++)
	{
		digitalWrite(CLK,0);
		digitalWrite(R1, ((u8g2.tile_buf_ptr[i%64+(row/8)*64]>>(row%8))&1));//发送上半屏
		digitalWrite(R2, ((u8g2.tile_buf_ptr[i%64+(row/8)*64+128]>>(row%8))&1));//发送下半屏
		digitalWrite(CLK,1);
	}

	hc138sacn(row);            //选行
	digitalWrite(STB, 1);      //数据确认
	digitalWrite(STB, 0);
	if (DisplayLight>0)
	{
		digitalWrite(OE, 0);  //开启显示
	}
	delayMicroseconds(DisplayLight) ;  //亮度调节
	digitalWrite(OE, 1);  //关闭显示
	row++;
	if (row>15)
	{
		row = 0;
		NonStopTask();
	}
}

unsigned long CurrentDisplayTime;
unsigned long LastDisplayTime;
void DisplayTimerTask()
{
	CurrentDisplayTime = micros();
	if (abs(CurrentDisplayTime - LastDisplayTime) > Cycle)
	{
		LastDisplayTime = CurrentDisplayTime;
		DisplayOneLine();
	}
}

void loop() 
{
	DisplayTimerTask();
}

void NonStopTask() 
{
	TenthSecondsSinceStartTask();
	ArduinoOTA.handle();
}

void OnTenthSecond()
{
	if (TenthSecondsSinceStart % 10 == 0)
	{
		OnSecond();
		SecondsSinceStart++;

		if (SecondsSinceStart % 3600 == 5)//每小时运行，并且保证启动5秒后也运行一次
		{
			OnHour();
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
	}
}


char* WeekStr[7] = { "Sunday"
							,"Monday"
							,"Tuesday"
							,"Wednesday"
							,"Thursday"
							,"Friday"
							,"Saturday" };




bool ssl_waitFor(const char* string) {
	char so_far = 0;
	char received;
	int counter = 0;
	do {
		if (!ssl_getch(&received))
		{
			return false;
		}
		counter++;
		if (received == string[so_far]) {
			so_far++;
		}
		else {
			so_far = 0;
		}
	} while (string[so_far] != 0);
	return true;
}

bool ssl_getValue(const char* string, unsigned char* Value, unsigned char* Len, unsigned char Start, unsigned char Max)
{
	char received;
	*Len = 0;
	unsigned char CurrentChar = 0;

	if (!ssl_waitFor(string)) return false;

	unsigned long RecvStartTime = SecondsSinceStart;
	while (1)
	{
		if (!ssl_getch(&received))
		{
			return false;
		}

		if ((received == '"') || ((*Len) >= Max))
		{
			return true;
		}
		else
		{
			if (CurrentChar >= Start)
			{
				*(Value + (*Len)) = received;
				(*Len) = (*Len) + 1;
			}
			else
			{
				CurrentChar++;
			}
		}

		if (SecondsSinceStart - RecvStartTime > TimeOut)
		{
			return false;
		}
	}
}

bool ssl_getch(char* RetData)
{
	//static bool WaitFinsih = false;
	//static unsigned char CompareIndex = 0;
	//static bool NewLine = false;

	unsigned long RecvStartTime = SecondsSinceStart;
	while (1)
	{
		if (client.available() > 0)
		{
			*RetData = client.read();
			return true;
		}
		else
		{
			//NonStopTask();
			DisplayTimerTask();
		}

		if (SecondsSinceStart - RecvStartTime > TimeOut)
		{
			return false;
		}
	}
}

//收到的天气数据，举例：
//{"results":[{"location":{"id":"WTSQQYHVQ973","name":"南京","country":"CN","path":"南京,南京,江苏,中国","timezone":"Asia/Shanghai",
//"timezone_offset":"+08:00"},"daily":[
//{"date":"2021-02-26","text_day":"小雨","code_day":"13","text_night":"小雨","code_night":"13","high":"7","low":"5","rainfall":"6.1","precip":"","wind_direction":"东北","wind_direction_degree":"45","wind_speed":"23.4","wind_scale":"4","humidity":"91"},
//{"date":"2021-02-27","text_day":"阴","code_day":"9","text_night":"阴","code_night":"9","high":"14","low":"6","rainfall":"0.0","precip":"","wind_direction":"东","wind_direction_degree":"90","wind_speed":"32.8","wind_scale":"5","humidity":"71"},
//{"date":"2021-02-28","text_day":"阴","code_day":"9","text_night":"小雨","code_night":"13","high":"17","low":"11","rainfall":"0.0","precip":"","wind_direction":"东南","wind_direction_degree":"135","wind_speed":"32.8","wind_scale":"5","humidity":"77"}],
//"last_update":"2021-02-26T11:00:00+08:00"}]}
void OnHour()
{



	long now;
	char host[] = "api.seniverse.com";

	printf("Conneting weather server\r\n");
	if (client.connect(host, 80)) {
		Serial.println("connected");

		client.println("GET " + URL + " HTTP/1.1");
		client.print("Host: "); client.println(host);
		client.println("User-Agent: arduino/1.0");
		client.println("");

		TimeOut = 10;
		if(!ssl_waitFor("200 OK")) return;
		TimeOut = 2;

		
		printf("Got weather info OK\r\n");

		//print everything recived
		//now = millis();
		//while (millis() - now < 1500)
		//{
		//	while (client.available())
		//	{
		//		char c = client.read();
		//		printf("%c", c);
		//	}
		//}


		unsigned char valueLen;

		for (int i = 0; i < 3; i++)//循环取3天的天气信息
		{
			//先找到 date":"， 往后跳5个字符，然后最多取5个字符，得到日期。
			ssl_getValue("date\":\"", (unsigned char*)Date[i], &valueLen, 5, 5);
			Date[i][valueLen] = 0;
			printf("date:%s\r\n", Date[i]);

			//先找到 t_day":"， 往后跳0个字符，然后最多取6个字符，得到白天天气。
			ssl_getValue("t_day\":\"", (unsigned char*)Weather[i], &valueLen, 0, 6);
			Weather[i][valueLen] = 0;
			printf("Weather:%s\r\n", Weather[i]);

			//先找到 high":"， 往后跳0个字符，然后最多取3个字符，得到温度。
			ssl_getValue("high\":\"", (unsigned char*)HighTemperature[i], &valueLen, 0, 3);
			HighTemperature[i][valueLen] = 0;
			printf("High:%s\r\n", HighTemperature[i]);

			//先找到 low":"， 往后跳0个字符，然后最多取3个字符，得到温度。
			ssl_getValue("low\":\"", (unsigned char*)LowTemperature[i], &valueLen, 0, 3);
			LowTemperature[i][valueLen] = 0;
			printf("Low:%s\r\n", LowTemperature[i]);
		}

	}
	else
	{
		Serial.println("connecte failed");
	}
}

void OnSecond()
{
	static char sprint_buf[20];

	time_t now = time(nullptr); //获取当前时间


	//转换成年月日的数字，可以更加自由的显示。
	struct   tm* timenow;
	timenow = localtime(&now);
	unsigned char tempHour = timenow->tm_hour;
	unsigned char tempMinute = timenow->tm_min;
	unsigned char tempSecond = timenow->tm_sec;
	unsigned char tempDay = timenow->tm_mday;
	unsigned char tempMonth = timenow->tm_mon + 1;
	unsigned int tempYear = timenow->tm_year + 1900;
	unsigned char tempWeek = timenow->tm_wday;


	u8g2_ClearBuffer(&u8g2);//清空显存
	//u8g2_SetFont(&u8g2, u8g2_font_crox3t_tn);
	////第一行大字，显示时间
	//sprintf(sprint_buf, "%02d:%02d:%02d"
	//	, tempHour
	//	, tempMinute
	//	, tempSecond
	//);
	//u8g2_DrawStr(&u8g2, 3, 12, sprint_buf);

	////第二行小字，显示日期
	//u8g2_SetFont(&u8g2, u8g2_font_IPAandRUSLCD_tf);//字体5*7
	//sprintf(sprint_buf, "%04d-%02d-%02d"
	//	, tempYear
	//	, tempMonth
	//	, tempDay
	//);
	//u8g2_DrawStr(&u8g2, 0, 22, sprint_buf);

	////第三行小字，显示星期
	//sprintf(sprint_buf, "%s"
	//	, WeekStr[tempWeek]
	//);
	//u8g2_DrawStr(&u8g2, 0, 31, sprint_buf);

	u8g2_SetFont(&u8g2, u8g2_font_IPAandRUSLCD_tf);//字体5*7

	u8g2_DrawStr(&u8g2, 0, 8, Date[0]);
	u8g2_DrawStr(&u8g2, 0, 16, LowTemperature[0]);
	u8g2_DrawStr(&u8g2, 18, 16, HighTemperature[0]);

	u8g2_DrawStr(&u8g2, 32, 8, Date[1]);
	u8g2_DrawStr(&u8g2, 32, 16, LowTemperature[1]);
	u8g2_DrawStr(&u8g2, 18+32, 16, HighTemperature[1]);

	//显示中文
	u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_gb2312);
	u8g2_DrawUTF8(&u8g2, 0, 30, Weather[0]);
	u8g2_DrawUTF8(&u8g2, 32, 30, Weather[1]);
}