/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2023.7.7
----------------------------------------------------------------*/

#include "JPEGVideoSource.hh"
#include "JPEG2000VideoRTPSource.hh"


class JPEGVideoStreamSource: public JPEGVideoSource
{
public:
	static JPEGVideoStreamSource* createNew(UsageEnvironment& env, FramedSource* source);

private:
	JPEGVideoStreamSource(UsageEnvironment& env, FramedSource* source);
	// called only by createNew()

	virtual ~JPEGVideoStreamSource();

public: // redefined virtual functions
	virtual void doGetNextFrame();

	virtual u_int8_t type()
	{
		return fType;
	};
	
	virtual u_int8_t qFactor()
	{
		return 60;
	};
	
	virtual u_int8_t width()
	{
		//printf("w = %d\n", fWidth);
		return fWidth;
	};
	
	virtual u_int8_t height()
	{
		//printf("h = %d\n", fHeight);s
		return fHeight;
	};
	
	virtual u_int16_t widthPixels()
	{
		printf("w = %d\n", fWidthPixels);
		return fWidthPixels;
	};
	
	virtual u_int16_t heightPixels()
	{
		printf("h = %d\n", fHeightPixels);
		return fHeightPixels;
	};
	
	virtual u_int8_t const* quantizationTables(u_int8_t& precision, u_int16_t& length);

private:
	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes, 
				struct timeval presentationTime, unsigned durationInMicroseconds);

private:
	FramedSource* fSource;
	u_int8_t fType;
	u_int8_t fWidth, fHeight; // actual dimensions /8
	u_int8_t fPrecision;
	u_int8_t fQuantizationTable[128];
	u_int16_t fQtableLength;
	u_int16_t fWidthPixels, fHeightPixels; // actual pixels, can support width/height more than 2048
};

