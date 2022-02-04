/*---------------------------------------------------------------- 
sigma star版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
*/

#ifndef __SPIPANEL_H__
#define __SPIPANEL_H__

/*
设计思路：
*/

// Panel GPIO Operation
#define PANEL_GPIO_DC 	47	//数据与指令
#define PANEL_GPIO_BLK 	108	//背光
#define PANEL_GPIO_RES 	46	//复位

#define USE_HORIZONTAL 0	// 旋转0-1代表90°，2-3代表180°
#define PANEL_WIDTH 240
#define PANEL_HEIGHT 240

#define MAX(x, y) (((x) >= (y)) ? ((x)) : ((y)))

class SpiPanel{
public:
	static SpiPanel* getInstance();

	int enable();
	int disable();

	int panelFill(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color);

private:
	SpiPanel();
	~SpiPanel();
	SpiPanel(const SpiPanel&);
	SpiPanel& operator=(const SpiPanel&);

	// SPI Device Operation. Data transfer
	int fdSpiDev = -1;
	const char *spiDevPath = "/dev/spidev1.0";
	int spiDevOpen();
	int spiDevClose();
	int spiDevRead(void *dataBuf, unsigned long dataBufLen);
	int spiDevWrite(void *dataBuf, unsigned long dataBufLen);

	// GPIO Operation
	int fdGpioValRES = -1;
	int fdGpioValDC = -1;
	int fdGpioValBLK = -1;
	int gpioInit();
	int gpioDeinit();
	int gpioSetVal(unsigned int gpioIndex, unsigned int val);

	// Panel Operation
	int PanelInit();
	int PanelDeinit();
	int PanelWriteBus(void *dataBuf, unsigned long dataBufLen);
	int PanelWriteCmd(void *dataBuf, unsigned long dataBufLen);
	int PanelWriteData(void *dataBuf, unsigned long dataBufLen);
	int PanelSetAddress();

	bool bEnable;
};

#endif

