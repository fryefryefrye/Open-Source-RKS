#include"ZiMo.h"
#include <Ticker.h>//定时器
/*全局变量定义*/
unsigned long suoyin[500];//索引
unsigned char RX_flag;//接收数据类型，颜色，数据
unsigned int zishu;//接收字数 8*16为一个字
unsigned int zishu_num;//已移动字数
unsigned int zi;//滚动字
unsigned char row;//行扫描
unsigned char liang;//亮度
unsigned char Timer_ms;//时间
unsigned char colo;//颜色
unsigned char sp,spe;//速度
unsigned char offset;//流动列
//GPIO口设定
const unsigned char R1=9;//  rx  上半屏红色数据L低电平有效
const unsigned char R2=10;// tx  下半屏红色数据L低电平有效
const unsigned char G1=2;//  D4  上半屏绿色数据L低电平有效
const unsigned char G2=0;//  D3  下半屏绿色数据L低电平有效
const unsigned char STR=4;// D2  锁存 LT
const unsigned char CLK=5;// D1  时钟 SK
const unsigned char LA=15;// D8  138-A
const unsigned char LB=13;// D7  138-B
const unsigned char LC=12;// D6  138-C
const unsigned char LD=14;// D5  138-D
const unsigned char EN=16;// D0  使能

Ticker flipper;

void bianma(unsigned int add1,unsigned int add2,unsigned int add3)
{
  unsigned char i;
  unsigned int add_num[]={0,512,4608,8704,12800,16896};
  if(add1>=0xe4&&add1<=0xe9)//在UTF8汉字编码内
  {
    add1=add1-0xe4;
    if(add1==0)
      add2=add2-0xb8;
    else
      add2=add2-0x80;
    add3=add3-0x80;
    //因为一个汉字显示是32个字节，而显示要16字节，所以拆分成两半
    suoyin[zishu]=(add_num[add1]+add2*64+add3)*32;//放到索引
    zishu++;//累加按顺序放入显示字节，也是显示个数
    suoyin[zishu]=(add_num[add1]+add2*64+add3)*32+16;//放到索引
    zishu++;
  }
  else if(add1>=0x20&&add1<=0x7e)//这个区间为ASCII字符
  {
    add1=add1-0x20;
    //显示一个ASCII字符要16字节
    suoyin[zishu]=669696+add1*16;//放到索引
    zishu++;
  }
  else if(add1==0xe2&&add2==0x84&&add3==0x83)//℃
  {
    suoyin[zishu]=671216;
    zishu++;
  }
}
void Auto_brightness()//自动亮度
{
  unsigned int ADC;
  ADC=analogRead(0);//读取ADC 0-1024
  if(ADC<=200)
    colo=0x32;
  else if(ADC>=250)
    colo=0x31;
    
  //亮度设置
  if(ADC<=70)
    liang=10;
  else if(ADC<=110)
    liang=50;
  else if(ADC<=200)
    liang=110;
  else if(ADC<=400)
    liang=150;
  else if(ADC<=500)
    liang=210;
  else if(ADC<=700)
    liang=250;
  else
    liang=255;
  //analogWrite(EN,liang);
}
void dis_row()//行扫描
{  
  digitalWrite(LA,row&0x01);
  digitalWrite(LB,row&0x02);
  digitalWrite(LC,row&0x04);
  digitalWrite(LD,row&0x08);
  digitalWrite(STR,LOW);//锁存拉低，产生上升沿
  digitalWrite(STR,HIGH);//锁存拉高
  digitalWrite(EN,HIGH);
  delayMicroseconds(liang);
  digitalWrite(EN,LOW);
}
void dis_send1(unsigned char colo,unsigned char dat)//向第一行595发送数据
{
  if(colo==0x31)//红色
  {
    digitalWrite(G2,HIGH);
    digitalWrite(G1,HIGH);
    digitalWrite(R2,HIGH);
    shiftOut(R1,CLK,MSBFIRST,dat);//向595发送数据
  }
  else if(colo==0x32)//绿色
  {
    digitalWrite(G2,HIGH);
    digitalWrite(R1,HIGH);
    digitalWrite(R2,HIGH);
    shiftOut(G1,CLK,MSBFIRST,dat);//向595发送数据
  }
  else if(colo==0x33)//橙色
  {
    unsigned char i;
    digitalWrite(G2,HIGH);
    digitalWrite(R2,HIGH);
    for(i=0;i<8;i++)
    {
      digitalWrite(R1,dat&0x80);
      digitalWrite(G1,dat&0x80);
      digitalWrite(CLK,HIGH);//时钟拉高，产生下降沿
      digitalWrite(CLK,LOW);//时钟拉低
      dat=dat<<1;//左移一位
    }
  }
}
void dis_send2(unsigned char colo,unsigned char dat,unsigned char colu)//向第二行595发送数据
{
  unsigned char i;
  if(colo==0x31)//红色
  {
    digitalWrite(G2,HIGH);
    digitalWrite(G1,HIGH);
    digitalWrite(R1,HIGH);
    for(i=0;i<colu;i++)
    {
      digitalWrite(R2,dat&0x80);
      digitalWrite(CLK,HIGH);//时钟拉高，产生下降沿
      digitalWrite(CLK,LOW);//时钟拉低
      dat=dat<<1;//左移一位
    }
  }
  else if(colo==0x32)//绿色
  {
    digitalWrite(G1,HIGH);
    digitalWrite(R1,HIGH);
    digitalWrite(R2,HIGH);
    for(i=0;i<colu;i++)
    {
      digitalWrite(G2,dat&0x80);
      digitalWrite(CLK,HIGH);//时钟拉高，产生下降沿
      digitalWrite(CLK,LOW);//时钟拉低
      dat=dat<<1;//左移一位
    }
  }
  else if(colo==0x33)//橙色
  {
    digitalWrite(G1,HIGH);
    digitalWrite(R1,HIGH);
    for(i=0;i<colu;i++)
    {
      digitalWrite(G2,dat&0x80);
      digitalWrite(R2,dat&0x80);
      digitalWrite(CLK,HIGH);//时钟拉高，产生下降沿
      digitalWrite(CLK,LOW);//时钟拉低
      dat=dat<<1;//左移一位
    }
  }
}
void display_roll()//流动显示
{
  unsigned char i;
  if(zi>64)//小于64个字就流动显示
  {
    if(offset==8)//移出一个字
    {
      offset=0;//重新计数，移下一个字
      zishu_num++;
      if(zishu_num>=zi)//全部流动完
        zishu_num=0;//重新计数，重新流动
    }
    for(i=0;i<32;i++)
      dis_send2(colo,indexes[suoyin[zishu_num+i+32]+row],8);//显示字
    dis_send2(colo,indexes[suoyin[zishu_num+64]+row],offset);//加载下一列
  }
  else//大于就固定显示
  {
    for(i=0;i<32;i++)
      dis_send2(colo,indexes[suoyin[i+32]+row],8);
  }
}
void display_send()
{
  for(row=0;row<16;row++)
  {
    unsigned char i;
    for(i=0;i<32;i++)//扫描前16行
      dis_send1(colo,indexes[suoyin[i]+row]);//扫描第一行
    dis_row();//完成一次行扫
    display_roll();
    dis_row();//完成一次行扫
  }
  sp++;
  if(sp>spe)//速度
  {
    sp=0;
    offset++;//加载一列
  }
}

