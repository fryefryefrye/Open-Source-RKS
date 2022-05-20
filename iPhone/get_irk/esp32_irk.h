
/*
This example code is in the Public Domain (or CC0 licensed, at your option.)
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.
*/

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

//
//char* esp_key_type_to_str(esp_ble_key_type_t key_type) {
//	char* key_str = nullptr;
//	switch (key_type) {
//		case ESP_LE_KEY_NONE:
//			key_str = (char*) "ESP_LE_KEY_NONE";
//			break;
//		case ESP_LE_KEY_PENC:
//			key_str = (char*) "ESP_LE_KEY_PENC";
//			break;
//		case ESP_LE_KEY_PID:
//			key_str = (char*) "ESP_LE_KEY_PID";
//			break;
//		case ESP_LE_KEY_PCSRK:
//			key_str = (char*) "ESP_LE_KEY_PCSRK";
//			break;
//		case ESP_LE_KEY_PLK:
//			key_str = (char*) "ESP_LE_KEY_PLK";
//			break;
//		case ESP_LE_KEY_LLK:
//			key_str = (char*) "ESP_LE_KEY_LLK";
//			break;
//		case ESP_LE_KEY_LENC:
//			key_str = (char*) "ESP_LE_KEY_LENC";
//			break;
//		case ESP_LE_KEY_LID:
//			key_str = (char*) "ESP_LE_KEY_LID";
//			break;
//		case ESP_LE_KEY_LCSRK:
//			key_str = (char*) "ESP_LE_KEY_LCSRK";
//			break;
//		default:
//			key_str = (char*) "INVALID BLE KEY TYPE";
//			break;
//	}
//	return key_str;
//} // esp_key_type_to_str


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

void BT_Init();



/*
This example code is in the Public Domain (or CC0 licensed, at your option.)
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
* DEFINES
****************************************************************************************
*/

#define HRPS_HT_MEAS_MAX_LEN            (13)

#define HRPS_MANDATORY_MASK             (0x0F)
#define HRPS_BODY_SENSOR_LOC_MASK       (0x30)
#define HRPS_HR_CTNL_PT_MASK            (0xC0)


///Attributes State Machine
enum
{
	HRS_IDX_SVC,

	HRS_IDX_HR_MEAS_CHAR,
	HRS_IDX_HR_MEAS_VAL,
	HRS_IDX_HR_MEAS_NTF_CFG,

	HRS_IDX_BOBY_SENSOR_LOC_CHAR,
	HRS_IDX_BOBY_SENSOR_LOC_VAL,

	HRS_IDX_HR_CTNL_PT_CHAR,
	HRS_IDX_HR_CTNL_PT_VAL,

	HRS_IDX_NB,
};


#define GATTS_TABLE_TAG "SEC_GATTS_DEMO"

#define HEART_PROFILE_NUM                         1
#define HEART_PROFILE_APP_IDX                     0
#define ESP_HEART_RATE_APP_ID                     0x55
#define EXAMPLE_DEVICE_NAME                       "ESP_BLE_SECURITY"
#define HEART_RATE_SVC_INST_ID                    0

#define GATTS_DEMO_CHAR_VAL_LEN_MAX               0x40

#define ADV_CONFIG_FLAG                           (1 << 0)
#define SCAN_RSP_CONFIG_FLAG                      (1 << 1)

static uint8_t adv_config_done = 0;

static uint16_t heart_rate_handle_table[HRS_IDX_NB];

static uint8_t test_manufacturer[3]={'E', 'S', 'P'};

static uint8_t sec_service_uuid[16] = {
	/* LSB <--------------------------------------------------------------------------------> MSB */
	//first uuid, 16bit, [12],[13] is the value
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x18, 0x0D, 0x00, 0x00,
};

// config adv data
//static 
esp_ble_adv_data_t heart_rate_adv_config; 

// config scan response data
//static 
esp_ble_adv_data_t heart_rate_scan_rsp_config;

//static 
esp_ble_adv_params_t heart_rate_adv_params;

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

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
										esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst heart_rate_profile_tab[HEART_PROFILE_NUM] = {
	[HEART_PROFILE_APP_IDX] = {
		.gatts_cb = gatts_profile_event_handler,
		.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
	},

};

/*
*  Heart Rate PROFILE ATTRIBUTES
****************************************************************************************
*/

/// Heart Rate Sensor Service
static const uint16_t heart_rate_svc = ESP_GATT_UUID_HEART_RATE_SVC;

#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))
static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_READ;

