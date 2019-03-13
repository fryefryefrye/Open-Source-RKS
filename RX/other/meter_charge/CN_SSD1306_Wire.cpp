/***************************************************************************
* 最新版本的“I2C_wtih_Wire”会发布在 http://www.14blog.com/archives/1358
* 
* 建议和问题反馈，请发邮件至 hello14blog@gmail.com
***************************************************************************/

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "Wire.h"

#include "CN_SSD1306_Wire.h"
#include "codetab.c"

//定义类
//CN_SSD1306_Wire::CN_SSD1306_Wire()//int rst
//{
//  //_rst = rst;
//  //pinMode(rst, OUTPUT);
//}

//IIC写命令 -- 命令操作很少，对速度影响不大
void CN_SSD1306_Wire::WriteCommand(unsigned char ins)
{
  Wire.beginTransmission(0x78 >> 1);//0x78 >> 1
  Wire.write(0x00);//0x00
  Wire.write(ins);
  Wire.endTransmission();
}

//开始IIC写数据 -- 这样可以让一组数据发送完成后再关闭IIC，能很大程度提速
void CN_SSD1306_Wire::WriteData(unsigned char dat)
{
  Wire.beginTransmission(0x78 >> 1);//0x78 >> 1
  Wire.write(0x40);//0x40
  Wire.write(dat);
  Wire.endTransmission();
}

//设置起始点坐标
void CN_SSD1306_Wire::IIC_SetPos(unsigned char x, unsigned char y)
{
  WriteCommand(0xb0+y);
  WriteCommand(((x&0xf0)>>4)|0x10);//|0x10
  //WriteCommand(((x&0xf0)>>4));//|0x10
  //WriteCommand((x&0x0f)|0x01);//|0x01
  WriteCommand((x&0x0f));//|0x01
}

//全屏显示 -- Fill_Screen(0x00,0x00)可用作清屏
void CN_SSD1306_Wire::Fill_Screen(unsigned char dat)
{
   unsigned char x,y;

     WriteCommand(0x02);    /*set lower column address*/       
     WriteCommand(0x10);    /*set higher column address*/
     WriteCommand(0xB0);    /*set page address*/
     for(y=0;y<8;y++)
        {
           WriteCommand(0xB0+y);    /*set page address*/
           //WriteCommand(0x02);    /*set lower column address*/       
           //WriteCommand(0x10);    /*set higher column address*/
		   WriteCommand(0x10);    /*set lower column address*/       
		   WriteCommand(0x0);    /*set higher column address*/
           for(x=0;x<128;x++)
             {
              WriteData(dat);
              //WriteData(dat2);
             }
       }
}

//显示16x16的中文
void CN_SSD1306_Wire::ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int adder=32*N;
	IIC_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteData(CN16x16[adder]);
		adder += 1;
	}
	IIC_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteData(CN16x16[adder]);
		adder += 1;
	}
}

//显示32x32的中文
void CN_SSD1306_Wire::ShowCN3232(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned char i=0;
	unsigned int adder=32*N;

	unsigned int CN3232[32];

	for(wm = 0;wm < 32;wm++)
	{
		CN3232[wm] = 0;
		for(i = 0;i < 8;i++)
		{
			CN3232[wm] = CN3232[wm] + (((((unsigned int)CN16x16[adder]>>i)&1)*3)<<(2*i));
		}
		adder += 1;	
	}

	IIC_SetPos(x , y);
	for(wm = 0;wm < 32;wm++)
	{
		WriteData(*((unsigned char*)(&CN3232[wm/2])));
	}


	IIC_SetPos(x,y + 1);
	for(wm = 0;wm < 32;wm++)
	{
		WriteData(*(((unsigned char*)(&CN3232[wm/2]))+1));
	}

	IIC_SetPos(x , y+2);
	for(wm = 0;wm < 32;wm++)
	{
		WriteData(*(((unsigned char*)(&CN3232[wm/2+16]))));
	}


	IIC_SetPos(x,y + 3);
	for(wm = 0;wm < 32;wm++)
	{
		WriteData(*(((unsigned char*)(&CN3232[wm/2+16]))+1));
	}

}

//显示8*16的英文
void CN_SSD1306_Wire::ShowASCII816(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int adder=16*N;
	IIC_SetPos(x , y);
	for(wm = 0;wm < 8;wm++)
	{
		WriteData(ASCII816[adder]);
		adder += 1;
	}


	IIC_SetPos(x,y + 1);
	for(wm = 0;wm < 8;wm++)
	{
		WriteData(ASCII816[adder]);
		adder += 1;
	}
}