void Timer_500ms()
{
  while(Serial.available()>0)
  {
    unsigned int RX_temp;
    RX_temp=Serial.read();//读取数据
    if(RX_temp=='$')//数据开头
    {
      RX_flag=1;//开始接收数据
      zishu=0;//清除字数，重新计数
    }
    else if(RX_flag==1)//接收显示数据
    {
      if(RX_temp!=0x0d)//不是换行
      {
        if(RX_temp>0x80)//UTF-8编码
          bianma(RX_temp,Serial.read(),Serial.read());
        else//ASCII编码
          bianma(RX_temp,0,0);
      }
      else//是换行
      {
        RX_flag=0;//接收完成，清除标志
        if(zishu>64)
          zi=zishu-64;
        if(zishu==64)
          zi=zishu;
      }
    }
  }
  Timer_ms++;
  if(Timer_ms>=6)
  {
    Timer_ms=0;
    Auto_brightness();//自动亮度
  }
  
}

void setup()
{
  Serial.begin(115200);
  Serial.setRxBufferSize(1024);
  liang=100;//亮度控制
  spe=2;//速度控制
  colo=0x31;//红色
  //设置IO口模式
  pinMode(R1,OUTPUT);
  pinMode(R2,OUTPUT);
  pinMode(G1,OUTPUT);
  pinMode(G2,OUTPUT);
  pinMode(LA,OUTPUT);
  pinMode(LB,OUTPUT);
  pinMode(LC,OUTPUT);
  pinMode(LD,OUTPUT);
  pinMode(CLK,OUTPUT);
  pinMode(STR,OUTPUT);
  pinMode(EN,OUTPUT);
//  analogWriteRange(255);//pwm最大值
//  analogWriteFreq(40000);//频率
//  analogWrite(EN,liang);//初始化亮度
  flipper.attach_ms(500,Timer_500ms);//开启定时器；停止定时器：flipper.detach();  
}

void loop()
{
  display_send();
}
