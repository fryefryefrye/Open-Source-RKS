#include <TridentTD_EasyFreeRTOS32.h>
TridentOS   task1;

#include <SimpleKalmanFilter.h>

#define CONFIG_BLE_SMP_ENABLE

#include <SPI.h>
#include "RF24.h"
RF24 radio(2,4);//D5 D6 D7



#include<WiFi.h>
#include<WiFiUdp.h>
//#include<mDNS.h>
#include<ArduinoOTA.h>
//#include<WiFiMulti.h>
#include<time.h>
#define timezone 8




//#include "BLEDevice.h"
//#include "BLEUtils.h"
//#include "BLEScan.h"
//#include "BLEAdvertisedDevice.h"
//#include "BLEBeacon.h"
//BLEAdvertising *pAdvertising;





#define SCAN_TIME	60     // seconds



const char* ssid = "frye_iot";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
tEsp32Data Esp32Data;
unsigned char DebugLogIndex = 27;
unsigned long TenthSecondsSinceStart = 0;
unsigned long SecondsSinceStart = 0;


void MyPrintf(const char *fmt, ...);
void NonStopTask();

void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
void SacnBleDevice();
float calcDistByRSSI(int rssi);

#define CONFIG_BT_ENABLED
#include <esp_gap_bt_api.h>
#include <esp_gap_ble_api.h>
#include <esp_bt.h> 
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gatts_api.h>
#include <esp_gattc_api.h>
#include <nvs_flash.h>
#include <esp_gatt_common_api.h>

#define PROFILE_NUM 2
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1

struct gatts_profile_inst {
	esp_gatts_cb_t gatts_cb;
	uint16_t gatts_if;
	uint16_t app_id;
	uint16_t conn_id;
	uint16_t service_handle;
	esp_gatt_srvc_id_t service_id;
	uint16_t char_handle;
	esp_bt_uuid_t char_uuid;
	esp_gatt_perm_t perm;
	esp_gatt_char_prop_t property;
	uint16_t descr_handle;
	esp_bt_uuid_t descr_uuid;
};

static uint8_t adv_service_uuid128[32] = {
	/* LSB <--------------------------------------------------------------------------------> MSB */
	//first uuid, 16bit, [12],[13] is the value
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
	//second uuid, 32bit, [12], [13], [14], [15] is the value
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};
// The length of adv data must be less than 31 bytes
//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
//adv data
static esp_ble_adv_data_t adv_data = {
	.set_scan_rsp = false,
	.include_name = true,
	.include_txpower = false,
	.min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
	.max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
	.appearance = 0x00,
	.manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
	.p_manufacturer_data =  NULL, //&test_manufacturer[0],
	.service_data_len = 0,
	.p_service_data = NULL,
	.service_uuid_len = sizeof(adv_service_uuid128),
	.p_service_uuid = adv_service_uuid128,
	.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
	.set_scan_rsp = true,
	.include_name = true,
	.include_txpower = true,
	//.min_interval = 0x0006,
	//.max_interval = 0x0010,
	.appearance = 0x00,
	.manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
	.p_manufacturer_data =  NULL, //&test_manufacturer[0],
	.service_data_len = 0,
	.p_service_data = NULL,
	.service_uuid_len = sizeof(adv_service_uuid128),
	.p_service_uuid = adv_service_uuid128,
	.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

///Declare the static function
static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
	[PROFILE_A_APP_ID] = {
		.gatts_cb = gatts_profile_a_event_handler,
		.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
	},
	[PROFILE_B_APP_ID] = {
		.gatts_cb = gatts_profile_b_event_handler,                   /* This demo does not implement, similar as profile A */
		.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
		},
};


static void GapEventHandler(esp_gap_ble_cb_event_t  event,esp_ble_gap_cb_param_t* param);
static void ClientEventHandler(
						esp_gattc_cb_event_t      event,
						esp_gatt_if_t             gattc_if,
						esp_ble_gattc_cb_param_t* evtParam);
static void ServerEventHandler(
						esp_gatts_cb_event_t event, 
						esp_gatt_if_t gatts_if, 
						esp_ble_gatts_cb_param_t *param);

