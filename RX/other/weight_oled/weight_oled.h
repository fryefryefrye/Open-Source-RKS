

#define MIN_WEIGHT_DIFF 10
#define MIN_CUP_WEIGHT 50
#define STABLECOUNTER 5
#define MIN_STABLE 3

#define WEIGHT_CLK 2
#define WEIGHT_DAT 3


#define BUZZ 4


#include <printf.h> // from RF24


//display
#include <Wire.h>
#include "CN_SSD1306_Wire\CN_SSD1306_Wire.h"
CN_SSD1306_Wire Displayer(8);//HardWare I2C


#include "HX711\HX711.h"
HX711 scale(WEIGHT_DAT, WEIGHT_CLK);

unsigned long SecondsSinceStart;
unsigned char Alarm;
int Weight;

unsigned long TimeOutForDrink;
unsigned long TimeOutEmpty = 0;
int LastWeight = 0;
int AccumulatedDrinking = 0;
//bool Pause = false;

void SecondsSinceStartTask();
void OnSecond();
void ProcessWeight();
void ShowState();
void Beep();
void Beep3();


void setup()
{
	pinMode(BUZZ, OUTPUT);

	Beep();

	TimeOutForDrink = 0;

	Displayer.Initial();
	delay(10);

	Displayer.Fill_Screen(0x00,0x00);





	Serial.begin(115200);
	Serial.println(F("Weight"));
	printf_begin();


	printf("raw reading from the ADC = %ld \r\n",scale.read());
	printf("average of 20 raw from the ADC = %ld \r\n",scale.read_average(20));

	scale.set_scale(386.5f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
	scale.tare();				        // reset the scale to 0

	Beep();



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
		//printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
		OnSecond();
	}
}

void OnSecond()
{

	if (TimeOutForDrink > 28800)
	{
		//TimeOutForDrink = 0;
		AccumulatedDrinking = 0;
		//Pause = true;
	}
	

	Weight = scale.get_units(10);

	ProcessWeight();
	ShowState();

	//if (!Pause)
	//{
		TimeOutForDrink++;
	//}

	if (TimeOutForDrink%(60*30) == 0)
	{
		Beep();
	}
	
	if(TimeOutEmpty == 300)
	{
		Beep3();
	}


}

void ProcessWeight()
{
	static int StableWeight;
	static unsigned char StableWeightCounter;

	//if (Weight == StableWeight)
	if (abs(Weight - StableWeight)<MIN_STABLE)
	{
		StableWeightCounter++;
	} 
	else
	{
		StableWeight = Weight;
		StableWeightCounter = 0;
	}

	if (StableWeightCounter < STABLECOUNTER)
	{
		//printf("no Stable, counter =  %d \r\n",StableWeightCounter);
		return;
	}

	if (Weight < MIN_CUP_WEIGHT)
	{
		
		TimeOutEmpty ++ ;
		//printf("Empty, weight =  %d TimeOutEmpty = %d \r\n",Weight,TimeOutEmpty);
		return;
	}
	else
	{
		TimeOutEmpty = 0;
	}
	

	//printf("Stable, weight =  %d \r\n ",Weight);

	//printf("Weight change  %d \r\n ",abs(Weight - LastWeight));
	if (abs(Weight - LastWeight) > MIN_WEIGHT_DIFF)
	{
		
		printf("Weight changed to %d \r\n",Weight);
		//Pause = false;
		Beep();
		if (Weight < LastWeight) //Reduce
		{
			printf("Weight changed. Reduce %d \r\n",LastWeight - Weight);
			AccumulatedDrinking = AccumulatedDrinking + (LastWeight - Weight);
			TimeOutForDrink = 0;
		} 
		else
		{
			printf("Weight changed. Added \r\n");
		}
		LastWeight = Weight;
	} 

}

void ShowState()
{
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
		Displayer.ShowASCII816(8,4,Weight/100%10);
		Displayer.ShowASCII816(8*2,4,Weight/10%10);
		Displayer.ShowASCII816(8*3,4,Weight%10);
	}


	Displayer.ShowASCII1632(16*4,4,AccumulatedDrinking/1000%10);
	Displayer.ShowASCII1632(16*5,4,AccumulatedDrinking/100%10);
	Displayer.ShowASCII1632(16*6,4,AccumulatedDrinking/10%10);
	Displayer.ShowASCII1632(16*7,4,AccumulatedDrinking%10);


	Displayer.ShowASCII1632(16*2,0,TimeOutForDrink/60/100%10);
	Displayer.ShowASCII1632(16*3,0,TimeOutForDrink/60/10%10);
	Displayer.ShowASCII1632(16*4,0,TimeOutForDrink/60%10);
	Displayer.ShowASCII1632(16*5,0,11);
	Displayer.ShowASCII1632(16*6,0,TimeOutForDrink%60/10%10);
	Displayer.ShowASCII1632(16*7,0,TimeOutForDrink%60%10);

}

void Beep()
{
	digitalWrite(BUZZ, HIGH);
	delay(50);
	digitalWrite(BUZZ, LOW);
}

void Beep3()
{
	digitalWrite(BUZZ, HIGH);
	delay(50);
	digitalWrite(BUZZ, LOW);
	delay(50);
	digitalWrite(BUZZ, HIGH);
	delay(50);
	digitalWrite(BUZZ, LOW);
	delay(50);
	digitalWrite(BUZZ, HIGH);
	delay(50);
	digitalWrite(BUZZ, LOW);
}
