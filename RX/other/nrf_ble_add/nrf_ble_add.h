byte addresses[6] = {0x55,0x56,0x57,0x58,0x59,0x60};// should be same with tx
unsigned char  HopCH[3] = {105,76,108};//Which RF channel to communicate on, 0-125. We use 3 channels to hop.should be same with tx

#define DATA_LENGTH 4
#define DEGBUG_OUTPUT

#include <SPI.h>
#include "RF24.h"
#include "RF24BLE.h"
#include <printf.h>

//// The MAC address of the beacon
//#define MY_MAC_0  0x60
//#define MY_MAC_1  0x59
//#define MY_MAC_2  0x58
//#define MY_MAC_3  0x57
//#define MY_MAC_4  0x56
//#define MY_MAC_5  0x55
#define MAC_LEN 6
//unsigned char MyMac[MAC_LEN] = {0x60,0x59,0x58,0x57,0x56,0x55};
//unsigned char MyMac[MAC_LEN] = {0x63,0xD2,0xEB,0x43,0x0B,0x5E};
unsigned char MyMac[MAC_LEN] = {0xd0,0x1E ,0x63 ,0x63 ,0x9A ,0x6E};

#define ANDROID		 0x42
#define IPHONE		0x40

//EXPECTED receive packet length must be same as that of the length of the sent packet
#define RECV_PAYLOAD_SIZE 28


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
RF24BLE BLE(radio);
/**********************************************************/

unsigned char nRFCurrCH = 0;
bool isBLE = false;
bool isnRF = false;
void nRFDataCheckTask();
void BleDataCheckTask();
void DataCheckTask();
void BLEnRFSwitch();
void InitRfRecv();
void StartnRF();
void StopnRF();
void StartBle();
void StopBle();

void SendScanReq(unsigned char * Mac);


void SecondsSinceStartTask();
void OnSeconds();
bool ParseData(unsigned char * data,unsigned char len,unsigned char code,unsigned char * results);
unsigned long SecondsSinceStart;
bool check_crc(unsigned char *ptr, unsigned char stru_len);
unsigned int cal_crc(unsigned char *ptr, unsigned char len);

unsigned char input[32]={0};

unsigned char GotData[DATA_LENGTH];
unsigned long PackageCounter = 0;
unsigned int Volt = 0;




void setup()
{



    Serial.begin(115200);
    Serial.println(F("RF24_BLE_Read"));
    printf_begin();

	InitRfRecv();

	//StartnRF();


	//BLE.transmitBegin();
	StartBle();

	radio.disableCRC();
	radio.powerUp();
	radio.setAutoAck(false);
	radio.setAddressWidth(4);
	radio.setRetries(0, 0);
	radio.setDataRate(RF24_1MBPS);
	radio.setPALevel(RF24_PA_MAX);



}

void InitRfRecv()
{


	SPI.begin();
	radio.begin();
	radio.setPALevel(RF24_PA_MIN);


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

}

void StartnRF()
{
	isnRF = true;

	radio.stopListening();
	//printf("stopListening\r\n");
	radio.closeReadingPipe(0);
	//printf("closeReadingPipe\r\n");
	radio.setAddressWidth(5);
	//printf("setAddressWidth\r\n");
	radio.setPayloadSize(4);
	//printf("setPayloadSize\r\n");
	radio.setDataRate(RF24_2MBPS); //RF24_250KBPS  //RF24_2MBPS     //RF24_1MBPS
	//printf("setDataRate\r\n");
	radio.setChannel(HopCH[nRFCurrCH]);
	//printf("setChannel\r\n");
	radio.setCRCLength(RF24_CRC_8); //RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit
	//printf("setCRCLength\r\n");

	//Open a writing and reading pipe on each radio, with opposite addresses
	//radio.openWritingPipe(addresses);
	radio.openReadingPipe(0, addresses);
	//printf("openReadingPipe\r\n");

	radio.startListening();
	//printf("startListening\r\n");

}
void StopnRF()
{
	isnRF = false;
	//radio.stopListening();

}
void StartBle()
{
	isBLE = true;

	BLE.recvRestar(nRFCurrCH);//recv
}
void StopBle()
{
	isBLE = false;
}