void ESP_BT_Init(); 
void ESP_BT_StartService();

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, 
										  esp_gatt_if_t gatts_if, 
										  esp_ble_gatts_cb_param_t *param) 
{
	printf("gatts_profile_a_event_handler\r\n");

	static uint8_t adv_config_done = 0;
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

	switch (event) {
case ESP_GATTS_REG_EVT:
	ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
	gl_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true;
	gl_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00;
	gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
	gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_A;

	esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name("esp32");
	if (set_dev_name_ret){
		printf("set device name failed, error code = %x\r\n", set_dev_name_ret);
	}
#ifdef CONFIG_SET_RAW_ADV_DATA
	esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
	if (raw_adv_ret){
		ESP_LOGE(GATTS_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
	}
	adv_config_done |= adv_config_flag;
	esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
	if (raw_scan_ret){
		ESP_LOGE(GATTS_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
	}
	adv_config_done |= scan_rsp_config_flag;
#else
	//config adv data
	esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
	if (ret){
		ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
	}
	adv_config_done |= adv_config_flag;
	//config scan response data
	ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
	if (ret){
		ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
	}
	adv_config_done |= scan_rsp_config_flag;

#endif
	esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_A);
	break;
	}
	/*switch (event) {*/
}

static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, 
										  esp_gatt_if_t gatts_if, 
										  esp_ble_gatts_cb_param_t *param) 
{
	printf("gatts_profile_b_event_handler\r\n");
	/*switch (event) {*/
}


void BleTask(void*){
	VOID SETUP(){               
		//Serial.println("Hello Blink Task");  
		//pinMode(2,OUTPUT);
	}

	VOID LOOP() {               
		SacnBleDevice();
	}
}


void setup() 
{       



	delay(50);                      
	Serial.begin(115200);


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
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


	byte mac[6];
	WiFi.softAPmacAddress(mac);
	//printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);




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


	Esp32Data.DataType = 20;


	//task1.start(BleTask);

}

void loop() 
{
	NonStopTask();
}

void NonStopTask()
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();

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


	

	//if (SecondsSinceStart == 10)
	//{
	//	task1.start(BleTask);
	//}






	if (SecondsSinceStart == 5)
	{
		ESP_BT_StartService();
	}

	if (SecondsSinceStart > 30)
	{
		int dev_num = esp_ble_get_bond_device_num();
		printf("bond_device_num:%d\r\n",dev_num);
		esp_ble_bond_dev_t * p_dev_list = new esp_ble_bond_dev_t[dev_num];
		esp_ble_get_bond_device_list(&dev_num,p_dev_list);
		for(unsigned char i = 0; i<dev_num; i++)
		{
			printf("irk:%d\r\n",p_dev_list[i].bond_key.pid_key.irk);
		}
	}



	//Esp32Data.Triger = false;
	//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	//m_WiFiUDP.write((const uint8_t*)&Esp32Data, sizeof(tEsp32Data));
	//m_WiFiUDP.endPacket(); 

}

void OnTenthSecond()
{
	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}
}
char* esp_key_type_to_str(esp_ble_key_type_t key_type) {
	char* key_str = nullptr;
	switch (key_type) {
		case ESP_LE_KEY_NONE:
			key_str = (char*) "ESP_LE_KEY_NONE";
			break;
		case ESP_LE_KEY_PENC:
			key_str = (char*) "ESP_LE_KEY_PENC";
			break;
		case ESP_LE_KEY_PID:
			key_str = (char*) "ESP_LE_KEY_PID";
			break;
		case ESP_LE_KEY_PCSRK:
			key_str = (char*) "ESP_LE_KEY_PCSRK";
			break;
		case ESP_LE_KEY_PLK:
			key_str = (char*) "ESP_LE_KEY_PLK";
			break;
		case ESP_LE_KEY_LLK:
			key_str = (char*) "ESP_LE_KEY_LLK";
			break;
		case ESP_LE_KEY_LENC:
			key_str = (char*) "ESP_LE_KEY_LENC";
			break;
		case ESP_LE_KEY_LID:
			key_str = (char*) "ESP_LE_KEY_LID";
			break;
		case ESP_LE_KEY_LCSRK:
			key_str = (char*) "ESP_LE_KEY_LCSRK";
			break;
		default:
			key_str = (char*) "INVALID BLE KEY TYPE";
			break;
	}
	return key_str;
} // esp_key_type_to_str


