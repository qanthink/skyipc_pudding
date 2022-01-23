#include "H265FramedLiveSource.hh"
#include "iostream"

#include "venc.h"
#include "live555rtsp.h"

using namespace std;

H265FramedLiveSource::H265FramedLiveSource(
		UsageEnvironment &env, unsigned preferredFrameSize, unsigned playTimePerFrame)
: FramedSource(env)
{
	cout << "Call H265FramedLiveSource::H265FramedLiveSource()" << endl;
	readSize = 0;
	databufSize = 0;
}

H265FramedLiveSource *H265FramedLiveSource::createNew(
		UsageEnvironment &env, unsigned preferredFrameSize, unsigned playTimePerFrame)
{
	cout << "Call H265FramedLiveSource::createNew()" << endl;
	H265FramedLiveSource* newSource = 
		new H265FramedLiveSource(env, preferredFrameSize, playTimePerFrame);
	return newSource;
}

unsigned H265FramedLiveSource::maxFrameSize() const 
{
	return 140000;
}

H265FramedLiveSource::~H265FramedLiveSource()
{
	cout << "Call H265FramedLiveSource::~H265FramedLiveSource()" << endl;
	readSize = 0;
	databufSize = 0;
}

static Venc::stStreamPack_t stStreamPack;

void H265FramedLiveSource::doGetNextFrame()
{
	//cout << "Call H265FramedLiveSource::doGetNextFrame()." << endl;
	
	/* ---------------------------------------------------------------------------------
	tips1: 从编码器获取H.264/H.265/MJPEG数据，并写入live555的底层数据区fTo, 长度信息写入fFramedSize;
	tips2: fTo的容量上限为fMaxSize, 若我们的数据超过fMaxSize, 则需要对fTo执行多次写操作。
	tips3: 不可使用循环语句在一次doGetNextFrame()中完成tips2中提到的操作。
			需要再多次doGetNextFrame()操作中完成。
			猜测live555的底层逻辑是在每次写fTo后，立刻分析写入的数据，分析结束后才可执行下次写入。
			故而需要在多次doGetNextFrame()中完成。
	tips4: 若存在多次写操作，则借助readSize标记已经读取并写入的数据长度。
			直到readsize == bufsize, 则认为所有数据均已写入到fTo.
			此时再次进入tips1.
	--------------------------------------------------------------------------------- */

	fFrameSize = 0;
	if(0 == (databufSize - readSize))
	{
		memset(&stStreamPack, 0, sizeof(Venc::stStreamPack_t));
		
		int ret = 0;
		Live555Rtsp *pLive555Rtsp = Live555Rtsp::getInstance();
		ret = pLive555Rtsp->recvH26xFrame(&stStreamPack);
		//Venc *pVenc = Venc::getInstance();
		//pVenc->rcvStream(Venc::vencMainChn, &stStreamPack);
		if(0 != ret)
		{
			cerr << "Fail to call pLive555Rtsp->recvH26xFrame(&stStreamPack) in doGetNextFrame(), ret = " << ret << endl;
			return;
		}

		readSize = 0;
		databufSize = stStreamPack.u32Len;
		fFrameSize = 0;
		fNumTruncatedBytes = 0;

		if(fMaxSize == databufSize)
		{
			cout << "fMaxSize == databufSize" << endl;
		}
	}

	if((databufSize - readSize) > fMaxSize)
	{
		//memcpy(fTo, stStreamPack.u8Pack + readSize, fMaxSize);
		#if 1
		fFrameSize = fMaxSize;
		#else
		fFrameSize = databufSize - readSize;
		#endif
		readSize += fMaxSize;
		fNumTruncatedBytes = databufSize - readSize;
		//fDurationInMicroseconds = 5;
	}
	else
	{
		memcpy(fTo, stStreamPack.u8Pack + readSize, databufSize - readSize);
		fFrameSize = databufSize - readSize;
		readSize += (databufSize - readSize);
		fNumTruncatedBytes = 0;
		//fDurationInMicroseconds = 5;
	}

	// Set the 'presentation time':
	if(fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0)
	{
		// This is the first frame, so use the current time:
		gettimeofday(&fPresentationTime, NULL);
	}
	else
	{
		// Increment by the play time of the previous frame (20 ms)
		unsigned uSeconds   = fPresentationTime.tv_usec + 1000000.0 / 60;
		fPresentationTime.tv_sec += uSeconds / 1000000;
		fPresentationTime.tv_usec = uSeconds % 1000000;
	}

	fDurationInMicroseconds = 1000000.0 / 60;
	nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
		(TaskFunc*)FramedSource::afterGetting, this);

	//cout << "End of call H265FramedLiveSource::doGetNextFrame()." << endl;
	
	return;
}

#if 0
void H265FramedLiveSource::doStopGettingFrames()
{

}
#endif