void BLEnRFSwitch()
{
	if (isBLE)
	{
		printf("BLEnRFSwitch to nRF \r\n");
		nRFCurrCH++;
		if (nRFCurrCH > 2)
		{
			nRFCurrCH = 0;
		}
		StopBle();
		StartnRF();
	}
	else
	{
		printf("BLEnRFSwitch to Ble \r\n");
		StartBle();
		StopnRF();
	}
}

void loop()
{
	//BLE Sending

	//for (uint8_t channel = 0; channel < 3; channel++){	// Channel hopping do not alter
	//	Serial.println("Start LE advertizing");
	//	BLE.setPhone(ANDROID);
	//	BLE.setMAC(MY_MAC_0, MY_MAC_1, MY_MAC_2, MY_MAC_3, MY_MAC_4, MY_MAC_5);
	//	//BLE.setName("nrfBle");
	//	//unsigned long dataBytes = 0x01020304;
	//	unsigned long dataBytes = 0x04030201;
	//	BLE.setData(&dataBytes, sizeof(dataBytes));
	//	BLE.sendADV(channel);
	//}
	//delay(500);    // Broadcasting interval

	//BLE.printPacket();





	BleDataCheckTask();
	nRFDataCheckTask();

    SecondsSinceStartTask();
    //nRFTask();

} // Loop


bool ParseData(unsigned char * data,unsigned char len,unsigned char code,unsigned char * results)
{
	for (byte i = 0; i < RECV_PAYLOAD_SIZE; i++)
	{
		if (data[i] == len)
		{
			if (data[i+1] == code)
			{
				byte j;
				for (j = 0; j < len; j++)
				{
					results[j] = data[i+j+2];
				}
				results[j-1] = 0;
				return true;
			}
		}
	}
	return false;
}

bool check_crc(unsigned char *ptr, unsigned char stru_len)
{
	unsigned int crc_should = cal_crc(ptr,stru_len-2);
	unsigned int crc_got = *((unsigned int*)(ptr+stru_len-2));
	unsigned char L = crc_got&0xFF;
	unsigned char H = crc_got>>8;
	crc_got = L;
	crc_got = crc_got<<8;
	crc_got = crc_got + H;
	if ( crc_should == crc_got)	
	{
		return true;
	}
	else
	{
		//printf("crc checke failed. should:0x%04X recv:0x%04X \n",crc_should,crc_got);
		return false;
	}
}

#define crc_mul 0x1021  //生成多项式
unsigned int cal_crc(unsigned char *ptr, unsigned char len)
{
	unsigned char i;
	unsigned int crc=0;
	while(len-- != 0)
	{
		for(i=0x80; i!=0; i>>=1)
		{
			if((crc&0x8000)!=0)
			{
				crc<<=1;
				crc^=(crc_mul);
			}else{
				crc<<=1;
			}
			if((*ptr&i)!=0)
			{
				crc ^= (crc_mul);
			}
		}
		ptr ++;
	}
	return (crc);
}



