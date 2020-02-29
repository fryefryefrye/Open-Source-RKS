#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <DS3231.h>
#include <Wire.h>
#include <DHTesp.h>
#include <TimeLib.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <aJSON.h>  
#include "Day.h"
/**********************必须修改项**********************/
const char ssid[]="=====";//WiFi名==
const char pass[]="=====";//WiFi密码==
String DEVICEID="=====";//你的设备编号==
String APIKEY="=====";//设备密码==
/**********************可以修改项**********************/
const unsigned char postingInterval=40;//每隔40秒向服务器报到一次
const char host_Web[]="keting-clock";//Web-URL
static const char ntpServerName[]="ntp.ntsc.ac.cn";//NTP地址
unsigned char DHT_PIN=16;//DHT11数据读取
/**********************不可修改项**********************/
//贝壳物联
const char* host="www.bigiot.net";
const int httpPort=8181;
//NTP
unsigned int localPort=8888;//端口
const int NTP_PACKET_SIZE=48;
byte packetBuffer[NTP_PACKET_SIZE];
//全局变量
unsigned char shi,fen,miao,yue,ri,zhou,nian;//时分秒月日周年
bool Century=false;//世纪
bool h12;//12小时制
bool PM;//上午下午标志
bool dp;//秒点闪烁
bool on_off;//开关
bool init_flag;//上电标志
unsigned char wendu,shidu;//温度湿度
String print_send1;//第一行发送的数据
String print_send2;//第二行发送的数据
unsigned int Timer_ms;//计时