const char* gapEventToString(uint32_t eventType) {
	switch (eventType) {
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
			return "ESP_GAP_BLE_ADV_START_COMPLETE_EVT";
		case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:					  /* !< When stop adv complete, the event comes */
			return "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT";
		case ESP_GAP_BLE_AUTH_CMPL_EVT:							  /* Authentication complete indication. */
			return "ESP_GAP_BLE_AUTH_CMPL_EVT";
		case ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT:
			return "ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT";
		case ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT:
			return "ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT";
		case ESP_GAP_BLE_KEY_EVT:									/* BLE  key event for peer device keys */
			return "ESP_GAP_BLE_KEY_EVT";
		case ESP_GAP_BLE_LOCAL_IR_EVT:							   /* BLE local IR event */
			return "ESP_GAP_BLE_LOCAL_IR_EVT";
		case ESP_GAP_BLE_LOCAL_ER_EVT:							   /* BLE local ER event */
			return "ESP_GAP_BLE_LOCAL_ER_EVT";
		case ESP_GAP_BLE_NC_REQ_EVT:								 /* Numeric Comparison request event */
			return "ESP_GAP_BLE_NC_REQ_EVT";
		case ESP_GAP_BLE_OOB_REQ_EVT:								/* OOB request event */
			return "ESP_GAP_BLE_OOB_REQ_EVT";
		case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:						  /* passkey notification event */
			return "ESP_GAP_BLE_PASSKEY_NOTIF_EVT";
		case ESP_GAP_BLE_PASSKEY_REQ_EVT:							/* passkey request event */
			return "ESP_GAP_BLE_PASSKEY_REQ_EVT";
		case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:
			return "ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT";
		case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
			return "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_RESULT_EVT:
			return "ESP_GAP_BLE_SCAN_RESULT_EVT";
		case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_START_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT";
		case ESP_GAP_BLE_SEC_REQ_EVT:								/* BLE  security request */
			return "ESP_GAP_BLE_SEC_REQ_EVT";
		case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
			return "ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT";
		case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
			return "ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT";
		case ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT:
			return "ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT";
		case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
			return "ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT";



		default:
			printf("gapEventToString: Unknown event type %d 0x%.2x\r\n", eventType, eventType);
			return "Unknown event type";
	}
} // gapEventToString
const char* gattServerEventTypeToString(esp_gatts_cb_event_t eventType) {
	switch (eventType) {
		case ESP_GATTS_REG_EVT:
			return "ESP_GATTS_REG_EVT";
		case ESP_GATTS_READ_EVT:
			return "ESP_GATTS_READ_EVT";
		case ESP_GATTS_WRITE_EVT:
			return "ESP_GATTS_WRITE_EVT";
		case ESP_GATTS_EXEC_WRITE_EVT:
			return "ESP_GATTS_EXEC_WRITE_EVT";
		case ESP_GATTS_MTU_EVT:
			return "ESP_GATTS_MTU_EVT";
		case ESP_GATTS_CONF_EVT:
			return "ESP_GATTS_CONF_EVT";
		case ESP_GATTS_UNREG_EVT:
			return "ESP_GATTS_UNREG_EVT";
		case ESP_GATTS_CREATE_EVT:
			return "ESP_GATTS_CREATE_EVT";
		case ESP_GATTS_ADD_INCL_SRVC_EVT:
			return "ESP_GATTS_ADD_INCL_SRVC_EVT";
		case ESP_GATTS_ADD_CHAR_EVT:
			return "ESP_GATTS_ADD_CHAR_EVT";
		case ESP_GATTS_ADD_CHAR_DESCR_EVT:
			return "ESP_GATTS_ADD_CHAR_DESCR_EVT";
		case ESP_GATTS_DELETE_EVT:
			return "ESP_GATTS_DELETE_EVT";
		case ESP_GATTS_START_EVT:
			return "ESP_GATTS_START_EVT";
		case ESP_GATTS_STOP_EVT:
			return "ESP_GATTS_STOP_EVT";
		case ESP_GATTS_CONNECT_EVT:
			return "ESP_GATTS_CONNECT_EVT";
		case ESP_GATTS_DISCONNECT_EVT:
			return "ESP_GATTS_DISCONNECT_EVT";
		case ESP_GATTS_OPEN_EVT:
			return "ESP_GATTS_OPEN_EVT";
		case ESP_GATTS_CANCEL_OPEN_EVT:
			return "ESP_GATTS_CANCEL_OPEN_EVT";
		case ESP_GATTS_CLOSE_EVT:
			return "ESP_GATTS_CLOSE_EVT";
		case ESP_GATTS_LISTEN_EVT:
			return "ESP_GATTS_LISTEN_EVT";
		case ESP_GATTS_CONGEST_EVT:
			return "ESP_GATTS_CONGEST_EVT";
		case ESP_GATTS_RESPONSE_EVT:
			return "ESP_GATTS_RESPONSE_EVT";
		case ESP_GATTS_CREAT_ATTR_TAB_EVT:
			return "ESP_GATTS_CREAT_ATTR_TAB_EVT";
		case ESP_GATTS_SET_ATTR_VAL_EVT:
			return "ESP_GATTS_SET_ATTR_VAL_EVT";
		case ESP_GATTS_SEND_SERVICE_CHANGE_EVT:
			return "ESP_GATTS_SEND_SERVICE_CHANGE_EVT";
		default:
			return "Unknown";
	}
} // gattServerEventTypeToString


