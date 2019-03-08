#include <printf.h>

#include "TimeLib.h"

bool GpsUpdate(unsigned char k);
void OnSeconds();

struct tGpsData
{
	unsigned long iTOW;
	long fTOW;
	int week;
	unsigned char fix;
};

tGpsData GpsData;
unsigned char * pGpsData;

//Serial data
int comdata ;

//GPS
bool is_GPS_data = false;
#define GPS_HEAD_LEN 6
unsigned char GPS_data_head_array[GPS_HEAD_LEN] =  {0xB5, 0x62, 0x01, 0x06, 0x34 ,0x00};
unsigned char GPS_data_Index = 0;

time_t t;
bool bGPS_Valid;
unsigned long SecondsSinceStart = 0;
unsigned long LastMillis;
unsigned long CurrentMillis;

#include <SoftwareSerial.h>
SoftwareSerial SoftSerial(2, 3); //rx 2,tx 3


#include "Adafruit_NeoPixel.h"
#define PIN 4
#define MAX_LED 21
#define LED1  5
#define LED2  6
unsigned char LED_PWM;
int val = 0;
int LedOn = 0;
uint32_t color_array[MAX_LED];
uint32_t color;
Adafruit_NeoPixel strip = Adafruit_NeoPixel( MAX_LED, PIN, NEO_RGB + NEO_KHZ800 );
//unsigned char DIGITAL_DISPLAY[10][8] = { //设置0-9数字所对应数组
//	{ 1,0,0,0,0,1,0,0 }, // = 0
//	{ 1,0,0,1,1,1,1,1 }, // = 1
//	{ 1,1,0,0,1,0,0,0 }, // = 2
//	{ 1,0,0,0,1,0,1,0 }, // = 3
//	{ 1,0,0,1,0,0,1,1 }, // = 4
//	{ 1,0,1,0,0,0,1,0 }, // = 5
//	{ 1,0,1,0,0,0,0,0 }, // = 6
//	{ 1,0,0,0,1,1,1,1 }, // = 7
//	{ 1,0,0,0,0,0,0,0 }, // = 8
//	{ 1,0,0,0,0,0,1,0 } // = 9

unsigned char DIGITAL_DISPLAY[10][7] = { //设置0-9数字所对应数组
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
	//{ 1,1,0,0,0,0,0 }, // = 1
	//{ 1,0,1,1,0,1,1 }, // = 2
	//{ 1,1,1,0,0,1,1 }, // = 3
	//{ 1,1,0,0,1,0,1 }, // = 4
	//{ 0,1,0,0,0,1,0 }, // = 5
	//{ 1,1,0,0,0,0,0 }, // = 6
	//{ 1,0,0,1,1,1,1 }, // = 7
	//{ 0,0,0,0,0,0,0 }, // = 8
	//{ 0,0,0,0,0,1,0 } // = 9

};

void setup()
{


	Serial.begin(115200);
	//Serial.println(F("GPS_Time"));
	printf_begin();
	printf("sizeof(tGpsData) = %d \r\n",sizeof(tGpsData));
	SoftSerial.begin(38400);

	pinMode(LED1,OUTPUT);
	pinMode(LED2,OUTPUT);

	// 初始化库
	strip.begin();
	// 发送数据，默认每个点的颜色为0，所以初始的每个点都是不亮的
	strip.show();

	pGpsData = (unsigned char *)&GpsData;

}

void loop()
{

		if (SoftSerial.available() > 0)
		{
			comdata = SoftSerial.read();
			if (GpsUpdate(comdata))
			{
				

				t = (unsigned long)GpsData.week*7*24*3600+GpsData.iTOW/1000;
				t = (unsigned long)t +8*3600 - 18;//+10*365*24*3600+5*24*3600;
				//t = (unsigned long)t +10*365*24*3600;
			}

			//Serial.write(comdata);

		}



		CurrentMillis = millis();
		if (abs(CurrentMillis - LastMillis) > 1000)
		{
			LastMillis = CurrentMillis;
			SecondsSinceStart++;
			OnSeconds();
			//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
		}



}





bool GpsUpdate(unsigned char k)
{
	//static unsigned char SepIndex = 0;
	//static unsigned char LastSeconds = 0;
	if (is_GPS_data)
	{
		GPS_data_Index++;
		pGpsData[GPS_data_Index-GPS_HEAD_LEN-1] = k;

		if (GPS_data_Index - GPS_HEAD_LEN -1 > sizeof(tGpsData))
		{
			GPS_data_Index=0;
			is_GPS_data=false;
			//printf("GPS_data got\r\n");
			return true;
		}
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

void OnSeconds()
{
	unsigned char Numbers[6];

	unsigned char Hour;


	//static unsigned char LED_PWM = 0;
	//LED_PWM = LED_PWM + 10;
	//printf("LED_PWM = %d \r\n",LED_PWM);
	//analogWrite(LED1, LED_PWM);
	//analogWrite(LED2, LED_PWM);

	printf("Date Time = %d-%02d-%02d %02d:%02d:%02d fix = %x \r\n",year(t) ,month(t),day(t),hour(t),minute(t),second(t),GpsData.fix);

	Numbers[0] = second(t)%10;
	Numbers[1] = second(t)/10;
	Numbers[2] = minute(t)%10;
	Numbers[3] = minute(t)/10;


	Hour = hour(t);

	if (Hour > 12)
	{
		Hour = Hour - 12;
	}

	Numbers[4] = Hour%10;
	Numbers[5] = Hour/10;

	//for(int i = 0; i < 6;i++)
	//{
	//	printf("Numbers = %d\r\n",Numbers[i]);
	//}

	for(int i = 0; i < MAX_LED;i++)
	{
		if(DIGITAL_DISPLAY[Numbers[i/7+2]][i%7] == 1)
		{
			val = 10;
		}
		else
		{
			val = 0;
		}
		color = strip.Color(val, val, val);
		strip.setPixelColor(i, color);
	}
	strip.show();

	if(DIGITAL_DISPLAY[Numbers[4]][6] == 1)//2=分，各位
	{
		LED_PWM = 10;
	}
	else
	{
		LED_PWM = 0;
	}
	analogWrite(LED2, LED_PWM);

	if(Numbers[5]>0)
	{
		LED_PWM = 10;
	}
	else
	{
		LED_PWM = 0;
	}
	analogWrite(LED1, LED_PWM);
}