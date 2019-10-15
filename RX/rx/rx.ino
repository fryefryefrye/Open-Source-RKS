

/*************************user modify settings****************************/
byte addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char  HopCH[3] = {105,76,108};//Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx
#define TIME_OUT_CLOSE_DOOR 5000		//ms
#define DATA_LENGTH 4					//use fixed data length 1-32
#define BUZZON 1000				//set lenght of the buzz
#define BUZZOFF 30000			//set interval of the buzz
/*****************************************************/



#include <SPI.h>
#include "RF24.h"
#include "printf.h"


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/



unsigned long PackageCounter = 0;
unsigned char CurrCH = 0;
unsigned long LastChangeCHTime = 0;
unsigned long LastGetTime = 0;
unsigned long CurrTime = 0;
unsigned char GotData[DATA_LENGTH];
unsigned long Volt;   //unit: mV,

#define DOOR 10
bool DoorOnOff = false;
bool DoorLastOnOff = false;
unsigned char  DoorLastOnID = 0;

#define BUZZ 9
bool BuzzOnOff;
bool AlarmOnOff;



void setup()
{

    pinMode(DOOR, OUTPUT);

    Serial.begin(115200);
    Serial.println(F("RF24_WiFi_ID_Read"));
    printf_begin();



    radio.begin();
    radio.setPALevel(RF24_PA_MIN);
    radio.setAddressWidth(5);
    radio.setPayloadSize(4);
    radio.setDataRate(RF24_2MBPS); //RF24_250KBPS  //RF24_2MBPS     //RF24_1MBPS
    radio.setChannel(105);
    radio.setCRCLength(RF24_CRC_8); //RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit

    //Open a writing and reading pipe on each radio, with opposite addresses
    radio.openWritingPipe(addresses);
    radio.openReadingPipe(0,addresses);
    radio.closeReadingPipe(1);
    radio.closeReadingPipe(2);
    radio.closeReadingPipe(3);
    radio.closeReadingPipe(4);
    radio.closeReadingPipe(5);
    radio.setAutoAck(1,false);
    radio.setAutoAck(2,false);
    radio.setAutoAck(3,false);
    radio.setAutoAck(4,false);
    radio.setAutoAck(5,false);
    radio.setAutoAck(0,false);
    //Start the radio listening for data
    radio.startListening();
}

void loop()
{


    if( radio.available())
    {
        // Variable for the received timestamp
        while (radio.available())                                     // While there is data ready
        {
            radio.read( GotData, sizeof(unsigned long) );             // Get the payload
        }
        Volt=1.2*(GotData[DATA_LENGTH-2]*256+GotData[DATA_LENGTH-1])*3*1000/4096;

        LastGetTime = millis();
        PackageCounter ++;


        Serial.print(PackageCounter);
        Serial.print(" ");
        Serial.print(F("Get data "));
        for(char i=0; i<DATA_LENGTH ; i++)
        {
            printf("%d,",GotData[i]);
        }
        printf("Volt:%d ",Volt);
        printf("CH:%d\r\n",CurrCH);
    }


    CurrTime = millis();


    if (abs(CurrTime - LastChangeCHTime>1000))//RF_HOP
    {
        CurrCH++;
        if (CurrCH>2)
        {
            CurrCH = 0;
        }
        LastChangeCHTime = millis();
        radio.stopListening();
        radio.setChannel(HopCH[CurrCH]);
        radio.startListening();
    }



    if (
        (
            (
                (CurrTime>=LastGetTime)
                ?
                (CurrTime - LastGetTime<TIME_OUT_CLOSE_DOOR)
                :
                ((0xFFFFFFFF-LastGetTime)+CurrTime<TIME_OUT_CLOSE_DOOR)
            )
        )
        &&
        (
            LastGetTime!=0
        )
    )// millis() is stored in a long. it will overflow in 49 days. so a little complex here.

    {
        DoorOnOff = true;
        AlarmOnOff = true;
    }
    else
    {
        DoorOnOff = false;
        AlarmOnOff = false;
    }

    if ( Serial.available() )
    {

    }
    Door_task();
    Buzz_task();
} // Loop

void Door_task()
{
    if (DoorLastOnOff != DoorOnOff)//update relay when door status change
    {
        DoorLastOnOff = DoorOnOff;
        if(DoorOnOff)
        {
            printf("Open door. \r\n");
            digitalWrite(DOOR, HIGH);
        }
        else
        {

            printf("Close door  \r\n");
            digitalWrite(DOOR, LOW);
        }
    }
}


void Buzz_task()
{

    static signed int BuzzOn;
    static signed int BuzzOff;
    if (AlarmOnOff)
    {
        if (BuzzOnOff)
        {
            BuzzOn++;
            if (BuzzOn > BUZZON)
            {
                BuzzOnOff = false;
                BuzzOn = 0;
            }
            else
            {
                BuzzOnOff = true;
            }
        }
        else
        {
            BuzzOff++;
            if (BuzzOff > BUZZOFF)
            {
                BuzzOnOff = true;
                BuzzOff = 0;
            }
            else
            {
                BuzzOnOff = false;
            }
        }
    }
    else
    {
        BuzzOn = 0;
        BuzzOff = 0;
        BuzzOnOff = false;
    }

    if (BuzzOnOff)
    {
        digitalWrite(BUZZ, HIGH);
    }
    else
    {
        digitalWrite(BUZZ, LOW);
    }
}