const char* gattClientEventTypeToString(esp_gattc_cb_event_t eventType) {
	switch (eventType) {
		case ESP_GATTC_ACL_EVT:
			return "ESP_GATTC_ACL_EVT";
		case ESP_GATTC_ADV_DATA_EVT:
			return "ESP_GATTC_ADV_DATA_EVT";
		case ESP_GATTC_ADV_VSC_EVT:
			return "ESP_GATTC_ADV_VSC_EVT";
		case ESP_GATTC_BTH_SCAN_CFG_EVT:
			return "ESP_GATTC_BTH_SCAN_CFG_EVT";
		case ESP_GATTC_BTH_SCAN_DIS_EVT:
			return "ESP_GATTC_BTH_SCAN_DIS_EVT";
		case ESP_GATTC_BTH_SCAN_ENB_EVT:
			return "ESP_GATTC_BTH_SCAN_ENB_EVT";
		case ESP_GATTC_BTH_SCAN_PARAM_EVT:
			return "ESP_GATTC_BTH_SCAN_PARAM_EVT";
		case ESP_GATTC_BTH_SCAN_RD_EVT:
			return "ESP_GATTC_BTH_SCAN_RD_EVT";
		case ESP_GATTC_BTH_SCAN_THR_EVT:
			return "ESP_GATTC_BTH_SCAN_THR_EVT";
		case ESP_GATTC_CANCEL_OPEN_EVT:
			return "ESP_GATTC_CANCEL_OPEN_EVT";
		case ESP_GATTC_CFG_MTU_EVT:
			return "ESP_GATTC_CFG_MTU_EVT";
		case ESP_GATTC_CLOSE_EVT:
			return "ESP_GATTC_CLOSE_EVT";
		case ESP_GATTC_CONGEST_EVT:
			return "ESP_GATTC_CONGEST_EVT";
		case ESP_GATTC_CONNECT_EVT:
			return "ESP_GATTC_CONNECT_EVT";
		case ESP_GATTC_DISCONNECT_EVT:
			return "ESP_GATTC_DISCONNECT_EVT";
		case ESP_GATTC_ENC_CMPL_CB_EVT:
			return "ESP_GATTC_ENC_CMPL_CB_EVT";
		case ESP_GATTC_EXEC_EVT:
			return "ESP_GATTC_EXEC_EVT";
			//case ESP_GATTC_GET_CHAR_EVT:
			//			return "ESP_GATTC_GET_CHAR_EVT";
			//case ESP_GATTC_GET_DESCR_EVT:
			//			return "ESP_GATTC_GET_DESCR_EVT";
			//case ESP_GATTC_GET_INCL_SRVC_EVT:
			//			return "ESP_GATTC_GET_INCL_SRVC_EVT";
		case ESP_GATTC_MULT_ADV_DATA_EVT:
			return "ESP_GATTC_MULT_ADV_DATA_EVT";
		case ESP_GATTC_MULT_ADV_DIS_EVT:
			return "ESP_GATTC_MULT_ADV_DIS_EVT";
		case ESP_GATTC_MULT_ADV_ENB_EVT:
			return "ESP_GATTC_MULT_ADV_ENB_EVT";
		case ESP_GATTC_MULT_ADV_UPD_EVT:
			return "ESP_GATTC_MULT_ADV_UPD_EVT";
		case ESP_GATTC_NOTIFY_EVT:
			return "ESP_GATTC_NOTIFY_EVT";
		case ESP_GATTC_OPEN_EVT:
			return "ESP_GATTC_OPEN_EVT";
		case ESP_GATTC_PREP_WRITE_EVT:
			return "ESP_GATTC_PREP_WRITE_EVT";
		case ESP_GATTC_READ_CHAR_EVT:
			return "ESP_GATTC_READ_CHAR_EVT";
		case ESP_GATTC_REG_EVT:
			return "ESP_GATTC_REG_EVT";
		case ESP_GATTC_REG_FOR_NOTIFY_EVT:
			return "ESP_GATTC_REG_FOR_NOTIFY_EVT";
		case ESP_GATTC_SCAN_FLT_CFG_EVT:
			return "ESP_GATTC_SCAN_FLT_CFG_EVT";
		case ESP_GATTC_SCAN_FLT_PARAM_EVT:
			return "ESP_GATTC_SCAN_FLT_PARAM_EVT";
		case ESP_GATTC_SCAN_FLT_STATUS_EVT:
			return "ESP_GATTC_SCAN_FLT_STATUS_EVT";
		case ESP_GATTC_SEARCH_CMPL_EVT:
			return "ESP_GATTC_SEARCH_CMPL_EVT";
		case ESP_GATTC_SEARCH_RES_EVT:
			return "ESP_GATTC_SEARCH_RES_EVT";
		case ESP_GATTC_SRVC_CHG_EVT:
			return "ESP_GATTC_SRVC_CHG_EVT";
		case ESP_GATTC_READ_DESCR_EVT:
			return "ESP_GATTC_READ_DESCR_EVT";
		case ESP_GATTC_UNREG_EVT:
			return "ESP_GATTC_UNREG_EVT";
		case ESP_GATTC_UNREG_FOR_NOTIFY_EVT:
			return "ESP_GATTC_UNREG_FOR_NOTIFY_EVT";
		case ESP_GATTC_WRITE_CHAR_EVT:
			return "ESP_GATTC_WRITE_CHAR_EVT";
		case ESP_GATTC_WRITE_DESCR_EVT:
			return "ESP_GATTC_WRITE_DESCR_EVT";
		default:
			log_v("Unknown GATT Client event type: %d", eventType);
			return "Unknown";
	}
} // gattClientEventTypeToString

