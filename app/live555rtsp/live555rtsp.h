/*---------------------------------------------------------------- 
sigma star版权所有。
作者：allen.li
时间：2020.7.10
----------------------------------------------------------------*/

#ifndef __LIVE555RTSP_H__
#define __LIVE555RTSP_H__

#include "H265FramedLiveSource.hh"
#include "H265LiveVideoFileServerMediaSubsession.hh"
#include "queue.h"
#include "venc.h"

void *createRtspServerBy265LocalFile(void *h265FilePath);

class Live555Rtsp{
public:
	static Live555Rtsp *getInstance();
	int enable();
	int disable();

	int setFrameQueueDepth(const unsigned int queueDepth = 15);

	int createServerByLiveStream();
	int destroyLiveStreamServer();
	//int createServerByLocalFile();
	//int destroyLocalFileServer();

	int sendH26xFrame(const Venc::stStreamPack_t *pstPacket);
	int sendH26xFrame_block(const Venc::stStreamPack_t *pstPacket);
	int recvH26xFrame(Venc::stStreamPack_t *pstPacket);
private:
	Live555Rtsp();
	~Live555Rtsp();
	Live555Rtsp(const Live555Rtsp&);
	Live555Rtsp& operator=(const Live555Rtsp&);

	bool bEnable;
	pthread_t tid = -1;
	pthread_cond_t cond;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	volatile bool bRouteRunning = false;
	RTSPServer* rtspServer = NULL;
	Queue<Venc::stStreamPack_t> frameQueue{3};

	// 类中创建线程，必须使用一个空格函数传递this 指针。
	void *routeLiveStream(void *arg);
	static void *__routeLiveStream(void *arg);
	//void *routeLocalFile(void *arg);
	//static void *__routeLocalFile(void *arg);
	
	void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,char const* streamName);
	void doEventLoop(BasicTaskScheduler0 *Basicscheduler);
};

#endif
