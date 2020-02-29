

#include<WiFi.h>
#include<WiFiUdp.h>
#include<ArduinoOTA.h>
#include<time.h>
#define timezone 8

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"

#include "irk.h"

//#include "D:\GitHub\Open-Source-RKS\RX\other\common\bt_names.h"






//#define SCAN_TIME	60     // seconds



const char* ssid = "frye_iot3";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;

#define BLE_KEYLESS
#define KEYLESS
#include "Z:\bt\web\datastruct.h"
tBleKeyLessData BleKeyLessData;
unsigned char DebugLogIndex = 27;
unsigned long TenthSecondsSinceStart = 0;
unsigned long SecondsSinceStart = 0;
uint32_t duration = 60;

unsigned char nRFMac[6] ={0x45,0x46,0x47,0x48,0x59,0x60};
bool isTagDataOK = false;
bool isTagDataProcessing = false;
struct tTagData
{
	unsigned char Code;
	unsigned char Id;
	unsigned char VoltH;
	unsigned char VoltL;
	unsigned char Type;
	long RSSI;
};

tTagData tempTagData;
tTagData TagData;

unsigned long LastTagGotTime[BLE_TYPE_MAX][BLE_NAME_MAX];
unsigned long LastTagSentTime[BLE_TYPE_MAX][BLE_NAME_MAX];



//SimpleKalmanFilter * pKalman[BLE_NAME_NUMBER];


void MyPrintf(const char *fmt, ...);
void NonStopTask();

void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
void SacnBleDevice();
float calcDistByRSSI(int rssi);
void BT_Init();

esp_ble_scan_params_t ble_scan_params;

void setup() 
{       
	delay(50);                      
	Serial.begin(115200);

	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA

	byte mac[6];
	WiFi.softAPmacAddress(mac);
	printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	for (byte i=0;i<6;i++)
	{
		BleKeyLessData.Mac[i] = mac[i];
	}


	for (unsigned char i = 0;i<USB_CHARGE_NUMBER;i++)
	{
		if (memcmp(&BleKeyLessData.Mac[0],&KeyLessStationMacList[i][0],sizeof(unsigned long)*6) == 0)
		{
			DebugLogIndex = 40 + i;
			if (i == 3)
			{
				char* ssid = "frye_iot2";  //Wifi名称
				printf("use ssid frye_iot \r\n");
			} 
			break;
		}
	}

	Serial.print("Is connection routing, please wait");  
	WiFi.begin(ssid, password); //Wifi接入到网络
	Serial.println("\nConnecting to WiFi");
	//如果Wifi状态不是WL_CONNECTED，则表示连接失败
	while (WiFi.status() != WL_CONNECTED) {  
		Serial.print("."); 
		delay(1000);    //延时等待接入网络
	}



	//设置时间格式以及时间服务器的网址
	configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
	Serial.println("\nWaiting for time");
	while (!time(nullptr)) {
		Serial.print(".");
		delay(1000);    
	}
	Serial.println("");


	m_WiFiUDP.begin(5050); 

	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X AP:%s\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],ssid);

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else { // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			Serial.println("Auth Failed");
		} else if (error == OTA_BEGIN_ERROR) {
			Serial.println("Begin Failed");
		} else if (error == OTA_CONNECT_ERROR) {
			Serial.println("Connect Failed");
		} else if (error == OTA_RECEIVE_ERROR) {
			Serial.println("Receive Failed");
		} else if (error == OTA_END_ERROR) {
			Serial.println("End Failed");
		}
	});
	ArduinoOTA.begin();
	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());


	BleKeyLessData.DataType = 20;

	//for(unsigned char i = 0; i<BLE_NAME_NUMBER; i++)
	//{
	//	pKalman[i] = new SimpleKalmanFilter(2, 2, 0.01);
	//}


	ble_scan_params.scan_type              = BLE_SCAN_TYPE_PASSIVE;
	ble_scan_params.own_addr_type          = BLE_ADDR_TYPE_PUBLIC;
	ble_scan_params.scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL;
	ble_scan_params.scan_interval          = 0x50;
	ble_scan_params.scan_window            = 0x30;
	ble_scan_params.scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE;


	BT_Init();


}

void loop() 
{
	NonStopTask();
}

