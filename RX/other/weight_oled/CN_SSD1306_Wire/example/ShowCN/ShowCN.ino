#include <Wire.h>
#include <CN_SSD1306_Wire.h>

#include "codetab.c"   //codetab

CN_SSD1306_Wire lucky(8);//HardWare I2C

void setup()
{
  lucky.Initial();
  delay(10);
}

void loop()
{
  unsigned char i;
  lucky.Fill_Screen(0xff,0xff);
  delay(2000);
  lucky.Fill_Screen(0x00,0x00);
  for(i=0;i<=3;i++)
  {
    lucky.ShowCN(0+i*16,i*2,0);
    lucky.ShowCN(16+i*16,i*2,1);
    lucky.ShowCN(32+i*16,i*2,2);
    lucky.ShowCN(48+i*16,i*2,3);
    lucky.ShowCN(64+i*16,i*2,4);
  }
  delay(5000);
}
