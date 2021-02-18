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

#define timezone 8

unsigned long Cycle = 1000;//刷新间隔，微秒
unsigned long DisplayLight = 50;//亮度
const char* ssid = "wang";  //Wifi名称
const char* password = "12345678";  //Wifi密码

char *time_str;   
char H1,H2,M1,M2,S1,S2;

unsigned long SecondsSinceStart = 0;
unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnSecond();
void NonStopTask();

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
	u8g2_SetFont(&u8g2, u8g2_font_crox3t_tn);
	//第一行大字，显示时间
	sprintf(sprint_buf, "%02d:%02d:%02d"
		, tempHour
		, tempMinute
		, tempSecond
	);
	u8g2_DrawStr(&u8g2, 3, 12, sprint_buf);

	//第二行小字，显示日期
	u8g2_SetFont(&u8g2, u8g2_font_IPAandRUSLCD_tf);//字体5*7
	sprintf(sprint_buf, "%04d-%02d-%02d"
		, tempYear
		, tempMonth
		, tempDay
	);
	u8g2_DrawStr(&u8g2, 0, 22, sprint_buf);

	//第三行小字，显示星期
	sprintf(sprint_buf, "%s"
		, WeekStr[tempWeek]
	);
	u8g2_DrawStr(&u8g2, 0, 31, sprint_buf);

	//显示中文的方法  但是这个屏幕最多只能显示八个汉字。本身这个源文件，必须是UTF-8格式
	//u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_gb2312);
	//u8g2_DrawUTF8(&u8g2, 0, 14, "好好学习");
	//u8g2_DrawUTF8(&u8g2, 0, 30, "天天向上");
}