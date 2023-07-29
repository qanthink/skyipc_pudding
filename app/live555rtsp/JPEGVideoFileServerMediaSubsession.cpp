/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2023.7.7
----------------------------------------------------------------*/

#include "JPEGVideoFileServerMediaSubsession.hh"	// From Live Lib
#include "JPEGVideoRTPSink.hh"		// From Live Lib
#include "ByteStreamFileSource.hh"	// From Live Lib
#include "JPEGVideoRTPSource.hh"	// unused
#include "JPEGVideoFramerSource.hh"	// Implement by myself.

JPEGVideoFileServerMediaSubsession*
JPEGVideoFileServerMediaSubsession::createNew(UsageEnvironment& env, 
					const char *filePath, Boolean reuseFirstSource)
{
	printf("JPEGVideoFileServerMediaSubsession::createNew\n");
	return new JPEGVideoFileServerMediaSubsession(env, filePath, reuseFirstSource);
}

JPEGVideoFileServerMediaSubsession::JPEGVideoFileServerMediaSubsession(UsageEnvironment& env,
					const char *filePath, Boolean reuseFirstSource)
	: FileServerMediaSubsession(env, filePath, reuseFirstSource), 
	fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL)
{
}

JPEGVideoFileServerMediaSubsession::~JPEGVideoFileServerMediaSubsession()
{
	printf("Call JPEGVideoFileServerMediaSubsession::~JPEGVideoFileServerMediaSubsession().\n");
	delete[] fAuxSDPLine;
	fAuxSDPLine = NULL;
	printf("Call JPEGVideoFileServerMediaSubsession::~JPEGVideoFileServerMediaSubsession() end.\n");
}

static void afterPlayingDummy(void* clientData)
{
	printf("Call JPEGVideoFileServerMediaSubsession::afterPlayingDummy().\n");
	JPEGVideoFileServerMediaSubsession* subsess = (JPEGVideoFileServerMediaSubsession*)clientData;
	subsess->afterPlayingDummy1();
	printf("Call JPEGVideoFileServerMediaSubsession::afterPlayingDummy() end.\n");
}

void JPEGVideoFileServerMediaSubsession::afterPlayingDummy1()
{
	printf("Call JPEGVideoFileServerMediaSubsession::afterPlayingDummy1().\n");
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
	printf("Call JPEGVideoFileServerMediaSubsession::afterPlayingDummy1() end.\n");
}

/*-----------------------------------------------------------------------------
描--述：检查SDP 会话描述协议。
参--数：
返回值：
注--意：Live555 注释中提到，这个函数的意义在于
		Used to implement "getAuxSDPLine()"
		所以它会被getAuxSDPLine() 调用。
-----------------------------------------------------------------------------*/
static void checkForAuxSDPLine(void* clientData)
{
	printf("Call JPEGVideoFileServerMediaSubsession::checkForAuxSDPLine().\n");
	JPEGVideoFileServerMediaSubsession* subsess = (JPEGVideoFileServerMediaSubsession*)clientData;
	subsess->checkForAuxSDPLine1();
	printf("Call JPEGVideoFileServerMediaSubsession::checkForAuxSDPLine() end.\n");
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：作用见checkForAuxSDPLine()
-----------------------------------------------------------------------------*/
void JPEGVideoFileServerMediaSubsession::checkForAuxSDPLine1()
{
	printf("Call JPEGVideoFileServerMediaSubsession::checkForAuxSDPLine1().\n");
	nextTask() = NULL;

	char const* dasl;
	if(fAuxSDPLine != NULL)
	{
		// Signal the event loop that we're done:
		printf("fAuxSDPLine != NULL, call setDoneFlag().\n");
		setDoneFlag();
	}
	else if(fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL)
	{
		printf("fDummyRTPSink != NULL && dasl != NULL, call setDoneFlag().\n");
		fAuxSDPLine = strDup(dasl);
		fDummyRTPSink = NULL;

		// Signal the event loop that we're done:
		setDoneFlag();
	}
	else if(!fDoneFlag)
	{
		printf("!fDoneFlag, sleep + loop + checkForAuxSDPLine().\n");
		// try again after a brief delay:
		int uSecsToDelay = 100000; // 100 ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
				(TaskFunc*)checkForAuxSDPLine, this);
	}

	printf("Call JPEGVideoFileServerMediaSubsession::checkForAuxSDPLine1() end.\n");
}

/*-----------------------------------------------------------------------------
描--述：获取绘画描述协议SDP.
参--数：
返回值：
注--意：虚函数，需要重定义，会被底层调用。调用顺序：
		底层 -> getAuxSDPLine() -> checkForAuxSDPLine() -> checkForAuxSDPLine1().
-----------------------------------------------------------------------------*/
char const* JPEGVideoFileServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
	printf("Call JPEGVideoFileServerMediaSubsession::getAuxSDPLine().\n");
	if(NULL != fAuxSDPLine)
	{
		printf("fAuxSDPLine:\n%s\n", fAuxSDPLine);
		return fAuxSDPLine; // it's already been set up (for a previous client)
	}

	if(NULL == fDummyRTPSink)
	{
		// we're not already setting it up for another, concurrent stream
		// Note: For H265 video files, the 'config' information (used for several payload-format
		// specific parameters in the SDP description) isn't known until we start reading the file.
		// This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
		// and we need to start reading data from our file until this changes.
		fDummyRTPSink = rtpSink;

		// Start reading the file:
		fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

		// Check whether the sink's 'auxSDPLine()' is ready:
		checkForAuxSDPLine(this);

		#if 1
		/* 自己实现。MJPEG 最重要的参数是宽、高。*/
		char const* addSDPFormat =
						"b=AS:%d\r\n"
						"a=framerate:%d\r\n"
						"a=framesize:%d %d-%d\r\n";

		unsigned addSDPFormatSize = strlen(addSDPFormat)
						 + 4 /* max char len */
						 + 4 /* max char len */
						 + 4 + 4 + 4
						 + 4;

		JPEGVideoStreamSource *fDummySource = (JPEGVideoStreamSource *)inputSource;

		char* fmtp = new char[addSDPFormatSize];
		sprintf(fmtp,
				addSDPFormat,
				4096,
				30,
				rtpSink->rtpPayloadType(),		// 这个值一定是26.
				fDummySource->widthPixels(),
				fDummySource->heightPixels()
		);
		printf("fmtp:\n%s\n", fmtp);
		printf("w = %d, h = %d\n", fDummySource->widthPixels(), fDummySource->heightPixels());
		fAuxSDPLine = strDup(fmtp);
		#endif
	}
	
	envir().taskScheduler().doEventLoop(&fDoneFlag);

	printf("Call JPEGVideoFileServerMediaSubsession::getAuxSDPLine() end.\n");
	return fAuxSDPLine;
}

FramedSource* JPEGVideoFileServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
	printf("Call JPEGVideoFileServerMediaSubsession::createNewStreamSource().\n");
	estBitrate = 500; // kbps, estimate

	// Create the video source:
	ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(envir(), fFileName);
	if(NULL == fileSource)
	{
		return NULL;
	}
	fFileSize = fileSource->fileSize();
	printf("fileSource->fileSize() = %d\n", fFileSize);

	printf("Call JPEGVideoFileServerMediaSubsession::createNewStreamSource() end.\n");
	// Create a framer for the Video Elementary Stream:
	return JPEGVideoStreamSource::createNew(envir(), fileSource);
}

RTPSink* JPEGVideoFileServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
				unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/)
{
	return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
}

