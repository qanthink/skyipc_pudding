/*---------------------------------------------------------------- 
sigma star版权所有。
作者：allen.li
时间：2020.7.15
----------------------------------------------------------------*/

/*
	本文件基于live555库，开发了rtspserver.
*/

/*
	=================RTSP Server推本地流的方法=================
	第一步：
	在程序的开始处，创建任务调度器、使用环境、RTSP服务器；
	在结束处，开始任务调度。
	
	TaskScheduler *scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);
	RTSPServer *rtspServer = RTSPServer::createNew(*env, ourPort);
	...
	env->taskScheduler().doEventLoop();
	
	第二步（可选项）：
	打印服务器地址信息。
	*env << rtspServer->rtspURLPrefix();

	第三步：
	创建ServerMediaSession服务器主会话，指定会话名称，并添加视频子会话和音频子会话。
	ServerMediaSession *sms = ServerMediaSession::createNew(*env, "sessionName");
	sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*env, "test.265", reuseFirstSource));

	第四步：
	将主会话加入RTSP Server.
	rtspServer->addServerMediaSession(sms);
*/

/*
	======================= 熟悉live555的类 =======================
	参考本路径下"classOnDemandServerMediaSubsession__inherit__graph.png",
	或"http://www.live555.com/liveMedia/doxygen/html/classOnDemandServerMediaSubsession.html".
	有如下继承关系：
	Medium <-- ServerMediaSession <-- OnDemandServerMediaSubsession <-- FileServerMediaSubsession <-- H265VideoFileServerMediaSubsession
	
	重点关注 OnDemandServerMediaSubsession类，该类是ServermediaSubSession 的一个中间实现，定义了一些点播服务的特性。
	我们推文件流、实时流的方法，都要通过继承该类来实现。
	参考代码推文件流，继承该类后定义的新类是H265VideoFileServerMediaSubsession.
	我们推实时流，继承该类后定义的新类为H265VideoLiveServerMediaSubssion.
*/

/*
	学习资料：
	https://www.jianshu.com/p/60fcc41c2369
	https://blog.csdn.net/marcosun_sw/article/details/86149356
	https://www.cnblogs.com/weixinhum/p/3916676.html	最有用！！！
*/

#include "live555rtsp.h"
#include "iostream"
#include "venc.h"

using namespace std;

Live555Rtsp::Live555Rtsp()
{
	enable();

	bEnable = true;
}

Live555Rtsp::~Live555Rtsp()
{
	bEnable = false;
	
	disable();
}

