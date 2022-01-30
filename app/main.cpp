/*---------------------------------------------------------------- 
sigma star版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "iostream"

#include "sys.h"
#include "sensor.hpp"
#include "mi_vif.h"
#include "mi_vpe.h"
#include "mi_venc.h"
#include "ai.hpp"
#include "ao.hpp"
#include "aac.h"
#include "aad.h"
#include "mi_rgn.h"
#include "wifi.h"
#include "avtp.h"
#include "testing.h"
#include "ircut.h"
#include "live555rtsp.h"
#include "ffmpeg.h"
#include "queue.h"

#include "spidev.h"
#include "spi.h"
//#include <linux/gpio.h>

using namespace std;

void sigHandler(int sig);

static const char *device = "/dev/spidev1.0"; //根据使用哪一组spi确认节点
static uint8_t mode = 0; /* SPI通信使用全双工，设置CPOL＝0，CPHA＝0。 */
static uint8_t bits = 8; /* ８ｂiｔｓ读写，MSB first。*/
static uint32_t speed = 60*1000*1000;/* 设置传输速度 */
static uint16_t delay = 0;
static int g_SPI_Fd = 0;

static void pabort(const char *s)
{
    perror(s);
    abort();
}


/**
* 功 能：同步数据传输
* 入口参数 ：
* TxBuf -> 发送数据首地址
* len -> 交换数据的长度
* 出口参数：
* RxBuf -> 接收数据缓冲区
* 返回值：0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Transfer(const uint8_t *TxBuf, uint8_t *RxBuf, int len)
{
    int ret;
    int fd = g_SPI_Fd;


    struct spi_ioc_transfer tr ={
        .tx_buf = (unsigned long) TxBuf,
        .rx_buf = (unsigned long) RxBuf,
        .len =len,
        .delay_usecs = delay,
    };


    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
        perror("can't send spi message\n");
    else
    {
#if SPI_DEBUG
        int i;
        printf("nsend spi message Succeed\n");
        printf("nSPI Send [Len:%d]: \n", len);
        for (i = 0; i < len; i++)
        {
            if (i % 8 == 0)
                printf("nt\n");
            printf("0x%02X \n", TxBuf[i]);
        }
        printf("n");


        printf("SPI Receive [len:%d]:\n", len);
        for (i = 0; i < len; i++)
        {
            if (i % 8 == 0)
                printf("nt\n");
            printf("0x%02X \n", RxBuf[i]);
        }
        printf("\n");
#endif
    }
    return ret;
}


/**
* 功 能：发送数据
* 入口参数 ：
* TxBuf -> 发送数据首地址
＊len -> 发送与长度
＊返回值：0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Write(uint8_t *TxBuf, int len)
{
    int ret;
    int fd = g_SPI_Fd;


    ret = write(fd, TxBuf, len);
    if (ret < 0)
        perror("SPI Write error\n");
    else
    {
#if SPI_DEBUG
        int i;
        printf("SPI Write [Len:%d]: \n", len);
        for (i = 0; i < len; i++)
        {
            if (i % 8 == 0)
                printf("\n\t");
            printf("0x%02X \n", TxBuf[i]);
        }
        printf("\n");
#endif
    }

    return ret;
}


/**
* 功 能：接收数据
* 出口参数：
* RxBuf -> 接收数据缓冲区
* rtn -> 接收到的长度
* 返回值：>=0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Read(uint8_t *RxBuf, int len)
{
    int ret;
    int fd = g_SPI_Fd;
    ret = read(fd, RxBuf, len);
    if (ret < 0)
        printf("SPI Read error\n");
    else
    {
#if SPI_DEBUG
        int i;
        printf("SPI Read [len:%d]:\n", len);
        for (i = 0; i < len; i++)
        {
            if (i % 8 == 0)
                printf("\n\t");
            printf("0x%02X \n", RxBuf[i]);
        }
        printf("\n");
#endif
    }
    return ret;
}


/**
* 功 能：打开设备 并初始化设备
* 入口参数 ：
* 出口参数：
* 返回值：0 表示已打开 0XF1 表示SPI已打开 其它出错
* 开发人员：Lzy 2013－5－22
*/
int SPI_Open(void)
{
    int fd;
    int ret = 0;


    if (g_SPI_Fd != 0) /* 设备已打开 */
        return 0xF1;

    fd = open(device, O_RDWR);
    if (fd < 0)
        pabort("can't open device\n");
    else
        printf("SPI - Open Succeed. Start Init SPI...\n");


    g_SPI_Fd = fd;
    /*
    * spi mode
    */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode\n");


    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode\n");


    /*
    * bits per word
    */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word\n");


    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word\n");


    /*
    * max speed hz
    */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz\n");


    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz\n");


    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d KHz (%d MHz)\n", speed / 1000, speed / 1000 / 1000);


    return ret;
}


