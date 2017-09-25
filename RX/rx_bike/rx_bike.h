
/*************************user modify settings****************************/
byte addresses[6] = { 0x55, 0x56, 0x57, 0x58, 0x59, 0x60 }; // should be same with tx
unsigned char HopCH[3] = { 105, 76, 108 }; //Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define TIME_OUT_TURN_OFF_BIKE 15		//s
#define TIME_OUT_LOCK_WAIT_HOME 60
#define WAIT_KEY_IN_HOME 30
#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 2000				//set lenght of the buzz
#define BUZZOFF 5000			//set interval of the buzz
#define DEGBUG_OUTPUT

#define BUZZ 9
#define RELAY 10

/*****************************************************/


#include <SPI.h>
#include "RF24.h"
#include <printf.h>


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

unsigned char Alarm = 0;

unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long CurrTime = 0;
unsigned long LastChangeCHTime = 0;

unsigned long LastTagGetTime = 0;
unsigned long LastHomeGetTime = 0;
unsigned long LastKeyGetTime = 0; //ms

unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,
unsigned long SecondsSinceStart;

bool LastOn = false;
bool Auto = true;

void SecondsSinceStartTask();
void nRFTask();
void ChHopTask();
void RF_task();
void Buzz_task();
void CheckTime_task();
void OnKeyPress();

void setup()
{

    pinMode(RELAY, OUTPUT);
    pinMode(BUZZ, OUTPUT);

#ifdef DEGBUG_OUTPUT
    Serial.begin(115200);
    Serial.println(F("RF24_Bike_Read"));
    printf_begin();
#endif

    radio.begin();
    radio.setPALevel(RF24_PA_MIN);
    radio.setAddressWidth(5);
    radio.setPayloadSize(4);
    radio.setDataRate(RF24_2MBPS); //RF24_250KBPS  //RF24_2MBPS     //RF24_1MBPS
    radio.setChannel(105);
    radio.setCRCLength(RF24_CRC_8); //RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit

    //Open a writing and reading pipe on each radio, with opposite addresses
    radio.openWritingPipe(addresses);
    radio.openReadingPipe(0, addresses);
    radio.closeReadingPipe(1);
    radio.closeReadingPipe(2);
    radio.closeReadingPipe(3);
    radio.closeReadingPipe(4);
    radio.closeReadingPipe(5);
    radio.setAutoAck(1, false);
    radio.setAutoAck(2, false);
    radio.setAutoAck(3, false);
    radio.setAutoAck(4, false);
    radio.setAutoAck(5, false);
    radio.setAutoAck(0, false);
    //Start the radio listening for data
    radio.startListening();
}

void loop()
{

    SecondsSinceStartTask();
    nRFTask();
    CheckTime_task();
    ChHopTask();
    Buzz_task();


} // Loop

void CheckTime_task()
{

    if (!Auto)
    {
        return;
    }

    if (LastOn)
    {
        if ((SecondsSinceStart - LastTagGetTime == TIME_OUT_TURN_OFF_BIKE/2) && (LastTagGetTime != 0)&&(Alarm == 0))
        {
            Alarm = 3;
        }

        if ((SecondsSinceStart - LastTagGetTime > TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0))
        {
            Alarm = 2;
            digitalWrite(RELAY, LOW);
            LastOn = false;
#ifdef DEGBUG_OUTPUT
            printf("Turn OFF \r\n");
#endif
        }
    }
    else // last OFF
    {
        if ((SecondsSinceStart - LastTagGetTime < TIME_OUT_TURN_OFF_BIKE) && (LastTagGetTime != 0))
        {
            Alarm = 1;

#ifdef DEGBUG_OUTPUT
            printf("Turn ON \r\n");
#endif
            digitalWrite(RELAY, HIGH);
            LastOn = true;
        }
    }
}

void OnKeyPress()
{

    if (LastOn)
    {
        Alarm = 3;
        digitalWrite(RELAY, LOW);
        LastOn = false;
        Auto = false;

#ifdef DEGBUG_OUTPUT
        printf("Turn OFF manually \r\n");
#endif
    }
    else
    {
        Alarm = 1;
        digitalWrite(RELAY, HIGH);
        LastOn = true;
        Auto = true;

#ifdef DEGBUG_OUTPUT
        printf("Turn ON manually \r\n");
#endif
    }
}

void nRFTask()
{
    if (radio.available())
    {
        // Variable for the received timestamp
        while (radio.available())                                     // While there is data ready
        {
            radio.read(GotData, DATA_LENGTH);             // Get the payload
        }

        PackageCounter++;

        Volt = 1.2 * (GotData[DATA_LENGTH - 2] * 256 + GotData[DATA_LENGTH - 1]) * 3 * 1000 / 4096;

        if (GotData[0] == 0)
        {

            LastTagGetTime = SecondsSinceStart;
        }
        else if (GotData[0] == 1)
        {
#ifdef DEGBUG_OUTPUT
            printf("LastHomeGetTime offset = %d \r\n", SecondsSinceStart - LastHomeGetTime);
#endif
            LastHomeGetTime = SecondsSinceStart;
        }
        else if (GotData[0] == 2)
        {
            LastTagGetTime = SecondsSinceStart;
            if (millis() - LastKeyGetTime > 500)
            {
                LastKeyGetTime = millis();
                OnKeyPress();
            }
        }

#ifdef DEGBUG_OUTPUT
        Serial.print(PackageCounter);
        Serial.print(" ");
        Serial.print(F("Get data "));
        for (char i = 0; i < DATA_LENGTH; i++)
        {
            printf("%d,", GotData[i]);
        }
        printf("Volt:%d ", Volt);
        printf("CH:%d\r\n", CurrCH);
#endif
    }

}
void ChHopTask()
{
    if (SecondsSinceStart - LastChangeCHTime > 0)             //RF_HOP every seconds
    {
        CurrCH++;
        if (CurrCH > 2)
        {
            CurrCH = 0;
        }
        LastChangeCHTime = SecondsSinceStart;
        radio.stopListening();
        radio.setChannel(HopCH[CurrCH]);
        radio.startListening();

        //printf("CH change \r\n");
    }
}

unsigned long LastMillis = 0;
void SecondsSinceStartTask()
{
    unsigned long CurrentMillis = millis();
    if (abs(CurrentMillis - LastMillis) > 1000)
    {
        LastMillis = CurrentMillis;
        SecondsSinceStart++;
        //printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
        //printf("LastTagGetTime = %d \r\n",LastTagGetTime);
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
