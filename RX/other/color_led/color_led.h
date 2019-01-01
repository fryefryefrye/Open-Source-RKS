#include "Adafruit_NeoPixel.h"
#include <printf.h>

#define PIN 6
#define MAX_LED 20
#define MAX_LIGHT 20



int val = 0;
int LedOn = 0;
uint32_t color_array[MAX_LED];
uint32_t color;



// Parameter 1 = ws2811级联数量
// Parameter 2 = arduino PIN
// Parameter 3 = pixel type flags, add together as needed:
// NEO_KHZ800 800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEO_KHZ400 400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
// NEO_GRB Pixels are wired for GRB bitstream (most NeoPixel products)
// NEO_RGB Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel( MAX_LED, PIN, NEO_RGB + NEO_KHZ800 );

void setColor(uint8_t r, uint8_t g, uint8_t b);

void setup()
{

	Serial.begin(115200);
	Serial.println(F("color_led"));
	printf_begin();

	// 初始化库
	strip.begin();
	// 发送数据，默认每个点的颜色为0，所以初始的每个点都是不亮的
	strip.show();
}

void loop()
{


	// 随机色流水
	//for(int i = MAX_LED; i > 0;i--)
	//{
	//	color_array[i] = color_array[i-1];
	//	printf("copy data from %d to %d \r\n",i-1,i);
	//}

	//color_array[0] = strip.Color(random(20), random(20), random(20));

	//for(int i = 0; i < MAX_LED;i++)
	//{
	//	strip.setPixelColor(i, color_array[i]);
	//}

	//strip.show();
	//delay(50);




	 //快速流水
	//for(int i = 0; i < MAX_LED;i++)
	//{
	//	if(i == LedOn)
	//	{
	//		val = 10;
	//	}
	//	else
	//	{
	//		val = 0;
	//	}
	//	color = strip.Color(val, val, val);
	//	strip.setPixelColor(i, color);
	//}
	//strip.show();
	//delay(500);

	//LedOn++;
	//if (LedOn > MAX_LED)
	//{
	//	LedOn = 0;
	//}





	////渐变色,单色亮灭。
	static uint8_t mode = 0;
	static uint8_t r=0; 
	static uint8_t g=0; 
	static uint8_t b=0;
	switch (mode)
	{
		case 0:
			r++;
			if (r == MAX_LIGHT)
			{
				mode = 1;
			}
			break;
		case 1:
			r--;
			if (r == 0)
			{
				mode = 2;
			}
			break;
		case 2:
			g++;
			if (g == MAX_LIGHT)
			{
				mode = 3;
			}
			break;
		case 3:
			g--;
			if (g == 0)
			{
				mode = 4;
			}
			break;
		case 4:
			b++;
			if (b == MAX_LIGHT)
			{
				mode = 5;
			}
			break;
		case 5:
			b--;
			if (b == 0)
			{
				mode = 6;
			}
			break;
		case 6:
			r++;
			g++;
			if (r == MAX_LIGHT)
			{
				mode = 7;
			}
			break;
		case 7:
			r--;
			g--;
			if (r == 0)
			{
				mode = 8;
			}
			break;
		case 8:
			g++;
			b++;
			if (g == MAX_LIGHT)
			{
				mode = 9;
			}
			break;
		case 9:
			g--;
			b--;
			if (g == 0)
			{
				mode = 10;
			}
			break;
		case 10:
			b++;
			r++;
			if (b == MAX_LIGHT)
			{
				mode = 11;
			}
			break;
		case 11:
			b--;
			r--;
			if (b == 0)
			{
				mode = 12;
			}
			break;
		case 12:
			b++;
			r++;
			g++;
			if (b == MAX_LIGHT)
			{
				mode = 13;
			}
			break;
		case 13:
			b--;
			r--;
			g--;
			if (b == 0)
			{
				mode = 0;
			}
			break;
	}
	setColor(r, g, b);
	delay(200);

//
////变换色
//static uint8_t mode = 0;
//static uint8_t r=0; 
//static uint8_t g=0; 
//static uint8_t b=0;
//switch (mode)
//{
//case 0:
//	r++;
//	if (r == MAX_LIGHT)
//	{
//		mode = 1;
//	}
//	break;
//case 1:
//	r--;
//	g++;
//	if (r == 0)
//	{
//		mode = 2;
//	}
//	break;
//case 2:
//	g--;
//	b++;
//	if (g == 0)
//	{
//		mode = 3;
//	}
//	break;
//case 3:
//	b--;
//	r++;
//	if (b == 0)
//	{
//		mode = 1;
//	}
//	break;
//}
//setColor(r, g, b);
//delay(20);


	
}

void setColor(uint8_t r, uint8_t g, uint8_t b)
{
	printf("color %d %d %d \r\n",r,g,b);

	color = strip.Color(b, r, g);//b,r,G

	for(int i = 0; i < MAX_LED;i++)
	{
		strip.setPixelColor(i, color);
	}
	strip.show();

}