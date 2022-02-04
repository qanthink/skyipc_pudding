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

class SpiPanel{
public:
	static SpiPanel* getInstance();

	int enable();
	int disable();

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
	int spiDevRead(unsigned char *dataBuf, unsigned int dataBufLen);
	int spiDevWrite(unsigned char *dataBuf, unsigned int dataBufLen);

	// GPIO Operation
	int fdGpioValRES = -1;
	int fdGpioValDC = -1;
	int fdGpioValBLK = -1;
	int gpioInit();
	int gpioDeinit();
	int gpioSetVal(unsigned gpioIndex, unsigned val);

	// Panel Operation
	int PanelInit();
	int PanelDeinit();
	int PanelWriteBus(unsigned char *dataBuf, unsigned int dataBufLen);
	int PanelWriteCmd(unsigned char *dataBuf, unsigned int dataBufLen);
	int PanelWriteData(unsigned char *dataBuf, unsigned int dataBufLen);
	int PanelSetAddress(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2);

	bool bEnable;
};

#endif

