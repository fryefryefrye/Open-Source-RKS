


#include <time.h>
#define timezone 8

#include <Ethernet.h>

// #ifdef FIRST_BUILD
// #include<WiFi.h>
// #else
#include<ESP8266WiFi.h>
#include<ESP8266WiFiMulti.h>
//#endif


#include <time.h>
#define timezone 8

const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
unsigned long MS_TIMER = 0;
unsigned long timer,lasttime=0;
char *time_str;
char H1,H2,M1,M2,S1,S2;
char Counter = 0;




unsigned char Distance()
{
    static unsigned long distacne;
    unsigned char res_char[2];

    //init ultra sonic
    while(Serial.available()>0)
    {
        Serial.read();
    }
    delay(10);
    Serial.write(0x55);
    delay(100);
    if(Serial.available()>=2)
    {
        res_char[1] = Serial.read();
        res_char[0] = Serial.read();
        distacne = res_char[1]*256 + res_char[0];
        if (distacne>1500)
        {
            distacne = 1500;
        }
    }
    return distacne/10;
}

signed char Temperature()
{
    static signed char Temp;
    while(Serial.available()>0)
    {
        Serial.read();
    }
    delay(10);
    Serial.write(0x50);

    delay(100);
    if(Serial.available()>=1)
    {
        Temp = Serial.read()-45;
    }
    return Temp;
}

void setup()
{


    delay(50);
    //Serial.begin(9600);
    Serial.begin(115200);

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);//设置模式为STA
    Serial.print("Is connection routing, please wait");
    WiFi.begin(ssid, password); //Wifi接入到网络
    Serial.println("\nConnecting to WiFi");
//如果Wifi状态不是WL_CONNECTED，则表示连接失败
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);    //延时等待接入网络
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());     //打印连接上wifi后获取的ip地址


    //设置时间格式以及时间服务器的网址
    configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("\nWaiting for time");
    while (!time(nullptr))
    {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("");


}

void loop()
{



#define AVG_COUNTER 15
    signed char Temp;
    unsigned char Dis[AVG_COUNTER];
    unsigned char AvgDis;
    unsigned long SumDis;

    unsigned char Light[AVG_COUNTER];
    unsigned char AvgLight;
    unsigned long SumLight;

    unsigned char i;

    timer = millis();
    if ((timer - lasttime)>=1000)
    {
        time_t now = time(nullptr); //获取当前时间
        time_str = ctime(&now);
        H1 = time_str[11];
        H2 = time_str[12];
        M1 = time_str[14];
        M2 = time_str[15];
        S1 = time_str[17];
        S2 = time_str[18];
        Serial.printf("%s%c%c:%c%c:%c%c\n",time_str,H1,H2,M1,M2,S1,S2);
        lasttime = timer;

        //Dis[Counter] = Distance();
        Serial.printf("Dis = %d \n", Dis[Counter]);
        delay(10);

        Light[Counter] = 100-(analogRead(A0)/11);
        Serial.printf("Light = %d \n",  Light[Counter]);

        Counter++;

        // if (Counter == AVG_COUNTER)
        // {
            // SumDis = 0;
            // for(i = 0; i<AVG_COUNTER; i++)
            // {
                // SumDis = SumDis + Dis[i];
            // }
            // AvgDis = SumDis/AVG_COUNTER;

            // SumLight = 0;
            // for(i = 0; i<AVG_COUNTER; i++)
            // {
                // SumLight = SumLight + Light[i];
            // }
            // AvgLight = SumLight/AVG_COUNTER;


            // Temp = Temperature();


            // Serial.printf("Update Dis = %d \n", AvgDis);
            // Serial.printf("Update Light = %d \n", AvgLight);
            // Serial.printf("Update Temp = %d \n", Temp);
            // Counter = 0;
        // }
    }
}


