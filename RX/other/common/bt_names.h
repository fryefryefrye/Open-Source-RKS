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