/**
* 功 能：关闭SPI模块
*/
int SPI_Close(void)
{
    int fd = g_SPI_Fd;


    if (fd == 0) /* SPI是否已经打开*/
        return 0;
    close(fd);
    g_SPI_Fd = 0;


    return 0;
}

/**
* 功 能：自发自收测试程序
* 接收到的数据与发送的数据如果不一样 ，则失败
* 说明：
* 在硬件上需要把输入与输出引脚短跑
* 开发人员：Lzy 2013－5－22
*/
int SPI_LookBackTest(void)
{
    int ret, i;
    const int BufSize = 16;
    uint8_t tx[BufSize], rx[BufSize];

    bzero(rx, sizeof(rx));
    for (i = 0; i < BufSize; i++)
        tx[i] = i;

    printf("nSPI - LookBack Mode Test...\n");
    ret = SPI_Transfer(tx, rx, BufSize);
    if (ret > 1)
    {
        ret = memcmp(tx, rx, BufSize);
        if (ret != 0)
        {
            printf("tx:\n");
            for (i = 0; i < BufSize; i++)
            {
                printf("%d ", tx[i]);
            }
            printf("\n");
            printf("rx:\n");
            for (i = 0; i < BufSize; i++)
            {
                printf("%d ", rx[i]);
            }
            printf("\n");
            perror("LookBack Mode Test error\n");
        }
        else
            printf("SPI - LookBack Mode OK\n");
    }

    return ret;
}

#if 0
static void setGpioValue(int port, const char *direction, int value)
{
	char cmd[100] = {0};
	sprintf(cmd, "echo %d > /sys/class/gpio/export", port);
	cout << cmd << endl;
	system(cmd);

	sprintf(cmd, "echo %s > /sys/class/gpio/gpio%d/direction", direction, port);
	cout << cmd << endl;
	system(cmd);

	sprintf(cmd, "echo %d > /sys/class/gpio/gpio%d/value", value, port);
	cout << cmd << endl;
	system(cmd);

	sprintf(cmd, "echo %d > /sys/class/gpio/unexport", port);
	cout << cmd << endl;
	system(cmd);
}
#endif

void spiPanelReset()
{
	int port = 46;
	const char *direction = "out";
	int val = 0;	// 0 = low; 1 = high;

	//setGpioValue(port, "out", val);
	usleep(100 * 1000);		// n * 1000 = n ms.
	
	val = 1;
	//setGpioValue(port, "out", val);
	usleep(100 * 1000);		// n * 1000 = n ms.
}

void spiPanelWrite(uint8_t *dataBuf, int dataLen)
{
	#if 0
	#endif

	SPI_Write(dataBuf, dataLen);
}

void spiPanelWriteCmd(uint8_t *dataBuf, int dataLen)
{
	int port = 47;
	const char *direction = "out";
	int val = 0;	// 0 = low; 1 = high;

	val = 0;
	//setGpioValue(port, "out", val);
	//usleep(100 * 1000);		// n * 1000 = n ms.
	spiPanelWrite(dataBuf, dataLen);
}

