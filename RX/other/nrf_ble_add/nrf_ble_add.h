byte addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char  HopCH[3] = {105,76,108};//Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx

#define DATA_LENGTH 4
#define DEGBUG_OUTPUT


#include <printf.h>

#include "nrf_ble.h"




void SecondsSinceStartTask();
void OnSeconds();
unsigned long SecondsSinceStart;
tTagData TagData;

char * TypeNameList[4] = {"nRF_Native","BLE_nRF","BLE_IOS","BLE_Name"};
//0:nRF_Native
//1:BLE_nRF
//2:BLE_IOS 
//3:BLE_Name



void setup()
{
    Serial.begin(115200);
    Serial.println(F("RF24_BLE_Read"));
    printf_begin();

	BLEnRfInit();

}


void loop()
{


	if(BLEnRFDataCheckTask(&TagData))
	{
		printf("Code:%d ID:%d Volt:%04d Type%d:%s\r\n"
			,TagData.Code
			,TagData.Id
			,TagData.Volt
			,TagData.Type
			,TypeNameList[TagData.Type]);
	}

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
		OnSeconds();
        //printf("SecondsSinceStart = %d \r\n",SecondsSinceStart);
    }
}


void OnSeconds()
{
	BLEnRFSwitch();
}
