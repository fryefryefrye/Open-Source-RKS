

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
#include <printf.h>
#include "TimeLib.h"
//#include "SerialESP8266wifi.h"
#include <EEPROM.h>


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



struct tRecord
{
    unsigned long tag_time = 0;
    unsigned char code = 0;
    unsigned char ID = 0;
    unsigned int volt = 0;
};
tRecord Record;
unsigned char RecordLength = sizeof(Record);
unsigned char RecordCounter = 1024/RecordLength;
unsigned char NextRecord = 0;
time_t t;
unsigned long SecondsSince1970;
//unsigned long RealTimeOffset = 1502202450;
unsigned long RealTimeOffset = 3711269916-2208988800;
unsigned long LastMillis = 0;





void setup()
{

    pinMode(DOOR, OUTPUT);

    Serial.begin(9600);
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


    NextRecord = FindLastRecord();
    
}

void loop()
{

//Record.tag_time++;
//Record.code++;
//Record.ID++;
//Record.volt++;
//WriteRecord(NextRecord,&Record);
//ReadRecord(NextRecord,&Record);
// NextRecord++;


    SecondsSince1970Task();

    //printf("SecondsSince1970 %d \r\n",SecondsSince1970);



    delay(100);










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
           Record.tag_time=SecondsSince1970+RealTimeOffset;
Record.code= GotData[0];
Record.ID =  GotData[1];
Record.volt = Volt;
        InsertRecord(&Record);
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

void InsertRecord(tRecord * pRecord)
{
        WriteRecord(NextRecord,pRecord);
        
      NextRecord++;
      if(NextRecord>=RecordCounter)
      {
        NextRecord = 0;
      }

}


void WriteRecord(unsigned char index,tRecord * pRecord)
{
    unsigned char i;
    for(i=0; i<RecordLength; i++)
    {
        EEPROM.write(index*RecordLength+i,*(((unsigned char *)(pRecord))+i));
    }
    t = pRecord->tag_time;
    printf("Write Record! Index %d. Date Time = %d-%d-%d %d:%d:%d Code:%d. ID:%d. Volt:%d mV\r\n",index,year(t) ,month(t),day(t),hour(t),minute(t),second(t),pRecord->code,pRecord->ID,pRecord->volt);
}

void ReadRecord(unsigned char index,tRecord * pRecord)
{
    unsigned char i;
    for(i=0; i<RecordLength; i++)
    {
        *(((unsigned char *)(pRecord))+i) = EEPROM.read(index*RecordLength+i);
    }

    t = pRecord->tag_time;
    printf("Read Record! Index %03d. Date Time = %d-%02d-%02d %02d:%02d:%02d Code:%d. ID:%d. Volt:%d mV\r\n",index,year(t) ,month(t),day(t),hour(t),minute(t),second(t),pRecord->code,pRecord->ID,pRecord->volt);
}

unsigned char FindLastRecord()
{
  tRecord Record;
    unsigned char i;
    unsigned char MinIndex = 0;
    unsigned long MinTime = 0xFFFFFFFF;
    for(i=0; i<RecordCounter; i++)
    {
        ReadRecord(i,&Record);
        if(MinTime>Record.tag_time)
        {
          MinTime = Record.tag_time;
          MinIndex = i;          
        }
    }

    t = MinTime;
    printf("Min Record index %03d! Date Time = %d-%02d-%02d %02d:%02d:%02d \r\n",MinIndex,year(t) ,month(t),day(t),hour(t),minute(t),second(t));

return MinIndex;
  
}

void SecondsSince1970Task()
{
    unsigned long CurrentMillis = millis();
    if
    (
        (CurrentMillis>=LastMillis)
        ?
        ((CurrentMillis-LastMillis)> 1000)
        :
        ((0xFFFFFFFF-LastMillis)+CurrentMillis>1000)
    )
    {
        LastMillis = (CurrentMillis/1000)*1000;
        SecondsSince1970++;

        t=SecondsSince1970+RealTimeOffset;
        printf("Date Time = %d-%02d-%02d %02d:%02d:%02d \r\n",year(t) ,month(t),day(t),hour(t),minute(t),second(t));

    }
}







