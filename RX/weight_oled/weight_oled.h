

#define WEIGHT_CLK 2
#define WEIGHT_DAT 3


#define BUZZ 4
#define BUZZON 4000				//set lenght of the buzz 2000
#define BUZZOFF 7000			//set interval of the buzz 5000

#include <printf.h>

//display
#include <Wire.h>
#include "CN_SSD1306_Wire\CN_SSD1306_Wire.h"
#include "HX711\HX711.h"

CN_SSD1306_Wire Displayer(8);//HardWare I2C

HX711 scale(WEIGHT_DAT, WEIGHT_CLK);

unsigned long SecondsSinceStart;
unsigned char Alarm;
int Weight;


void SecondsSinceStartTask();
void Buzz_task();





void setup()
{

	Displayer.Initial();
	delay(10);

	Displayer.Fill_Screen(0x00,0x00);


	pinMode(WEIGHT_CLK, OUTPUT);
	pinMode(WEIGHT_DAT, INPUT);

	pinMode(BUZZ, OUTPUT);


	Serial.begin(115200);
	Serial.println(F("Weight"));
	printf_begin();






	printf("raw reading from the ADC = %ld \r\n",scale.read());
	printf("average of 20 raw from the ADC = %ld \r\n",scale.read_average(20));

	scale.set_scale(386.5f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
	scale.tare();				        // reset the scale to 0



	//for(wm = 0;wm < 16;wm++)
	//{
	//	//printf("ASCII816_2 %x \r\n", ASCII816_2[adder]);
	//	ASCII1632[wm] = 0;
	//	for(i = 0;i < 8;i++)
	//	{
	//		//printf("((((unsigned int)ASCII816_2[adder]>>i)&1)*2) %x \r\n", ((((unsigned int)ASCII816_2[adder]>>i)&1)*2))<<(2*i);

	//		//printf("2<<2 %x \r\n", 2<<(2*i);

	//		ASCII1632[wm] = ASCII1632[wm] + (((((unsigned int)ASCII816_2[adder]>>i)&1)*3)<<(2*i));
	//	}
	//	printf("ASCII1632 %x \r\n", ASCII1632[wm]);
	//	adder += 1;	
	//}


	//Displayer.IIC_SetPos(0 , 0);
	//for(wm = 0;wm < 8;wm++)
	//{
	//	Displayer.WriteData(*((unsigned char*)(&ASCII1632[wm/2])));

	//	printf("*((unsigned char*)(&ASCII1632[wm/2])) = %x \r\n",*((unsigned char*)(&ASCII1632[wm/2])));
	//}


	//Displayer.IIC_SetPos(0,0 + 1);
	//for(wm = 0;wm < 8;wm++)
	//{
	//	Displayer.WriteData(*(((unsigned char*)(&ASCII1632[wm/2]))+1));
	//			printf("*((unsigned char*)(&ASCII1632[wm/2]+1)) = %x \r\n",(*(((unsigned char*)(&ASCII1632[wm/2]))+1)));
	//}

	//Displayer.IIC_SetPos(1 , 0);
	//for(wm = 0;wm < 8;wm++)
	//{
	//	Displayer.WriteData(*(((unsigned char*)(&ASCII1632[wm+8/2]))));
	//}


	//Displayer.IIC_SetPos(1,0 + 1);
	//for(wm = 0;wm < 8;wm++)
	//{
	//	Displayer.WriteData(*(((unsigned char*)(&ASCII1632[wm+8/2]))+1));
	//}


}

void loop()
{

	SecondsSinceStartTask();

} // Loop


unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 1000)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;


		printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);

		Weight = scale.get_units(10);

		Serial.print("Average weight:");
		Serial.println(Weight);


		
		Displayer.ShowASCII1632(0,4,Weight/100%10);

		Displayer.ShowASCII1632(16,4,Weight/10%10);
		Displayer.ShowASCII1632(32,4,Weight%10);




	}
}

void Buzz_task()
{

	static signed int BuzzOn;
	static signed int BuzzOff;
	static bool BuzzHigh;

	if (Alarm > 0)
	{
		if (BuzzHigh)
		{
			BuzzOn++;
			if (BuzzOn > BUZZON)
			{
				BuzzOn = 0;
				BuzzHigh = false;
				digitalWrite(BUZZ, LOW);
				Alarm--;
			}
		}
		else
		{
			BuzzOff++;
			if (BuzzOff > BUZZOFF)
			{
				BuzzOff = 0;
				BuzzHigh = true;
				digitalWrite(BUZZ, HIGH);

			}
		}
	}
	else
	{
	}
}

