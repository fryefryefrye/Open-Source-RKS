


void setup()
{

	pinMode(A0, OUTPUT);
	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);

	digitalWrite(A0, LOW);

	Serial.begin(115200);
	while (!Serial);             // Leonardo: wait for serial monitor
	Serial.println("\n Scanner");


}


void loop()
{

	//digitalWrite(A0, LOW);
	delay(10000);  
	digitalWrite(A0, HIGH);
	//delay(1000);           // wait 5 seconds for next scan
}
