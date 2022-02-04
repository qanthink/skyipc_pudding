/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
*/

#include "spipanel.h"
#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

using namespace std;

SpiPanel* SpiPanel::getInstance()
{
	static SpiPanel spiPanel;
	return &spiPanel;
}

SpiPanel::SpiPanel()
{
	enable();
}

SpiPanel::~SpiPanel()
{
	disable();
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::enable()
{
	cout << "Call SpiPanel::enable()." << endl;

	if(bEnable)
	{
		return 0;
	}

	spiDevOpen();
	gpioInit();
	PanelInit();

	bEnable = true;
	cout << "Call SpiPanel::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::disable()
{
	cout << "Call SpiPanel::disable()." << endl;

	bEnable = false;
	PanelDeinit();
	gpioDeinit();
	spiDevClose();
	
	cout << "Call SpiPanel::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：打开SPI设备
参--数：无
返回值：返回SPI设备文件描述符
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::spiDevOpen()
{
	cout << "Call SpiPanel::spiDevOpen()." << endl;

	if(-1 != fdSpiDev)
	{
		cerr << "Spi device has been already opened. fdSpiDev = " << fdSpiDev << endl;
		return fdSpiDev;
	}

	fdSpiDev = open(spiDevPath, O_RDWR);
	if(-1 == fdSpiDev)
	{
		cerr << "Fail to call open(2) in SpiPanel::spiDevOpen(), errno = " << errno << ", " << strerror(errno);
	}
	
	cout << "Call SpiPanel::spiDevOpen() end." << endl;
	return fdSpiDev;
}

/*-----------------------------------------------------------------------------
描--述：关闭SPI设备
参--数：无
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::spiDevClose()
{
	cout << "Call SpiPanel::spiDevClose()." << endl;

	if(-1 == fdSpiDev)
	{
		cerr << "Spi device has been already closed. fdSpiDev = " << fdSpiDev << endl;
		return fdSpiDev;
	}

	int ret = 0;
	ret = close(fdSpiDev);
	if(-1 == ret)
	{
		cerr << "Fail to call close(2) in SpiPanel::spiDevClose(), errno = " << errno << ", " << strerror(errno);
	}
	fdSpiDev = -1;
	
	cout << "Call SpiPanel::spiDevClose() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：读SPI设备
参--数：dataBuf, 读出数据的缓冲区；dataBufLen, 读出数据的长度。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::spiDevRead(unsigned char *dataBuf, unsigned int dataBufLen)
{
	cout << "Call SpiPanel::spiDevRead()." << endl;

	if(-1 == fdSpiDev)
	{
		cerr << "Fail to call SpiPanel::spiDevRead(). Device is not opened." << endl;
		return -1;
	}

	memset(dataBuf, 0, dataBufLen);
	struct spi_ioc_transfer stSpiTr ={
		.tx_buf = (unsigned)NULL,
		.rx_buf = (unsigned)dataBuf,
		.len = dataBufLen,
		.delay_usecs = 0,
	};

	int ret = 0;
	ret = ioctl(fdSpiDev, SPI_IOC_MESSAGE(1), &stSpiTr);
	if(-1 == ret)
	{
		cerr << "Fail to call ioctl(2) in SpiPanel::spiDevRead(). errno = " 
			<< errno << ", " << strerror(errno) << endl;
	}

	cout << "Call SpiPanel::spiDevRead() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：写SPI设备
参--数：dataBuf, 写入数据的缓冲区；dataBufLen, 写入数据的长度。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::spiDevWrite(unsigned char *dataBuf, unsigned int dataBufLen)
{
	//cout << "Call SpiPanel::spiDevWrite()." << endl;

	if(-1 == fdSpiDev)
	{
		cerr << "Fail to call SpiPanel::spiDevWrite(). Device is not opened." << endl;
		return -1;
	}

	
	struct spi_ioc_transfer stSpiTr ={
		.tx_buf = (unsigned)dataBuf,
		.rx_buf = (unsigned)NULL,
		.len = dataBufLen,
		.delay_usecs = 0,
	};

	int ret = 0;
	ret = ioctl(fdSpiDev, SPI_IOC_MESSAGE(1), &stSpiTr);
	if(-1 == ret)
	{
		cerr << "Fail to call ioctl(2) in SpiPanel::spiDevWrite(). errno = " 
			<< errno << ", " << strerror(errno) << endl;
	}

	//cout << "Call SpiPanel::spiDevWrite() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：初始化GPIO
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::gpioInit()
{
	cout << "Call SpiPanel::gpioInit()." << endl;
	
	char gpioCMD[128] = {0};
	char gpioFullPath[128] = {0};
	const char *directionOut = "out";
	const char *gpioPathPrefix = "/sys/class/gpio/";

	sprintf(gpioCMD, "echo %u > %sexport", PANEL_GPIO_DC, gpioPathPrefix);
	system(gpioCMD);
	sprintf(gpioCMD, "echo %u > %sexport", PANEL_GPIO_BLK, gpioPathPrefix);
	system(gpioCMD);
	sprintf(gpioCMD, "echo %u > %sexport", PANEL_GPIO_RES, gpioPathPrefix);
	system(gpioCMD);

	sprintf(gpioCMD, "echo %s > %s/gpio%u/direction", directionOut, gpioPathPrefix, PANEL_GPIO_DC);
	system(gpioCMD);
	sprintf(gpioCMD, "echo %s > %s/gpio%u/direction", directionOut, gpioPathPrefix, PANEL_GPIO_BLK);
	system(gpioCMD);
	sprintf(gpioCMD, "echo %s > %s/gpio%u/direction", directionOut, gpioPathPrefix, PANEL_GPIO_RES);
	system(gpioCMD);
	
	sprintf(gpioFullPath, "%sgpio%u/value", gpioPathPrefix, PANEL_GPIO_DC);
	fdGpioValDC = open(gpioFullPath, O_RDWR);
	if(-1 == fdGpioValDC)
	{
		cerr << "Fail to call open(3) in SpiPanel::gpioInit(). errno = " 
			<< errno << strerror(errno) << endl;
		return -1;
	}

	sprintf(gpioFullPath, "%sgpio%u/value", gpioPathPrefix, PANEL_GPIO_BLK);
	fdGpioValBLK = open(gpioFullPath, O_RDWR);
	if(-1 == fdGpioValBLK)
	{
		cerr << "Fail to call open(3) in SpiPanel::gpioInit(). errno = " 
			<< errno << strerror(errno) << endl;
		return -1;
	}

	sprintf(gpioFullPath, "%sgpio%u/value", gpioPathPrefix, PANEL_GPIO_RES);
	fdGpioValRES = open(gpioFullPath, O_RDWR);
	if(-1 == fdGpioValRES)
	{
		cerr << "Fail to call open(3) in SpiPanel::gpioInit(). errno = " 
			<< errno << strerror(errno) << endl;
		return -1;
	}

	return 0;
	cout << "Call SpiPanel::gpioInit() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：去初始化GPIO
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::gpioDeinit()
{
	cout << "Call SpiPanel::gpioDeinit()." << endl;

	char gpioCMD[128] = {0};
	const char *gpioPathPrefix = "/sys/class/gpio/";

	if(-1 != fdGpioValDC)
	{
		close(fdGpioValDC);
	}

	if(-1 != fdGpioValBLK)
	{
		close(fdGpioValBLK);
	}

	if(-1 != fdGpioValRES)
	{
		close(fdGpioValRES);
	}

	sprintf(gpioCMD, "echo %u > %sunexport", PANEL_GPIO_DC, gpioPathPrefix);
	system(gpioCMD);
	sprintf(gpioCMD, "echo %u > %sunexport", PANEL_GPIO_BLK, gpioPathPrefix);
	system(gpioCMD);
	sprintf(gpioCMD, "echo %u > %sunexport", PANEL_GPIO_RES, gpioPathPrefix);
	system(gpioCMD);

	cout << "Call SpiPanel::gpioDeinit() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：给GPIO赋值
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::gpioSetVal(unsigned gpioIndex, unsigned val)
{
	//cout << "Call SpiPanel::gpioSetVal()." << endl;

	char gpioVal[2] = {0};
	(0 == val) ? (sprintf(gpioVal, "%s", "0")) : (sprintf(gpioVal, "%s", "1"));	

	int ret = 0;
	switch(gpioIndex)
	{
		case PANEL_GPIO_DC:
			if(-1 != fdGpioValDC)
			{
				ret = write(fdGpioValDC, gpioVal, 1);
			}
			break;
		case PANEL_GPIO_BLK:
			if(-1 != fdGpioValBLK)
			{
				ret = write(fdGpioValBLK, gpioVal, 1);
			}
			break;
		case PANEL_GPIO_RES:
			if(-1 != fdGpioValRES)
			{
				ret = write(fdGpioValRES, gpioVal, 1);
			}
			break;
		default:
			cerr << "Bad gpio index in SpiPanel::gpioSetVal()." << endl;
			break;
	}

	if(-1 == ret)
	{
		cerr << "Fail to call write(2) in SpiPanel::gpioSetVal(). errno = " 
			<< errno << ", " << strerror(errno) << endl;
	}

	//cout << "Call SpiPanel::gpioSetVal() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：初始化屏
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::PanelInit()
{
	cout << "Call SpiPanel::PanelInit()." << endl;

	//LCD_RES_Clr();
	gpioSetVal(PANEL_GPIO_RES, 0);
	usleep(100 * 1000);	// n * 1000 = n ms
	
	//LCD_RES_Set();
	gpioSetVal(PANEL_GPIO_RES, 1);
	usleep(100 * 1000);	// n * 1000 = n ms

	//LCD_BLK_Set();
	gpioSetVal(PANEL_GPIO_BLK, 1);
	usleep(100 * 1000);	// n * 1000 = n ms
	
	//************* Start Initial Sequence **********//
	//LCD_WR_REG(0x11); //Sleep out 
	unsigned char data[128] = {0x11};
	PanelWriteCmd(data, 1);
	usleep(120 * 1000);	// n * 1000 = n ms	//Delay 120ms 
	
	//************* Start Initial Sequence **********// 
	//LCD_WR_REG(0x36);
	data[0] = 0x36;
	PanelWriteCmd(data, 1);

	switch(USE_HORIZONTAL)
	{
		case 0:
			//LCD_WR_DATA8(0x00);
			data[0] = 0x00;
			PanelWriteData(data, 1);
			break;
		case 1:
			//LCD_WR_DATA8(0xC0);
			data[0] = 0xC0;
			PanelWriteData(data, 1);
			break;
		case 2:
			//LCD_WR_DATA8(0x70);
			data[0] = 0x70;
			PanelWriteData(data, 1);
			break;
		case 3:
			//LCD_WR_DATA8(0xA0);
			data[0] = 0xA0;
			PanelWriteData(data, 1);
			break;
		default:
			cerr << "In Lcd_Init(), out of range." << endl;
			//LCD_WR_DATA8(0x00);
			data[0] = 0x00;
			PanelWriteData(data, 1);
			break;
	}
#if 1
	data[0] = 0x3A;
	PanelWriteCmd(data, 1);

	data[0] = 0x05;
	PanelWriteData(data, 1);

	data[0] = 0xB2;
	PanelWriteCmd(data, 1);
	
	data[0] = 0x0C;
	PanelWriteData(data, 1);

	data[0] = 0x0C;
	PanelWriteData(data, 1);

	data[0] = 0x00;
	PanelWriteData(data, 1);

	data[0] = 0x33;
	PanelWriteData(data, 1);

	data[0] = 0x33;
	PanelWriteData(data, 1);

	data[0] = 0xB7;
	PanelWriteCmd(data, 1);

	data[0] = 0x35;
	PanelWriteData(data, 1);

	data[0] = 0xBB;
	PanelWriteCmd(data, 1);

	data[0] = 0x37;
	PanelWriteData(data, 1);

	data[0] = 0xC0;
	PanelWriteCmd(data, 1);

	data[0] = 0x2C;
	PanelWriteData(data, 1);

	data[0] = 0xC2;
	PanelWriteCmd(data, 1);

	data[0] = 0x01;
	PanelWriteData(data, 1);

	data[0] = 0xC3;
	PanelWriteCmd(data, 1);

	data[0] = 0x12;
	PanelWriteData(data, 1);

	data[0] = 0xC4;
	PanelWriteCmd(data, 1);

	data[0] = 0x20;
	PanelWriteData(data, 1);

	data[0] = 0xC6;
	PanelWriteCmd(data, 1);

	data[0] = 0x0F;
	PanelWriteData(data, 1);

	data[0] = 0xD0;
	PanelWriteCmd(data, 1); 

	data[0] = 0xA4;
	PanelWriteData(data, 1);

	data[0] = 0xA1;
	PanelWriteData(data, 1);

	data[0] = 0xE0;
	PanelWriteCmd(data, 1);

	data[0] = 0xD0;
	PanelWriteData(data, 1);

	data[0] = 0x04;
	PanelWriteData(data, 1);

	data[0] = 0x0D;
	PanelWriteData(data, 1);

	data[0] = 0x11;
	PanelWriteData(data, 1);

	data[0] = 0x13;
	PanelWriteData(data, 1);

	data[0] = 0x2B;
	PanelWriteData(data, 1);

	data[0] = 0x3F;
	PanelWriteData(data, 1);

	data[0] = 0x54;
	PanelWriteData(data, 1);

	data[0] = 0x4C;
	PanelWriteData(data, 1);

	data[0] = 0x18;
	PanelWriteData(data, 1);

	data[0] = 0x0D;
	PanelWriteData(data, 1);

	data[0] = 0x0B;
	PanelWriteData(data, 1);

	data[0] = 0x1F;
	PanelWriteData(data, 1);

	data[0] = 0x23;
	PanelWriteData(data, 1);

	data[0] = 0xE1;
	PanelWriteCmd(data, 1);

	data[0] = 0xD0;
	PanelWriteData(data, 1);

	data[0] = 0x04;
	PanelWriteData(data, 1);

	data[0] = 0x0C;
	PanelWriteData(data, 1);

	data[0] = 0x11;
	PanelWriteData(data, 1);

	data[0] = 0x13;
	PanelWriteData(data, 1);

	data[0] = 0x2C;
	PanelWriteData(data, 1);

	data[0] = 0x3F;
	PanelWriteData(data, 1);

	data[0] = 0x44;
	PanelWriteData(data, 1);

	data[0] = 0x51;
	PanelWriteData(data, 1);

	data[0] = 0x2F;
	PanelWriteData(data, 1);

	data[0] = 0x1F;
	PanelWriteData(data, 1);

	data[0] = 0x1F;
	PanelWriteData(data, 1);

	data[0] = 0x20;
	PanelWriteData(data, 1);

	data[0] = 0x23;
	PanelWriteData(data, 1);

	data[0] = 0x21;
	PanelWriteCmd(data, 1); 

	data[0] = 0x29;
	PanelWriteCmd(data, 1); 
	#endif
	
	cout << "Call SpiPanel::PanelInit() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：屏去初始化
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::PanelDeinit()
{
	cout << "Call SpiPanel::PanelInit()." << endl;


	cout << "Call SpiPanel::PanelDeinit() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：写屏的总线
参--数：dataBuf, 数据缓存；dataBufLen, 数据地址。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::PanelWriteBus(unsigned char *dataBuf, unsigned int dataBufLen)
{
	//cout << "Call SpiPanel::PanelWriteBus()." << endl;

	spiDevWrite(dataBuf, dataBufLen);
	
	//cout << "Call SpiPanel::PanelWriteBus() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：写命令到屏
参--数：dataBuf, 数据缓存；dataBufLen, 数据地址。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::PanelWriteCmd(unsigned char *dataBuf, unsigned int dataBufLen)
{
	//cout << "Call SpiPanel::PanelWriteCmd()." << endl;

	//LCD_DC_Clr();//写命令
	gpioSetVal(PANEL_GPIO_DC, 0);
	PanelWriteBus(dataBuf, dataBufLen);
	
	//cout << "Call SpiPanel::PanelWriteCmd() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：写数据到屏
参--数：dataBuf, 数据缓存；dataBufLen, 数据地址。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::PanelWriteData(unsigned char *dataBuf, unsigned int dataBufLen)
{
	//cout << "Call SpiPanel::PanelWriteData()." << endl;

	//LCD_DC_Set();//写数据
	gpioSetVal(PANEL_GPIO_DC, 1);
	PanelWriteBus(dataBuf, dataBufLen);
	
	//cout << "Call SpiPanel::PanelWriteData() end." << endl;
}