/// Heart Rate Sensor Service - Heart Rate Measurement Characteristic, notify
static const uint16_t heart_rate_meas_uuid = ESP_GATT_HEART_RATE_MEAS;
static const uint8_t heart_measurement_ccc[2] ={ 0x00, 0x00};


/// Heart Rate Sensor Service -Body Sensor Location characteristic, read
static const uint16_t body_sensor_location_uuid = ESP_GATT_BODY_SENSOR_LOCATION;
static const uint8_t body_sensor_loc_val[1] = {0x00};


/// Heart Rate Sensor Service - Heart Rate Control Point characteristic, write&read
static const uint16_t heart_rate_ctrl_point = ESP_GATT_HEART_RATE_CNTL_POINT;
static const uint8_t heart_ctrl_point[1] = {0x00};

/// Full HRS Database Description - Used to add attributes into the database
static const esp_gatts_attr_db_t heart_rate_gatt_db[HRS_IDX_NB] =
{
	// Heart Rate Service Declaration
	[HRS_IDX_SVC]                    =
	{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
	sizeof(uint16_t), sizeof(heart_rate_svc), (uint8_t *)&heart_rate_svc}},

	// Heart Rate Measurement Characteristic Declaration
	[HRS_IDX_HR_MEAS_CHAR]            =
	{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
	CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_notify}},

	// Heart Rate Measurement Characteristic Value
	[HRS_IDX_HR_MEAS_VAL]             =
	{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&heart_rate_meas_uuid, ESP_GATT_PERM_READ,
	HRPS_HT_MEAS_MAX_LEN,0, NULL}},

	// Heart Rate Measurement Characteristic - Client Characteristic Configuration Descriptor
	[HRS_IDX_HR_MEAS_NTF_CFG]        =
	{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
	sizeof(uint16_t),sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},

	// Body Sensor Location Characteristic Declaration
	[HRS_IDX_BOBY_SENSOR_LOC_CHAR]  =
	{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
	CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

	// Body Sensor Location Characteristic Value
	[HRS_IDX_BOBY_SENSOR_LOC_VAL]   =
	{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&body_sensor_location_uuid, ESP_GATT_PERM_READ_ENCRYPTED,
	sizeof(uint8_t), sizeof(body_sensor_loc_val), (uint8_t *)body_sensor_loc_val}},

	// Heart Rate Control Point Characteristic Declaration
	[HRS_IDX_HR_CTNL_PT_CHAR]          =
	{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
	CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

	// Heart Rate Control Point Characteristic Value
	[HRS_IDX_HR_CTNL_PT_VAL]             =
	{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&heart_rate_ctrl_point, ESP_GATT_PERM_WRITE_ENCRYPTED|ESP_GATT_PERM_READ_ENCRYPTED,
	sizeof(uint8_t), sizeof(heart_ctrl_point), (uint8_t *)heart_ctrl_point}},
};

static char *esp_key_type_to_str(esp_ble_key_type_t key_type)
{
	char *key_str = NULL;
	switch(key_type) {
case ESP_LE_KEY_NONE:
	key_str = "ESP_LE_KEY_NONE";
	break;
case ESP_LE_KEY_PENC:
	key_str = "ESP_LE_KEY_PENC";
	break;
case ESP_LE_KEY_PID:
	key_str = "ESP_LE_KEY_PID";
	break;
case ESP_LE_KEY_PCSRK:
	key_str = "ESP_LE_KEY_PCSRK";
	break;
case ESP_LE_KEY_PLK:
	key_str = "ESP_LE_KEY_PLK";
	break;
case ESP_LE_KEY_LLK:
	key_str = "ESP_LE_KEY_LLK";
	break;
case ESP_LE_KEY_LENC:
	key_str = "ESP_LE_KEY_LENC";
	break;
case ESP_LE_KEY_LID:
	key_str = "ESP_LE_KEY_LID";
	break;
case ESP_LE_KEY_LCSRK:
	key_str = "ESP_LE_KEY_LCSRK";
	break;
default:
	key_str = "INVALID BLE KEY TYPE";
	break;

	}

	return key_str;
}

