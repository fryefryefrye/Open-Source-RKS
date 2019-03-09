

#define SERVER_ADDRESS "192.168.0.17"


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


#define RELAY2				D0
#define RELAY1				D1
#define SOFT_SERIAL_RX		D2
#define IIC_DAT				D3
#define IIC_CLK				D4
//#define AC_POWER | LOCAL_CONTROL			D5
#define MIDEA_AC_RX			D6   
#define MIDEA_AC_TX			D7 


//// pcd board connection
//#define RELAY2				D3
//#define RELAY1				D2
//#define SOFT_SERIAL_RX		D1
//
//
////other
//#define IIC_DAT				D4
//#define IIC_CLK				D0
////#define AC_POWER | LOCAL_CONTROL			D5
//#define MIDEA_AC_RX			D6   
//#define MIDEA_AC_TX			D7 




//#define 		D8 can not start when high input
unsigned char LOCAL_CONTROL;
unsigned char AC_POWER;


//#define RELAY1				D9 //UART RX
//#define RELAY2				D10//UART TX


#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8

//IIC current and volt
//#include <Wire.h>
//#include "Adafruit_INA219.h"
//Adafruit_INA219 ina219_1(INA219_ADDRESS);

#include "ir_Midea.h"
IRMideaAC m_IRMideaAC(D7);


//#include "DHT.h"
//DHT dht(DHTPIN, DHT11);


#include <Wire.h>     //The DHT12 uses I2C comunication.
#include "DHT12.h"
DHT12 dht12;          //Preset scale CELSIUS and ID 0x5c.

#include <SoftwareSerial.h>
SoftwareSerial swSer(SOFT_SERIAL_RX, SOFT_SERIAL_RX, false, 256);
bool IsSoftwareSerialOn = true; 

const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;


#include "Z:\bt\web\datastruct.h"
tRoomData RoomData;

#define RF_COMMAND_DELAY 4
#define RF_COMMAND_LEN 3
#define RF_COMMAND_KEY_COUNTER 5
#define RF_COMMAND_FUNCTION_COUNTER 4
unsigned char RelayPin[LIGHT_NUMBER] = {RELAY1,RELAY2};
unsigned char RoomIndex = 0xFF;


unsigned char PreSetRfCommand[ROOM_NUMBER][RF_COMMAND_FUNCTION_COUNTER][RF_COMMAND_KEY_COUNTER][RF_COMMAND_LEN]
={

	//0x82,0x7D,0x3A,0x69,0x3F,0x48 阳台
	{{{0xC3, 0x7C, 0x68},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x64},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0x6B, 0xE1, 0xA1},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x61},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}}

	//,{0xA2,0x20,0xA6,0x21,0xAF,0x77 Wemos
	,{{{0xC3, 0x7C, 0x68},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x64},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x62},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x61},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}}

	//0x82,0x7D,0x3A,0x3D,0x5F,0x75 南房间
	,{{{0xC3, 0x7C, 0x62},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x61},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	//ac ,number 4 on bed
	,{{0x6B, 0xE1, 0xA8},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}}

	//0x82,0x7D,0x3A,0x3D,0x5F,0xC0 Mini
	,{{{0xC3, 0x7C, 0x62},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x61},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x62},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}
	,{{0xC3, 0x7C, 0x61},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00},{0x00, 0x00, 0x00}}}

};


unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();
void GetRfCommand(unsigned char RfData);
void CheckRfCommand(unsigned char * RfCommand);


void IR_DEconde_INT();
bool IsIrIntOn = false; 
void CheckAcData();
void ProcessAcData();
unsigned char acTemperature(unsigned char code);
void AcOperation(unsigned char OpCode);
void MyPrintf(const char *fmt, ...);

#define  SAMPLE_NUMBER 200

