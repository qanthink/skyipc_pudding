/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2023.7.7
----------------------------------------------------------------*/

#pragma once

#include "FileServerMediaSubsession.hh"		// From Live Lib

class JPEGVideoFileServerMediaSubsession: public FileServerMediaSubsession{
public:
	static JPEGVideoFileServerMediaSubsession* createNew(UsageEnvironment& env,
					const char *filePath, Boolean reuseFirstSource = true);

	// Used to implement "getAuxSDPLine()":
	void checkForAuxSDPLine1();
	void afterPlayingDummy1();

protected:
	// called only by createNew();
	JPEGVideoFileServerMediaSubsession(UsageEnvironment& env,
					const char *filePath, Boolean reuseFirstSource);
	virtual ~JPEGVideoFileServerMediaSubsession();

	void setDoneFlag(){fDoneFlag = ~0;}

protected: // redefined virtual functions
	virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, 
			unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

private:
	char* fAuxSDPLine;
	char fDoneFlag; // used when setting up "fAuxSDPLine"
	RTPSink* fDummyRTPSink;	// ditto
};

