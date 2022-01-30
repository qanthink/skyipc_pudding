/*---------------------------------------------------------------- 
sigma star版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
*/

#ifndef __SPI_H__
#define __SPI_H__

/*
设计思路：
*/

#include "unistd.h"

#define WHITE         	 0xFFFF

#define LCD_W 240
#define LCD_H 240

#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned int

#define GPIO_SCLK 48
#define GPIO_MOSI 49
#define GPIO_RES 46
#define GPIO_DC 47
#define GPIO_BLK 108

#define USE_HORIZONTAL 0

extern int fd_SCLK;
extern int fd_MOSI;
extern int fd_RES;
extern int fd_DC;
extern int fd_BLK;

#define delay_ms(x) usleep(((x) * 1000))

class Spi{
public:
	static Spi* getInstance();

	int enable();
	int disable();

	int fun();
	void LCD_Init(void);
	void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);
	
	void LCD_RES_Clr();
	void LCD_RES_Set();
	void LCD_BLK_Clr();
	void LCD_BLK_Set();
	void LCD_DC_Clr();
	void LCD_DC_Set();
	void LCD_SCLK_Clr();
	void LCD_SCLK_Set();
	void LCD_MOSI_Clr();
	void LCD_MOSI_Set();
		
	void LCD_WR_REG();
	void LCD_WR_REG(u8 dat);
	void LCD_WR_DATA8(u8 dat);
	void LCD_WR_DATA(u16 dat);
	void LCD_Writ_Bus(u8 dat);
	void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数

	void GPIO_INIT();	

private:
	Spi();
	~Spi();
	Spi(const Spi&);
	Spi& operator=(const Spi&);

	bool bEnable;
};

#endif

