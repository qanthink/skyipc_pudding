/*---------------------------------------------------------------- 
sigma star版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "iostream"

#include "sys.h"
#include "sensor.hpp"
#include "vif.h"
#include "vpe.h"
#include "venc.h"
#include "ai.hpp"
#include "ao.hpp"
#include "aac.h"
#include "aad.h"
#include "rgn.h"
#include "wifi.h"
#include "avtp.h"
#include "testing.h"
#include "ircut.h"
#include "live555rtsp.h"
#include "ffmpeg.h"
#include "queue.h"

#include "spidev.h"

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


int main(int argc, const char *argv[])
{
	/*
		统一采用C++单例设计模式，getInstance() 为各模块实例的统一入口函数。
		单例模式不存在重复初始化的问题，在调用之初执行构造，后续其它地方的调用时间开销小。
		风格尽量趋近于C, 避免C++11 及之后的高级用法。
		库函数尽可能使用linux 标准库函数，高效，可调试性高。暂不考虑linux->otherOS 的移植问题。
	*/

#if 0
	signal(SIGINT, sigHandler);

	IrCut::getInstance()->resetFilter();	// ircut滤波片复位
	IrCut::getInstance()->closeFilter();

	// 系统初始化
	Sys *pSys = Sys::getInstance();
	// 出图模块初始化。数据流向：sensor -> vif -> vpe -> venc -> 应用处理。
	Sensor *pSensor = Sensor::getInstance();// sensor 初始化
	pSensor->setFps(60);
	Vif *pVif = Vif::getInstance();			// VIF 初始化
	Vpe *pVpe = Vpe::getInstance();			// VPE 初始化
	pVpe->createMainPort(Vpe::vpeMainPort);	// 创建VPE 主码流
	//pVpe->createSubPort(Vpe::vpeSubPort);	// 创建VPE 子码流
	//pVpe->createJpegPort(Vpe::vpeJpegPort);	// 创建VPE JPEG码流
	
	Venc *pVenc = Venc::getInstance();		// VENC 初始化
	pVenc->createMainStream(Venc::vencMainChn, NULL);	// 创建VENC主码流
	//pVenc->createSubStream(Venc::vencSubChn, NULL);		// 创建VENC子码流
	//pVenc->createJpegStream(Venc::vencJpegChn, NULL);	// 创建VENC-JPEG码流
	
	// 绑定VIF -> VPE. 只需要绑定一次，用REALTIME
	pSys->bindVif2Vpe(Vif::vifPort, Vpe::vpeInputPort, 60, 60, E_MI_SYS_BIND_TYPE_REALTIME, 0);
	// 绑定VPE -> VENC, 如果有多路码流，则需要绑定多次。
	pSys->bindVpe2Venc(Vpe::vpeMainPort, Venc::vencMainChn, 60, 60, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	//pSys->bindVpe2Venc(Vpe::vpeSubPort, Venc::vencSubChn, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);
	//pSys->bindVpe2Venc(Vpe::vpeJpegPort, Venc::vencJpegChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);

	// 初始化OSD
	#if (1 == (USE_OSD))
	Rgn *pRgn = Rgn::getInstance();
	#endif
	
	// AI 和AO 属于两个模块，分别初始化。
	#if (1 == (USE_AI))
	audioIn *pAudioIn = audioIn::getInstance();
	#endif
	
	#if (1 == (USE_AO))
	audioOut *pAudioOut = audioOut::getInstance();
	#endif
	
	#if (1 == (USE_FAAC_FAAD))
	// 音频编/解码初始化。aac = audio coder; aad = audio decoder.
	Aac *pAac = Aac::getInstance();
	Aad *pAad = Aad::getInstance();
	#endif

	#if (1 == (USE_WIFILINK))
	Wifi *pWifi = Wifi::getInstance();
	pWifi->enable();
	#endif

	/*
		至此，SENSOR, VIF, VPE, VENC, AI, AO, OSD 均已初始化完成。
		以下代码的功能为通过音视频传输协议将数据传输给另一台设备。
	*/
	
	#if (1 == (USE_MINDSDK_AUDIO))
	// avtp 传输协议。
	Mindsdk_Audio& audio = *Mindsdk_Audio::getInstance();
	audio.cb_recvAudio = recvAudio;
	audio.start();
	#endif

	#if (1 == (USE_MINDSDK_VIDEO))
	// avtp 传输协议。
	Mindsdk_Video &Video = *Mindsdk_Video::getInstance();
	Video.cb_startVideo = startVideo;
	Video.cb_stopVideo = stopVideo;
	Video.cb_getNextViFrame = getNextViFrame;
	Video.cb_changeBit = changeBit;
	Video.start();
	#endif

	int ret = 0;
	#if (1 == (TEST_RTSPSERVER_LOCALFILE))
	pthread_t tidRtsp = -1;
	const char *fileName = NULL;
	//fileName = "/mnt/linux/Downloads/videotest/1.mp4";
	fileName = "/mnt/linux/Downloads/material/test.264";
	ret = pthread_create(&tidRtsp, NULL, createRtspServerBy265LocalFile, (void *)fileName);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_create(3) for RTSP, " << strerror(errno) << endl;
	}
	cout << "routeVideo" << endl;
	#endif
	
	#if (1 == (TEST_RTSPSERVER_LIVESTREAM))
	Live555Rtsp *pLive555Rtsp = Live555Rtsp::getInstance();
	#endif

	#if (1 == (USE_FFMPEG))
	Ffmpeg *pFfmpeg = Ffmpeg::getInstance();
	#endif

	// 创建3个线程，分别用于测试AI, AO, 出图。
	#if (1 == (USE_AI))
	pthread_t tidAi = -1;
	ret = pthread_create(&tidAi, NULL, routeAi, NULL);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_create(3) for AI, " << strerror(errno) << endl;
	}
	#endif

	#if (1 == (USE_AO))
	pthread_t tidAo = -1;
	// AO. 参数为本地音频文件的路径。写死的，16位宽 16000采样率
	ret = pthread_create(&tidAo, NULL, routeAo, (void *)"pcm_16000_16bit.pcm");
	if(0 != ret)
	{
		cerr << "Fail to call pthread_create(3) for AO, " << strerror(errno) << endl;
	}
	#endif

	#if(1 == (USE_OSD))
	// OSD 功能
	pthread_t tidOsd = -1;
	ret = pthread_create(&tidOsd, NULL, routeOsd, NULL);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_create(3) for OSD, " << strerror(errno) << endl;
	}
	#endif

	#if (1 == (USE_VENC))
	pthread_t tidVideo = -1;
	cout << "routeVideo" << endl;
	// 测试出图的线程。参数为VENC 的通道号，支持主码流和子码流。
	ret = pthread_create(&tidVideo, NULL, routeVideo, (void *)Venc::vencMainChn);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_create(3) for VENC, " << strerror(errno) << endl;
	}
	cout << "routeVideo" << endl;
	#endif

	//testQueue();

	g_bRunning = true;		// sigHandler() 对其取反。
	while(g_bRunning)
	{
		sleep(1);
	}

	#if (1 == (USE_AI))
	pthread_join(tidAi, NULL);
	#endif
	
	#if (1 == (USE_AO))
	pthread_join(tidAo, NULL);
	#endif
	
	#if (1 == (USE_VENC))
	pthread_join(tidVideo, NULL);
	#endif

	#if (1 == (USE_OSD))
	pthread_join(tidOsd, NULL);
	#endif

	#if (1 == (TEST_RTSPSERVER_LIVESTREAM))
	pLive555Rtsp->destroyLiveStreamServer();
	#endif

	#if (1 == (TEST_RTSPSERVER_LOCALFILE))
	pthread_join(tidRtsp, NULL);
	#endif

	cout << "Sleep()" << endl;
	sleep(0.5);
#endif

	int ret = 0;
	ret = SPI_Open();
	cout << "call spi_open(). ret = " << ret <<endl;

	ret = SPI_LookBackTest();
	cout << "call SPI_LookBackTest(). ret = " << ret <<endl;

	ret = SPI_Close();
	cout << "call SPI_Close(). ret = " << ret <<endl;	
	
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