void ShowState()
{

	//static bool UnitShowed = false;
	static bool DataStored = false;
	static unsigned int CellVolt = 0;
	static unsigned char ShowIndex = 0;

	//if (!UnitShowed)
	//{
	//	Displayer.ShowASCII1632(32,4,12);
	//	Displayer.ShowASCII1632(16*5,4,10);
	//	Displayer.ShowASCII1632(16*7,4,11);
	//	Displayer.ShowASCII816(0,0,13);//C
	//	Displayer.ShowASCII816(0,2,14);//D
	//	Displayer.ShowASCII816(3*8,0,15);//.
	//	Displayer.ShowASCII816(3*8+56-16,0,15);//.
	//	Displayer.ShowASCII816(3*8,2,15);//.
	//	Displayer.ShowASCII816(3*8+56-16,2,15);//.
	//	Displayer.ShowASCII816(9*8+56-24,0,15);//.
	//	Displayer.ShowASCII816(9*8+56-24+16,0,12);//V
	//	Displayer.ShowASCII816(9*8+56-24,2,15);//.
	//	Displayer.ShowASCII816(9*8+56-24+16,2,16);//%


	//	UnitShowed = true;
	//}


	//if (!DataStored)
	//{
	//	CellVolt = FullVolt/13;
	//	DataStored = true;
	//	ShowIndex++;
	//}


	//switch(ShowIndex)
	//{
	//case 1:
	//	Displayer.ShowASCII1632(0,4,CellVolt/100%10);
	//	break;
	//case 2:
	//	Displayer.ShowASCII1632(16,4,CellVolt/10%10);
	//	break;
	//case 3:
	//	Displayer.ShowASCII1632(16*3,4,Current/10000%10);
	//	break;
	//case 4:
	//	Displayer.ShowASCII1632(16*4,4,Current/1000%10);
	//	break;
	//case 5:
	//	Displayer.ShowASCII1632(16*6,4,Current/100%10);
	//	break;
	//}

	ShowIndex++;

	if (ShowIndex>=6)
	{
		ShowIndex = 0;
		DataStored = false;
	}



}

void ShowChargeInfo()
{


	//static bool UnitShowed = false;
	static bool DataStored = false;
	static unsigned int DisplayerCharge;
	static unsigned int DisplayerDisCharge;
	static unsigned int DisplayerLastCharge;
	static unsigned int DisplayerLastDisCharge;
	static unsigned int DisplayerFullVolt;
	static unsigned int DisplayerRemain;
	static unsigned char ShowIndex = 0;



	if (!DataStored)
	{
		//DisplayerCharge =Charge/360000;//3600; //Charge/36000; //mah
		//DisplayerDisCharge =DisCharge/360000;//3600; //DisCharge/36000; //mah
		//DisplayerLastCharge = LastCharge/360000;
		//DisplayerLastDisCharge = LastDisCharge/360000;
		//DisplayerFullVolt = FullVolt;
		//if (Capability >= DisCharge)
		//{
		//	DisplayerRemain = (Capability - DisCharge)/(Capability/1000);
		//} 
		//else
		//{
		//	DisplayerRemain = 0;
		//}
		//DataStored = true;
		//ShowIndex++;
	}







	//switch(ShowIndex)
	//{
	//case 1:
	//	Displayer.ShowASCII816(8,0,DisplayerCharge/100%10);
	//	Displayer.ShowASCII816(8+56-16,0,DisplayerLastCharge/100%10);
	//	break;
	//case 2:
	//	Displayer.ShowASCII816(2*8,0,DisplayerCharge/10%10);
	//	Displayer.ShowASCII816(2*8+56-16,0,DisplayerLastCharge/10%10);
	//	break;
	//case 3:

	//	break;
	//case 4:
	//	Displayer.ShowASCII816(4*8,0,DisplayerCharge/1%10);
	//	Displayer.ShowASCII816(4*8+56-16,0,DisplayerLastCharge/1%10);
	//	break;
	//case 5:
	//	//Displayer.ShowASCII816(5*8,0,DisplayerCharge/1%10);
	//	//Displayer.ShowASCII816(5*8+56,0,DisplayerLastCharge/1%10);
	//	break;
	//case 6:
	//	Displayer.ShowASCII816(1*8,2,DisplayerDisCharge/100%10);
	//	Displayer.ShowASCII816(1*8+56-16,2,DisplayerLastDisCharge/100%10);
	//	break;
	//case 7:
	//	Displayer.ShowASCII816(2*8,2,DisplayerDisCharge/10%10);
	//	Displayer.ShowASCII816(2*8+56-16,2,DisplayerLastDisCharge/10%10);
	//	break;
	//case 8:

	//	break;
	//case 9:
	//	Displayer.ShowASCII816(4*8,2,DisplayerDisCharge/1%10);
	//	Displayer.ShowASCII816(4*8+56-16,2,DisplayerLastDisCharge/1%10);
	//	break;
	//case 10:
	//	//Displayer.ShowASCII816(5*8,2,DisplayerDisCharge/1%10);
	//	//Displayer.ShowASCII816(5*8+56,2,DisplayerLastDisCharge/1%10);
	//	break;
	//case 11:
	//	Displayer.ShowASCII816(7*8+56-24,0,DisplayerFullVolt/10000%10);
	//	Displayer.ShowASCII816(8*8+56-24,0,DisplayerFullVolt/1000%10);
	//	Displayer.ShowASCII816(10*8+56-24,0,DisplayerFullVolt/100%10);
	//	break;
	//case 12:
	//	if (DisplayerRemain>=999)
	//	{
	//		DisplayerRemain = 999;
	//	}
	//	Displayer.ShowASCII816(7*8+56-24,2,DisplayerRemain/100%10);
	//	Displayer.ShowASCII816(8*8+56-24,2,DisplayerRemain/10%10);
	//	Displayer.ShowASCII816(10*8+56-24,2,DisplayerRemain/1%10);
	//	break;


	//}



	ShowIndex++;

	if (ShowIndex>=13)
	{
		ShowIndex = 0;
		DataStored = false;
	}

}