static char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req)
{
	char *auth_str = NULL;
	switch(auth_req) {
case ESP_LE_AUTH_NO_BOND:
	auth_str = "ESP_LE_AUTH_NO_BOND";
	break;
case ESP_LE_AUTH_BOND:
	auth_str = "ESP_LE_AUTH_BOND";
	break;
case ESP_LE_AUTH_REQ_MITM:
	auth_str = "ESP_LE_AUTH_REQ_MITM";
	break;
	//case ESP_LE_AUTH_REQ_BOND_MITM:
	//    auth_str = "ESP_LE_AUTH_REQ_BOND_MITM";
	//    break;
case ESP_LE_AUTH_REQ_SC_ONLY:
	auth_str = "ESP_LE_AUTH_REQ_SC_ONLY";
	break;
case ESP_LE_AUTH_REQ_SC_BOND:
	auth_str = "ESP_LE_AUTH_REQ_SC_BOND";
	break;
case ESP_LE_AUTH_REQ_SC_MITM:
	auth_str = "ESP_LE_AUTH_REQ_SC_MITM";
	break;
case ESP_LE_AUTH_REQ_SC_MITM_BOND:
	auth_str = "ESP_LE_AUTH_REQ_SC_MITM_BOND";
	break;
default:
	auth_str = "INVALID BLE AUTH REQ";
	break;
	}

	return auth_str;
}

static void show_bonded_devices(void)
{
	int dev_num = esp_ble_get_bond_device_num();

	esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
	esp_ble_get_bond_device_list(&dev_num, dev_list);
	ESP_LOGI(GATTS_TABLE_TAG, "Bonded devices list : %d", dev_num);
	for (int i = 0; i < dev_num; i++) 
	{
		esp_log_buffer_hex(GATTS_TABLE_TAG, (void *)dev_list[i].bd_addr, sizeof(esp_bd_addr_t));

		printf("Add:");
		for(unsigned char j = 0; j<sizeof(esp_bd_addr_t); j++)
		{
			printf("%02X",*(((byte*)(dev_list[i].bd_addr))+j));
		}
		printf("\r\n");

		printf("IRK:");
		for(unsigned char j = 0; j<sizeof(esp_bt_octet16_t); j++)
		{
			printf(",0x%02X",*(((byte*)(dev_list[i].bond_key.pid_key.irk))+j));
		}
		printf("\r\n");

	}

	free(dev_list);
}

