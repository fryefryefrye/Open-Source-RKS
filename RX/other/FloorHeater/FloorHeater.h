




//D0 = GPIO16; 
//D1 = GPIO5; 
//D2 = GPIO4;	LED on esp8266
//D3 = GPIO0;can not download when connected to low
//D4 = GPIO2;	
//D5 = GPIO14;  
//D6 = GPIO12;
//D7 = GPIO13;
//D8 = GPIO15;  can not start when high input
//D9 = GPIO3; UART RX
//D10 = GPIO1; UART TX
//LED_BUILTIN = GPIO16 (auxiliary constant for the board LED, not a board pin);


#define IIC_DAT				D5
#define IIC_CLK				D6

#define RELAY_CLOSE			D2
#define RELAY_OPEN			D1
#define FLOW_RATE			D8
#define TEMPERATURE			A0


//AD for 0 10 20 30 40 50 60 70 80 C
//R = 47k  v=3.3v
unsigned int TemperatureNtcSampling[9] = {752,662,570,475,379,300,234,176,130};
#define TEMPERATURE_AD_MIN 130
#define TEMPERATURE_AD_MAX 752 // Assume 0 C




#define VALVE_OPER_RATE		8//0.1s for every shift
#define VALVE_OPER_DELAY	10 //0.1s
#define VALVE_REST_DELAY	150 //0.1s








#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8


const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 25;
tFloorHeaterData FloorHeaterData;
tFloorHeaterCommand FloorHeaterCommand;
bool ResetingValve = false;
static unsigned long OperationStartTime[FLOOR_HEATER_CHANNEL_NUMBER] = {0};//0.1s



#include <Wire.h>     //The DHT12 uses I2C comunication.
#include "DHT12.h"
DHT12 dht12;          //Preset scale CELSIUS and ID 0x5c.


unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();


void ValveTask(unsigned char ChannelId);
void Flow_INT();
//void OpenValve(unsigned long OperatingTime);
//void CloseValve(unsigned long OperatingTime);
//unsigned long ValveStartTime[FLOOR_HEATER_CHANNEL_NUMBER] = 0;
//unsigned char TargetValveOpeningDegree[FLOOR_HEATER_CHANNEL_NUMBER] = {0};


bool NtcTemp(unsigned long *TempTen,unsigned int Ad);
void MyPrintf(const char *fmt, ...);


void setup() 
{       

	pinMode(RELAY_CLOSE, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY_CLOSE, LOW);
	pinMode(RELAY_OPEN, OUTPUT);//set the pin to be OUTPUT pin.
	digitalWrite(RELAY_OPEN, LOW);


	Wire.begin(IIC_DAT,IIC_CLK);

	//pinMode(KEY,INPUT_PULLUP);

	FloorHeaterData.DataType = 17;

	pinMode(FLOW_RATE, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(FLOW_RATE), Flow_INT, RISING);


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




	byte mac[6];
	WiFi.softAPmacAddress(mac);
	//printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	for (byte i=0;i<6;i++)
	{
		//UsbChargeData.Mac[i] = mac[i];
	}

	//for (unsigned char i = 0;i<ROOM_NUMBER;i++)
	//{
	//	if (memcmp(&RoomData.Mac[0],&RoomMacAddress[i][0],sizeof(unsigned long)*6) == 0)
	//	{
	//		MyPrintf("room ID=%d \r\n",i);
	//		RoomIndex = i;
	//		break;
	//	}
	//}


	m_WiFiUDP.begin(5050); 

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


}

void loop() 
{
	static bool isDataInited = false;
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();

	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tFloorHeaterCommand))
	{
		//MyPrintf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		//printf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		m_WiFiUDP.read((char *)&FloorHeaterCommand,sizeof(tFloorHeaterCommand));
		if (FloorHeaterCommand.ResetValve)
		{
			ResetingValve = true;
			digitalWrite(RELAY_OPEN, HIGH);
			OperationStartTime[0] = TenthSecondsSinceStart;
			MyPrintf("Reset valves, open all\r\n");
		}
		if ((TenthSecondsSinceStart < 200)&&(!isDataInited))
		{
			for(unsigned int i = 0; i<FLOOR_HEATER_CHANNEL_NUMBER; i++)
			{
				FloorHeaterData.Channel[i].OpeningDegree = FloorHeaterCommand.TargetOpeningDegree[i];
			}
			isDataInited = true;
		}
	}
}

void OnTenthSecond()
{

	if (TenthSecondsSinceStart%10 == 0)
	{
		OnSecond();
	}

	for(unsigned int i = 0; i<FLOOR_HEATER_CHANNEL_NUMBER; i++)
	{
		ValveTask(i);
	}
}

void Flow_INT()
{
	FloorHeaterData.Channel[0].FlowRate++;
}