void GapEventHandler(
								esp_gap_ble_cb_event_t event,
								esp_ble_gap_cb_param_t *param) 
{
	printf("Received a GAP event: %s \r\n",gapEventToString(event));

	switch(event) 
	{
	case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:    
		///the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
		///show the passkey number to the user to input it in the peer device.
		printf("The passkey Notify number:%d\r\n", param->ble_security.key_notif.passkey);
		break;

	case ESP_GAP_BLE_KEY_EVT:
		//shows the ble key info share with peer device to the user.
		printf("key type = %s\r\n", esp_key_type_to_str(param->ble_security.ble_key.key_type));
		break;

	case ESP_GAP_BLE_AUTH_CMPL_EVT: {
		esp_bd_addr_t bd_addr;
		memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr,  
			sizeof(esp_bd_addr_t));
		printf("remote BD_ADDR: %08x%04x\r\n",\
			(bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) +   
			bd_addr[3],  
			(bd_addr[4] << 8) + bd_addr[5]);
		printf("address type = %d\r\n",   
			param->ble_security.auth_cmpl.addr_type);  
		printf("pair status = %s\r\n",  
			param->ble_security.auth_cmpl.success ? "success" : "fail");
		break;

	case ESP_GAP_BLE_SEC_REQ_EVT:
		 /* send the positive (true) security response to the peer device to accept the security request.
		 If not accept the security request, should send the security response with negative(false) accept value*/
		 esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
     break;
	default:
		printf("Gap event not processed \r\n");
		break;

		}
	}
}

