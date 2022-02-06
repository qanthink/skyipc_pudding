/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
*/

#include "spipanel.h"
#include "lcdfont.h"

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
	panelInit();

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
	panelDeinit();
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
		return -1;
	}

	int ret = 0;
	// spi mode
	ret = ioctl(fdSpiDev, SPI_IOC_WR_MODE, &mode);
	if(-1 == ret)
	{
		cerr << "can't set spi mode\n" << endl;
	}

	ret = ioctl(fdSpiDev, SPI_IOC_RD_MODE, &mode);
	if(-1 == ret)
	{
		cerr << "can't get spi mode\n" << endl;
	}


	// bits per word
	ret = ioctl(fdSpiDev, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if(-1 == ret)
	{
		cerr << "can't set bits per word\n" << endl;
	}

	ret = ioctl(fdSpiDev, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if(-1 == ret)
	{
		cerr << "can't get bits per word\n" << endl;
	}

	// max speed hz
	ret = ioctl(fdSpiDev, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if(-1 == ret)
	{
		cerr << "can't set max speed hz\n" << endl;
	}

	ret = ioctl(fdSpiDev, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if(-1 == ret)
	{
		cerr << "can't get max speed hz\n" << endl;
	}
	
	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d KHz (%d MHz)\n", speed / 1000, speed / 1000 / 1000);
	
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
	struct spi_ioc_transfer stSpiTr = {
		.tx_buf = (unsigned long)NULL,
		.rx_buf = (unsigned long)dataBuf,
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
int SpiPanel::spiDevWrite(const void *dataBuf, unsigned long dataBufLen)
{
	//cout << "Call SpiPanel::spiDevWrite()." << endl;

	if(-1 == fdSpiDev)
	{
		cerr << "Fail to call SpiPanel::spiDevWrite(). Device is not opened." << endl;
		return -1;
	}

	unsigned char rxBuf[128];
	struct spi_ioc_transfer stSpiTr = {
		.tx_buf = (unsigned long)dataBuf,
		.rx_buf = (unsigned long)NULL,
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
int SpiPanel::panelInit()
{
	cout << "Call SpiPanel::PanelInit()." << endl;

	gpioSetVal(PANEL_GPIO_RES, 0);
	usleep(100 * 1000);		// n * 1000 = n ms.
	gpioSetVal(PANEL_GPIO_RES, 1);
	usleep(100 * 1000);		// n * 1000 = n ms.

	gpioSetVal(PANEL_GPIO_BLK, 0);
	gpioSetVal(PANEL_GPIO_BLK, 1);

	gpioSetVal(PANEL_GPIO_DC, 0);
	gpioSetVal(PANEL_GPIO_DC, 1);
	
	//************* Start Initial Sequence **********//	
	unsigned char data = 0;
	data = 0x11;
	panelWriteCmd(&data, 1);
	usleep(120 * 1000);		// n * 1000 = n ms.
	
	data = 0x36;
	panelWriteCmd(&data, 1);

	switch(USE_HORIZONTAL)
	{
		case 0:
			data = 0x00;
			break;
		case 1:
			data = 0xC0;
			break;
		case 2:
			data = 0x70;
			break;
		case 3:
			data = 0xA0;
			break;
		default:
			cerr << "In Lcd_Init(), out of range." << endl;
			data = 0x00;
			break;
	}
	panelWriteData(&data, 1);
	
	data = 0x3A;
	panelWriteCmd(&data, 1);
	data = 0x05;
	panelWriteData(&data, 1);

	data = 0xB2;
	panelWriteCmd(&data, 1);
	data = 0x0C;
	panelWriteData(&data, 1);
	data = 0x0C;
	panelWriteData(&data, 1);
	data = 0x00;
	panelWriteData(&data, 1);
	data = 0x33;
	panelWriteData(&data, 1);
	data = 0x33;
	panelWriteData(&data, 1);

	data = 0xB7;
	panelWriteCmd(&data, 1);
	data = 0x35;
	panelWriteData(&data, 1);

	data = 0xBB;
	panelWriteCmd(&data, 1);
	//data = 0x37;
	data = 0x19;
	panelWriteData(&data, 1);

	data = 0xC0;
	panelWriteCmd(&data, 1);
	data = 0x2C;
	panelWriteData(&data, 1);

	data = 0xC2;
	panelWriteCmd(&data, 1);
	data = 0x01;
	panelWriteData(&data, 1);

	data = 0xC3;
	panelWriteCmd(&data, 1);
	data = 0x12;
	panelWriteData(&data, 1);

	data = 0xC4;
	panelWriteCmd(&data, 1);
	data = 0x20;
	panelWriteData(&data, 1);

	data = 0xC6;
	panelWriteCmd(&data, 1);
	data = 0x0F;
	panelWriteData(&data, 1);

	data = 0xD0;
	panelWriteCmd(&data, 1); 
	data = 0xA4;
	panelWriteData(&data, 1);
	data = 0xA1;
	panelWriteData(&data, 1);

	data = 0xE0;
	panelWriteCmd(&data, 1);
	data = 0xD0;
	panelWriteData(&data, 1);
	data = 0x04;
	panelWriteData(&data, 1);
	data = 0x0D;
	panelWriteData(&data, 1);
	data = 0x11;
	panelWriteData(&data, 1);
	data = 0x13;
	panelWriteData(&data, 1);
	data = 0x2B;
	panelWriteData(&data, 1);
	data = 0x3F;
	panelWriteData(&data, 1);
	data = 0x54;
	panelWriteData(&data, 1);
	data = 0x4C;
	panelWriteData(&data, 1);
	data = 0x18;
	panelWriteData(&data, 1);
	data = 0x0D;
	panelWriteData(&data, 1);
	data = 0x0B;
	panelWriteData(&data, 1);
	data = 0x1F;
	panelWriteData(&data, 1);
	data = 0x23;
	panelWriteData(&data, 1);

	data = 0xE1;
	panelWriteCmd(&data, 1);
	data = 0xD0;
	panelWriteData(&data, 1);
	data = 0x04;
	panelWriteData(&data, 1);
	data = 0x0C;
	panelWriteData(&data, 1);
	data = 0x11;
	panelWriteData(&data, 1);
	data = 0x13;
	panelWriteData(&data, 1);
	data = 0x2C;
	panelWriteData(&data, 1);
	data = 0x3F;
	panelWriteData(&data, 1);
	data = 0x44;
	panelWriteData(&data, 1);
	data = 0x51;
	panelWriteData(&data, 1);
	data = 0x2F;
	panelWriteData(&data, 1);
	data = 0x1F;
	panelWriteData(&data, 1);
	data = 0x1F;
	panelWriteData(&data, 1);
	data = 0x20;
	panelWriteData(&data, 1);
	data = 0x23;
	panelWriteData(&data, 1);
	data = 0x21;
	panelWriteCmd(&data, 1);

	data = 0x29;
	panelWriteCmd(&data, 1);
	
	cout << "Call SpiPanel::PanelInit() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：屏去初始化
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelDeinit()
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
int SpiPanel::panelWriteBus(const void *dataBuf, unsigned long dataBufLen)
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
int SpiPanel::panelWriteCmd(const void *dataBuf, unsigned long dataBufLen)
{
	//cout << "Call SpiPanel::panelWriteCmd()." << endl;

	gpioSetVal(PANEL_GPIO_DC, 0);
	panelWriteBus(dataBuf, dataBufLen);
	
	//cout << "Call SpiPanel::panelWriteCmd() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：写数据到屏
参--数：dataBuf, 数据缓存；dataBufLen, 数据地址。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelWriteData(const void *dataBuf, unsigned long dataBufLen)
{
	//cout << "Call SpiPanel::panelWriteData()." << endl;

	gpioSetVal(PANEL_GPIO_DC, 1);
	panelWriteBus(dataBuf, dataBufLen);
	
	//cout << "Call SpiPanel::panelWriteData() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置屏坐标
参--数：x0, y0, 起始坐标；x1, y1, 终点坐标。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelSetAddress(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1)
{
	//cout << "Call SpiPanel::PanelSetAddress()." << endl;

	unsigned max = MAX((PANEL_WIDTH), (PANEL_HEIGHT));
	if(x0 > max || y0 > max || x1 > max || y1 > max)
	{
		cerr << "Fail to call SpiPanel::panelSetAddress(). Argument is out of range."
			<< "x0, y0, x1, y1 = " << x0 << ", " << y0 << ", " << x1 << ", " << y1 << endl;
		return -1;
	}

	switch(USE_HORIZONTAL)
	{
		case 0:
			break;
		case 1:
			y0 += 80;
			y1 += 80;
			break;
		case 2:
			break;
		case 3:
			x0 += 80;
			x1 += 80;
			break;
		default:
			cerr << "Call SpiPanel::PanelSetAddress(). Macro 'USE_HORIZONTAL' is out of range." 
				<< "Use default value 0." << endl;
			break;
	}

	unsigned int data = 0x2a;
	panelWriteCmd(&data, 1);	// 列地址设置
	data = x0 >> 8;
	panelWriteData(&data, 1);
	data = x0;
	panelWriteData(&data, 1);
	data = x1 >> 8;
	data = x1;
	panelWriteData(&data, 1);
	
	data = 0x2b;
	panelWriteCmd(&data, 1);	// 行地址设置
	data = y0 >> 8;
	panelWriteData(&data, 1);
	data = y0;
	panelWriteData(&data, 1);
	data = y1 >> 8;
	panelWriteData(&data, 1);
	data = y1;
	panelWriteData(&data, 1);
	
	data = 0x2c;
	panelWriteCmd(&data, 1);	// 储存器写
	
	//cout << "Call SpiPanel::PanelSetAddress() end." << endl;
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

	panelSetAddress(x0, y0, x1, y1);	//设置显示范围
	
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned short mycolor = color;
	for(i= y0; i< y1; ++i)
	{
		for(j = x0; j < x1; ++j)
		{
			panelWriteData(&mycolor, 2);
		}
		//cout << "i = " << i << endl;
	}
	
	cout << "Call SpiPanel::panelFill() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：在指定位置画一个点。
参--数：x, y, 坐标；color, 色彩。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelDrawPoint(unsigned short x, unsigned short y, unsigned short color)
{
	//cout << "Call SpiPanel::panelDrawPoint()." << endl;
	
	panelSetAddress(x, y, x, y);//设置光标位置
	panelWriteData(&color, 2);

	//cout << "Call SpiPanel::panelDrawPoint() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：在指定位置画一条线。
参--数：x0, y0, x1, y1, 起始点和终点坐标；color, 色彩。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelDrawLine(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color)
{
	cout << "Call SpiPanel::panelDrawLine()." << endl;
	int xerr = 0, yerr = 0, delta_x = 0, delta_y = 0, distance = 0;
	int incx = 0, incy = 0, uRow = 0, uCol = 0;
	
	delta_x = x1-x0;	//计算坐标增量 
	delta_y = y1-y0;
	uRow = x0;			//画线起点坐标
	uCol = y0;
	
	if(delta_x > 0)
	{
		incx = 1;		//设置单步方向
	}
	else if(0 == delta_x)
	{
		incx = 0;		//垂直线
	}
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	
	if(delta_y > 0)
	{
		incy = 1;
	}
	else if(0 == delta_y)
	{
		incy = 0;		//水平线
	}
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	
	if(delta_x > delta_y)
	{
		distance = delta_x;	//选取基本增量坐标轴
	}
	else
	{
		distance = delta_y;
	}
	
	unsigned short i = 0;
	for(i = 0; i < distance + 1; ++i)
	{
		panelDrawPoint(uRow, uCol, color);	//画点
		xerr += delta_x;
		yerr += delta_y;
		if(xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if(yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}

	cout << "Call SpiPanel::panelDrawLine() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：在指定位置画一个矩形。
参--数：x0, y0, x1, y1, 起始点和终点坐标；color, 色彩。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelDrawRectangle(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color)
{
	cout << "Call SpiPanel::panelDrawRectangle()." << endl;
	
	panelDrawLine(x0, y0, x1, y0, color);
	panelDrawLine(x0, y0, x0, y1, color);
	panelDrawLine(x0, y1, x1, y1, color);
	panelDrawLine(x1, y0, x1, y1, color);

	cout << "Call SpiPanel::panelDrawRectangle() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：在指定位置画一个圆。
参--数：x, y, 圆心坐标；color, 色彩。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelDrawCircle(unsigned short x, unsigned short y, unsigned char r, unsigned short color)
{
	cout << "Call SpiPanel::panelDrawCircle()." << endl;
	
	int a = 0;
	int b = 0;
	
	b = r;
	while(a <= b)
	{
		panelDrawPoint(x - b, y - a, color);	//3
		panelDrawPoint(x + b, y - a, color);	//0
		panelDrawPoint(x - a, y + b, color);	//1
		panelDrawPoint(x - a, y - b, color);	//2
		panelDrawPoint(x + b, y + a, color);	//4
		panelDrawPoint(x + a, y - b, color);	//5
		panelDrawPoint(x + a, y + b, color);	//6
		panelDrawPoint(x - b, y + a, color);	//7
		
		++a;
		if((a * a + b * b) > (r * r))	//判断要画的点是否过远
		{
			--b;
		}
	}

	cout << "Call SpiPanel::panelDrawCircle() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：求幂。
参--数：m, 底数；n, 指数。
返回值：返回计算结果。
注--意：
-----------------------------------------------------------------------------*/
unsigned int SpiPanel::mathPow(unsigned char m, unsigned char n)
{
	//cout << "Call SpiPanel::mathPow()." << endl;

	unsigned int result = 1;
	while(n--)
	{
		result *= m;
	}

	//cout << "Call SpiPanel::mathPow() end." << endl;
	return result;
}; 	//求幂

/*-----------------------------------------------------------------------------
描--述：在指定位显示一个字符。
参--数：x, y, 坐标；num 要显示的字符；fc 字的颜色；bc 字的背景色；
		sizey 字号；mode: 0, 非叠加模式，1, 叠加模式
返回值：
注--意：
-----------------------------------------------------------------------------*/
int SpiPanel::panelShowChar(unsigned short x, unsigned short y, unsigned char num, unsigned short fc, unsigned short bc, unsigned char sizey, unsigned char mode)
{
	cout << "Call SpiPanel::panelShowChar()." << endl;
#if 1
	unsigned char temp = 0, sizex = 0, t = 0, m = 0;
	unsigned short i = 0, TypefaceNum = 0;//一个字符所占字节大小
	unsigned short x0 = 0;
	
	x0 = x;
	sizex = sizey / 2;
	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
	num -= ' ';		//得到偏移后的值
	panelSetAddress(x, y, x + sizex - 1, y + sizey - 1);	//设置光标位置
	for(i = 0; i< TypefaceNum; ++i)
	{ 
		if(12 == sizey)
		{
			temp = ascii_1206[num][i];		//调用6x12字体
		}
		else if(16 == sizey)
		{
			temp = ascii_1608[num][i];		//调用8x16字体
		}
		else if(24 == sizey)
		{
			temp = ascii_2412[num][i];		//调用12x24字体
		}
		else if(32 == sizey)
		{
			temp = ascii_3216[num][i];		//调用16x32字体
		}
		else
		{
			return 0;
		}
		
		for(t = 0; t < 8; ++t)
		{
			#if 0
			if(!mode)	//非叠加模式
			{
				if(temp & (0x01 << t))
				{
					unsigned char data = 0;
					data = fc >> 8;
					panelWriteData(&data, 1);
					data = fc ;
					panelWriteData(&data, 1);
				}
				else
				{
					unsigned short data = 0;
					data = bc >> 8;
					panelWriteData(&data, 1);
					data = bc ;
					panelWriteData(&data, 1);
				}
				
				++m;
				if(0 == (m % sizex))
				{
					m = 0;
					break;
				}
			}
			#endif
			//else		//叠加模式
			{
				if(temp & (0x01 << t))
				{
					panelDrawPoint(x, y, fc);	//画一个点
				}
				else
				{
					if(!mode)
					{
						
					}
					else
					{
						panelDrawPoint(x, y, bc);	
					}
				}
				
				++x;
				if((x - x0) == sizex)
				{
					x = x0;
					++y;
					break;
				}
			}
		}
	}
#else
	unsigned char temp,sizex,t,m=0;
	unsigned short i,TypefaceNum;//一个字符所占字节大小
	unsigned short x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	panelSetAddress(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else return 0;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t)){
					//LCD_WR_DATA(fc);
					unsigned char data = fc >> 8;
					panelWriteData(&data, 1);
					data = fc;
					panelWriteData(&data, 1);
				}
				else {
					//LCD_WR_DATA(bc);
					unsigned char data = bc >> 8;
					panelWriteData(&data, 1);
					data = bc;
					panelWriteData(&data, 1);
				}
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t)){
					//LCD_DrawPoint(x,y,fc);
					panelDrawPoint(x, y, fc);
				}
				else
					{}
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
#endif
	cout << "Call SpiPanel::panelShowChar() end." << endl;
	return 0;
};				//
	
int SpiPanel::panelShowString(unsigned short x, unsigned short y, const unsigned char *p, unsigned short fc, unsigned short bc, unsigned char sizey, unsigned char mode)
{

	return 0;
}; //显示字符串
	
int SpiPanel::panelShowIntNum(unsigned short x, unsigned short y, unsigned short num, unsigned char len, unsigned short fc, unsigned short bc, unsigned char sizey)
{

	return 0;
};				//显示整数变量
	
int SpiPanel::panelShowFloatNum(unsigned short x, unsigned short y, float num, unsigned char len, unsigned short fc, unsigned short bc, unsigned char sizey)
{

	return 0;
}; 					//显示两位小数变量