void ValveTask(unsigned char ChannelId)
{
	static signed long OperationDiff[FLOOR_HEATER_CHANNEL_NUMBER] = {0}; //>0 means we should open valve
	static signed long LastOperationDiff[FLOOR_HEATER_CHANNEL_NUMBER] = {0}; //>0 means we should open valve
	static unsigned long ValveOperDelay[FLOOR_HEATER_CHANNEL_NUMBER] = {0};
	signed long ThisOperationDiff = 0;



	//disable valve task
	//all valve on for 30s
	//set actule postion to 100%
	//enable valve task
	if(ResetingValve)
	{
		if (ChannelId == 0)//for test
		{
			if (TenthSecondsSinceStart - OperationStartTime[ChannelId] > VALVE_REST_DELAY)
			{
				MyPrintf("Reset valves, resume valve task\r\n");
				digitalWrite(RELAY_OPEN, LOW);
				FloorHeaterData.Channel[ChannelId].OpeningDegree = 10;
				ResetingValve = false;
				OperationStartTime[ChannelId] = 0;
			}
		}

	}
	else
	{
		if (OperationDiff[ChannelId] == 0)//if Valve not in operating, get new command if we have
		{
			ThisOperationDiff = FloorHeaterCommand.TargetOpeningDegree[ChannelId] - FloorHeaterData.Channel[ChannelId].OpeningDegree;

			if ((ThisOperationDiff!=0)&&(ThisOperationDiff == LastOperationDiff[ChannelId]))
			{
				ValveOperDelay[ChannelId]++;
				printf("OperationDiff found, ValveOperDelay = %d id = %d\r\n",ValveOperDelay[ChannelId],ChannelId);
				if (ValveOperDelay[ChannelId] > VALVE_OPER_DELAY)
				{
					OperationDiff[ChannelId] = FloorHeaterCommand.TargetOpeningDegree[ChannelId] - FloorHeaterData.Channel[ChannelId].OpeningDegree;
					FloorHeaterData.Channel[ChannelId].OpeningDegree = FloorHeaterCommand.TargetOpeningDegree[ChannelId];
					ValveOperDelay[ChannelId] = 0;
					MyPrintf("Operation start,OperationDiff = %d  id = %d\r\n",OperationDiff[ChannelId],ChannelId);
				} 
			}
			else
			{
				ValveOperDelay[ChannelId] = 0;
			}
			LastOperationDiff[ChannelId] = ThisOperationDiff;
		}
		else //Valve in operating
		{
			if (OperationStartTime[ChannelId] == 0)
			{
				if (OperationDiff[ChannelId]>0)
				{
					digitalWrite(RELAY_OPEN, HIGH);
				} 
				else
				{
					digitalWrite(RELAY_CLOSE, HIGH);
				}
				OperationStartTime[ChannelId] = TenthSecondsSinceStart;
			}
			else
			{
				if (TenthSecondsSinceStart - OperationStartTime[ChannelId] > abs(OperationDiff[ChannelId]) * VALVE_OPER_RATE)
				{
					digitalWrite(RELAY_OPEN, LOW);
					digitalWrite(RELAY_CLOSE, LOW);
					OperationStartTime[ChannelId] = 0;
					OperationDiff[ChannelId] = 0;
					MyPrintf("Floor Heater valve Operation finished. id = %d\r\n",ChannelId);
				} 
			}
		}

	}

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


	unsigned long AnalogValue= analogRead(TEMPERATURE);
	NtcTemp(&(FloorHeaterData.Channel[0].Temperature),AnalogValue);
	//FloorHeaterData.Channel[0].Temperature = AnalogValue;//*100/1024;
	//MyPrintf("AnalogValue = %d  \r\n",AnalogValue);
	//MyPrintf("digitalRead(KEY) = %d  \r\n",digitalRead(KEY));




	if(dht12.read() == 0)		
	{
		FloorHeaterData.Channel[0].FlowRate = dht12.LastTemperature*10;
		//MyPrintf("AnalogValue = %d  NTC temp = %d IIC Temp = %d \r\n",AnalogValue,FloorHeaterData.Channel[0].Temperature,FloorHeaterData.Channel[0].FlowRate);
	}
	else
	{

		//MyPrintf("IIC read failed  \r\n");
	}


	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)&FloorHeaterData, sizeof(tFloorHeaterData));
	m_WiFiUDP.endPacket(); 

}

bool NtcTemp(unsigned long *TempTen,unsigned int Ad)
{
	//unsigned int TemperatureNtcSampling[9] = {752,662,570,475,379,300,234,176,130};
	byte i;
	if((Ad>TEMPERATURE_AD_MIN)&&(Ad<TEMPERATURE_AD_MAX))
	{
		for (i=0;i<9;i++)
		{
			if (Ad>TemperatureNtcSampling[i])
			{
				break;
			}
		}

		//MyPrintf("i = %d  \r\n",i);
		
		unsigned long Fact10 = (TemperatureNtcSampling[i-1] - TemperatureNtcSampling[i]);
		//MyPrintf("Fact10 = %d  \r\n",Fact10);
		//MyPrintf("TemperatureNtcSampling[i-1]-Ad = %d  \r\n",TemperatureNtcSampling[i-1]-Ad);
		*TempTen = (i-1)*100+(TemperatureNtcSampling[i-1]-Ad)*100/Fact10;
		return true;
	}
	else
	{
		return false;
	}


	

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
	m_WiFiUDP.write((const char*)send_buf, sizeof(tDebugData)+n);
	m_WiFiUDP.endPacket(); 


}
