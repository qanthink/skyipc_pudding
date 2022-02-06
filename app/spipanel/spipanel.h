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

	// panel 基本点线功能
	int panelFill(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color);	//在指定区域填充颜色
	int panelDrawPoint(unsigned short x, unsigned short y, unsigned short color);	//在指定位置画一个点
	int panelDrawLine(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color);	//在指定位置画一条线
	int panelDrawRectangle(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color);//在指定位置画一个矩形
	int panelDrawCircle(unsigned short x, unsigned short y, unsigned char r, unsigned short color);	//在指定位置画一个圆

	// panel 显示数字+字符
	int panelShowChar(unsigned short x, unsigned short y, unsigned char ch, unsigned short fc, unsigned short bc, unsigned short sizey, bool bCoverMode);			//显示一个字符
	int panelShowString(unsigned short x, unsigned short y, const char *pText, unsigned short fc, unsigned short bc, unsigned short sizey, bool bCoverMode);	//显示字符串
	int panelShowIntNum(unsigned short x, unsigned short y, long long int num, unsigned short fc, unsigned short bc, unsigned short sizey, bool bCoverMode);	//显示整数变量
	int panelShowFloatNum(unsigned short x, unsigned short y, double num, unsigned short fc, unsigned short bc, unsigned short sizey, bool bCoverMode);				//显示两位小数变量

	// panel 显示中文汉字
	int PanelShowChineseFont(unsigned short x, unsigned short y, const char *pFont, unsigned short fc, unsigned short bc, unsigned char fontSize, unsigned char bCoverMode);//显示单个12x12汉字
	int PanelShowChineseText(unsigned short x, unsigned short y, const char *pText, unsigned short fc, unsigned short bc, unsigned char fontSize, unsigned char bCoverMode);//显示汉字串

private:
	SpiPanel();
	~SpiPanel();
	SpiPanel(const SpiPanel&);
	SpiPanel& operator=(const SpiPanel&);

	// SPI Device Operation. Data transfer
	int fdSpiDev = -1;
	const char *spiDevPath = "/dev/spidev1.0";
	unsigned int mode = 2;	// 本程序用到的SPI屏，设置CPOL=1，CPHA=?, 所以mode = 2或3
	unsigned int bits = 8;	// 8bits读写，MSB first
	unsigned int speed = 2 * 1000 * 1000;	// 设置传输速度，N * 1000 * 1000 = N MHz
	int spiDevOpen();
	int spiDevClose();
	int spiDevRead(void *dataBuf, unsigned long dataBufLen);
	int spiDevWrite(const void *dataBuf, unsigned long dataBufLen);

	// GPIO Operation
	int fdGpioValRES = -1;
	int fdGpioValDC = -1;
	int fdGpioValBLK = -1;
	int gpioInit();
	int gpioDeinit();
	int gpioSetVal(unsigned int gpioIndex, unsigned int val);

	// Panel Operation
	int panelInit();
	int panelDeinit();
	int panelWriteBus(const void *dataBuf, unsigned long dataBufLen);
	int panelWriteCmd(const void *dataBuf, unsigned long dataBufLen);
	int panelWriteData(const void *dataBuf, unsigned long dataBufLen);
	int panelSetAddress(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1);

	// panel绘图相关计算函数
	unsigned int mathPow(unsigned char m, unsigned char n);		//求幂

	bool bEnable;
};

#endif