/*  ---------------------------------------------------------------------------
描--述：VENC 模块获取实例的唯一入口
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Live555Rtsp* Live555Rtsp::getInstance()
{
	static Live555Rtsp live555Rtsp;
	return &live555Rtsp;
}

/* ---------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::enable()
{
	cout << "Call Live555Rtsp::enable()." << endl;

	setFrameQueueDepth(15);
	createServerByLiveStream();
	bEnable = true;
	
	cout << "Call Live555Rtsp::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::disable()
{
	cout << "Call Live555Rtsp::disable()." << endl;

	destroyLiveStreamServer();
	bEnable = false;

	cout << "Call Live555Rtsp::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：显示RTSP流信息。
参--数：rtspServer 指向RTSP服务器对象的指针；sms 指向会话的指针；streamName 流的名称
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void Live555Rtsp::announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,char const* streamName)
{
	char *url = rtspServer->rtspURL(sms);
	UsageEnvironment &env = rtspServer->envir();
	env << streamName << "\n";
	env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::setFrameQueueDepth(const unsigned int queueDepth)
{
	cout << "Call Ffmpeg::createFrameBufQueue()." << endl;
	int ret = 0;
	ret == frameQueue.setQueueDepth(queueDepth);
	if(0 != ret)
	{
		cerr << "Fail to call Ffmpeg::createFrameBufQueue()." << endl;
		return ret;
	}

	cout << "Call Ffmpeg::createFrameBufQueue() end." << endl;
	return queueDepth;
}

/*-----------------------------------------------------------------------------
描--述：显示RTSP流信息。
参--数：rtspServer 指向RTSP服务器对象的指针；sms 指向会话的指针；streamName 流的名称
返回值：无
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::sendH26xFrame(const Venc::stStreamPack_t *pstPacket)
{
	//cout << "Call Live555Rtsp::sendH26xFrame()." << endl;
	if(!bRouteRunning)
	{
		return -1;
	}

	int ret = 0;
	ret= pthread_mutex_lock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_lock(3), ret = " << ret << ". " << endl;
	}
	
	frameQueue.push(pstPacket);
	ret= pthread_cond_signal(&cond);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_cond_signal(3), ret = " << ret << ". " << endl;
	}

	ret= pthread_mutex_unlock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_unlock(3), ret = " << ret << ". " << endl;
	}

	//cout << "End of call Live555Rtsp::sendH26xFrame()." << endl;	
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：显示RTSP流信息。
参--数：rtspServer 指向RTSP服务器对象的指针；sms 指向会话的指针；streamName 流的名称
返回值：无
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::sendH26xFrame_block(const Venc::stStreamPack_t *pstPacket)
{
	//cout << "Call Live555Rtsp::sendH26xFrame()." << endl;
	if(!bRouteRunning)
	{
		return -1;
	}

	int ret = 0;
	ret= pthread_mutex_lock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_lock(3), ret = " << ret << ". " << endl;
	}

	while(frameQueue.isFull() && bRouteRunning)
	{
		ret= pthread_mutex_unlock(&mutex);
		if(0 != ret)
		{
			cerr << "Fail to call pthread_mutex_unlock(3), ret = " << ret << ". " << endl;
		}
		
		usleep(1);

		ret= pthread_mutex_lock(&mutex);
		if(0 != ret)
		{
			cerr << "Fail to call pthread_mutex_lock(3), ret = " << ret << ". " << endl;
		}
	}
	
	frameQueue.push(pstPacket);
	ret= pthread_cond_signal(&cond);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_cond_signal(3), ret = " << ret << ". " << endl;
	}

	ret= pthread_mutex_unlock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_unlock(3), ret = " << ret << ". " << endl;
	}

	//cout << "End of call Live555Rtsp::sendH26xFrame()." << endl;	
	return 0;
}


/*-----------------------------------------------------------------------------
描--述：显示RTSP流信息。
参--数：rtspServer 指向RTSP服务器对象的指针；sms 指向会话的指针；streamName 流的名称
返回值：无
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::recvH26xFrame(Venc::stStreamPack_t *pstPacket)
{
	//cout << "Call Live555Rtsp::recvH26xFrame()." << endl;
	int ret = 0;
	ret = pthread_mutex_lock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_lock(3), ret = " << ret << ". " << endl;
		return -1;
	}
	//cout << "ready goto while()." << endl;
	while(frameQueue.isEmpty() && bRouteRunning)
	{
		//cout << "in while()" << endl;
		ret = pthread_cond_wait(&cond, &mutex);
		if(0 != ret)
		{
			cerr << "Fail to call pthread_cond_wait(3), ret = " << ret << ". " << endl;
			pthread_mutex_unlock(&mutex);
			return -1;
		}
	}
	//cout << "pop" << endl;
	frameQueue.pop(pstPacket);
	ret = pthread_mutex_unlock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_unlock(3), ret = " << ret << ". " << strerror(errno) << endl;
		return -1;
	}

	//cout << "End of call Live555Rtsp::recvH26xFrame()." << endl;	
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：创建RTSP服务器推本地H265流。
参--数：h265FilePath 本地265文件路径。
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *createRtspServerBy265LocalFile(void *h265FilePath)
{
	OutPacketBuffer::maxSize = 2880000;	// for debug
	
	// step1: 创建任务调度器、使用环境、RTSP服务器。
	TaskScheduler *scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);
	Port ourPort = 544;
	RTSPServer *rtspServer = RTSPServer::createNew(*env, ourPort);

	// step2: 打印Server IP address.
	*env << "RTSP Address:\n"
		<< rtspServer->rtspURLPrefix() << "sessionName\n";
	
	// step3: 创建服务器会话，指定推流的文件，并添加视频子会话。
	Boolean reuseFirstSource = false;
	ServerMediaSession *sms = ServerMediaSession::createNew(*env, "main_stream");	// 注意第二个参数stream_name不要用大写字母。
	sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*env, (const char *)h265FilePath, reuseFirstSource));

	// step4: 将会话加入RTPS Server.
	rtspServer->addServerMediaSession(sms);
	cout << "File name = " << (const char *)h265FilePath << endl;
	//announceStream(rtspServer, sms, main_stream);
	env->taskScheduler().doEventLoop();
	return (void *)0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::createServerByLiveStream()
{
	cout << "Call Live555Rtsp::createServerByLiveStream()." << endl;
	
	int ret = 0;
	ret = pthread_mutex_init(&mutex, NULL);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_init(3), errno = " << ret << strerror(ret) << endl;
		return ret;
	}
	cout << "Success to call pthread_mutex_init(3)." << endl;
	
	ret = pthread_cond_init(&cond, NULL);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_cond_init(3), errno = " << ret << strerror(ret) << endl;
		return ret;
	}
	cout << "Success to call pthread_cond_init(3)." << endl;
	
	ret = pthread_create(&tid, NULL, __routeLiveStream, this);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_create(3), errno = " << errno << strerror(errno) << endl;
		tid = -1;
		return ret;
	}
	cout << "Success to call pthread_create(3). Create rtsp server." << endl;
	
	cout << "Call Live555Rtsp::createServerByLiveStream() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::destroyLiveStreamServer()
{
	cout << "Call Live555Rtsp::destroyLiveStreamServer()." << endl;
	
	if(!bRouteRunning)
	{
		return -1;
	}
	bRouteRunning = false;
	usleep(100);
	
	int ret = 0;
	ret = pthread_cond_broadcast(&cond);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_cond_broadcast(3), ret = " << ret << ". " << endl;
	}
	cout << "Call pthread_cond_broadcast() end." << endl;
	usleep(100);
	
	ret = pthread_mutex_unlock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_unlock(3), ret = " << ret << ". " << endl;
	}
	cout << "Call pthread_mutex_unlock() end." << endl;
	
	ret = pthread_mutex_destroy(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_destroy(3), ret = " << ret << ". " << endl;
	}	
	cout << "Call pthread_mutex_destroy() end." << endl;
	
	ret = pthread_cond_destroy(&cond);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_cond_destroy(3), ret = " << ret << ". " << endl;
	}
	cout << "Call pthread_cond_destroy() end." << endl;

	if(NULL != rtspServer)
	{
		cout << "close(RTSPServer)." << endl;
		Medium::close(rtspServer);
		rtspServer = NULL;
	}
	
	tid = -1;
	
	cout << "Call Live555Rtsp::destroyLiveStreamServer() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *Live555Rtsp::__routeLiveStream(void *arg)
{
	pthread_detach(pthread_self());

	Live555Rtsp *pThis = (Live555Rtsp *)arg;
	return pThis->routeLiveStream(NULL);
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *Live555Rtsp::routeLiveStream(void *arg)
{
	cout << "Call Live555Rtsp::routeLiveStream()." << endl;
	OutPacketBuffer::maxSize = 128 * 1024;	// for debug
	
	// 循环事件中需要使用BasicTaskScheduler0 类型的调度对象。
	BasicTaskScheduler0* basicscheduler = NULL;
	basicscheduler = BasicTaskScheduler::createNew();

	// 创建RTSP服务器的过程中使用的是TaskScheduler 类型的任务调度对象。故而需要类型转换。
	TaskScheduler* scheduler = NULL;
	scheduler = basicscheduler;

	// 创建RTSP环境。
	UsageEnvironment* env = NULL;
	env = BasicUsageEnvironment::createNew(*scheduler);

	// 设置端口号
	Port ourPort = 554;		// Port为0意味着让RTSP服务器自己选择端口。
	UserAuthenticationDatabase* authDB = NULL;	// 无认证。
	rtspServer = RTSPServer::createNew(*env, ourPort, authDB);
	if(NULL == rtspServer)
	{
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		return (void *)-1;
	}

	// 建立名为streamName的RTSP流。
	char const* streamName = "livestream";
	ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName);

	// 添加会话。
	Boolean reuseFirstSource = true;
	sms->addSubsession(H265VideoLiveServerMediaSubssion::createNew(*env, reuseFirstSource));
	rtspServer->addServerMediaSession(sms);

	// 打印RTSP服务器信息，流信息；然后执行循环事件。
	announceStream(rtspServer, sms, streamName);
	doEventLoop(basicscheduler);

	cout << "Call Live555Rtsp::routeLiveStream() end." << endl;
	return (void *)0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void Live555Rtsp::doEventLoop(BasicTaskScheduler0 *Basicscheduler)
{
	bRouteRunning = true;
	while(bRouteRunning)
	{
		Basicscheduler->SingleStep();
	}
}

