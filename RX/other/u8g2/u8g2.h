#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>


U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,SCL,SDA); //配置构造函数

void setup() {
	u8g2.begin(); //启动u8g2驱动程序 / 可按需更改u8x8驱动程序
}

void loop() {
	u8g2.firstPage();
	do {
		//u8g2.setFont(u8g2_font_unifont_t_chinese1);
		u8g2.setFont(u8g2_font_wqy16_t_gb2312b);
		u8g2.drawUTF8(0,15,"编译上传编译上传");
        u8g2.drawUTF8(0,15+16,"编译上传编译上传");
        u8g2.drawUTF8(0,15+16*2,"编译上传编译上传");
        u8g2.drawUTF8(0,15+16*3,"编译上传编译上传");
	} while ( u8g2.nextPage() );
	delay(1000);
}