void spiPanelWriteData(uint8_t *dataBuf, int dataLen)
{
	int port = 47;
	const char *direction = "out";
	int val = 0;	// 0 = low; 1 = high;

	val = 1;
	//setGpioValue(port, "out", val);
	//usleep(100 * 1000);		// n * 1000 = n ms.
	spiPanelWrite(dataBuf, dataLen);
}

int main(int argc, const char *argv[])
{
	Spi *spi = Spi::getInstance();
	spi->GPIO_INIT();
	spi->fun();

#if 0
	int ret = 0;
	ret = SPI_Open();
	cout << "call spi_open(). ret = " << ret <<endl;

	//ret = SPI_LookBackTest();
	//cout << "call SPI_LookBackTest(). ret = " << ret <<endl;
	//sleep(10);
	cout << "reset" << endl;
	spiPanelReset();

	//sleep(100);
	
	const unsigned int BUFLEN = 128;
	unsigned char dataBuf[BUFLEN];
 
	cout << "write cmd" << endl;
	dataBuf[0] = 0x11;
	spiPanelWriteCmd(dataBuf, 1); //Sleep out 
	usleep(120 * 1000);              //Delay 120ms 
	//************* Start Initial Sequence **********// 
	dataBuf[0] = 0x36;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x00;
	//sleep(5);
	cout << "Write Data" << endl;
	spiPanelWriteData(dataBuf, 1);
	//sleep(5);
	cout << "Write Over" << endl;
	//spiPanelWriteData(0xC0);
	//spiPanelWriteData(0x70);
	//spiPanelWriteData(0xA0);

	dataBuf[0] = 0x3A;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x05;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xB2;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x0C;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x0C;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x00;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x33;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x33;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xB7;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x35;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xBB;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x37;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xC0;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x2C;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xC2;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x01;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xC3;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x12;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xC4;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x20;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xC6;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0x0F;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xD0;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0xA4;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0xA1;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xE0;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0xD0;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x04;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x0D;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x11;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x13;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x2B;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x3F;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x54;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x4C;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x18;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x0D;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x0B;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x1F;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x23;
	spiPanelWriteData(dataBuf, 1);

	dataBuf[0] = 0xE1;
	spiPanelWriteCmd(dataBuf, 1);
	dataBuf[0] = 0xD0;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x04;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x0C;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x11;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x13;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x2C;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x3F;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x44;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x51;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x2F;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x1F;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x1F;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x20;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x23;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x21;
	spiPanelWriteCmd(dataBuf, 1);

	dataBuf[0] = 0x29;
	spiPanelWriteCmd(dataBuf, 1);

	//==============SET XY============
	dataBuf[0] = 0x2a;
	spiPanelWriteCmd(dataBuf, 1);//列地址设置
	dataBuf[0] = 10;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 10;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x2b;
	spiPanelWriteCmd(dataBuf, 1);//行地址设置
	dataBuf[0] = 10;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 10;
	spiPanelWriteData(dataBuf, 1);
	dataBuf[0] = 0x2c;
	spiPanelWriteCmd(dataBuf, 1);//储存器写

	//==============SET COLOR============
	int i = 0;
	int j = 0;
	
	for(i=0;i<10;i++)
	{													   	 	
		for(j=0;j<10;j++)
		{
			dataBuf[0] = 0xFF;
			dataBuf[1] = 0xFF;
			spiPanelWriteData(dataBuf, 2);
		}
	}

	ret = SPI_Close();
	cout << "call SPI_Close(). ret = " << ret <<endl;
#endif

	return 0;
}

#if 0
void sigHandler(int sig)
{
	switch (sig)
	{
	case SIGINT:
		cerr << "Receive SIGINT!!!" << endl;
		g_bRunning = false;
		//exit(0);
		break;
	default:
		g_bRunning = false;
		break;
	}
}
#endif
