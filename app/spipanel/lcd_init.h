#ifndef __LCD_INIT_H__
#define __LCD_INIT_H__

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#define LCD_W 240
#define LCD_H 240

extern int LCD_SCL;
extern int LCD_SDA;
extern int LCD_RES;
extern int LCD_DC;
extern int LCD_BLK;

//-----------------LCD端口定义----------------

#define LCD_SCLK_Clr() LCD_SCL=0//SCL=SCLK
#define LCD_SCLK_Set() LCD_SCL=1

#define LCD_MOSI_Clr() LCD_SDA=0//SDA=MOSI
#define LCD_MOSI_Set() LCD_SDA=1

#define LCD_RES_Clr() LCD_RES=0//RES
#define LCD_RES_Set() LCD_RES=1

#define LCD_DC_Clr() LCD_DC=0//DC
#define LCD_DC_Set() LCD_DC=1

#define LCD_BLK_Clr()  LCD_BLK=0//BLK
#define LCD_BLK_Set()  LCD_BLK=1

void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(unsigned char dat);//模拟SPI时序
void LCD_WR_DATA8(unsigned char dat);//写入一个字节
void LCD_WR_DATA(unsigned short dat);//写入两个字节
void LCD_WR_REG(unsigned char dat);//写入一个指令
void LCD_Address_Set(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif

