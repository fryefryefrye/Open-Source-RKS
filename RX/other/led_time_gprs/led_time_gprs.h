#include <printf.h>

#include "TimeLib.h"

bool GpsUpdate(unsigned char k);
void OnSeconds();

unsigned char CurrentOn = 0; //1 GPS   2 GPRS
void ChangeToGprs();
void ChangeToGps();

bool _esp8266_waitFor(const char *string);
bool _esp8266_getch(char * RetData);
void NonStopTask();
bool InitGprs();
bool SendGprsUpdate();
unsigned long cal_crc(unsigned char *ptr, unsigned char len);
unsigned long RestartGprsSeconds = 0;
bool RestartGprsTask();



bool GprsOK = false;
bool NeedSendUpdate = false;
bool SendingUpdate = false;

unsigned char SendFailCounter = 0;
unsigned char InitGprsFailCounter = 0;

#include "Z:\bt\web\datastruct.h"
tLedTimeData LedTimeData;
tLedTimeCommand LedTimeCommand;

struct tGpsData
{
	unsigned long iTOW;
	long fTOW;
	int week;
	unsigned char fix;
};

tGpsData GpsData;
unsigned char * pGpsData;
bool NeedTimeUpdate = false;

//Serial data
int comdata ;

//GPS
bool is_GPS_data = false;
#define GPS_HEAD_LEN 6
unsigned char GPS_data_head_array[GPS_HEAD_LEN] =  {0xB5, 0x62, 0x01, 0x06, 0x34 ,0x00};
unsigned char GPS_data_Index = 0;

time_t t;
tmElements_t tm;
bool bGPS_Valid;
unsigned long SecondsSinceStart = 0;
unsigned long TenthSecondsSinceStart = 0;
unsigned long LastMillis;
unsigned long CurrentMillis;

#include <SoftwareSerial.h>



SoftwareSerial GprsSerial(10, 11); //rx 7,tx 8
SoftwareSerial GpsSerial(2, 3); //rx 2,tx 3
unsigned long WaitForTimeOut = 300; //0.1s


#include "Adafruit_NeoPixel.h"
#define PIN 4
#define MAX_LED 23
#define RELAY  5
//#define LED1  5
//#define LED2  6
unsigned char LED_PWM;
uint32_t color;
Adafruit_NeoPixel strip = Adafruit_NeoPixel( MAX_LED, PIN, NEO_RGB + NEO_KHZ800 );
unsigned long GetRGB(unsigned long ColorIndex,unsigned long Brightness);

//0-9数字所对应数组
unsigned char DIGITAL_DISPLAY[10][7] = { 
	{ 1,1,1,1,1,1,0 }, // = 0
	{ 1,1,0,0,0,0,0 }, // = 1
	{ 1,0,1,1,0,1,1 }, // = 2
	{ 1,1,1,0,0,1,1 }, // = 3
	{ 1,1,0,0,1,0,1 }, // = 4
	{ 0,1,1,0,1,1,1 }, // = 5
	{ 0,1,1,1,1,1,1 }, // = 6
	{ 1,1,0,0,0,1,0 }, // = 7
	{ 1,1,1,1,1,1,1 }, // = 8
	{ 1,1,1,0,1,1,1 }, // = 9
};

void setup()
{


	Serial.begin(115200);
	Serial.println(F("GPRS_LED_Time"));
	printf_begin();

	GpsSerial.begin(38400);
	GpsSerial.stopListening();
	GprsSerial.begin(9600);
	GprsSerial.stopListening();

	//pinMode(LED1,OUTPUT);
	//pinMode(LED2,OUTPUT);

	pinMode(RELAY,OUTPUT);
	digitalWrite(RELAY,LOW);
	delay(5000);
	digitalWrite(RELAY,HIGH);
	



	// 初始化库
	strip.begin();
	// 发送数据，默认每个点的颜色为0，所以初始的每个点都是不亮的
	strip.show();

	pGpsData = (unsigned char *)&GpsData;

	ChangeToGprs();

	LedTimeData.Brightness = 10;
	LedTimeData.ColorIndex = 7;
	color = GetRGB(LedTimeData.ColorIndex,LedTimeData.Brightness);

	//color = strip.Color(10, 10, 10);
}

void loop()
{
	if (NeedTimeUpdate)
	{
		ChangeToGps();
		if (GpsSerial.available() > 0)
		{
			comdata = GpsSerial.read();
			if (GpsUpdate(comdata))
			{
				t = (unsigned long)(GpsData.week+522)*7*24*3600+GpsData.iTOW/1000;
				t = (unsigned long)t +8*3600 - 18;
				t = (unsigned long)t +64800;
				t = (unsigned long)t +64800;
				t = (unsigned long)t +64800;
				t = (unsigned long)t +64800;
				printf("Date Time = %d-%02d-%02d %02d:%02d:%02d fix = %x \r\n",year(t) ,month(t),day(t),hour(t),minute(t),second(t),GpsData.fix);
				NeedTimeUpdate = false;
				ChangeToGprs();
			}
			//Serial.write(comdata);
		}
	} 
	else
	{
		if ((!GprsOK)&&(RestartGprsSeconds == 0))
		{
			printf("start InitGprs\r\n");
			if (InitGprs())
			{
				printf("InitGprs ok\r\n");
				InitGprsFailCounter = 0;
				GprsOK = true;
			}
			else
			{
				InitGprsFailCounter++;
				printf("Init Gprs failed = %d\r\n",InitGprsFailCounter);
				if (InitGprsFailCounter > 3)
				{
					InitGprsFailCounter = 0;
					printf("Re start Gprs  \r\n");
					RestartGprsSeconds = SecondsSinceStart;
					GprsOK = false;
				}
			}
		}

		if ((GprsOK)&&(NeedSendUpdate))
		{
			SendingUpdate = true;
			NeedSendUpdate =false;
			if (SendGprsUpdate())
			{
				//printf("\r\n Send Gprs Update ok\r\n");
				SendFailCounter = 0;
			} 
			else
			{
				SendFailCounter ++;
				printf("Send Gprs Update failed = %d\r\n",SendFailCounter);

				if (SendFailCounter > 20)
				{
					SendFailCounter = 0;
					printf("Re InitGprs \r\n");
					GprsOK = false;
				}
			}
			SendingUpdate = false;
		}
	}
	if (RestartGprsSeconds !=0)
	{
		RestartGprsTask();
	}

	NonStopTask();
}

void NonStopTask()
{
	CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 100)
	{
		LastMillis = CurrentMillis;
		TenthSecondsSinceStart++;
		if (TenthSecondsSinceStart%10==0)
		{
			SecondsSinceStart++;
			OnSeconds();
		}
		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
		//printf("LedTimeCommand.Brightness = %d \r\n",LedTimeCommand.Brightness);
	}
}

bool RestartGprsTask()
{
	if (SecondsSinceStart-RestartGprsSeconds<2)
	{
		digitalWrite(RELAY,LOW);
	}

	if (SecondsSinceStart-RestartGprsSeconds>10)
	{
		digitalWrite(RELAY,HIGH);
		RestartGprsSeconds = 0;
	}

}

bool InitGprs()
{

	WaitForTimeOut = 300;

	GprsSerial.print(F("AT+CPOF\r\n"));

	if (!_esp8266_waitFor("SMS Ready")) return false;
	printf("\r\nSMS Ready\r\n");

	GprsSerial.print(F("AT+CGATT=1\r\n"));
	if (!_esp8266_waitFor("OK")) return false;
	printf("\r\nattach ok\r\n");

	GprsSerial.print(F("AT+CGACT=1,1\r\n"));
	if (!_esp8266_waitFor("OK")) return false;
	printf("\r\n connect network ok\r\n");

	WaitForTimeOut = 10;

	GprsSerial.print(F("AT+CIPMUX=0\r\n"));
	if (!_esp8266_waitFor("OK")) return false;
	printf("\r\n disable MUX ok\r\n");

	GprsSerial.print(F("AT+CIPHEAD=1\r\n"));
	if (!_esp8266_waitFor("OK")) return false;
	printf("\r\n disable IP head ok\r\n");


	WaitForTimeOut = 100;

	GprsSerial.print(F("AT+CIPSTART=\"UDP\",\"fryefryefrye.myds.me\",5050\r\n"));
	if (!_esp8266_waitFor("T OK")) return false;
	printf("\r\n connect server ok\r\n");

	WaitForTimeOut = 10;
	return true;
}

bool SendGprsUpdate()
{
	unsigned char data;
	unsigned char dataIndex = 0;
	unsigned char *pCommand;

	WaitForTimeOut = 10;
	printf("\r\n");

	GprsSerial.print(F("AT+CIPSEND="));
	GprsSerial.print(sizeof(tLedTimeData));
	GprsSerial.print(F("\r\n"));

	if(!_esp8266_waitFor(">")) return false;

	LedTimeData.DataType = 9;
	LedTimeData.RunningDateTime = (unsigned long)t-3600*8;

	LedTimeData.Brightness = LedTimeCommand.Brightness;
	LedTimeData.ColorIndex = LedTimeCommand.ColorIndex;

	LedTimeData.Sum = cal_crc((unsigned char *)&LedTimeData,sizeof(tLedTimeData)-4);

	for(unsigned char i = 0; i<(sizeof(tLedTimeData)) ; i++)
	{
		data = *(((char*)(&LedTimeData))+i);
		GprsSerial.write(data);
	}

	WaitForTimeOut = 50;

	if(!_esp8266_waitFor("SEND OK")) return false;

	if(!_esp8266_waitFor("+IPD")) return false;

	WaitForTimeOut = 10;

	if(!_esp8266_waitFor(":")) return false;

	tLedTimeCommand TempLedTimeCommand;
	pCommand = (unsigned char *)&TempLedTimeCommand;

	while(1)
	{

		if (_esp8266_getch((char *)&data))
		{
			pCommand[dataIndex] = data;
			dataIndex++;

			if (dataIndex>=sizeof(tLedTimeCommand))
			{
				//break;
				unsigned long crc_should = cal_crc((unsigned char *)&TempLedTimeCommand,sizeof(tLedTimeCommand)-4);
				if (TempLedTimeCommand.Sum == crc_should)
				{
					memcpy(&LedTimeCommand,&TempLedTimeCommand,sizeof(tLedTimeCommand));
					color = GetRGB(LedTimeCommand.ColorIndex,LedTimeCommand.Brightness);
				}
				else
				{
					printf("Command crc check failed. should:0x%04X recv:0x%04X\r\n",crc_should,TempLedTimeCommand.Sum);
				}
				return true;
			}
		}
		else
		{
			printf("can not wait more data\r\n");
			return false;
		}
	}



}

//bool GetGprsTime()
//{
//
//	unsigned char data;
//	unsigned char dataIndex = 0;
//
//
//	WaitForTimeOut = 300;
//
//	//GprsSerial.print(F("AT+QLTS\r\n"));
//
//	if(!_esp8266_waitFor("+NITZ:")) return false;
//
//	while(1)
//	{
//
//		if (_esp8266_getch((char *)&data))
//		{
//			TimeString[dataIndex] = data;
//			dataIndex++;
//
//			if (dataIndex>=TIME_STRING_LEN)
//			{
//				//break;
//				TimeString[dataIndex] = 0;
//
//				//Update time to UTC
//				tm.Hour = (TimeString[9]-0x30)*10+(TimeString[10]-0x30);
//				tm.Minute = (TimeString[12]-0x30)*10+(TimeString[13]-0x30);
//				tm.Second = (TimeString[15]-0x30)*10+(TimeString[16]-0x30);
//				tm.Day = (TimeString[6]-0x30)*10+(TimeString[7]-0x30);
//				tm.Month = (TimeString[3]-0x30)*10+(TimeString[4]-0x30);
//				tm.Year = (TimeString[0]-0x30)*10+(TimeString[1]-0x30)+30;
//				t = makeTime(tm);
//
//				t = t + 3600*8;
//
//
//				return true;
//			}
//		}
//		else
//		{
//			printf("can not wait more data\r\n");
//			return false;
//		}
//	}
//
//
//}

bool GpsUpdate(unsigned char k)
{
	if (is_GPS_data)
	{
		pGpsData[GPS_data_Index-GPS_HEAD_LEN] = k;

		if (GPS_data_Index - GPS_HEAD_LEN >= sizeof(tGpsData)-1)
		{
			GPS_data_Index=0;
			is_GPS_data=false;
			printf("GPS_data got\r\n");
			return true;
		}
		GPS_data_Index++;
	}
	else
	{
		if (GPS_data_head_array[GPS_data_Index] == k)
		{
			GPS_data_Index++;
			if (GPS_data_Index >= GPS_HEAD_LEN)
			{
				//printf("GPS_data_head got\r\n");
				is_GPS_data = true;
			}
		}
		else
		{
			GPS_data_Index=0;
		}
	}
	return false;
}

void ChangeToGprs()
{
	//1 GPS   2 GPRS

	if (CurrentOn !=2 )
	{
		GpsSerial.stopListening();
		GprsSerial.listen();
		CurrentOn = 2;
	}

}
void ChangeToGps()
{
	if (CurrentOn !=1 )
	{
		GprsSerial.stopListening();
		GpsSerial.listen();
		CurrentOn = 1;
	}
}

