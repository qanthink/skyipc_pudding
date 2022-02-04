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
int SpiPanel::spiDevRead(void *dataBuf, unsigned long dataBufLen)
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
int SpiPanel::spiDevWrite(void *dataBuf, unsigned long dataBufLen)
{
	//cout << "Call SpiPanel::spiDevWrite()." << endl;

	if(-1 == fdSpiDev)
	{
		cerr << "Fail to call SpiPanel::spiDevWrite(). Device is not opened." << endl;
		return -1;
	}

	#if 0
	if(1 == dataBufLen)
	{
		unsigned char data = 0;
		data = *((unsigned char *)dataBuf);
		cout << "data1 = " << hex << (unsigned)data << endl;
	}
	else if(2 == dataBufLen)
	{
		unsigned char data0 = 0;
		unsigned char data1 = 0;
		data0 = *((unsigned char *)dataBuf);
		data1 = ((*((unsigned int *)dataBuf)) >> 8);
		cout << "data2 = " << hex << (unsigned)data0 << ", " << (unsigned)data1 << endl;
	}
	#endif

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
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：给GPIO赋值
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::gpioSetVal(unsigned int gpioIndex, unsigned int val)
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

	gpioSetVal(PANEL_GPIO_RES, 0);
	usleep(100 * 1000);	// n * 1000 = n ms
	gpioSetVal(PANEL_GPIO_RES, 1);
	usleep(100 * 1000);	// n * 1000 = n ms

	gpioSetVal(PANEL_GPIO_BLK, 0);
	usleep(100 * 1000);	// n * 1000 = n ms
	gpioSetVal(PANEL_GPIO_BLK, 1);
	usleep(100 * 1000);	// n * 1000 = n ms

	gpioSetVal(PANEL_GPIO_DC, 0);
	usleep(100 * 1000);	// n * 1000 = n ms
	gpioSetVal(PANEL_GPIO_DC, 1);
	usleep(100 * 1000);	// n * 1000 = n ms
	
	//************* Start Initial Sequence **********//	
	unsigned char data[1] = {0x11};
	PanelWriteCmd(data, 1);
	usleep(120 * 1000);	// n * 1000 = n ms	//Delay 120ms 
	
	//************* Start Initial Sequence **********// 
	data[0] = 0x36;
	PanelWriteCmd(data, 1);

	switch(USE_HORIZONTAL)
	{
		case 0:
			data[0] = 0x00;
			PanelWriteData(data, 1);
			break;
		case 1:
			data[0] = 0xC0;
			PanelWriteData(data, 1);
			break;
		case 2:
			data[0] = 0x70;
			PanelWriteData(data, 1);
			break;
		case 3:
			data[0] = 0xA0;
			PanelWriteData(data, 1);
			break;
		default:
			cerr << "In Lcd_Init(), out of range." << endl;
			data[0] = 0x00;
			PanelWriteData(data, 1);
			break;
	}
	
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
int SpiPanel::PanelWriteBus(void *dataBuf, unsigned long dataBufLen)
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
int SpiPanel::PanelWriteCmd(void *dataBuf, unsigned long dataBufLen)
{
	//cout << "Call SpiPanel::PanelWriteCmd()." << endl;

	gpioSetVal(PANEL_GPIO_DC, 0);
	PanelWriteBus(dataBuf, dataBufLen);
	
	//cout << "Call SpiPanel::PanelWriteCmd() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：写数据到屏
参--数：dataBuf, 数据缓存；dataBufLen, 数据地址。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::PanelWriteData(void *dataBuf, unsigned long dataBufLen)
{
	//cout << "Call SpiPanel::PanelWriteData()." << endl;

	gpioSetVal(PANEL_GPIO_DC, 1);
	PanelWriteBus(dataBuf, dataBufLen);
	
	//cout << "Call SpiPanel::PanelWriteData() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置屏坐标
参--数：x1, y1, 起始坐标；x2, y2, 终点坐标。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::PanelSetAddress()
{
	cout << "Call SpiPanel::PanelSetAddress()." << endl;

	unsigned int x1 = 0;
	unsigned int y1 = 0;
	unsigned int x2 = PANEL_WIDTH;
	unsigned int y2 = PANEL_HEIGHT;

	switch(USE_HORIZONTAL)
	{
		case 0:
			break;
		case 1:
			y1 += 80;
			y2 += 80;
			break;
		case 2:
			break;
		case 3:
			x1 += 80;
			x2 += 80;
			break;
		default:
			cerr << "Call SpiPanel::PanelSetAddress(). Macro 'USE_HORIZONTAL' is out of range." 
				<< "Use default value 0." << endl;
			break;
	}

	unsigned char data = 0x2a;
	PanelWriteCmd(&data, 1);	// 列地址设置
	PanelWriteData(&x1, 2);
	PanelWriteData(&x2, 2);
	data = 0x2b;
	PanelWriteCmd(&data, 1);	// 行地址设置
	PanelWriteData(&y1, 2);
	PanelWriteData(&y2, 2);
	data = 0x2c;
	PanelWriteCmd(&data, 1);	// 储存器写
	
	cout << "Call SpiPanel::PanelSetAddress() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：在矩形范围内填充色彩。
参--数：x0, y0, x1, y1, 起始点和终点坐标；color, 色彩。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelFill(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color)
{
	cout << "Call SpiPanel::panelFill()." << endl;

	unsigned short max = 0;
	max = MAX((PANEL_WIDTH), (PANEL_HEIGHT));

	if(x0 > max || y0 > max || x1 > max || y1 > max)
	{
		cerr << "Fail to call SpiPanel::panelFill(). Argument is out of range."
			<< "x0, y0, x1, y1 = " << x0 << ", " << y0 << ", " << x1 << ", " << y1 << endl;
		return -1;
	}

	PanelSetAddress();	//设置显示范围
	
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned short mycolor = color;
	for(i= y0; i< y1; ++i)
	{
		for(j = x0; j < x1; ++j)
		{
			PanelWriteData(&mycolor, 2);
		}
		//cout << "i = " << i << endl;
	}
	
	cout << "Call SpiPanel::panelFill() end." << endl;
}

