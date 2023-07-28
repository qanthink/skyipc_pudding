/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include <iostream>
#include <signal.h>
#include <thread>

#include "testing.h"
#include "sys.h"
#include "sensor.h"
#include "vif.h"
#include "isp.h"
#include "vpe.h"
//#include "divp.h"
#include "venc.h"
#include "ai.hpp"
#include "ao.hpp"
#include "rgn.h"
#if (1 == (USE_UVC))
#include "uvc_uac.h"
#endif

//#include "aac.h"
//#include "aad.h"
//#include "avtp.h"
#include "ircutled.h"
//#include "spipanel.h"
//#include "ethernet.h"
#include "live555rtsp.h"
//#include "mp4container.h"


using namespace std;

void sigHandler(int sig);

int main(int argc, const char *argv[])
{
	/*
		统一采用C++单例设计模式，getInstance() 为各模块实例的统一入口函数。
		单例模式不存在重复初始化的问题，在调用之初执行构造，后续其它地方的调用时间开销小。
		风格尽量趋近于C, 避免C++11 及之后的高级用法。
		库函数尽可能使用linux 标准库函数，高效，可调试性高。暂不考虑linux->otherOS 的移植问题。
	*/

	signal(SIGINT, sigHandler);

	/* ==================== 第一部分，系统初始化 ==================== */
	#if 1
	IrCutLed *pIrCutLed = IrCutLed::getInstance();
	IrCutLed::getInstance()->openFilter();

	// 系统初始化
	Sys *pSys = Sys::getInstance();

	// Sensor 初始化。数据流向：sensor -> vif -> vpe -> (DIVP) -> venc -> 应用处理。
	Sensor *pSensor = Sensor::getInstance();	// sensor 初始化
	pSensor->setFps(20);

	unsigned int snrW = 0;
	unsigned int snrH = 0;
	pSensor->getSnrWH(&snrW, &snrH);
	cout << "snrW, snrH = " << snrW << ", " << snrH << endl;

	// VIF 初始化
	Vif *pVif = Vif::getInstance();

	// VPE 初始化，并绑定前级VIF.
	Vpe *pVpe = Vpe::getInstance();
	pSys->bindVif2Vpe(Vif::vifPort, Vpe::vpeInputPort, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);

	#if(1 == (USE_IQ_SERVER))
	Isp *pIsp = Isp::getInstance();
	pIsp->openIqServer(snrW, snrH, Vpe::vpeCh);
	char iqFilePath[128] = {"/customer"};
	pIsp->setIqServerDataPath(iqFilePath);
	#endif

	Venc *pVenc = Venc::getInstance();
	#if (1 != (USE_UVC))
	// 创建子码流
	unsigned int subW = 1280;
	unsigned int subH = 720;
	#if (1 == (USE_VENC_SUB))
	pVpe->createPort(Vpe::vpeSubPort, subW, subH);
	pVenc->createH26xStream(Venc::vencSubChn, subW, subH, Venc::vesTypeH264);
	pVenc->changeBitrate(Venc::vencSubChn, 0.25 * 1024);
	pSys->bindVpe2Venc(Vpe::vpeSubPort, Venc::vencSubChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	#endif

	// 创建主码流
	#if (1 == (USE_VENC_MAIN))
	pVpe->createPort(Vpe::vpeMainPort, snrW, snrH);
	pVenc->createH26xStream(Venc::vencMainChn, snrW, snrH, Venc::vesTypeH264);
	pVenc->changeBitrate(Venc::vencMainChn, 1 * 1024);
	pSys->bindVpe2Venc(Vpe::vpeMainPort, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	#endif

	// 创建jpeg码流
	#if (1 == (USE_VENC_JPEG))
	pVpe->createPort(Vpe::vpeSubPort, snrW, snrH);
	pVenc->createJpegStream(Venc::vencSubChn, snrW, snrH);
	//pVenc->changeBitrate(Venc::vencSubChn, 0.01 * 1024);
	pSys->bindVpe2Venc(Vpe::vpeSubPort,Venc::vencSubChn, 20, 20, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	#endif
	#endif

	// VENC 也可以实现图像的Crop 和Scale, 但是建议在VPE 中做。
	//pVenc->setCrop(Venc::vencMainChn, (2560 - 1920) / 2, (1440 - 1080) / 2, 1920, 1080);

	// 初始化OSD
	#if (1 == (USE_OSD))
	Rgn *pRgn = Rgn::getInstance();
	#endif
	
	// 音频编/解码初始化。aac = audio coder; aad = audio decoder.
	#if (1 == (USE_FAAC_FAAD))
	Aac *pAac = Aac::getInstance();
	Aad *pAad = Aad::getInstance();
	#endif

	// AI 和AO 属于两个模块，分别初始化。
	#if (1 == (USE_AI))
	AudioIn *pAudioIn = AudioIn::getInstance();
	#endif
	
	#if (1 == (USE_AO))
	AudioOut *pAudioOut = AudioOut::getInstance();
	#endif
	#endif

	/*
		至此，SENSOR, VIF, VPE, VENC, AI, AO, OSD 均已初始化完成。
	*/

	/* ==================== 第二部分，应用初始化 ==================== */
	// 音视频传输协议测试
	#if (1 == (USE_AVTP_AUDIO))
	AvtpAudioClient avtpAudioClient("192.168.0.200");
	pAvtpAudioClient = &avtpAudioClient;
	#endif

	#if (1 == (USE_AVTP_VIDEO))
	AvtpVideoClient avtpVideoClient("192.168.0.200");
	pAvtpVideoClient = &avtpVideoClient;
	thread thChangeBitrate(avtpChangeKbps, pAvtpVideoClient, 3);
	#endif

	// RTSP 推网络流
	#if ((1 == (USE_RTSPSERVER_LOCALFILE)) || (1 == (USE_RTSPSERVER_LIVESTREAM_MAIN)) \
			|| (1 == (USE_RTSPSERVER_LIVESTREAM_SUB)) || 1 == (USE_RTSPSERVER_LIVESTREAM_JPEG))
	Live555Rtsp live555Rtsp;
	pLive555Rtsp = &live555Rtsp;
	#endif
	
	// RTSP 推本地流
	#if (1 == (USE_RTSPSERVER_LOCALFILE))
	const char *filePath = NULL;
	//filePath = "/mnt/linux/Downloads/videotest/1.mp4";
	filePath = "/mnt/linux/Downloads/material/test.264";
	const char *streamName = "stream";
	pLive555Rtsp->addStream(filePath, streamName, emEncTypeH264);
	#endif

	// FFMPEG 保存MP4.
	#if (1 == (USE_FFMPEG_SAVE_MP4))
	Mp4Container *pMp4Container = Mp4Container::getInstance();
	#endif

	// 获取音频AI 的线程
	#if (1 == (USE_AI))
	thread thAi(routeAi, (void *)NULL);
	#endif

	// AO 音频测试
	#if (1 == (USE_AO))
	// AO. 参数为本地音频文件的路径。写死的，16位宽 16000采样率
	#if (1 == (USE_AO_LOCAL_FILE))
	thread thAo(routeAoFromFile, (void *)"/mnt/linux/Audios/pcm/xiaopingguo_mono_16b_16000.pcm");
	#endif

	// AO 音频测试，播放网络音频
	#if (1 == (USE_AO_NET_PCM))
	thread thAo(routeAoNetPcm, (void *)NULL);
	#endif
	#endif

	// OSD 功能
	#if(1 == (USE_OSD))
	thread thOsd(routeOsd, (void *)NULL);
	#endif

	// 测试主码流
	#if (1 == (USE_VENC_MAIN))
	cout << "routeVideoMain" << endl;
	thread thVideoMain(routeVideo, (void *)Venc::vencMainChn);
	#endif

	// 测试子码流
	#if (1 == (USE_VENC_SUB))
	cout << "routeVideoSub" << endl;
	thread thVideoSub(routeVideo, (void *)Venc::vencSubChn);
	#endif

	// 测试JPEG 码流
	#if (1 == (USE_VENC_JPEG))
	cout << "routeVideoJpeg" << endl;
	thread thVideoJpeg(routeVideo, (void *)Venc::vencSubChn);
	#endif

	int ret = 0;
	this_thread::sleep_for(chrono::microseconds(1));	// sleep for rtsp file read.
	#if ((1 == (USE_RTSPSERVER_LOCALFILE)) || (1 == (USE_RTSPSERVER_LIVESTREAM_MAIN)) \
		|| (1 == (USE_RTSPSERVER_LIVESTREAM_SUB)) || (1 == (USE_RTSPSERVER_LIVESTREAM_JPEG)))
	thread thRtsp([&](){pLive555Rtsp->eventLoop();});		// lambda 表达式太好用啦！
	thRtsp.detach();
	#endif

	// 测试SPI 屏。
	#if (1 == (USE_SPI_PANEL))
	thread thSpiPanel(routeSpiPanel, (void *)NULL);
	#endif

	// 测试网络
	#if (1 == (TEST_ETHERNET))
	testEthernet();
	Ethernet *pEthernet = pEthernet->getInstance();
	pEthernet->showWlanInfOnPanel();
	#endif

	// 测试UVC
	#if (1 == (USE_UVC))
	UvcUac *pUvcUac = UvcUac::getInstance();
	pUvcUac->startUvc();
	#endif

	g_bRunning = true;		// sigHandler() 对其取反。
	while(g_bRunning)
	{
		sleep(0.5);
		static int sleepCntSec = 0;
		if(10 == sleepCntSec++ * 2)
		{
			cout << "Progress running." << endl;
			sleepCntSec = 0;
		}

		//int ret = 0;
		#if (1 == USE_INTERACTION)
		ret = interAction();
		if(1 == ret)
		{
			break;
		}
		#endif
	}
	cout << "jump out from while(g_bRunning)" << endl;

	/* ==================== 第三部分，应用析构 ==================== */
	#if ((1 == (USE_RTSPSERVER_LOCALFILE)) || (1 == (USE_RTSPSERVER_LIVESTREAM_MAIN)) \
		|| (1 == (USE_RTSPSERVER_LIVESTREAM_SUB)) || (1 == (USE_RTSPSERVER_LIVESTREAM_JPEG)))
	//pLive555Rtsp->~Live555Rtsp();
	cout << "rtsp join()" << endl;
	//thRtsp.join();	// 前期做了detach.
	cout << "rtsp join." << endl;
	#endif
	
	#if (1 == (USE_AI))
	thAi.join();
	cout << "ai join." << endl;
	#endif
	
	#if (1 == (USE_AO))
	thAo.join();
	cout << "ao join." << endl;
	#endif
									
	#if (1 == (USE_VENC_JPEG))
	thVideoJpeg.join();
	cout << "vencJpeg join." << endl;
	#endif
	
	#if (1 == (USE_VENC_SUB))
	thVideoSub.join();
	cout << "vencSub join." << endl;
	#endif

	#if (1 == (USE_VENC_MAIN))
	thVideoMain.join();
	cout << "vencMain join." << endl;
	#endif

	#if (1 == (USE_OSD))
	thOsd.join();
	cout << "osd join." << endl;
	#endif

	#if (1 == (USE_AVTP_VIDEO))
	thChangeBitrate.join();
	cout << "avtp video join." << endl;
	#endif

	#if (1 == (USE_SPI_PANEL))
	thSpiPanel.join();
	cout << "spi panel join." << endl;
	#endif

	#if (1 == (USE_UVC))
	pUvcUac->stopUvc();
	#endif

	#if(1 == (USE_IQ_SERVER))
	pIsp->closeIqServer();
	#endif

	cout << "Sleep()" << endl;
	sleep(0.5);
	
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：信号处理函数。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
void sigHandler(int sig)
{
	cout << "Call sigHandler()." << endl;
	
	switch (sig)
	{
	case SIGINT:
		cerr << "Receive SIGINT!!!" << endl;
		g_bRunning = false;
		break;
	default:
		g_bRunning = false;
		break;
	}

	cout << "Call sigHandler() end." << endl;
	return;
}