//显示8*16的英文
void CN_SSD1306_Wire::ShowASCII1632(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned char i=0;
	unsigned int adder=16*N;

	unsigned int ASCII1632[16];

	for(wm = 0;wm < 16;wm++)
	{
		ASCII1632[wm] = 0;
		for(i = 0;i < 8;i++)
		{
			ASCII1632[wm] = ASCII1632[wm] + (((((unsigned int)ASCII816[adder]>>i)&1)*3)<<(2*i));
		}
		adder += 1;	
	}

	IIC_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteData(*((unsigned char*)(&ASCII1632[wm/2])));
	}


	IIC_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteData(*(((unsigned char*)(&ASCII1632[wm/2]))+1));
	}

	IIC_SetPos(x , y+2);
	for(wm = 0;wm < 16;wm++)
	{
		WriteData(*(((unsigned char*)(&ASCII1632[wm/2+8]))));
	}


	IIC_SetPos(x,y + 3);
	for(wm = 0;wm < 16;wm++)
	{
		WriteData(*(((unsigned char*)(&ASCII1632[wm/2+8]))+1));
	}
}



//SSD1106初始化
void CN_SSD1306_Wire::Initial()
{
//  digitalWrite(_rst, LOW);
  Wire.begin();
    WriteCommand(0xAE);//display off
  
  WriteCommand(0x00);//set lower column address
  WriteCommand(0x10);//set higher column address
  
  WriteCommand(0x40);//set display start line
  
  WriteCommand(0xB0);//set page address
  
  WriteCommand(0x81);//对比度设置
  WriteCommand(0xCF);//0~255（对比度值……效果不是特别明显）
  
  WriteCommand(0xA1);//set segment remap
  
  WriteCommand(0xA6);//normal / reverse
  
  WriteCommand(0xA8);//multiplex ratio
  WriteCommand(0x3F);//duty = 1/64
  
  WriteCommand(0xC8);//Com scan direction
  
  WriteCommand(0xD3);//set display offset
  WriteCommand(0x00);
  
  WriteCommand(0xD5);//set osc division
  WriteCommand(0x80);
  
  WriteCommand(0xD9);//set pre-charge period
  WriteCommand(0xF1);
  
  WriteCommand(0xDA);//set COM pins
  WriteCommand(0x12);
  
  WriteCommand(0xDB);//set vcomh
  WriteCommand(0x40);
  
  WriteCommand(0x8D);//set charge pump enable
  WriteCommand(0x14);
  
  WriteCommand(0xAF);//display ON
#if 0 //SH1106
  WriteCommand(0xAE);    /*display off*/

  WriteCommand(0x02);    /*set lower column address*/
  WriteCommand(0x10);    /*set higher column address*/

  WriteCommand(0x40);    /*set display start line*/

  WriteCommand(0xB0);    /*set page address*/

  WriteCommand(0x81);    /*contract control*/
  WriteCommand(0x80);    /*128*/

  WriteCommand(0xA1);    /*set segment remap*/

  WriteCommand(0xA6);    /*normal / reverse*/

  WriteCommand(0xA8);    /*multiplex ratio*/
  WriteCommand(0x3F);    /*duty = 1/32*/

  WriteCommand(0xad);    /*set charge pump enable*/
  WriteCommand(0x8b);     /*    0x8a    外供VCC   */

  WriteCommand(0x30);    /*0X30---0X33  set VPP   9V 电压可以调整亮度!!!!*/

  WriteCommand(0xC8);    /*Com scan direction*/

  WriteCommand(0xD3);    /*set display offset*/
  WriteCommand(0x00);   /*   0x20  */

  WriteCommand(0xD5);    /*set osc division*/
  WriteCommand(0x80);    

  WriteCommand(0xD9);    /*set pre-charge period*/
  WriteCommand(0x1f);    /*0x22*/

  WriteCommand(0xDA);    /*set COM pins*/
  WriteCommand(0x12);//0x02 -- duanhang xianshi,0x12 -- lianxuhang xianshi!!!!!!!!!

  WriteCommand(0xdb);    /*set vcomh*/
  WriteCommand(0x40);     


  WriteCommand(0xAF);    /*display ON*/
#endif
}