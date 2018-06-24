
#define BUZZ 9
#define RELAY_OPEN 10
#define RELAY_CLOSE 6

#define KEY_OPEN 7
#define KEY_CLOSE 8

#define POSTION_OPEN 11
#define POSTION_CLOSE 12


#define KEY_PRESS_MIN 2

#include <printf.h>

unsigned long TenthSecondsSinceStart;
bool OpeningDoor = false;
bool ClosingDoor = false;
bool Stop = false;



void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OpenDoor();
void CloseDoor();
void OpenRelay(bool on);
void CloseRelay(bool on);

void setup()
{

	pinMode(BUZZ, OUTPUT);
	pinMode(RELAY_OPEN, OUTPUT);
	pinMode(RELAY_CLOSE, OUTPUT);

	digitalWrite(RELAY_OPEN,HIGH);
	digitalWrite(RELAY_CLOSE,HIGH);

	pinMode(KEY_OPEN, INPUT_PULLUP);
	pinMode(KEY_CLOSE, INPUT_PULLUP);
	pinMode(POSTION_OPEN, INPUT_PULLUP);
	pinMode(POSTION_CLOSE, INPUT_PULLUP);


	Serial.begin(115200);
	Serial.println(F("SlidingDoor"));
	printf_begin();

}

void loop()
{

	TenthSecondsSinceStartTask();
	OpenDoor();
	CloseDoor();


} // Loop


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

		//printf("postion = %d \r\n",digitalRead(POSTION_OPEN));

		//digitalWrite(RELAY_OPEN,digitalRead(POSTION_OPEN));
		//digitalWrite(RELAY_CLOSE,digitalRead(POSTION_CLOSE));
	}
}

void OnTenthSecond()
{
	static unsigned char KeyOpenCounter = 0;
	static unsigned char KeyCloseCounter = 0;

	if ((digitalRead(KEY_OPEN)))
	{

		KeyOpenCounter++;
		if (KeyOpenCounter > KEY_PRESS_MIN)
		{
			KeyOpenCounter = 0;
			if (ClosingDoor)
			{
				Stop = true;
				printf("Stop! \r\n");
			} 
			else
			{
				if (digitalRead(POSTION_OPEN))
				{

					OpeningDoor = true;
					printf("OpenDoor! Key press!  \r\n");
				}
				else
				{
					printf("OpenDoor Key press! But already opened \r\n");
				}
			}
		}
	}
	else
	{
		KeyOpenCounter = 0;

	}

	if ((digitalRead(KEY_CLOSE)))
	{

		KeyCloseCounter++;
		if (KeyCloseCounter > KEY_PRESS_MIN)
		{
			KeyCloseCounter = 0;
			if (OpeningDoor)
			{
				Stop = true;
				printf("Stop! \r\n");
			} 
			else
			{
				if (digitalRead(POSTION_CLOSE))
				{
					ClosingDoor = true;
					printf("CloseDoor Key press! \r\n");
				}
				else
				{
					printf("CloseDoor Key press! But already closed \r\n");
				}

			}
		}
	}
	else
	{
		KeyCloseCounter = 0;

	}
}

void OpenDoor()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (OpeningDoor)
	{
		if (!Stop)
		{
			if (Step == 0)
			{
				Step = 1;
				StartTime = TenthSecondsSinceStart;
				OpenRelay(true);
			}
			if (Step == 1)
			{
				if(TenthSecondsSinceStart -StartTime > 90)
				{
					OpenRelay(false);
					Step = 2;

					Step = 0;
					OpeningDoor = false;
					printf("Open door finished.\r\n");
				}
			}
			//if (Step == 2)
			//{
			//	if(TenthSecondsSinceStart -StartTime >4 )
			//	{
			//		OpenRelay(true);
			//		Step = 3;
			//	}
			//}
			//if (Step == 3)
			//{
			//	if(TenthSecondsSinceStart -StartTime >8 )
			//	{
			//		OpenRelay(false);
			//		Step = 0;
			//		OpeningDoor = false;
			//		printf("Open door finished.\r\n");
			//	}
			//}

		}
		else
		{
			OpenRelay(false);
			Stop = false;
			Step = 0;
			OpeningDoor = false;
			printf("Open door canceled.\r\n");
		}
	} 

}

void CloseDoor()
{
	static unsigned char Step = 0;
	static unsigned long StartTime = 0;
	if (ClosingDoor)
	{
		if (!Stop)
		{
			if (Step == 0)
			{
				Step = 1;
				StartTime = TenthSecondsSinceStart;
				CloseRelay(true);
			}
			if (Step == 1)
			{
				if(TenthSecondsSinceStart -StartTime > 90)
				{
					CloseRelay(false);
					Step = 2;

					Step = 0;
					ClosingDoor = false;
					printf("Close door finished.\r\n");
				}
			}
			//if (Step == 2)
			//{
			//	if(TenthSecondsSinceStart -StartTime >4 )
			//	{
			//		CloseRelay(true);
			//		Step = 3;
			//	}
			//}
			//if (Step == 3)
			//{
			//	if(TenthSecondsSinceStart -StartTime >8 )
			//	{
			//		CloseRelay(false);
			//		Step = 0;
			//		ClosingDoor = false;
			//		printf("Close door finished.\r\n");
			//	}
			//}

		}
		else
		{
			CloseRelay(false);
			Stop = false;
			Step = 0;
			ClosingDoor = false;
			printf("Close door canceled.\r\n");
		}
	} 


}

void OpenRelay(bool on)
{
	digitalWrite(RELAY_OPEN,!on);
	digitalWrite(BUZZ,on);
	printf("Open Relay to %d .\r\n",on);
}
void CloseRelay(bool on)
{
	digitalWrite(RELAY_CLOSE,!on);
	digitalWrite(BUZZ,on);
	printf("Close Relay to %d .\r\n",on);
}