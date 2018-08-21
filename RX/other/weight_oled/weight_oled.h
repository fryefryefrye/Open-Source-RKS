

#define MIN_WEIGHT_DIFF 10

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

unsigned long TimeOutForDrink;
int LastWeight = 0;
int AccumulatedDrinking = 0;

void SecondsSinceStartTask();
void Buzz_task();
void OnSecond();
void ProcessWeight();
void ShowState();
void Buzz_task();



void setup()
{

	TimeOutForDrink = 0;

	Displayer.Initial();
	delay(10);

	Displayer.Fill_Screen(0x00,0x00);


	//pinMode(WEIGHT_CLK, OUTPUT);
	//pinMode(WEIGHT_DAT, INPUT);

	pinMode(BUZZ, OUTPUT);


	Serial.begin(115200);
	Serial.println(F("Weight"));
	printf_begin();


	//Alarm = 5;






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
	Buzz_task();

} // Loop


unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 1000)
	{
		LastMillis = CurrentMillis;
		SecondsSinceStart++;


		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);


		OnSecond();



	}
}

void OnSecond()
{

	TimeOutForDrink++;

	Weight = scale.get_units(10);

	//Serial.print("Average weight:");
	//Serial.println(Weight);

	ProcessWeight();
	ShowState();

	if (TimeOutForDrink%(15*60) == 0)
	{
		digitalWrite(BUZZ, HIGH);
		delay(50);
		digitalWrite(BUZZ, LOW);
	}





}

void ProcessWeight()
{
	static int StableWeight;
	static unsigned char StableWeightCounter;

	if (Weight == StableWeight)
	{
		StableWeightCounter++;
	} 
	else
	{
		StableWeight = Weight;
		StableWeightCounter = 0;
	}

	if (StableWeightCounter <3)
	{

		//printf("no Stable, counter =  %d \r\n ",StableWeightCounter);
		return;
	}

	if (Weight < MIN_WEIGHT_DIFF)
	{
		//printf("Empty, weight =  %d \r\n ",Weight);
		return;
	}

	//printf("Stable, weight =  %d \r\n ",Weight);

	//printf("Weight change  %d \r\n ",abs(Weight - LastWeight));
	if (abs(Weight - LastWeight) > MIN_WEIGHT_DIFF)
	{
		
		printf("Weight changed to %d \r\n ",Weight);
		if (Weight < LastWeight) //Reduce
		{
			printf("Weight changed. Reduce %d \r\n ",LastWeight - Weight);
			AccumulatedDrinking = AccumulatedDrinking + (LastWeight - Weight);
			TimeOutForDrink = 0;
		} 
		else
		{
			printf("Weight changed. Added \r\n ");
		}
		LastWeight = Weight;
	} 
	else
	{
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

	//Displayer.ShowASCII816(8,0,DisplayerCharge/100%10);

	if (Weight >= 0)
	{
		Displayer.ShowASCII816(0,4,Weight/1000%10);
		Displayer.ShowASCII816(8,4,Weight/100%10);
		Displayer.ShowASCII816(8*2,4,Weight/10%10);
		Displayer.ShowASCII816(8*3,4,Weight%10);
	} 
	else
	{

		Weight = Weight * (-1);
		Displayer.ShowASCII816(0,4,10); //-
		Displayer.ShowASCII816(16,4,Weight/100%10);
		Displayer.ShowASCII816(32,4,Weight/10%10);
		Displayer.ShowASCII816(48,4,Weight%10);
	}



	//if (Weight >= 0)
	//{
	//	Displayer.ShowASCII1632(0,4,Weight/1000%10);
	//	Displayer.ShowASCII1632(16,4,Weight/100%10);
	//	Displayer.ShowASCII1632(32,4,Weight/10%10);
	//	Displayer.ShowASCII1632(48,4,Weight%10);
	//} 
	//else
	//{

	//	Weight = Weight * (-1);
	//	Displayer.ShowASCII1632(0,4,10); //-
	//	Displayer.ShowASCII1632(16,4,Weight/100%10);
	//	Displayer.ShowASCII1632(32,4,Weight/10%10);
	//	Displayer.ShowASCII1632(48,4,Weight%10);
	//}


	Displayer.ShowASCII1632(16*4,4,AccumulatedDrinking/1000%10);
	Displayer.ShowASCII1632(16*5,4,AccumulatedDrinking/100%10);
	Displayer.ShowASCII1632(16*6,4,AccumulatedDrinking/10%10);
	Displayer.ShowASCII1632(16*7,4,AccumulatedDrinking%10);

	//Displayer.ShowASCII1632(0,0,TimeOutForDrink/1000%10);
	//Displayer.ShowASCII1632(16,0,TimeOutForDrink/100%10);
	Displayer.ShowASCII1632(16*2,0,TimeOutForDrink/60/100%10);

	Displayer.ShowASCII1632(16*3,0,TimeOutForDrink/60/10%10);
	Displayer.ShowASCII1632(16*4,0,TimeOutForDrink/60%10);


	Displayer.ShowASCII1632(16*5,0,11);


	Displayer.ShowASCII1632(16*6,0,TimeOutForDrink%60/10%10);
	Displayer.ShowASCII1632(16*7,0,TimeOutForDrink%60%10);

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