static void __attribute__((unused)) remove_all_bonded_devices(void)
{
	int dev_num = esp_ble_get_bond_device_num();

	esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
	esp_ble_get_bond_device_list(&dev_num, dev_list);
	for (int i = 0; i < dev_num; i++) {
		esp_ble_remove_bond_device(dev_list[i].bd_addr);
	}

	free(dev_list);
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	esp_err_t ret;

	ESP_LOGV(GATTS_TABLE_TAG, "GAP_EVT, event:%d %s", event,gapEventToString(event));
	//ESP_LOGV(GATTS_TABLE_TAG, "GAP_EVT, event:%d", event);

	switch (event) 
	{
	case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
		adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
		if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&heart_rate_adv_params);
		}
		break;
	case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
		adv_config_done &= (~ADV_CONFIG_FLAG);
		if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&heart_rate_adv_params);
		}
		break;
	case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
		//advertising start complete event to indicate advertising start successfully or failed
		if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(GATTS_TABLE_TAG, "advertising start failed, error status = %x", param->adv_start_cmpl.status);
			break;
		}
		ESP_LOGI(GATTS_TABLE_TAG, "advertising start success");
		break;
	case ESP_GAP_BLE_PASSKEY_REQ_EVT:                           /* passkey request event */
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
		/* Call the following function to input the passkey which is displayed on the remote device */
		//esp_ble_passkey_reply(heart_rate_profile_tab[HEART_PROFILE_APP_IDX].remote_bda, true, 0x00);
		break;
		//case ESP_GAP_BLE_OOB_REQ_EVT: {
		//    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
		//    uint8_t tk[16] = {1}; //If you paired with OOB, both devices need to use the same tk
		//    esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk, sizeof(tk));
		//    break;
		//}
	case ESP_GAP_BLE_LOCAL_IR_EVT:                               /* BLE local IR event */
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
		break;
	case ESP_GAP_BLE_LOCAL_ER_EVT:                               /* BLE local ER event */
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
		break;
	case ESP_GAP_BLE_NC_REQ_EVT:
		/* The app will receive this evt when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
		show the passkey number to the user to confirm it with the number displayed by peer device. */
		esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%d", param->ble_security.key_notif.passkey);
		break;
	case ESP_GAP_BLE_SEC_REQ_EVT:
		/* send the positive(true) security response to the peer device to accept the security request.
		If not accept the security request, should send the security response with negative(false) accept value*/
		esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
		break;
	case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:  ///the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
		///show the passkey number to the user to input it in the peer device.
		ESP_LOGI(GATTS_TABLE_TAG, "The passkey Notify number:%06d", param->ble_security.key_notif.passkey);
		break;
	case ESP_GAP_BLE_KEY_EVT:
		//shows the ble key info share with peer device to the user.
		ESP_LOGI(GATTS_TABLE_TAG, "key type = %s", esp_key_type_to_str(param->ble_security.ble_key.key_type));
		break;
	case ESP_GAP_BLE_AUTH_CMPL_EVT: 
		esp_bd_addr_t bd_addr;
		memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
		ESP_LOGI(GATTS_TABLE_TAG, "remote BD_ADDR: %08x%04x",\
			(bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
			(bd_addr[4] << 8) + bd_addr[5]);
		ESP_LOGI(GATTS_TABLE_TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
		ESP_LOGI(GATTS_TABLE_TAG, "pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
		if(!param->ble_security.auth_cmpl.success) {
			ESP_LOGI(GATTS_TABLE_TAG, "fail reason = 0x%x",param->ble_security.auth_cmpl.fail_reason);
		} else {
			ESP_LOGI(GATTS_TABLE_TAG, "auth mode = %s",esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode));
		}
		show_bonded_devices();
		break;

	case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT: 
		ESP_LOGD(GATTS_TABLE_TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT status = %d", param->remove_bond_dev_cmpl.status);
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV");
		ESP_LOGI(GATTS_TABLE_TAG, "-----ESP_GAP_BLE_REMOVE_BOND_DEV----");
		esp_log_buffer_hex(GATTS_TABLE_TAG, (void *)param->remove_bond_dev_cmpl.bd_addr, sizeof(esp_bd_addr_t));
		ESP_LOGI(GATTS_TABLE_TAG, "------------------------------------");
		break;

	case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
		if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS){
			ESP_LOGE(GATTS_TABLE_TAG, "config local privacy failed, error status = %x", param->local_privacy_cmpl.status);
			return;
		}

		ret = esp_ble_gap_config_adv_data(&heart_rate_adv_config);
		if (ret){
			ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
		}else{
			adv_config_done |= ADV_CONFIG_FLAG;
		}

		ret = esp_ble_gap_config_adv_data(&heart_rate_scan_rsp_config);
		if (ret){
			ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
		}else{
			adv_config_done |= SCAN_RSP_CONFIG_FLAG;
		}
		break;

	default:
		ESP_LOGI(GATTS_TABLE_TAG, "event not process");
		break;
	}
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
										esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	ESP_LOGV(GATTS_TABLE_TAG, "event = %d  %s",event,gattServerEventTypeToString(event));
	//ESP_LOGV(GATTS_TABLE_TAG, "event = %x",event);
	switch (event) 
	{
	case ESP_GATTS_REG_EVT:
		esp_ble_gap_set_device_name(EXAMPLE_DEVICE_NAME);
		//generate a resolvable random address
		esp_ble_gap_config_local_privacy(true);
		esp_ble_gatts_create_attr_tab(heart_rate_gatt_db, gatts_if,
			HRS_IDX_NB, HEART_RATE_SVC_INST_ID);
		break;
	case ESP_GATTS_READ_EVT:
		break;
	case ESP_GATTS_WRITE_EVT:
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_WRITE_EVT, write value:");
		esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);
		break;
	case ESP_GATTS_EXEC_WRITE_EVT:
		break;
	case ESP_GATTS_MTU_EVT:
		break;
	case ESP_GATTS_CONF_EVT:
		break;
	case ESP_GATTS_UNREG_EVT:
		break;
	case ESP_GATTS_DELETE_EVT:
		break;
	case ESP_GATTS_START_EVT:
		break;
	case ESP_GATTS_STOP_EVT:
		break;
	case ESP_GATTS_CONNECT_EVT:
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT");
		/* start security connect with peer device when receive the connect event sent by the master */
		esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
		break;
	case ESP_GATTS_DISCONNECT_EVT:
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
		/* start advertising again when missing the connect */
		esp_ble_gap_start_advertising(&heart_rate_adv_params);
		break;
	case ESP_GATTS_OPEN_EVT:
		break;
	case ESP_GATTS_CANCEL_OPEN_EVT:
		break;
	case ESP_GATTS_CLOSE_EVT:
		break;
	case ESP_GATTS_LISTEN_EVT:
		break;
	case ESP_GATTS_CONGEST_EVT:
		break;
	case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
		ESP_LOGI(GATTS_TABLE_TAG, "The number handle = %x",param->add_attr_tab.num_handle);
		if (param->create.status == ESP_GATT_OK){
			if(param->add_attr_tab.num_handle == HRS_IDX_NB) {
				memcpy(heart_rate_handle_table, param->add_attr_tab.handles,
					sizeof(heart_rate_handle_table));
				esp_ble_gatts_start_service(heart_rate_handle_table[HRS_IDX_SVC]);
			}else{
				ESP_LOGE(GATTS_TABLE_TAG, "Create attribute table abnormally, num_handle (%d) doesn't equal to HRS_IDX_NB(%d)",
					param->add_attr_tab.num_handle, HRS_IDX_NB);
			}
		}else{
			ESP_LOGE(GATTS_TABLE_TAG, " Create attribute table failed, error code = %x", param->create.status);
		}
		break;
									   }

	default:
		ESP_LOGI(GATTS_TABLE_TAG, "event not process");
		break;
	}
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
								esp_ble_gatts_cb_param_t *param)
{

	/* If event is register event, store the gatts_if for each profile */
	if (event == ESP_GATTS_REG_EVT) {
		if (param->reg.status == ESP_GATT_OK) {
			heart_rate_profile_tab[HEART_PROFILE_APP_IDX].gatts_if = gatts_if;
		} else {
			ESP_LOGI(GATTS_TABLE_TAG, "Reg app failed, app_id %04x, status %d\n",
				param->reg.app_id,
				param->reg.status);
			return;
		}
	}

	do {
		int idx;
		for (idx = 0; idx < HEART_PROFILE_NUM; idx++) {
			if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
				gatts_if == heart_rate_profile_tab[idx].gatts_if) {
					if (heart_rate_profile_tab[idx].gatts_cb) {
						heart_rate_profile_tab[idx].gatts_cb(event, gatts_if, param);
					}
			}
		}
	} while (0);
}