void NonStopTask()
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();

	if ((!isTagDataProcessing)&&(isTagDataOK))
	{
		memcpy(&TagData,&tempTagData,sizeof(tTagData));
		isTagDataOK = false;

		LastTagGotTime[TagData.Type][TagData.Id] = SecondsSinceStart;

		if (SecondsSinceStart-LastTagSentTime[TagData.Type][TagData.Id] > MIN_UPDATE_INTERVAL)
		{
			LastTagSentTime[TagData.Type][TagData.Id] = SecondsSinceStart;
			BleKeyLessData.Triger = true;
			for(unsigned char i = 0;i < 4;i++)
			{
			 	*(((unsigned char *)(&(BleKeyLessData.KeyLessData)))+i) = *(((unsigned char *)(&(TagData)))+3-i);
			}
			BleKeyLessData.Type = TagData.Type;
			BleKeyLessData.RSSI = TagData.RSSI;

			BleKeyLessData.Triger = true;
			m_WiFiUDP.beginPacket("192.168.0.17", 5050);
			m_WiFiUDP.write((const uint8_t*)&BleKeyLessData, sizeof(tBleKeyLessData));
			m_WiFiUDP.endPacket(); 
			printf("Sent Type:%d ID:%d name:%s\r\n"
				,TagData.Type
				,TagData.Id
				,BleDisplayNameList[TagData.Type][TagData.Id]);
		}
		printf("Got Type:%d ID:%d\r\n",TagData.Type,TagData.Id);
	}


	//m_WiFiUDP.parsePacket(); 
	//unsigned int UdpAvailable = m_WiFiUDP.available();
	//if (UdpAvailable == sizeof(tCompressorCommand))
	//{
	//	//MyPrintf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
	//	tCompressorCommand tempCompressorCommand;
	//	m_WiFiUDP.read((char *)&tempCompressorCommand,sizeof(tRoomCommand));

	//	if (tempCompressorCommand.Triger == true)
	//	{
	//		RunningCounter = 60;
	//		MyPrintf("get Compressor trig from control\r\n");
	//	}
	//}

}


unsigned long LastMillis = 0;
void TenthSecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 100)
	{
		LastMillis = CurrentMillis;
		TenthSecondsSinceStart++;
		OnTenthSecond();
		//MyPrintf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
		//MyPrintf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
	}
}

void OnSecond()
{
	SecondsSinceStart++;
	time_t now = time(nullptr); //获取当前时间
	time_str = ctime(&now);
	H1 = time_str[11];
	H2 = time_str[12];
	M1 = time_str[14];
	M2 = time_str[15];
	S1 = time_str[17];
	S2 = time_str[18];
	//printf("%c%c:%c%c:%c%c\n",H1,H2,M1,M2,S1,S2);
	//Serial.printf(time_str);

	struct   tm     *timenow;
	timenow   =   localtime(&now);
	unsigned char Hour = timenow->tm_hour;
	unsigned char Minute = timenow->tm_min;


	

	if (SecondsSinceStart == 2)
	{
		esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
		if (scan_ret){
			ESP_LOGE(GATTC_TAG, "set scan params error, error code = %x", scan_ret);
		};
	}


	if ((SecondsSinceStart%60 == 3)&&(SecondsSinceStart > 3))
	{
		esp_ble_gap_start_scanning(duration);
	}


	//for(unsigned char i = 0; i<BLE_NAME_NUMBER; i++)
	//{
	//	printf("%s:%ds %.3f "
	//		,BleNameList[i].c_str()
	//		,Esp32Data.Timeout[i]
	//		,Esp32Data.KalmanDistance[i]);

	//}
	//printf("\r\n");


	//for(unsigned char i = 0; i<BLE_NAME_NUMBER; i++)
	//{
	//	Esp32Data.Timeout[i] ++;
	//}



	BleKeyLessData.Triger = false;
	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const uint8_t*)&BleKeyLessData, sizeof(tBleKeyLessData));
	m_WiFiUDP.endPacket(); 

}

void OnTenthSecond()
{
	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}
}





#define crc_mul 0x1021  //生成多项式
uint16_t cal_crc(unsigned char *ptr, unsigned char len)
{
	unsigned char i;
	uint16_t crc=0;
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
bool check_crc(unsigned char *ptr, unsigned char stru_len)
{
	uint16_t crc_should = cal_crc(ptr,stru_len-2);
	uint16_t crc_got = *((uint16_t*)(ptr+stru_len-2));
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
		printf("crc checke failed. should:0x%04X recv:0x%04X \n",crc_should,crc_got);
		return false;
	}
}



void SacnBleDevice()
{


	//int count = foundDevices.getCount();

	//printf("found %d Devices\r\n",count);

}

float calcDistByRSSI(int rssi)
{
	int iRssi = abs(rssi);
	float power = (iRssi-59)/(10*2.0);
	return pow(10, power);
}



void MyPrintf(const char *fmt, ...)
{


	static char send_buf[1024];
	char * sprint_buf = send_buf+sizeof(tDebugData);
	tDebugData * pDebugData = (tDebugData*)send_buf;

	int n;
	va_list args;

	va_start(args,fmt);
	n = vsprintf(sprint_buf, fmt, args);
	va_end(args);

	printf(sprint_buf);

	pDebugData->DataType = 3;
	pDebugData->RoomId = DebugLogIndex;
	pDebugData->Length = n;

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const uint8_t*)send_buf, sizeof(tDebugData)+n);
	m_WiFiUDP.endPacket(); 


}


static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	esp_err_t ret;
	uint8_t *adv_name = NULL;
	uint8_t *pManufacturer = NULL;

	uint8_t DataLen = 0;
	esp_ble_gap_cb_param_t *scan_result;

	//ESP_LOGV(GATTS_TABLE_TAG, "GAP_EVT, event:%d %s", event,gapEventToString(event));
	ESP_LOGV(GATTS_TABLE_TAG, "GAP_EVT, event:%d", event);

	switch (event) 
	{
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
        //the unit of the duration is second
        esp_ble_gap_start_scanning(duration);
        break;
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        //scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(__func__, "scan start failed, error status = %x", param->scan_start_cmpl.status);
            break;
        }
        ESP_LOGI(__func__, "scan start success");

        break;

	case ESP_GAP_BLE_SCAN_RESULT_EVT: 
		if (!isTagDataOK)
		{
			scan_result = (esp_ble_gap_cb_param_t *)param;
			isTagDataProcessing = true;
			if(scan_result->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) 
			{
				esp_log_buffer_hex(__func__, scan_result->scan_rst.bda, 6);
				//printf("BD Address 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n"
				//	,scan_result->scan_rst.bda[0]
				//,scan_result->scan_rst.bda[1]
				//,scan_result->scan_rst.bda[2]
				//,scan_result->scan_rst.bda[3]
				//,scan_result->scan_rst.bda[4]
				//,scan_result->scan_rst.bda[5]);

				for (byte i = 0; i < IRK_LIST_NUMBER; i++)
				{
					if(btm_ble_addr_resolvable(scan_result->scan_rst.bda,irk[i]))
					{
						//printf("MacAdd = %02X %02X %02X %02X %02X %02X Found:%s\r\n"
						//,scan_result->scan_rst.bda[0]
						//,scan_result->scan_rst.bda[1]
						//,scan_result->scan_rst.bda[2]
						//,scan_result->scan_rst.bda[3]
						//,scan_result->scan_rst.bda[4]
						//,scan_result->scan_rst.bda[5]
						//,IrkListName[i]);

						tempTagData.Type = BLE_TYPE_IOS;
						tempTagData.Code = 0;
						tempTagData.Id = i;
						tempTagData.VoltH = 0;
						tempTagData.VoltL = 0;
						tempTagData.RSSI = scan_result->scan_rst.rssi;
						isTagDataOK = true;
						isTagDataProcessing = false;
						return;
					}
				}

				ESP_LOGI(__func__, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
				adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,ESP_BLE_AD_TYPE_NAME_CMPL, &DataLen);
				if (DataLen != 0)
				{
					//printf("BD Name:%s \r\n",adv_name);
					for (byte i = 0; i < BLE_NAME_NUMBER; i++)
					{
						if(memcmp(BleNameList[i],adv_name,5) == 0)
						{
							tempTagData.Type = BLE_TYPE_NAME;
							tempTagData.Code = 0;
							tempTagData.Id = i;
							tempTagData.VoltH = 0;
							tempTagData.VoltL = 0;
							tempTagData.RSSI = scan_result->scan_rst.rssi;
							isTagDataOK = true;
							isTagDataProcessing = false;
							return;
						}
					}
				}

				pManufacturer = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE, &DataLen);

				//printf("BD data len:%d 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n"
				//,DataLen
				//,pManufacturer[0]
				//,pManufacturer[1]
				//,pManufacturer[2]
				//,pManufacturer[3]
				//,pManufacturer[4]
				//,pManufacturer[5]);

				if ((DataLen == 6)&&(memcmp(nRFMac,scan_result->scan_rst.bda,6) == 0))
				{
					if(check_crc(pManufacturer,6))
					{
						//printf("BD Address 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n"
						//,scan_result->scan_rst.bda[0]
						//,scan_result->scan_rst.bda[1]
						//,scan_result->scan_rst.bda[2]
						//,scan_result->scan_rst.bda[3]
						//,scan_result->scan_rst.bda[4]
						//,scan_result->scan_rst.bda[5]);

						//printf("BLE_nRF ID:%d volt:%d mv \r\n",pManufacturer[1],pManufacturer[2]*0xFF+pManufacturer[3]);
						tempTagData.Type = BLE_TYPE_NRF;
						tempTagData.Code = pManufacturer[0];
						tempTagData.Id = pManufacturer[1];
						tempTagData.VoltH = pManufacturer[2];
						tempTagData.VoltL = pManufacturer[3];
						tempTagData.RSSI = scan_result->scan_rst.rssi;
						isTagDataOK = true;
						isTagDataProcessing = false;
						return;
					}
				}
			}
		}

		break;
	
	default:
		ESP_LOGI(GATTS_TABLE_TAG, "event not process");
		break;
	}
}


void BT_Init()
{
	ESP_LOGE(GATTS_TABLE_TAG, "GATTS Starting...");

	btStart();

	esp_err_t errRc = ESP_OK;

	if(nvs_flash_init() != ESP_OK)
	{
		printf("nvs_flash_init failed \r\n");
	}  

	esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();

	if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED) {
		if(esp_bluedroid_init() != ESP_OK)
		{
			printf("esp_bluedroid_init failed \r\n");
		}  
	}

	if (bt_state != ESP_BLUEDROID_STATUS_ENABLED) {
		if(esp_bluedroid_enable() != ESP_OK)
		{
			printf("esp_bluedroid_enable failed \r\n");
		}  
	}

	esp_err_t ret;

	ret = esp_ble_gap_register_callback(gap_event_handler);
	if (ret){
		ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
		return;
	}

}