

#include <printf.h>
ICACHE_RAM_ATTR void blink();
void SecondsSinceStartTask();
void OnTenthSecond();

unsigned long PowerTimeDiff;
unsigned long LastPowerTime = 0;
bool GotPower = false;
unsigned long Power;

void setup()
{
	Serial.begin(115200);
	Serial.println(F("test_baro"));
	printf_begin();


	//used for RX

	pinMode(2, INPUT_PULLUP);
	//attachInterrupt(0, blink, RISING);


}

void loop()
{
	SecondsSinceStartTask();
}

unsigned long LastMillis = 0;
unsigned long TenthSecondsSinceStart = 0;
void SecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 100)
	{
		LastMillis = CurrentMillis;
		TenthSecondsSinceStart++;
		OnTenthSecond();
		//printf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
		//printf("LastTagGetTime = %d \r\n",LastTagGetTime);
	}
}

void OnTenthSecond()
{
	if (TenthSecondsSinceStart%10 == 0)
	{
		if (GotPower)
		{
			GotPower = false;
			Power = 36000000/16/PowerTimeDiff;
			printf("TimeDiff = %ld power = %ld \r\n",PowerTimeDiff,Power);
		}
		else
		{
			unsigned long CurrentDiff = millis()-LastPowerTime;
			if (PowerTimeDiff < CurrentDiff)
			{
				Power = 36000000/16/CurrentDiff;
				printf("ruduce power to = %ld \r\n",Power);
			}
		}
	}
}



ICACHE_RAM_ATTR void blink()//ÖÐ¶Ïº¯Êý
{


	unsigned long ThisPowerTime = millis();

	if (LastPowerTime == 0)
	{
		LastPowerTime = ThisPowerTime;
		return;
	}
	else
	{
		unsigned long ThisPowerDiff = ThisPowerTime - LastPowerTime;
		if (ThisPowerDiff > 10)
		{
			PowerTimeDiff = ThisPowerTime - LastPowerTime;
			LastPowerTime = ThisPowerTime;
			GotPower = true;
		}

	}


}