void setup()
{


	heart_rate_adv_config.set_scan_rsp = false;
	heart_rate_adv_config.include_txpower = true;
	heart_rate_adv_config.min_interval = 0x0006; //slave connection min interval; Time = min_interval * 1.25 msec
	heart_rate_adv_config.max_interval = 0x0010; //slave connection max interval; Time = max_interval * 1.25 msec
	heart_rate_adv_config.appearance = 0x00;
	heart_rate_adv_config.manufacturer_len = 0; //TEST_MANUFACTURER_DATA_LEN;
	heart_rate_adv_config.p_manufacturer_data =  NULL; //&test_manufacturer[0];
	heart_rate_adv_config.service_data_len = 0;
	heart_rate_adv_config.p_service_data = NULL;
	heart_rate_adv_config.service_uuid_len = sizeof(sec_service_uuid);
	heart_rate_adv_config.p_service_uuid = sec_service_uuid;
	heart_rate_adv_config.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);



	heart_rate_scan_rsp_config.set_scan_rsp = true;
	heart_rate_scan_rsp_config.include_name = true;
	heart_rate_scan_rsp_config.manufacturer_len = sizeof(test_manufacturer);
	heart_rate_scan_rsp_config.p_manufacturer_data = test_manufacturer;

	heart_rate_adv_params.adv_int_min        = 0x100;
	heart_rate_adv_params.adv_int_max        = 0x100;
	heart_rate_adv_params.adv_type           = ADV_TYPE_IND;
	heart_rate_adv_params.own_addr_type      = BLE_ADDR_TYPE_RANDOM;
	heart_rate_adv_params.channel_map        = ADV_CHNL_ALL;
	heart_rate_adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;


	BT_Init();

}

void loop() 
{
	delay(1000);
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

	ret = esp_ble_gatts_register_callback(gatts_event_handler);
	if (ret){
		ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
		return;
	}
	ret = esp_ble_gap_register_callback(gap_event_handler);
	if (ret){
		ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
		return;
	}
	ret = esp_ble_gatts_app_register(ESP_HEART_RATE_APP_ID);
	if (ret){
		ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
		return;
	}

	/* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
	esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;     //bonding with peer device after authentication
	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
	uint8_t key_size = 16;      //the key size should be 7~16 bytes
	uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
	uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
	//set static passkey
	uint32_t passkey = 123456;
	uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
	//uint8_t oob_support = ESP_BLE_OOB_DISABLE;
	esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
	//esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
	/* If your BLE device acts as a Slave, the init_key means you hope which types of key of the master should distribute to you,
	and the response key means which key you can distribute to the master;
	If your BLE device acts as a master, the response key means you hope which types of key of the slave should distribute to you,
	and the init key means which key you can distribute to the slave. */
	esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));


	//Delay 1s, clear all the bonded devices
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	remove_all_bonded_devices();
}