Ticker flipper1;
Ticker flipper2;
WiFiUDP udp;
IPAddress timeServerIP;
ESP8266WiFiMulti WiFiMulti;
DS3231 Clock;
DHTesp dht;
ESP8266WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
WiFiClient client1;
WiFiClient client2;
HTTPClient http;
//==============================================第一行显示
void send1()
{
  if(dp==1)
    dp=0;
  else
    dp=1;
//----------------------------------------------------------------------月
  if(yue<10)
    print_send1=print_send1+"$ "+yue;
  else
    print_send1=print_send1+"$"+yue;
//----------------------------------------------------------------------日
  print_send1=print_send1+"月"+ri/10+ri%10;
//----------------------------------------------------------------------农历月
  if(run_yue==1)
    print_send1=print_send1+"闰";
  else
    print_send1=print_send1+"日";
  switch(month_moon)
  {
    case 1:print_send1=print_send1+"正";break;
    case 2:print_send1=print_send1+"二";break;
    case 3:print_send1=print_send1+"三";break;
    case 4:print_send1=print_send1+"四";break;
    case 5:print_send1=print_send1+"五";break;
    case 6:print_send1=print_send1+"六";break;
    case 7:print_send1=print_send1+"七";break;
    case 8:print_send1=print_send1+"八";break;
    case 9:print_send1=print_send1+"九";break;
    case 10:print_send1=print_send1+"十";break;
    case 11:print_send1=print_send1+"冬";break;
    case 12:print_send1=print_send1+"腊";break;
  }
  print_send1=print_send1+"月";
  if(day_moon<=10)//初几
    print_send1=print_send1+"初";
  else if(day_moon<20)//十几
    print_send1=print_send1+"十";
  else if(day_moon==20)//二十
    print_send1=print_send1+"二";
  else if(day_moon<30)//二十几
    print_send1=print_send1+"廿";
  else//三十
    print_send1=print_send1+"三";
//----------------------------------------------------------------------农历日
  if(day_moon%10==0)//整数10
    print_send1=print_send1+"十";
  else//不是整数
  {
    switch(day_moon%10)
    {
      case 1:print_send1=print_send1+"一";break;
      case 2:print_send1=print_send1+"二";break;
      case 3:print_send1=print_send1+"三";break;
      case 4:print_send1=print_send1+"四";break;
      case 5:print_send1=print_send1+"五";break;
      case 6:print_send1=print_send1+"六";break;
      case 7:print_send1=print_send1+"七";break;
      case 8:print_send1=print_send1+"八";break;
      case 9:print_send1=print_send1+"九";break;
    }
  }
//----------------------------------------------------------------------时
  if(shi<10)
    print_send1=print_send1+" "+shi;
  else
    print_send1=print_send1+shi;
  if(dp==1)
    print_send1=print_send1+":";
  else
    print_send1=print_send1+" ";
//----------------------------------------------------------------------分
  print_send1=print_send1+fen/10+fen%10;
  if(dp==1)
    print_send1=print_send1+":";
  else
    print_send1=print_send1+" ";
//----------------------------------------------------------------------秒
  print_send1=print_send1+miao/10+miao%10; 
//----------------------------------------------------------------------周
  switch(zhou)
  {
    case 1:print_send1=print_send1+"壹";break;
    case 2:print_send1=print_send1+"贰";break;
    case 3:print_send1=print_send1+"叁";break;
    case 4:print_send1=print_send1+"肆";break;
    case 5:print_send1=print_send1+"伍";break;
    case 6:print_send1=print_send1+"陆";break;
    case 7:print_send1=print_send1+"日";break;
  }
//----------------------------------------------------------------------温度
  print_send1=print_send1+wendu/10+wendu%10+"℃"+shidu/10+shidu%10+"%";
}
//==============================================第二行显示
void send2()
{
  if((WiFiMulti.run()==WL_CONNECTED))
  {       
    http.begin(client1,"=========================================");//API
    int httpCode=http.GET();
    
    if(httpCode>0)
    {
      if(httpCode==HTTP_CODE_OK)
      {
        String payload1,payload2,payload3;
        int chars1,chars2;
        //解析
        String payload=http.getString();
        chars1=payload.indexOf("code");
        chars2=payload.indexOf("\"",chars1+7);
        payload1=payload.substring(chars1+7,chars2);
        if(payload1=="10000")//代码为1000是正确返回数据
        {
          print_send2="        现在:";
          //----------------------------------------------天气实况
          chars1=payload.indexOf("now");
          chars2=payload.indexOf("}",chars1+150);
          payload1=payload.substring(chars1,chars2);
          //天气实况文字
          chars1=payload1.indexOf("txt");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2+" 体感";
          //体感温度
          chars1=payload1.indexOf("fl");
          chars2=payload1.indexOf("\"",chars1+5);
          payload2=payload1.substring(chars1+5,chars2);
          print_send2=print_send2+payload2+"℃ 温度";
          //温度
          chars1=payload1.indexOf("tmp");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2+"℃ ";
          //风向
          chars1=payload1.indexOf("dir");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2;
          //风力
          chars1=payload1.indexOf("sc");
          chars2=payload1.indexOf("\"",chars1+5);
          payload2=payload1.substring(chars1+5,chars2);
          print_send2=print_send2+payload2+"级 空气";
          //空气质量
          chars1=payload.indexOf("qlty");
          chars2=payload.indexOf("\"",chars1+7);
          payload2=payload.substring(chars1+7,chars2);
          print_send2=print_send2+payload2+" 今天:";
          //----------------------------------------------节气，如果是今天就在这显示
          if(jieqi_day==0)//今天是节气
          {
            switch(jieqi_dat)//根据节气编号显示节气
            {
              case 1:print_send2=print_send2+"小寒 ";break;
              case 2:print_send2=print_send2+"大寒 ";break;
              case 3:print_send2=print_send2+"立春 ";break;
              case 4:print_send2=print_send2+"雨水 ";break;
              case 5:print_send2=print_send2+"惊蛰 ";break;
              case 6:print_send2=print_send2+"春分 ";break;
              case 7:print_send2=print_send2+"清明 ";break;
              case 8:print_send2=print_send2+"谷雨 ";break;
              case 9:print_send2=print_send2+"立夏 ";break;
              case 10:print_send2=print_send2+"小满 ";break;
              case 11:print_send2=print_send2+"芒种 ";break;
              case 12:print_send2=print_send2+"夏至 ";break;
              case 13:print_send2=print_send2+"小暑 ";break;
              case 14:print_send2=print_send2+"大暑 ";break;
              case 15:print_send2=print_send2+"立秋 ";break;
              case 16:print_send2=print_send2+"处暑 ";break;
              case 17:print_send2=print_send2+"白露 ";break;
              case 18:print_send2=print_send2+"秋分 ";break;
              case 19:print_send2=print_send2+"寒露 ";break;
              case 20:print_send2=print_send2+"霜降 ";break;
              case 21:print_send2=print_send2+"立冬 ";break;
              case 22:print_send2=print_send2+"小雪 ";break;
              case 23:print_send2=print_send2+"大雪 ";break;
              case 24:print_send2=print_send2+"冬至 ";break;
            }
          }
          //----------------------------------------------天气预报，今天
          chars1=payload.indexOf("daily_forecast");
          chars2=payload.indexOf("]");
          payload1=payload.substring(chars1,chars2);
          //今天白天天气状况文字
          chars1=payload1.indexOf("txt_d");
          chars2=payload1.indexOf("\"",chars1+8);
          payload2=payload1.substring(chars1+8,chars2);
          //今天夜间天气状况文字
          chars1=payload1.indexOf("txt_n");
          chars2=payload1.indexOf("\"",chars1+8);
          payload3=payload1.substring(chars1+8,chars2);
          //今天天气状况文字一样就只显示一个
          if(payload2==payload3)
            print_send2=print_send2+payload3+" ";
          else//不一样就转
            print_send2=print_send2+payload2+"转"+payload3+" ";
          //今天最高温度
          chars1=payload1.indexOf("max");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2+"/";         
          //今天最低温度
          chars1=payload1.indexOf("min");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2+"℃ ";
          //今天风向
          chars1=payload1.indexOf("dir");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2;
          //今天风力
          chars1=payload1.indexOf("sc");
          chars2=payload1.indexOf("\"",chars1+5);
          payload2=payload1.substring(chars1+5,chars2);
          print_send2=print_send2+payload2+"级 明天:";
          //----------------------------------------------节气，如果是明天就在这显示
          if(jieqi_day==1)//明天是节气
          {
            switch(jieqi_dat)//根据节气编号显示节气
            {
              case 1:print_send2=print_send2+"小寒 ";break;
              case 2:print_send2=print_send2+"大寒 ";break;
              case 3:print_send2=print_send2+"立春 ";break;
              case 4:print_send2=print_send2+"雨水 ";break;
              case 5:print_send2=print_send2+"惊蛰 ";break;
              case 6:print_send2=print_send2+"春分 ";break;
              case 7:print_send2=print_send2+"清明 ";break;
              case 8:print_send2=print_send2+"谷雨 ";break;
              case 9:print_send2=print_send2+"立夏 ";break;
              case 10:print_send2=print_send2+"小满 ";break;
              case 11:print_send2=print_send2+"芒种 ";break;
              case 12:print_send2=print_send2+"夏至 ";break;
              case 13:print_send2=print_send2+"小暑 ";break;
              case 14:print_send2=print_send2+"大暑 ";break;
              case 15:print_send2=print_send2+"立秋 ";break;
              case 16:print_send2=print_send2+"处暑 ";break;
              case 17:print_send2=print_send2+"白露 ";break;
              case 18:print_send2=print_send2+"秋分 ";break;
              case 19:print_send2=print_send2+"寒露 ";break;
              case 20:print_send2=print_send2+"霜降 ";break;
              case 21:print_send2=print_send2+"立冬 ";break;
              case 22:print_send2=print_send2+"小雪 ";break;
              case 23:print_send2=print_send2+"大雪 ";break;
              case 24:print_send2=print_send2+"冬至 ";break;
            }
          }
          //----------------------------------------------天气预报，明天
          chars1=payload.indexOf("daily_forecast");
          chars1=payload.indexOf("astro",chars1+150);
          chars2=payload.indexOf("astro",chars1+150);
          payload1=payload.substring(chars1,chars2);
          //明天白天天气状况文字
          chars1=payload1.indexOf("txt_d");
          chars2=payload1.indexOf("\"",chars1+8);
          payload2=payload1.substring(chars1+8,chars2);
          //明天夜间天气状况文字
          chars1=payload1.indexOf("txt_n");
          chars2=payload1.indexOf("\"",chars1+8);
          payload3=payload1.substring(chars1+8,chars2);
          //明天天气状况文字一样就只显示一个
          if(payload2==payload3)
            print_send2=print_send2+payload3+" ";
          else//不一样就转
            print_send2=print_send2+payload2+"转"+payload3+" ";
          //明天最高温度
          chars1=payload1.indexOf("max");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2+"/";         
          //明天最低温度
          chars1=payload1.indexOf("min");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2+"℃ ";
          //明天风向
          chars1=payload1.indexOf("dir");
          chars2=payload1.indexOf("\"",chars1+6);
          payload2=payload1.substring(chars1+6,chars2);
          print_send2=print_send2+payload2;
          //明天风力
          chars1=payload1.indexOf("sc");
          chars2=payload1.indexOf("\"",chars1+5);
          payload2=payload1.substring(chars1+5,chars2);
          print_send2=print_send2+payload2+"级 ";
          //----------------------------------------------节气
          if(jieqi_day==1||jieqi_day==0)//今天和明天是节气，就不在这显示           
            print_send2=print_send2+"               ";           
          else//今天和明天不是节气，就在这显示
          {
            if(jieqi_day==2)//后天是节气
            {
              print_send2=print_send2+"后天";
              switch(jieqi_dat)//根据节气编号显示节气
              {
                case 1:print_send2=print_send2+"小寒                ";break;
                case 2:print_send2=print_send2+"大寒                ";break;
                case 3:print_send2=print_send2+"立春                ";break;
                case 4:print_send2=print_send2+"雨水                ";break;
                case 5:print_send2=print_send2+"惊蛰                ";break;
                case 6:print_send2=print_send2+"春分                ";break;
                case 7:print_send2=print_send2+"清明                ";break;
                case 8:print_send2=print_send2+"谷雨                ";break;
                case 9:print_send2=print_send2+"立夏                ";break;
                case 10:print_send2=print_send2+"小满                ";break;
                case 11:print_send2=print_send2+"芒种                ";break;
                case 12:print_send2=print_send2+"夏至                ";break;
                case 13:print_send2=print_send2+"小暑                ";break;
                case 14:print_send2=print_send2+"大暑                ";break;
                case 15:print_send2=print_send2+"立秋                ";break;
                case 16:print_send2=print_send2+"处暑                ";break;
                case 17:print_send2=print_send2+"白露                ";break;
                case 18:print_send2=print_send2+"秋分                ";break;
                case 19:print_send2=print_send2+"寒露                ";break;
                case 20:print_send2=print_send2+"霜降                ";break;
                case 21:print_send2=print_send2+"立冬                ";break;
                case 22:print_send2=print_send2+"小雪                ";break;
                case 23:print_send2=print_send2+"大雪                ";break;
                case 24:print_send2=print_send2+"冬至                ";break;
              }
            }
            else//距离节气还有一段日子，显示下一个节气日期
            {                
              print_send2=print_send2+"还有"+jieqi_day+"天";
              switch(jieqi_dat)//根据节气编号显示节气
              {
                case 1:print_send2=print_send2+"小寒                ";break;
                case 2:print_send2=print_send2+"大寒                ";break;
                case 3:print_send2=print_send2+"立春                ";break;
                case 4:print_send2=print_send2+"雨水                ";break;
                case 5:print_send2=print_send2+"惊蛰                ";break;
                case 6:print_send2=print_send2+"春分                ";break;
                case 7:print_send2=print_send2+"清明                ";break;
                case 8:print_send2=print_send2+"谷雨                ";break;
                case 9:print_send2=print_send2+"立夏                ";break;
                case 10:print_send2=print_send2+"小满                ";break;
                case 11:print_send2=print_send2+"芒种                ";break;
                case 12:print_send2=print_send2+"夏至                ";break;
                case 13:print_send2=print_send2+"小暑                ";break;
                case 14:print_send2=print_send2+"大暑                ";break;
                case 15:print_send2=print_send2+"立秋                ";break;
                case 16:print_send2=print_send2+"处暑                ";break;
                case 17:print_send2=print_send2+"白露                ";break;
                case 18:print_send2=print_send2+"秋分                ";break;
                case 19:print_send2=print_send2+"寒露                ";break;
                case 20:print_send2=print_send2+"霜降                ";break;
                case 21:print_send2=print_send2+"立冬                ";break;
                case 22:print_send2=print_send2+"小雪                ";break;
                case 23:print_send2=print_send2+"大雪                ";break;
                case 24:print_send2=print_send2+"冬至                ";break;
              }
            }
          }
        }
        else//接收数据出错
          print_send2="**数据返回出错,请检查数据接口***";   
      }
      else//访问状态出错
        print_send2="********数据接口访问出错********";
    }
    else//访问失败
      print_send2="********数据接口访问失败********";  
    http.end();  
  }
}
//==============================================温湿度读取
void read_DHT()
{
  unsigned char temp;
  temp=dht.getHumidity();//读取湿度
  if(temp<100)
    shidu=temp;
  temp=dht.getTemperature();//读取温度
  if(temp<100)
    wendu=temp;
}
//==============================================500ms定时发送一次
void timer_500ms()
{
  Timer_ms++;//加一次是500ms
  NTP_Time();
  send1();
  print_send1=print_send1+print_send2;
  Serial.println(print_send1);
  print_send1=""; 
  print_send2="";
}
//==============================================定时向贝壳物联发送心跳包
void heartbeat()//定时向服务器发送心跳
{
  BKstatus();
}
//==============================================Web升级初始化
void init_Web()
{
  MDNS.begin(host_Web);
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("Ready! Open http://%s.local in your browser\n", host_Web);
}
//==============================================初始化函数
void setup()
{
  on_off=1;
  //串口初始化
  Serial.begin(115200);//设置波特率
  Serial.println();
  Serial.println();
  dht.setup(DHT_PIN,DHTesp::DHT11);//设置DHT温湿度传感器引脚，设置传感器型号（DHT12兼容DHT11）
  Wire.begin();//设置IIC总线为主机
//连接WiFi初始化
  print_send2 ="正在连接WiFi....                ";
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  while(WiFi.status()!=WL_CONNECTED)//等待连接到WiFi
  {    
    miao=Clock.getSecond();//读取3231信息
    fen=Clock.getMinute();
    shi=Clock.getHour(h12,PM);
    ri=Clock.getDate();
    yue=Clock.getMonth(Century);
    nian=Clock.getYear();
    zhou=Clock.getDoW();    
    //Clock.getTemperature();//读取3231温度
    Conversion(Century,nian,yue,ri);//农历计算
    read_jieqi(nian,yue,ri);//节气计算
    read_DHT();
    send1();
    if(WiFi.status()==WL_NO_SSID_AVAIL)//WiFi名错误
      print_send2 ="未搜索到可连接WiFi...           ";
    if(WiFi.status()==WL_CONNECT_FAILED)//WiFi密码错误
      print_send2 ="WiFi密码错误...                 ";
    print_send1=print_send1+print_send2;//正在连接WiFi
    Serial.println(print_send1);
    print_send1="";
    delay(500);
  }
  udp.begin(localPort);
  WiFi.hostByName(ntpServerName,timeServerIP);//获取IP
  
  init_Web();
  
  flipper1.attach_ms(500,timer_500ms);//开启定时器
  flipper2.attach(postingInterval,heartbeat);//开启定时器
}
//==============================================主函数
void loop() 
{
  if(WiFi.status()!=WL_CONNECTED)//WIFI连接不正常
  {
    flipper1.detach();//停止定时器
    flipper2.detach();//停止定时器
    init_flag=0;
    Timer_ms=0;   
    print_send1="";
    WiFi.reconnect();//重新连接WiFi 
    print_send2="***WiFi断开,正在尝试重新连接****";  
    while(WiFi.status()!=WL_CONNECTED)//等待连接到WiFi
    {
      miao=Clock.getSecond();//读取3231信息
      fen=Clock.getMinute();
      shi=Clock.getHour(h12,PM);
      ri=Clock.getDate();
      yue=Clock.getMonth(Century);
      nian=Clock.getYear();
      zhou=Clock.getDoW();    
      //Clock.getTemperature();//读取3231温度
      Conversion(Century,nian,yue,ri);//农历计算
      read_jieqi(nian,yue,ri);//节气计算
      shidu=dht.getHumidity();//读取湿度
      wendu=dht.getTemperature();//读取温度
      send1();
      print_send1=print_send1+print_send2;//正在连接WiFi
      Serial.println(print_send1);
      print_send1="";
      delay(500);
    }
    udp.begin(localPort);
    WiFi.hostByName(ntpServerName,timeServerIP);//获取IP
    
    init_Web();

    flipper2.attach(postingInterval,heartbeat);//开启定时器
    flipper1.attach_ms(500,timer_500ms);//开启定时器
  }

  if(!client2.connected())
  {
    if(!client2.connect(host,httpPort)) 
    {
      Serial.println("connection failed");
      return;
    }
  }
  
  server.handleClient();
  MDNS.update();    
  
  if(init_flag==0)
  {
    send2();    
    init_flag=1;    
  }
  if(Timer_ms>=120)//一分钟
  {
    send2();
    Timer_ms=0;
  }
  
  read_DHT();
  
  //接收服务器返回数据
  if(client2.available())//有数据接收
  {
    String inputString=client2.readStringUntil('\n');//读取数据
    inputString.trim();
    //Serial.println(inputString);//打印数据
    int len=inputString.length()+1;//计算数据长度
    if(inputString.startsWith("{") && inputString.endsWith("}"))//接收的是有效数据
    {
      char jsonString[len];
      inputString.toCharArray(jsonString,len);//把string转换成数值存到数组
      aJsonObject *msg = aJson.parse(jsonString);//ajson解析
      processMessage(msg);//处理数据
      aJson.deleteItem(msg);
    }
  }
}
//==============================================NTP获取数据
void sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer,0,NTP_PACKET_SIZE);
  packetBuffer[0]=0b11100011;   // LI, Version, Mode
  packetBuffer[1]=0;     // Stratum, or type of clock
  packetBuffer[2]=6;     // Polling Interval
  packetBuffer[3]=0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]=49;
  packetBuffer[13]=0x4E;
  packetBuffer[14]=49;
  packetBuffer[15]=52;
  udp.beginPacket(address,123); //NTP requests are to port 123
  udp.write(packetBuffer,NTP_PACKET_SIZE);
  udp.endPacket();
}
void NTP_Time()//从NTP获取时间
{
  sendNTPpacket(timeServerIP);//向时间服务器发送NTP数据包
  int cb=udp.parsePacket();
  if(!cb)//NTP获取出错
  {
    miao=Clock.getSecond();//读取3231信息
    fen=Clock.getMinute();
    shi=Clock.getHour(h12,PM);
    ri=Clock.getDate();
    yue=Clock.getMonth(Century);
    nian=Clock.getYear();
    zhou=Clock.getDoW();
    //Clock.getTemperature();//读取3231温度
  }
  else//获取数据正常
  {
    udp.read(packetBuffer,NTP_PACKET_SIZE);//读取数据包 
    unsigned long highWord=word(packetBuffer[40],packetBuffer[41]);
    unsigned long lowWord=word(packetBuffer[42],packetBuffer[43]);
    unsigned long secsSince1900=highWord<<16|lowWord;
    const unsigned long seventyYears=2208988800UL;
    
    //unsigned long epoch=secsSince1900-seventyYears;//Unix时间
    unsigned long epoch=secsSince1900-seventyYears+8*60*60;//北京时间   
    
    setTime(epoch);//转换时间
    nian=year()-2000;//年
    yue=month();//月
    ri=day();//日
    shi=hour();//时
    fen=minute();//分
    miao=second();//秒
    zhou=weekday()-1;//周，因为星期一是星期日，所以要减一
    if(zhou==0)
      zhou=7;  
    //写入时钟芯片
    if(nian!=Clock.getYear())//不等于时钟芯片内的信息
      Clock.setYear(nian);//写入准确信息
    if(ri!=Clock.getDate())
      Clock.setDate(ri);
    if(yue!=Clock.getMonth(Century))    
      Clock.setMonth(yue); 
    if(zhou!=Clock.getDoW())
      Clock.setDoW(zhou);
    if(shi!=Clock.getHour(h12,PM))
      Clock.setHour(shi);
    if(fen!=Clock.getMinute())
      Clock.setMinute(fen);
    if(miao!=Clock.getSecond())
      Clock.setSecond(miao);
  }
  if(shi==0&&fen==0&&miao==0&&init_flag==1)//零点强制获取数据
  {
    Timer_ms=0;
    init_flag=0;
  }
  Conversion(Century,nian,yue,ri);//农历计算
  read_jieqi(nian,yue,ri);//节气计算
}
//==============================================贝壳物联处理数据
void processMessage(aJsonObject *msg)
{
  aJsonObject* method=aJson.getObjectItem(msg,"M");
  aJsonObject* content=aJson.getObjectItem(msg,"C");     
  aJsonObject* client_id=aJson.getObjectItem(msg,"ID");
  if(!method)
    return; 
  String M=method->valuestring;
  if(M=="say")
  {  
    String C=content->valuestring;
    String F_C_ID=client_id->valuestring;
    if(C=="stop")//接收到的是停止指令
    {
      if(on_off==1)
      {
        on_off=0;
        Timer_ms=0;
        init_flag=0;
        flipper1.detach();//停止定时器      
        print_send1="                                ";
        print_send2="                                ";
        print_send1="$"+print_send1+print_send2;
        Serial.println(print_send1);
        print_send1="";
        print_send2="";               
      }
      sayToClient(F_C_ID,"OFF!");
    }
    else//接收到的不是停止指令
    {
      if(on_off==0)
      {
        on_off=1;
        Timer_ms=0;
        init_flag=0;
        print_send1="";
        print_send2="";
        flipper1.attach_ms(500,timer_500ms);//开启定时器        
      }
      sayToClient(F_C_ID,"ON!"); 
    }
  }
  if(M=="WELCOME TO BIGIOT")//连接上但没登录
  {
    checkout();//消除滞留
    delay(500);
    BKstatus();//查询一次状态
  }
  if(M=="connected")//状态是未登录
    checkIn();//登录设备
}
//==============================================贝壳物联发送指令
void checkIn()//设备登录
{
  String msg="{\"M\":\"checkin\",\"ID\":\"" + DEVICEID + "\",\"K\":\"" + APIKEY + "\"}\n";
  client2.print(msg);
}
void sayToClient(String client_id, String content)//向设备返回数据
{
  String msg="{\"M\":\"say\",\"ID\":\"" + client_id + "\",\"C\":\"" + content + "\"}\n";
  client2.print(msg);
}
void checkout()//强制设备下线
{
  String msg="{\"M\":\"checkout\",\"ID\":\"" + DEVICEID + "\",\"K\":\"" + APIKEY + "\"}\n";
  client2.print(msg);
}
void BKstatus()//查询当前设备状态
{
  String msg="{\"M\":\"status\"}\n";
  client2.print(msg);
}