void ClientEventHandler(esp_gattc_cb_event_t      event,
						esp_gatt_if_t             gattc_if,
						esp_ble_gattc_cb_param_t* evtParam)
{
	printf("Received a Client event: %s \r\n",gattClientEventTypeToString(event));
	switch(event) 
	{
	case ESP_GATTC_REG_EVT:
		esp_ble_gap_config_local_privacy(true);
		break;
	default:
		printf("Client event not processed \r\n");
		break;

	}
}

void ServerEventHandler(esp_gatts_cb_event_t event, 
						esp_gatt_if_t gatts_if, 
						esp_ble_gatts_cb_param_t *param)
{
	printf("Received a Server event: %s \r\n",gattServerEventTypeToString(event));
	switch(event) 
	{
		case ESP_GATTS_CONNECT_EVT:
			//start security connect with peer device when receive the connect event sent by the master.  
			esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);  
			break;


			/* If event is register event, store the gatts_if for each profile */
		case ESP_GATTS_REG_EVT:
				if (param->reg.status == ESP_GATT_OK) 
				{
					gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
				} 
				else 
				{
					printf("Reg app failed, app_id %04x, status %d\r\n",
						param->reg.app_id,
						param->reg.status);
					//return;
				}
			break;



		default:
			printf("Server event not processed \r\n");
			break;

	}

	do {
		int idx;
		for (idx = 0; idx < PROFILE_NUM; idx++) {
			if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
				gatts_if == gl_profile_tab[idx].gatts_if) {
					if (gl_profile_tab[idx].gatts_cb) {
						gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
					}
			}
		}
	} while (0);
}

void ESP_BT_Init() {

		esp_err_t errRc = ESP_OK;

		if(nvs_flash_init() != ESP_OK)
		{
			printf("nvs_flash_init failed \r\n");
		}  

/*		if(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT) != ESP_OK)
		{
			printf("esp_bt_controller_mem_release failed \r\n");
		} */ 
		esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
		if(esp_bt_controller_init(&bt_cfg) != ESP_OK)
		{
			printf("esp_bt_controller_init failed \r\n");
		}  
		if(esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK)
		{
			printf("esp_bt_controller_enable failed \r\n");
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

		if(esp_ble_gap_register_callback(GapEventHandler) != ESP_OK)
		{
			printf("esp_ble_gap_register_callback failed \r\n");
		}  
		if(esp_ble_gattc_register_callback(ClientEventHandler) != ESP_OK)
		{
			printf("esp_ble_gattc_register_callback failed \r\n");
		}  
		if(esp_ble_gatts_register_callback(ServerEventHandler) != ESP_OK)
		{
			printf("esp_ble_gatts_register_callback failed \r\n");
		}  
		if(esp_ble_gap_set_device_name("ESP32") != ESP_OK)
		{
			printf("esp_ble_gap_set_device_name failed \r\n");
		}  
		esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
		if(esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t)) != ESP_OK)
		{
			printf("esp_ble_gap_set_security_param failed \r\n");
		}  
		delay(1000);


		esp_err_t ret;

		ret = esp_ble_gatts_app_register(PROFILE_A_APP_ID);
		if (ret){
			printf("gatts app register error, error code = %x\r\n", ret);
			return;
		}
		ret = esp_ble_gatts_app_register(PROFILE_B_APP_ID);
		if (ret){
			printf("gatts app register error, error code = %x\r\n", ret);
			return;
		}
		esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
		if (local_mtu_ret){
			printf("set local  MTU failed, error code = %x\r\n", local_mtu_ret);
		}


		//vTaskDelay(200 / portTICK_PERIOD_MS); // Delay for 200 msecs as a workaround to an apparent Arduino environment issue.
} // init