void BleDataCheckTask()
{
	if (!isBLE)
	{
		return;
	}


	//receive shall be done when called
	//single channel receiving only 
	//since every channel will have the same information hence choose the optimum channel as per your envt.
	byte status=BLE.recvPacket((uint8_t*)input,RECV_PAYLOAD_SIZE);
	//if(status==RF24BLE_VALID)
	//{
	//	//printf("VALID:\r\n");
	//}//RF24BLE_VALID=1
	//else if(status==RF24BLE_CORRUPT)
	//{
	//	//printf("CORRUPT:\r\n");
	//}//RF24BLE_CORRUPT =0
	//else if(status==RF24BLE_TIMEOUT)
	//{
	//	//printf("TIMEOUT:\r\n");
	//}//RF24BLE_TIMEOUT =-1


	//for (byte i = 0; i < RECV_PAYLOAD_SIZE; i++){
	//	printf("%02X ",input[i]);
	//}
	//printf("\r\n");




	unsigned char AdMac[MAC_LEN];
	if(input[0]==0x40)
	{
		for (byte i = 0; i < MAC_LEN; i++)
		{
			AdMac[i] = input[i+2];
		}
		//printf("MacAdd = %02X %02X %02X %02X %02X %02X !\r\n",AdMac[0],AdMac[1],AdMac[2],AdMac[3],AdMac[4],AdMac[5]);

		//delay(200);
		SendScanReq(AdMac);
		return;
	}



	unsigned char results[6];
	if(ParseData(input,5,9,results))
	{
		printf("Name4 = %s!\r\n",results);
		//printf("data 4 = %02X %02X %02X %02X %02X %02X !\r\n",results[0],results[1],results[2],results[3],results[4],results[5]);
	}

	if(ParseData(input,6,9,results))
	{
		printf("Name5 = %s!\r\n",results);
	}

	if(ParseData(input,7,0xFF,results))
	{
		//printf("data 6 = 0x%02X 0x%02X 0x%02X 0x%02X!\r\n",results[0],results[1],results[2],results[3]);
		//printf("CRC check = %d \r\n",check_crc(results,6));
		if(check_crc(results,6))
		{
			printf("volt:%d mv ch:%d time:%d\r\n",results[2]*0xFF+results[3],BLE._recvChannel,SecondsSinceStart);
		}
	}

}

void SendScanReq(unsigned char * Mac)
{

	radio.stopListening();


	//for (uint8_t channel = 0; channel < 3; channel++)
	//{
	//	BLE.setPhone(0xC3);//set to scan request
	//	BLE.setRequestMAC(MyMac,Mac);
	//	BLE.sendADV(channel);
	//	delay(1);
	//}
	//BLE.recvChannelRoll();


	BLE.setPhone(0xC3);//set to scan request
	BLE.setRequestMAC(MyMac,Mac);
	BLE.sendADV(BLE._recvChannel);
	delay(1);
	radio.startListening();



	//BLE.printPacket();


}

void nRFDataCheckTask()
{
	unsigned char RemainDataLen = 0;
	unsigned char RemainData;

	if (!isnRF)
	{
		return;
	}


    if (radio.available())
    {
		radio.read(GotData, DATA_LENGTH);  

        while (radio.available())                                     // While there is data ready
        {
			radio.read(&RemainData, 1); 
			RemainDataLen++;
			printf("RemainData:%d\r\n",RemainDataLen);
			if (RemainDataLen>2)
			{
				break;
			}
                       // Get the payload
        }

        PackageCounter++;

        Volt = 1.2 * (GotData[DATA_LENGTH - 2] * 256 + GotData[DATA_LENGTH - 1]) * 3 * 1000 / 4096;

        if (GotData[0] == 0)
        {

            //LastTagGetTime = SecondsSinceStart;
			//if (Alarmed)
			//{
			//	Alarm = 1;
			//}
			
        }
        else if (GotData[0] == 1)
        {
#ifdef DEGBUG_OUTPUT
            //printf("LastHomeGetTime offset = %d \r\n", SecondsSinceStart - LastHomeGetTime);
#endif
            //LastHomeGetTime = SecondsSinceStart;
        }
        else if (GotData[0] == 2)
        {
            //LastTagGetTime = SecondsSinceStart;
            //if (millis() - LastKeyGetTime > 500)
            //{
            //    LastKeyGetTime = millis();
            //    OnKeyPress();
            //}
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
        printf("CH:%d\r\n", nRFCurrCH);
#endif
    }

}
//void ChHopTask()
//{
//	static unsigned char CurrCH = 0;
//	static unsigned long LastChangeCHTime = 0;
//    if (SecondsSinceStart - LastChangeCHTime > 0)             //RF_HOP every seconds
//    {
//        CurrCH++;
//        if (CurrCH > 2)
//        {
//            CurrCH = 0;
//        }
//        LastChangeCHTime = SecondsSinceStart;
//        radio.stopListening();
//        radio.setChannel(HopCH[CurrCH]);
//        radio.startListening();
//
//        //printf("CH change \r\n");
//    }
//}

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
        //printf("LastTagGetTime = %d \r\n",LastTagGetTime);
    }
}


void OnSeconds()
{
	//BLE.recvChannelRoll();
	//BLEnRFSwitch();
}