void OnSeconds()
{
	unsigned char Numbers[6];
	unsigned char Hour;

	LedTimeData.SunBrightness = analogRead(0);

	if (SecondsSinceStart%1 == 0)
	{
		if (!SendingUpdate)
		{
			NeedSendUpdate = true;	
		}
	}

	if (SecondsSinceStart%60 == 0)
	{
		NeedTimeUpdate = true;
	}

	t++;
	//printf("Date Time = %d-%02d-%02d %02d:%02d:%02d fix = %x \r\n",year(t) ,month(t),day(t),hour(t),minute(t),second(t),GpsData.fix);




	Hour = hour(t);

	if (Hour > 12)
	{
		Hour = Hour - 12;
	}


	//Numbers[0] = second(t)%10;
	//Numbers[1] = second(t)/10;
	//Numbers[2] = minute(t)%10;
	//Numbers[3] = minute(t)/10;
	//Numbers[4] = Hour%10;
	//Numbers[5] = Hour/10;


	Numbers[0] = minute(t)%10;
	Numbers[1] = minute(t)/10;
	Numbers[2] = Hour%10;
	Numbers[3] = Hour/10;

	for(int i = 0; i < 6;i++)
	{
		printf("Numbers = %d\r\n",Numbers[i]);
	}
	for(int i = 0; i < 21;i++)
	{
		if(DIGITAL_DISPLAY[Numbers[i/7]][i%7] == 1)
		{
			//val = 10;
			strip.setPixelColor(i, color);
		}
		else
		{
			//val = 0;
			strip.setPixelColor(i, 0);
		}
	}
	for(int i = 14; i < MAX_LED;i++)
	{

		strip.setPixelColor(i, 0);
	}
	strip.show();

	
}


bool _esp8266_waitFor(const char *string) {
	char so_far = 0;
	char received;
	int counter = 0;
	do {
		//received = _esp8266_getch();
		if (!_esp8266_getch(&received))
		{
			return false;
		}
		counter++;
		if (received == string[so_far]) {
			so_far++;
		} else {
			so_far = 0;
		}
	} while (string[so_far] != 0);
	return true;
}

bool _esp8266_getch(char * RetData)   
{
	unsigned long RecvStartTime = TenthSecondsSinceStart;
	while (1)
	{
		if (GprsSerial.available() > 0)
		{
			*RetData = GprsSerial.read();
			if ((*RetData>31)&&(*RetData<127))
			{
				Serial.write(*RetData);
			}
			
			//printf("get char: 0x%02X\r\n",*RetData);
			return true;
		}
		NonStopTask();
		if (TenthSecondsSinceStart - RecvStartTime > WaitForTimeOut)
		{
			return false;
		}
	}
}

unsigned long GetRGB(unsigned long ColorIndex,unsigned long Brightness)
{
	//{"黑","红","绿","蓝","黄","紫","青","白"};

	//RGB 实际顺序： 蓝红绿

	unsigned long ColorBrightInfo = 0;

	if (Brightness>100)
	{
		return strip.Color(10, 10, 10);
	}

	Brightness = Brightness/5;

	switch(ColorIndex)
	{
	case 0:
		return 0;
		break;
	case 3:
		ColorBrightInfo = strip.Color(255*Brightness/100, 0*Brightness/100, 0*Brightness/100);;//蓝
		break;
	case 1:
		ColorBrightInfo = strip.Color(0*Brightness/100, 255*Brightness/100, 0*Brightness/100);;//红
		break;
	case 2:
		ColorBrightInfo = strip.Color(0*Brightness/100, 0*Brightness/100, 255*Brightness/100);;//绿
		break;
	case 5:
		ColorBrightInfo = strip.Color(255*Brightness/100, 255*Brightness/100, 0*Brightness/100);;//紫
		break;
	case 6:
		ColorBrightInfo = strip.Color(255*Brightness/100, 0*Brightness/100, 255*Brightness/100);;//青
		break;
	case 4:
		ColorBrightInfo = strip.Color(0*Brightness/100, 255*Brightness/100, 255*Brightness/100);;//黄
		break;
	case 7:
		ColorBrightInfo = strip.Color(255*Brightness/100, 255*Brightness/100, 255*Brightness/100);
		break;
	default:
		ColorBrightInfo = strip.Color(10, 10, 10);
		break;
	}

	return ColorBrightInfo;
}
#define crc_mul 0x1021  //生成多项式
unsigned long cal_crc(unsigned char *ptr, unsigned char len)
{
	unsigned char i;
	unsigned long crc=0;
	while(len-- != 0)
	{
		for(i=0x80; i!=0; i>>=1)
		{
			if((crc&0x8000)!=0)
			{
				crc<<=1;
				crc^=(crc_mul);
			}else{
				crc<<=1;
			}
			if((*ptr&i)!=0)
			{
				crc ^= (crc_mul);
			}
		}
		ptr ++;
	}
	return (crc);
}
