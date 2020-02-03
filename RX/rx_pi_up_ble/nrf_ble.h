#include "irk.h"
#include <SPI.h>
#include "RF24.h"
#include "RF24BLE.h"

//pi
//esp_bt_octet16_t irk = {0x54,0x48,0x5F,0x52,0x45,0x51,0x5F,0x53,0x43,0x5F,0x42,0x4F,0x4E,0x44,0x0D,0x0A};

//7inch hua wei
//0x8A,0x1A,0x5B,0x3D,0x8A,0x57,0x42,0xF9,0x75,0x40,0x90,0xF4,0x04,0x15,0x08,0x7A

#define IRK_LIST_NUMBER 3
char * IrkListName[IRK_LIST_NUMBER] = {"HouSH","WangJ","HshPad"};
uint8_t irk[IRK_LIST_NUMBER][ESP_BT_OCTET16_LEN]= 
{
	//hsh iphone
	{0x92,0xE1,0x70,0x7B,0x84,0xDC,0x21,0x4D,0xA6,0x33,0xDC,0x3A,0x3A,0xB2,0x08,0x3F}
	//wang jun iphone
	,{0x17,0x0A,0xE5,0xA7,0xEF,0x8C,0xA8,0xBA,0x06,0xC1,0x54,0xEF,0x9A,0x7A,0x34,0xD0}
	//hshpad
	,{0x2E,0xB7,0xB3,0xD4,0xDC,0x5C,0x16,0x73,0xA7,0x9B,0x75,0x0E,0xEC,0xEB,0x60,0x2D}
};

#define BLE_LIST_NUMBER 3
char * BleListName[BLE_LIST_NUMBER] = {"HouXA","HouXB","HouXC"};


#define MAC_LEN 6
//EXPECTED receive packet length must be same as that of the length of the sent packet
#define RECV_PAYLOAD_SIZE 28

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
RF24BLE BLE(radio);
/**********************************************************/

struct tTagData
{
	//0:nRF_Native
	//1:BLE_nRF 
	//2:BLE_IOS 
	//3:BLE_Name

	unsigned char Code;
	unsigned char Id;
	unsigned char VoltH;
	unsigned char VoltL;
	unsigned char Type;
};


unsigned char nRFCurrCH = 0;
bool isBLE = false;
bool isnRF = false;
//function for public==============
void BLEnRfInit();
void BLEnRFSwitch();
bool BLEnRFDataCheckTask(tTagData * TagData);
//function for public end==============
bool nRFDataCheckTask(tTagData * TagData);
bool BleDataCheckTask(tTagData * TagData);
void InitRfRecv();
void StartnRF();
void StopnRF();
void StartBle();
void StopBle();

bool ParseData(unsigned char * data,unsigned char len,unsigned char code,unsigned char * results);
bool check_crc(unsigned char *ptr, unsigned char stru_len);
unsigned int cal_crc(unsigned char *ptr, unsigned char len);
unsigned char input[32]={0};

bool BLEnRFDataCheckTask(tTagData * TagData)
{
	if (isBLE)
	{
		return BleDataCheckTask(TagData);
	}
	else
	{
		return nRFDataCheckTask(TagData);
	}

	return false;
}


void BLEnRfInit()
{
	InitRfRecv();

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
	radio.closeReadingPipe(0);
	radio.setAddressWidth(5);
	radio.setPayloadSize(4);
	radio.setDataRate(RF24_2MBPS); //RF24_250KBPS  //RF24_2MBPS     //RF24_1MBPS
	radio.setChannel(HopCH[nRFCurrCH]);
	radio.setCRCLength(RF24_CRC_8); //RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit
	radio.openReadingPipe(0, addresses);
	radio.startListening();
}
void StopnRF()
{
	isnRF = false;
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
		//printf("BLEnRFSwitch to nRF \r\n");
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
		//printf("BLEnRFSwitch to Ble \r\n");
		StartBle();
		StopnRF();
	}
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

bool BleDataCheckTask(tTagData * TagData)
{


	byte status=BLE.recvPacket((uint8_t*)input,RECV_PAYLOAD_SIZE);

	unsigned char AdMac[MAC_LEN];
	if(input[0]==0x40)
	{
		for (byte i = 0; i < MAC_LEN; i++)
		{
			AdMac[MAC_LEN-1-i] = input[i+2];
		}

		for (byte i = 0; i < IRK_LIST_NUMBER; i++)
		{
			if(btm_ble_addr_resolvable(AdMac,irk[i]))
			{
				//printf("MacAdd = %02X %02X %02X %02X %02X %02X Found:%s\r\n"
				//	,AdMac[0],AdMac[1],AdMac[2],AdMac[3],AdMac[4],AdMac[5]
				//,IrkListName[i]);
				TagData->Type = 2;
				TagData->Code = 0;
				TagData->Id = i;
				TagData->VoltH = 0;
				TagData->VoltL = 0;
				return true;
			}
		}
		return false;
	}



	unsigned char results[6];

	if(ParseData(input,6,9,results))
	{
		//printf("Name5 = %s!\r\n",results);

		for (byte i = 0; i < BLE_LIST_NUMBER; i++)
		{
			if(memcmp(BleListName[i],results,5) == 0)
			{
				//printf("checked Name5 = %s!\r\n",results);
				TagData->Type = 3;
				TagData->Code = 0;
				TagData->Id = i;
				TagData->VoltH = 0;
				TagData->VoltL = 0;
				return true;
			}
		}
		return false;
	}

	if(ParseData(input,7,0xFF,results))
	{
		//printf("data 6 = 0x%02X 0x%02X 0x%02X 0x%02X!\r\n",results[0],results[1],results[2],results[3]);
		//printf("CRC check = %d \r\n",check_crc(results,6));
		if(check_crc(results,6))
		{
			//printf("volt:%d mv ch:%d time:%d\r\n",results[2]*0xFF+results[3],BLE._recvChannel,SecondsSinceStart);
			//printf("BLE_nRF ID:%d volt:%d mv \r\n",results[1],results[2]*0xFF+results[3]);
			TagData->Code = results[0];
			TagData->Id = results[1];
			TagData->VoltH = results[2];
			TagData->VoltL = results[3];
			TagData->Type = 1;
			return true;
		}
		return false;
	}
	return false;

}

bool nRFDataCheckTask(tTagData * TagData)
{
	unsigned char RemainDataLen = 0;
	unsigned char RemainData;
	unsigned char GotData[DATA_LENGTH];

	if (radio.available())
	{
		radio.read(GotData, DATA_LENGTH);  

		while (radio.available())                                     // While there is data ready
		{
			radio.read(&RemainData, 1); 
			RemainDataLen++;
			printf("RemainData:%d\r\n",RemainDataLen);
			if (RemainDataLen>0)
			{
				BLEnRFSwitch();
				return false;
			}
		}

		
		TagData->Code = GotData[0];
		TagData->Id = GotData[1];
		TagData->VoltH = GotData[2];
		TagData->VoltL = GotData[3];
		TagData->Type = 0;
		return true;

	}
}