byte IrData1[6] = {0,0,0,0,0,0};
byte IrData2[6] = {0,0,0,0,0,0};
byte * pIrData = IrData1;
bool isComamndStart = false;
bool FrameOK = false;
bool Frame1OK = false;
unsigned long PulseStartTime = 0;
bool isLastLong = true;
unsigned char BitCounter = 0;
unsigned long LastTime = 0;
unsigned char LastAcMode = 0;
unsigned char LastAcTemperature = 30;
bool LastAcOn = false;


unsigned long PowerTimeDiff;
unsigned long LastPowerTime = 0;
bool GotPower = false;
//unsigned long Power;
void AC_Power_INT();





void setup() 
{       

	pinMode(RELAY1, OUTPUT);//set the pin to be OUTPUT pin.
	pinMode(RELAY2, OUTPUT);//
	digitalWrite(RELAY1, HIGH);
	digitalWrite(RELAY2, HIGH);



	//AC RX
	pinMode(MIDEA_AC_RX, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(MIDEA_AC_RX), IR_DEconde_INT, RISING);
	IsIrIntOn = true; 


	//AC TX
	m_IRMideaAC.begin();



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
		RoomData.Mac[i] = mac[i];
	}

	for (unsigned char i = 0;i<ROOM_NUMBER;i++)
	{
		if (memcmp(&RoomData.Mac[0],&RoomMacAddress[i][0],sizeof(unsigned long)*6) == 0)
		{
			MyPrintf("room ID=%d \r\n",i);
			RoomIndex = i;
			break;
		}
	}


	if (RoomIndex == ROOM_BALCONY)
	{
			LOCAL_CONTROL = D5;
					pinMode(LOCAL_CONTROL, INPUT_PULLUP);
	} 
	else if(RoomIndex == ROOM_SOUTH)
	{
			AC_POWER = D5;

		pinMode(AC_POWER, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(AC_POWER), AC_Power_INT, RISING);

	}


	m_WiFiUDP.begin(5050); 




	Wire.begin(IIC_DAT,IIC_CLK);
	//ina219_1.begin();
	swSer.begin(9600);
	//dht.begin();


	RoomData.DataType = 2;
	RoomData.AcMode = 5;
	RoomData.AcTemperature = 27;




	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	//ArduinoOTA.setHostname("Test");

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");












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
	ArduinoOTA.handle();

	CheckAcData();

	while (swSer.available() > 0) {
		unsigned char recvdata = swSer.read();
		GetRfCommand(recvdata);
		yield();
	}

	TenthSecondsSinceStartTask();


	m_WiFiUDP.parsePacket(); 
	unsigned int UdpAvailable = m_WiFiUDP.available();
	if (UdpAvailable == sizeof(tRoomCommand))
	{
		//printf(" m_WiFiUDP.available() = %d\r\n",UdpAvailable);
		tRoomCommand tempRoomCommand;
		m_WiFiUDP.read((char *)&tempRoomCommand,sizeof(tRoomCommand));

		if (tempRoomCommand.LightChangeNotice == true)
		{
			for (byte i=0;i<LIGHT_NUMBER;i++)
			{
				RoomData.Light[i] = tempRoomCommand.Light[i];
				MyPrintf("get from control %d %d\r\n",i,RoomData.Light[i]);
				digitalWrite(RelayPin[i], !RoomData.Light[i]);
			}
		}
		if (tempRoomCommand.AcChangeNotice == true)
		{
			MyPrintf("get ac control code %d\r\n",tempRoomCommand.AcOpCode);
			AcOperation(tempRoomCommand.AcOpCode);
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
	//Serial.printf("%c%c:%c%c:%c%c\n",H1,H2,M1,M2,S1,S2);
	//Serial.printf(time_str);

	//MyPrintf("AcTemperature = %d  AcMode = %d \r\n",RoomData.AcTemperature,RoomData.AcMode);


	m_WiFiUDP.beginPacket(SERVER_ADDRESS, 5050);
	m_WiFiUDP.write((const char*)&RoomData, sizeof(tRoomData));
	m_WiFiUDP.endPacket(); 







	//swSer.enableRx(false);

	//Serial.print("Temperatura: ");
	//Serial.print(dht12.readTemperature());
	//Serial.print("*C  Humedad: ");
	//Serial.print(dht12.readHumidity());
	//Serial.println("%RH");

	if(dht12.read() == 0)		
	{
		RoomData.Humidity = dht12.LastHumidity*10;
		RoomData.RealTemperature = dht12.LastTemperature*10;
	}

	//MyPrintf("Humidity: %d %%	Temperature: %d *C \r\n",RoomData.Humidity,RoomData.RealTemperature);


	if (GotPower)
	{
		GotPower = false;
		RoomData.Power = 36000000/16/PowerTimeDiff;
		//MyPrintf("TimeDiff = %ld power = %ld \r\n",PowerTimeDiff,RoomData.Power);
	}
	else
	{
		unsigned long CurrentDiff = millis()-LastPowerTime;
		if (PowerTimeDiff < CurrentDiff)
		{
			RoomData.Power = 36000000/16/CurrentDiff;
			//MyPrintf("ruduce power to = %ld \r\n",RoomData.Power);
		}
	}

	//swSer.enableRx(true);

	////IIC读取
	//unsigned int Current = ina219_1.getCurrent_mA();
	//Serial.printf("Current  =  %d mA \r\n",Current);
	//unsigned int testvolt = ina219_1.getBusVoltage_V()*1000;
	//Serial.printf("test volt  =  %d mv \r\n",testvolt);

	//	//模拟量读取
	//	int Light = 100-(analogRead(A0)/11);
	//	Serial.printf("Light = %d \n",  Light);
}

void OnTenthSecond()
{
	static unsigned char SoftwareSerialOffTime = 0;

	static bool LastLocalControl;

	if (TenthSecondsSinceStart%10==0)
	{
		OnSecond();
	}

	if (!IsSoftwareSerialOn)
	{
		SoftwareSerialOffTime++;
		if (SoftwareSerialOffTime>5)
		{
			swSer.enableRx(true);
			IsSoftwareSerialOn = true;
			SoftwareSerialOffTime = 0;
		}
	}


	if (RoomIndex == ROOM_BALCONY)
	{
		bool LocalControl = digitalRead(LOCAL_CONTROL);
		if (LastLocalControl != LocalControl)
		{
			LastLocalControl = LocalControl;
			MyPrintf("LocalControl to %d \r\n",LocalControl);


			RoomData.Light[0] = !RoomData.Light[0] ;
			digitalWrite(RelayPin[0], !RoomData.Light[0]);
		} 
		else
		{
		}
	}
	



	//if (TenthSecondsSinceStart%200==0)
	//{
	//	digitalWrite(RELAY1, HIGH);
	//	digitalWrite(RELAY2, HIGH);
	//	MyPrintf("relay to high,  disconnect \r\n");

	//}

	//if (TenthSecondsSinceStart%200==100)
	//{
	//	digitalWrite(RELAY1, LOW);
	//	digitalWrite(RELAY2, LOW);
	//	MyPrintf("relay to low,  connect \r\n");

	//}




	//if (TenthSecondsSinceStart%200==0)
	//{

	//	// Reading temperature or humidity takes about 250 milliseconds!
	//	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	//	float h = dht.readHumidity();
	//	RoomData.Humidity = h;
	//	// Read temperature as Celsius (the default)
	//	float t = dht.readTemperature();
	//	RoomData.RealTemperature = t;

	//	// Check if any reads failed.
	//	if (!isnan(h) && !isnan(t)) 
	//	{
	//		MyPrintf("Humidity: %d %%	Temperature: %d *C \r\n",RoomData.Humidity,RoomData.RealTemperature);
	//	}
	//	else
	//	{
	//		Serial.println("Read Temperature failed");
	//	}
	//}
}

void GetRfCommand(unsigned char RfData)
{
	static unsigned char Index = 0;
	static unsigned char GetCommand[4];
	static bool start = false;

	if ((start == false)&&(RfData == 0xFD))
	{
		start = true;
		Index = 0;
		return;
	}

	if (start == true)
	{
		GetCommand[Index] = RfData;
		Index++;
		if (Index >=3)
		{
			CheckRfCommand(GetCommand);
			Index = 0;
			start = false;
		}
	}


	/*  0xFD 0xC3 0x7C 0x68 0x55 0xDF */

}

void CheckRfCommand(unsigned char * RfCommand)
{
	static unsigned long LastTrigerTimer;
	MyPrintf("CheckRfCommand 0x%02X, 0x%02X, 0x%02X\r\n",RfCommand[0],RfCommand[1],RfCommand[2]);
	for (byte j=0;j<RF_COMMAND_FUNCTION_COUNTER;j++)
	{
		for (byte i=0;i<RF_COMMAND_KEY_COUNTER;i++)
		{
			//MyPrintf("Check with 0x%02X 0x%02X 0x%02X\r\n",PreSetRfCommand[i][0],PreSetRfCommand[i][1],PreSetRfCommand[i][2]);
			if(memcmp(RfCommand,PreSetRfCommand[RoomIndex][j][i],RF_COMMAND_LEN)==0)
			{

				if (TenthSecondsSinceStart - LastTrigerTimer > RF_COMMAND_DELAY)
				{
					LastTrigerTimer = TenthSecondsSinceStart;

					if (j<LIGHT_NUMBER)
					{
						MyPrintf("Execute light Command %d \r\n",j);
						RoomData.Light[j] = !RoomData.Light[j] ;
						digitalWrite(RelayPin[j], !RoomData.Light[j]);
					} 
					else
					{

						if ((j == 3)&&(RoomIndex == ROOM_SOUTH))
						{
							swSer.enableRx(false);
							IsSoftwareSerialOn = false;
							AcOperation(AC_OP_ON_OFF);
						}
						if ((j == 2)&&(RoomIndex == ROOM_BALCONY))
						{
							MyPrintf("Execute light OFF r\n");
							RoomData.Light[0] = 0 ;
							digitalWrite(RelayPin[0], !RoomData.Light[0]);
						}
					}
				}
			}
		}
	}




}

void AcOperation(unsigned char OpCode)
{

	m_IRMideaAC.stateReset();

	switch(OpCode)
	{
	case AC_OP_ON_OFF:
		if (RoomData.AcMode == 5)
		{
			MyPrintf("Execute AC On \r\n");
			m_IRMideaAC.setPower(true);
			m_IRMideaAC.setMode(LastAcMode);
			m_IRMideaAC.setTemp(LastAcTemperature);
		}
		else
		{
			MyPrintf("Execute AC Off \r\n");
			//RoomData.AcMode = 5;
			m_IRMideaAC.setPower(false);
		}
		break;
	case AC_OP_MODE:
		MyPrintf("Execute AC mode \r\n");
		m_IRMideaAC.setPower(LastAcOn);
		m_IRMideaAC.setTemp(LastAcTemperature);
		switch(RoomData.AcMode)//0冷 1湿 2自 3热  4风 5关机
		{
		case 0:
			m_IRMideaAC.setMode(1);
			break;
		case 1:
			m_IRMideaAC.setMode(2);
			break;
		case 2:
			m_IRMideaAC.setMode(3);
			break;
		case 3:
			m_IRMideaAC.setMode(0);
			break;
		case 4:
			m_IRMideaAC.setMode(0);
			break;
		case 5:
			m_IRMideaAC.setPower(false);
			break;
		}
		break;
	case AC_OP_UP:
		MyPrintf("Execute AC up \r\n");
		m_IRMideaAC.setPower(LastAcOn);
		m_IRMideaAC.setMode(LastAcMode);
		m_IRMideaAC.setTemp(LastAcTemperature+1);
		break;
	case AC_OP_DOWN:
		MyPrintf("Execute AC down \r\n");
		m_IRMideaAC.setPower(LastAcOn);
		m_IRMideaAC.setMode(LastAcMode);
		m_IRMideaAC.setTemp(LastAcTemperature-1);
		break;

	case AC_OP_ON_COLD:
		MyPrintf("Execute AC down \r\n");
		m_IRMideaAC.setPower(true);
		m_IRMideaAC.setMode(0);
		m_IRMideaAC.setTemp(28);
		break;

	case AC_OP_ON_HEAT:
		MyPrintf("Execute AC down \r\n");
		m_IRMideaAC.setPower(true);
		m_IRMideaAC.setMode(3);
		m_IRMideaAC.setTemp(20);
		break;

	case AC_OP_OFF:
		MyPrintf("Execute AC Off \r\n");
		m_IRMideaAC.setPower(false);
		break;
	}

	m_IRMideaAC.send();
}

void AC_Power_INT()//中断函数
{
	unsigned long ThisPowerTime = millis();

	if (LastPowerTime == 0)
	{
		LastPowerTime = ThisPowerTime;
		return;
	}
	else
	{
		unsigned long ThisPowerDiff = ThisPowerTime - LastPowerTime;
		if (ThisPowerDiff > 10)
		{
			PowerTimeDiff = ThisPowerTime - LastPowerTime;
			LastPowerTime = ThisPowerTime;
			GotPower = true;
		}

	}

}

void IR_DEconde_INT()//中断函数
{
	unsigned long ThisTime;
	unsigned long DiffTime;
	unsigned int PulseTime;
	unsigned int IdleTime;




	if (BitCounter<SAMPLE_NUMBER)
	{
		if (LastTime == 0)//record the first timestamp
		{
			LastTime = micros();
		} 
		else
		{
			ThisTime = micros();
			DiffTime = ThisTime-LastTime;
			if (DiffTime < 100)
			{
				if (isLastLong)
				{
					PulseStartTime = ThisTime;
					isLastLong = false;
				}
			}
			else
			{
				PulseTime = LastTime - PulseStartTime;
				IdleTime = DiffTime;

				if (isComamndStart)
				{
					if (IdleTime > 3800)
					{
						
					}
					else if(!FrameOK)
					{
						if (IdleTime > 1000)//bit 1
						{
							pIrData[BitCounter/8] += 1<<(7-BitCounter%8);
						}
						else//bit 0
						{
						}
						BitCounter++;
						if (BitCounter>=48)
						{
							if (pIrData == IrData1)
							{
								BitCounter = 0;
								isComamndStart = false;
								pIrData = IrData2;
								Frame1OK = true;
							} 
							else
							{
								detachInterrupt(digitalPinToInterrupt(MIDEA_AC_RX));
								IsIrIntOn = false; 
								FrameOK = true;
							}
						}
					}
				}
				else
				{
					if ((PulseTime > 3800)&&(IdleTime > 3800))
					{
						isComamndStart = true;
					}
				}
				isLastLong = true;
			}
			LastTime = ThisTime;
		}
	}
}


void CheckAcData()
{

	static long FrameOKTime;

	if (Frame1OK)
	{
		delay(300);
		if (!FrameOK)
		{
			MyPrintf("Only get data 1\r\n");
			MyPrintf("Get IR data1:0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",IrData1[0],IrData1[1],IrData1[2],IrData1[3],IrData1[4],IrData1[5]);

		}
		Frame1OK = false;

	}

	if (FrameOK)
	{
		FrameOK = false;	
		FrameOKTime = TenthSecondsSinceStart;
		MyPrintf("Get IR data1:0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",IrData1[0],IrData1[1],IrData1[2],IrData1[3],IrData1[4],IrData1[5]);
		MyPrintf("Get IR data2:0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",IrData2[0],IrData2[1],IrData2[2],IrData2[3],IrData2[4],IrData2[5]);

		pIrData = IrData1;

		if((pIrData[0] == 0xb2)
			&&(pIrData[0] == (byte)(~(pIrData[1])))
			&&(pIrData[2] == (byte)(~(pIrData[3])))
			&&(pIrData[4] == (byte)(~(pIrData[5]))))
		{
			MyPrintf("Data 1 checked!\r\n");
			ProcessAcData();
			return;
		}
		else
		{
			MyPrintf("Data 1 faled!\r\n");
		}

		pIrData = IrData2;

		if((pIrData[0] == 0xb2)
			&&(pIrData[0] == (byte)(~(pIrData[1])))
			&&(pIrData[2] == (byte)(~(pIrData[3])))
			&&(pIrData[4] == (byte)(~(pIrData[5]))))
		{
			MyPrintf("Data 2 checked!\r\n");
			ProcessAcData();
			return;
		}
		else
		{
			MyPrintf("Data 2 faled!\r\n");
		}
	}


	//non stop task
	if((TenthSecondsSinceStart - FrameOKTime > 4)&&(IsIrIntOn == false)) 
	{
		for (unsigned char i = 0; i<6 ; i++)
		{
			IrData1[i] = 0;
			IrData2[i] = 0;
		}
		pIrData = IrData1;
		BitCounter = 0;
		LastTime = 0;
		PulseStartTime = 0;
		isLastLong = true;
		isComamndStart = false;
		attachInterrupt(digitalPinToInterrupt(MIDEA_AC_RX), IR_DEconde_INT, RISING);
		IsIrIntOn = true; 
	}
}

void ProcessAcData()
{
	if ((pIrData[2] == 0x7B)&&(pIrData[4] == 0xE0))
	{
		RoomData.AcMode = 5;
		//MyPrintf("Mode = 关机 \r\n");
	}
	else
	{
		RoomData.AcTemperature = acTemperature(pIrData[4]>>4);
		//MyPrintf("Temperature = %d \r\n",RoomData.AcTemperature);

		//原厂定义
		//0冷 1风湿 2自 3热 

		//自有定义
		//0冷 1湿 2自 3热  4风 5关机

		if (RoomData.AcTemperature == 0)
		{
			RoomData.AcMode = 4;
		} 
		else
		{
			RoomData.AcMode = (pIrData[4]>>2)&3;
		}
	}

	MyPrintf("Data checked! mode = %d, temp = %d\r\n",RoomData.AcMode,RoomData.AcTemperature);


	if (RoomData.AcMode != 5)
	{
		LastAcMode = RoomData.AcMode;
		LastAcTemperature = RoomData.AcTemperature;
		LastAcOn = true;
	}
	else
	{
		LastAcOn = false;
	}





}

unsigned char acTemperature(unsigned char code)
{
	switch(code)
	{
	case 0:
		return 17; 
		break;
	case 1:
		return 18; 
		break;
	case 3:
		return 19; 
		break;
	case 2:
		return 20; 
		break;
	case 6:
		return 21; 
		break;
	case 7:
		return 22; 
		break;
	case 5:
		return 23; 
		break;
	case 4:
		return 24; 
		break;
	case 12:
		return 25; 
		break;
	case 13:
		return 26; 
		break;
	case 9:
		return 27; 
		break;
	case 8:
		return 28; 
		break;
	case 10:
		return 29; 
		break;
	case 11:
		return 30; 
		break;
	case 14:
		return 0; 
		break;
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
	pDebugData->RoomId = RoomIndex;
	pDebugData->Length = n;

	m_WiFiUDP.beginPacket(SERVER_ADDRESS, 5050);
	m_WiFiUDP.write((const char*)send_buf, sizeof(tDebugData)+n);
	m_WiFiUDP.endPacket(); 


}
