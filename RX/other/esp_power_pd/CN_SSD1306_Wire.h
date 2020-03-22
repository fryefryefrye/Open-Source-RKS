/***************************************************************************
* 最新版本的“I2C_wtih_Wire”会发布在 http://www.14blog.com/archives/1358
* 
* 建议和问题反馈，请发邮件至 hello14blog@gmail.com
***************************************************************************/

#ifndef _CN_SSD1306_Wire_H
#define _CN_SSD1306_Wire_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

class CN_SSD1306_Wire
{
 public:
  //CN_SSD1306_Wire();//int rst
  
  void WriteCommand(unsigned char ins);
  void WriteData(unsigned char dat);
  void IIC_SetPos(unsigned char x, unsigned char y);
  void Fill_Screen(unsigned char dat);
  void ShowCN(unsigned char x, unsigned char y, unsigned char N);
  void ShowCN3232(unsigned char x, unsigned char y, unsigned char N);
  void ShowASCII816(unsigned char x, unsigned char y, unsigned char N);
  void ShowASCII1632(unsigned char x, unsigned char y, unsigned char N);
  void Initial();
  
  private:
  int _rst;
};
#endif