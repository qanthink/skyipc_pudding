/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
*/

#include "spi.h"
#include "iostream"
#include <sys/stat.h>
#include <fcntl.h>
#include "string.h"

using namespace std;

Spi* Spi::getInstance()
{
	static Spi spi;
	return &spi;
}

Spi::Spi()
{
	enable();
}

Spi::~Spi()
{
	disable();
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Spi::enable()
{
	cout << "Call Spi::enable()." << endl;

	if(bEnable)
	{
		return 0;
	}


	cout << "Call Spi::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Spi::disable()
{
	cout << "Call Spi::disable()." << endl;

	bEnable = false;
	
	cout << "Call Spi::disable() end." << endl;
	return 0;
}

/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void Spi::LCD_WR_REG(u8 dat)
{
	LCD_DC_Clr();//写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set();//写数据
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void Spi::LCD_WR_DATA8(u8 dat)
{
	LCD_Writ_Bus(dat);
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void Spi::LCD_WR_DATA(u16 dat)
{
	//cout << "aaa" << endl;
	LCD_Writ_Bus(dat>>8);
	//cout << "bbbb" << endl;
	LCD_Writ_Bus(dat);
}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void Spi::LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{
	if(USE_HORIZONTAL==0)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+80);
		LCD_WR_DATA(y2+80);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+80);
		LCD_WR_DATA(x2+80);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
}


/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void Spi::LCD_Writ_Bus(u8 dat)
{	
	u8 i;
	for(i=0;i<8;i++)
	{			  
		LCD_SCLK_Clr();
		if(dat&0x80)
		{
		   LCD_MOSI_Set();
		}
		else
		{
		   LCD_MOSI_Clr();
		}
		LCD_SCLK_Set();
		dat<<=1;
	}	
}


/******************************************************************************
      函数说明：LCD初始化函数
      入口数据：无
      返回值：  无
******************************************************************************/
void Spi::LCD_Init(void)
{
	cout << "000000000000000000000000000000000" << endl;
	LCD_RES_Clr();
	delay_ms(100);
	cout << "11111111111111111" << endl;
	LCD_RES_Set();
	delay_ms(100);
	cout << "22222222222222222222" << endl;
	LCD_BLK_Set();
	delay_ms(100);
	//************* Start Initial Sequence **********//
	cout << "333333333333333333333333" << endl;
	LCD_WR_REG(0x11); //Sleep out 
	delay_ms(120);              //Delay 120ms 
	//************* Start Initial Sequence **********// 
	cout << "4444444444444444444444" << endl;
	LCD_WR_REG(0x36);
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);

	cout << "5555555555555555555555555555" << endl;
	LCD_WR_REG(0x3A); 
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33); 

	LCD_WR_REG(0xB7); 
	LCD_WR_DATA8(0x35);  

	cout << "66666666666666666666666666666666" << endl;
	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x37);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x12);   

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20);  

	LCD_WR_REG(0xC6); 
	LCD_WR_DATA8(0x0F);    

	LCD_WR_REG(0xD0); 
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	cout << "77777777777777777777777777777777777777777777777777" << endl;
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);

	cout << "88888888888888888888888888888888888888888888" << endl;
	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);
	LCD_WR_REG(0x21); 

	LCD_WR_REG(0x29); 
} 

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void Spi::LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 i,j; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{
		cout << "i  = " << i << endl;
		for(j=xsta;j<xend;j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}

int fd_SCLK = -1;
int fd_MOSI = -1;
int fd_RES = -1;
int fd_DC = -1;
int fd_BLK = -1;

static void setGpioValue(int port, const char *direction, int value);


void Spi::GPIO_INIT()
{
	unsigned int port = GPIO_BLK;
	const char *direction = "out";
	char cmd[100] = {0};
	
	sprintf(cmd, "echo %d > /sys/class/gpio/export", port);
	//cout << cmd << endl;
	system(cmd);

	sprintf(cmd, "echo %s > /sys/class/gpio/gpio%d/direction", direction, port);
	//cout << cmd << endl;
	system(cmd);

	sprintf(cmd, "/sys/class/gpio/gpio%d/value", port);
	fd_BLK = open(cmd, O_RDWR);
	if(-1 == fd_BLK)
	{
		cerr << "Fail to call open(3) in GPIO_INIT(). errno = " << errno << endl;
	}

	port = GPIO_MOSI;
	sprintf(cmd, "echo %d > /sys/class/gpio/export", port);
	system(cmd);
	sprintf(cmd, "echo %s > /sys/class/gpio/gpio%d/direction", direction, port);
	system(cmd);
	sprintf(cmd, "/sys/class/gpio/gpio%d/value", port);
	fd_MOSI = open(cmd, O_RDWR);
	if(-1 == fd_MOSI)
	{
		cerr << "Fail to call open(3) in GPIO_INIT(). errno = " << errno << endl;
	}

	port = GPIO_RES;
	sprintf(cmd, "echo %d > /sys/class/gpio/export", port);
	system(cmd);
	sprintf(cmd, "echo %s > /sys/class/gpio/gpio%d/direction", direction, port);
	system(cmd);
	sprintf(cmd, "/sys/class/gpio/gpio%d/value", port);
	fd_RES = open(cmd, O_RDWR);
	if(-1 == fd_RES)
	{
		cerr << "Fail to call open(3) in GPIO_INIT(). errno = " << errno << endl;
	}

	port = GPIO_SCLK;
	sprintf(cmd, "echo %d > /sys/class/gpio/export", port);
	system(cmd);
	sprintf(cmd, "echo %s > /sys/class/gpio/gpio%d/direction", direction, port);
	system(cmd);
	sprintf(cmd, "/sys/class/gpio/gpio%d/value", port);
	fd_SCLK = open(cmd, O_RDWR);
	if(-1 == fd_SCLK)
	{
		cerr << "Fail to call open(3) in GPIO_INIT(). errno = " << errno << endl;
	}

	port = GPIO_DC;
	sprintf(cmd, "echo %d > /sys/class/gpio/export", port);
	system(cmd);
	sprintf(cmd, "echo %s > /sys/class/gpio/gpio%d/direction", direction, port);
	system(cmd);
	sprintf(cmd, "/sys/class/gpio/gpio%d/value", port);
	fd_DC = open(cmd, O_RDWR);
	if(-1 == fd_DC)
	{
		cerr << "Fail to call open(3) in GPIO_INIT(). errno = " << errno << endl;
	}

	//sleep(5);
	//cout << "begin scl" << endl;
	setGpioValue(GPIO_SCLK, NULL, 0);
	//sleep(5);
	setGpioValue(GPIO_SCLK, NULL, 1);
	//sleep(5);

	//cout << "begin sda" << endl;
	setGpioValue(GPIO_MOSI, NULL, 0);
	//sleep(5);
	setGpioValue(GPIO_MOSI, NULL, 1);
	//sleep(5);

	//cout << "begin res" << endl;
	setGpioValue(GPIO_RES, NULL, 0);
	//sleep(5);
	setGpioValue(GPIO_RES, NULL, 1);
	//sleep(5);

	//cout << "begin dc" << endl;
	setGpioValue(GPIO_DC, NULL, 0);
	//sleep(5);
	setGpioValue(GPIO_DC, NULL, 1);
	//sleep(5);

	//cout << "begin blk" << endl;
	setGpioValue(GPIO_BLK, NULL, 0);
	//sleep(5);
	setGpioValue(GPIO_BLK, NULL, 1);
	//sleep(5);
}

static void setGpioValue(int port, const char *direction, int value)
{
	char cmd[100] = {0};
	//sprintf(cmd, "echo %d > /sys/class/gpio/export", port);
	//cout << cmd << endl;
	//system(cmd);

	//sprintf(cmd, "echo %s > /sys/class/gpio/gpio%d/direction", direction, port);
	//cout << cmd << endl;
	//system(cmd);

	#if 0
	sprintf(cmd, "echo %d > /sys/class/gpio/gpio%d/value", value, port);
	//cout << cmd << endl;
	system(cmd);
	#else

	const char *val = NULL;
	0 == value ? val = "0" : val = "1";
	//cout << val << endl;
	
	int ret = 0;
	switch(port)
	{
		case GPIO_BLK:
		{
			ret = write(fd_BLK, val, sizeof(val));
			break;
		}
		case GPIO_DC:
		{
			ret = write(fd_DC, val, sizeof(val));
			break;
		}
		case GPIO_MOSI:
		{
			ret = write(fd_MOSI, val, sizeof(val));
			break;
		}
		case GPIO_RES:
		{
			ret = write(fd_RES, val, sizeof(val));
			break;
		}
		case GPIO_SCLK:
		{
			ret = write(fd_SCLK, val, sizeof(val));
			break;
		}
		default:
		{
			cout << "No Case." << endl;
		}
	}

	if(-1 == ret)
	{
		cerr << "Fail to call write(2). errno = " << errno << ", " << strerror(errno) << endl;
	}

	#endif

	//sprintf(cmd, "echo %d > /sys/class/gpio/unexport", port);
	//cout << cmd << endl;
	//system(cmd);
}

//#define LCD_RES_Clr() LCD_RES=0//RES
void Spi::LCD_RES_Clr()
{
	setGpioValue(GPIO_RES, "out", 0);
}

//#define LCD_RES_Set() LCD_RES=1
void Spi::LCD_RES_Set()
{
	setGpioValue(GPIO_RES, "out", 1);
}

//#define LCD_BLK_Clr()  LCD_BLK=0//BLK
void Spi::LCD_BLK_Clr()
{
	setGpioValue(GPIO_BLK, "out", 0);
}

//#define LCD_BLK_Set()  LCD_BLK=1
void Spi::LCD_BLK_Set()
{
	setGpioValue(GPIO_BLK, "out", 1);
}


//#define LCD_DC_Clr() LCD_DC=0//DC
void Spi::LCD_DC_Clr()
{
	setGpioValue(GPIO_DC, "out", 0);
}

//#define LCD_DC_Set() LCD_DC=1
void Spi::LCD_DC_Set()
{
	setGpioValue(GPIO_DC, "out", 1);
}

//#define LCD_SCLK_Clr() LCD_SCL=0//SCL=SCLK
void Spi::LCD_SCLK_Clr()
{
	setGpioValue(GPIO_SCLK, "out", 0);
}

//#define LCD_SCLK_Set() LCD_SCL=1
void Spi::LCD_SCLK_Set()
{
	setGpioValue(GPIO_SCLK, "out", 1);
}

//#define LCD_MOSI_Clr() LCD_SDA=0//SDA=MOSI
void Spi::LCD_MOSI_Clr()
{
	setGpioValue(GPIO_MOSI, "out", 0);
}

//#define LCD_MOSI_Set() LCD_SDA=1
void Spi::LCD_MOSI_Set()
{
	setGpioValue(GPIO_MOSI, "out", 1);
}


/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Spi::fun()
{
	cout << "Call Spi::fun()." << endl;

	u8 i,j;
	float t=0;
	LCD_Init();//LCD初始化
	//sleep(5);
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	while(1)
	{
		//LCD_ShowChinese(0,0,"中景园电子",RED,WHITE,32,0);
		//LCD_ShowString(0,40,"LCD_W:",RED,WHITE,16,0);
		//LCD_ShowIntNum(48,40,LCD_W,3,RED,WHITE,16);
		//LCD_ShowString(80,40,"LCD_H:",RED,WHITE,16,0);
		//LCD_ShowIntNum(128,40,LCD_H,3,RED,WHITE,16);
		//LCD_ShowString(80,40,"LCD_H:",RED,WHITE,16,0);
		//LCD_ShowString(0,70,"Increaseing Nun:",RED,WHITE,16,0);
		//LCD_ShowFloatNum1(128,70,t,4,RED,WHITE,16);
		sleep(10);
		t+=0.11;
		for(j=0;j<3;j++)
		{
			for(i=0;i<6;i++)
			{
				//LCD_ShowPicture(40*i,120+j*40,40,40,gImage_1);
			}
		}
	}
	
	cout << "Call Spi::fun() end." << endl;
	return 0;
}