//class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
//{
//	void onResult(BLEAdvertisedDevice advertisedDevice)
//	{
//		BLEAddress nRFBLEAddress("45:46:47:48:59:60");
//		if(advertisedDevice.getAddress().equals(nRFBLEAddress))
//		{
//			//printf("Advertised Device Add:%s Name:%s RSSI:%d \n"
//			//	,advertisedDevice.getAddress().toString().c_str()
//			//	,advertisedDevice.getName().c_str()
//			//	,advertisedDevice.getRSSI()
//
//			//	);
//
//			//Esp32Data.Triger = true;
//			//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
//			//m_WiFiUDP.write((const uint8_t*)&Esp32Data, sizeof(tEsp32Data));
//			//m_WiFiUDP.endPacket();
//
//			Esp32Data.Timeout[0] = 0;
//			Esp32Data.ReadDistance[0] = calcDistByRSSI(advertisedDevice.getRSSI());
//
//			//Esp32Data.Triger = true;
//			//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
//			//m_WiFiUDP.write((const uint8_t*)&Esp32Data, sizeof(tEsp32Data));
//			//m_WiFiUDP.endPacket(); 
//
//			printf("Manufacturer:");
//			for(unsigned char i = 0; i<advertisedDevice.getManufacturerData().length(); i++)
//			{
//				printf("%02X",advertisedDevice.getManufacturerData().c_str()[i]);
//			}
//			printf("\n");
//		}
//
//			
//		if ((advertisedDevice.haveName())&&(advertisedDevice.haveRSSI()))
//		{
//			for(unsigned char i = 0; i<BLE_NAME_NUMBER; i++)
//			{
//				if (BleNameList[i] == advertisedDevice.getName())
//				{
//					//Esp32Data.Timeout[i] = 0;
//					//Esp32Data.ReadDistance[i] = calcDistByRSSI(advertisedDevice.getRSSI());
//					//if (pKalman[i] != NULL)
//					//{
//					//	Esp32Data.KalmanDistance[i] = pKalman[i]->updateEstimate(Esp32Data.ReadDistance[i]);
//					//}
//
//
//						//printf("Kalman:	%f	%f\n"
//						//	,Esp32Data.ReadDistance
//						//	,Esp32Data.KalmanDistance
//						//	);
//						//Esp32Data.Triger = true;
//						//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
//						//m_WiFiUDP.write((const uint8_t*)&Esp32Data, sizeof(tEsp32Data));
//						//m_WiFiUDP.endPacket(); 
//
//
//					//printf("Found DeviceName: %s Distance:%f\n"
//					//	,advertisedDevice.getName().c_str()
//					//	,LastGetDistance[i]
//					//	);
//				}
//			}
//		}
//
//		advertisedDevice.getScan()->clearResults();
//	}
//};


void SacnBleDevice()
{
	//BLEDevice::init("");

	//
	//BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
	//pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	//pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
	//pBLEScan->setInterval(0x50);
	//pBLEScan->setWindow(0x30);

	//BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);

	////int count = foundDevices.getCount();

	////printf("found %d Devices\r\n",count);

}

void ESP_BT_StartService()
{

	printf("ESP_BT_StartService\r\n");

	ESP_BT_Init();

	//esp_bd_addr_t bd_addr;
	//bd_addr[0] = 0xDC;
	//bd_addr[1] = 0x2B;
	//bd_addr[2] = 0x2A;
	//bd_addr[3] = 0x14;
	//bd_addr[4] = 0xC0;
	//bd_addr[5] = 0x61;


	////if(esp_ble_set_encryption(bd_addr,ESP_LE_AUTH_REQ_SC_MITM_BOND) != ESP_OK)
	////{
	////	printf("esp_ble_set_encryption failed \r\n");
	////}

	////if(esp_ble_gap_config_local_privacy(true) != ESP_OK)
	////{
	////	printf("esp_ble_gap_config_local_privacy failed \r\n");
	////}

	////if(esp_ble_gap_security_rsp(bd_addr,true) != ESP_OK)
	////{
	////	printf("esp_ble_gap_security_rsp failed \r\n");
	////}

	//esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;//set the IO capability to No Input No Output
	//esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND; //bonding with peer device after authentication
	//uint8_t key_size = 16;      //the key size should be 7~16 bytes
	//uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
	//uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;

	//esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	//esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	//esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
	//esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
	//esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));


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
