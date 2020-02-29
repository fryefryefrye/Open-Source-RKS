#include <TridentTD_EasyFreeRTOS32.h>
TridentOS   task1;

//#include <SimpleKalmanFilter.h>

#include<WiFi.h>
#include<WiFiUdp.h>
#include<ArduinoOTA.h>
#include<time.h>
#define timezone 8



#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEScan.h"
//#include "BLEAdvertisedDevice.h"
//#include "BLEBeacon.h"






#define SCAN_TIME	60     // seconds



const char* ssid = "frye_iot2";  //Wifi名称
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


//SimpleKalmanFilter * pKalman[BLE_NAME_NUMBER];


void MyPrintf(const char *fmt, ...);
void NonStopTask();

void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
void SacnBleDevice();
float calcDistByRSSI(int rssi);


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
				
			} 
			//else if (i == 1)
			//{
			//	char* ssid = "frye_iot2";  //Wifi名称
			
			//}
			break;
		}
	}
	printf("use ssid %s \r\n",ssid);

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


	

	if (SecondsSinceStart == 10)
	{
		task1.start(BleTask);
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




class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
	void onResult(BLEAdvertisedDevice advertisedDevice)
	{
		printf("Advertised Device Add:%s RSSI:%d \n"
			,advertisedDevice.getAddress().toString().c_str()
			,advertisedDevice.getRSSI()
			);

		BLEAddress nRFBLEAddress("45:46:47:48:59:60");
		if(advertisedDevice.getAddress().equals(nRFBLEAddress))
		{
			//printf("Advertised Device Add:%s Name:%s RSSI:%d \n"
			//	,advertisedDevice.getAddress().toString().c_str()
			//	,advertisedDevice.getName().c_str()
			//	,advertisedDevice.getRSSI()

			//	);

			//Esp32Data.Triger = true;
			//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
			//m_WiFiUDP.write((const uint8_t*)&Esp32Data, sizeof(tEsp32Data));
			//m_WiFiUDP.endPacket();

			//Esp32Data.Timeout[0] = 0;
			//Esp32Data.ReadDistance[0] = calcDistByRSSI(advertisedDevice.getRSSI());

			//Esp32Data.Triger = true;
			//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
			//m_WiFiUDP.write((const uint8_t*)&Esp32Data, sizeof(tEsp32Data));
			//m_WiFiUDP.endPacket(); 

			printf("Manufacturer:");
			for(unsigned char i = 0; i<advertisedDevice.getManufacturerData().length(); i++)
			{
				printf("%02X",advertisedDevice.getManufacturerData().c_str()[i]);
			}
			printf("\n");
		}

			
		if ((advertisedDevice.haveName())&&(advertisedDevice.haveRSSI()))
		{
			for(unsigned char i = 0; i<BLE_NAME_NUMBER; i++)
			{
				if (BleNameList[i] == advertisedDevice.getName())
				{
					//Esp32Data.Timeout[i] = 0;
					//Esp32Data.ReadDistance[i] = calcDistByRSSI(advertisedDevice.getRSSI());
					//if (pKalman[i] != NULL)
					//{
					//	Esp32Data.KalmanDistance[i] = pKalman[i]->updateEstimate(Esp32Data.ReadDistance[i]);
					//}


						//printf("Kalman:	%f	%f\n"
						//	,Esp32Data.ReadDistance
						//	,Esp32Data.KalmanDistance
						//	);
						//Esp32Data.Triger = true;
						//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
						//m_WiFiUDP.write((const uint8_t*)&Esp32Data, sizeof(tEsp32Data));
						//m_WiFiUDP.endPacket(); 


					printf("Found DeviceName: %s \n"
						,advertisedDevice.getName().c_str()
						);
				}
			}
		}

		advertisedDevice.getScan()->clearResults();
	}
};


void SacnBleDevice()
{
	BLEDevice::init("");

	
	BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
	pBLEScan->setInterval(0x50);
	pBLEScan->setWindow(0x30);

	BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);

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
