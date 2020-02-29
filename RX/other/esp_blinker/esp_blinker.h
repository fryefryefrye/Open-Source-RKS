//
//
//
//#define BLINKER_PRINT Serial
//#define BLINKER_WIFI
//
//#include <Blinker.h>
//
char auth[] = "dd2d078b9b09";
char ssid[] = "frye";
char pswd[] = "52150337";
//
//// 新建组件对象
//BlinkerButton Button1("btn-abc");
//BlinkerNumber Number1("num-abc");
//
//int counter = 0;
//
//// 按下按键即会执行该函数
//void button1_callback(const String & state) {
//	BLINKER_LOG("get button state: ", state);
//	digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//}
//
//// 如果未绑定的组件被触发，则会执行其中内容
//void dataRead(const String & data)
//{
//	BLINKER_LOG("Blinker readString: ", data);
//	counter++;
//	Number1.print(counter);
//}
//
//void setup() {
//	// 初始化串口
//	Serial.begin(115200);
//
//#if defined(BLINKER_PRINT)
//	BLINKER_DEBUG.stream(BLINKER_PRINT);
//#endif
//
//	// 初始化有LED的IO
//	pinMode(LED_BUILTIN, OUTPUT);
//	digitalWrite(LED_BUILTIN, HIGH);
//	// 初始化blinker
//	Blinker.begin(auth, ssid, pswd);
//	Blinker.attachData(dataRead);
//	Button1.attach(button1_callback);
//}
//
//void loop() {
//	Blinker.run();
//}


#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#define BLINKER_ALIGENIE_MULTI_OUTLET

#include <Blinker.h>

//char auth[] = "Your Device Secret Key";
//char ssid[] = "Your WiFi network SSID or name";
//char pswd[] = "Your WiFi network WPA password or WEP key";

bool oState = false;

void aligeniePowerState(const String & state)
{
	BLINKER_LOG("need set power state: ", state);

	if (state == BLINKER_CMD_ON) {
		digitalWrite(LED_BUILTIN, HIGH);

		BlinkerAliGenie.powerState("on");
		BlinkerAliGenie.print();

		oState = true;
	}
	else if (state == BLINKER_CMD_OFF) {
		digitalWrite(LED_BUILTIN, LOW);

		BlinkerAliGenie.powerState("off");
		BlinkerAliGenie.print();

		oState = false;
	}
}

void aligenieQuery(int32_t queryCode)
{
	BLINKER_LOG("AliGenie Query codes: ", queryCode);

	switch (queryCode)
	{
	case BLINKER_CMD_QUERY_ALL_NUMBER :
		BLINKER_LOG("AliGenie Query All");
		BlinkerAliGenie.powerState(oState ? "on" : "off");
		BlinkerAliGenie.print();
		break;
	case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
		BLINKER_LOG("AliGenie Query Power State");
		BlinkerAliGenie.powerState(oState ? "on" : "off");
		BlinkerAliGenie.print();
		break;
	default :
		BlinkerAliGenie.powerState(oState ? "on" : "off");
		BlinkerAliGenie.print();
		break;
	}
}

void dataRead(const String & data)
{
	BLINKER_LOG("Blinker readString: ", data);

	Blinker.vibrate();

	uint32_t BlinkerTime = millis();
	Blinker.print(BlinkerTime);
	Blinker.print("millis", BlinkerTime);
}

void setup()
{
	Serial.begin(115200);

#if defined(BLINKER_PRINT)
	BLINKER_DEBUG.stream(BLINKER_PRINT);
#endif

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	Blinker.begin(auth, ssid, pswd);
	Blinker.attachData(dataRead);

	BlinkerAliGenie.attachPowerState(aligeniePowerState);
	BlinkerAliGenie.attachQuery(aligenieQuery);
}

void loop()
{
	Blinker.run();
}