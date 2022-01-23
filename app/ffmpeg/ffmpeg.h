/*---------------------------------------------------------------- 
sigma star版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
本程序基于ffmpeg 开源代码进行开发，请遵守ffmpeg 开源规则。
*/

#ifndef __FFMPEG_H__
#define __FFMPEG_H__

// FFMPEG需要使用C风格编译
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavutil/file.h>
}

#include <pthread.h>
#include "queue.h"
#include "venc.h"
#include "vector"

/*
主要的函数介绍：
void av_dump_format(AVFormatContext *ic, int index, const char *url, int is_output);
功能：
参数：

int avformat_alloc_output_context2(AVFormatContext **ctx, const AVOutputFormat *oformat,
	const char *format_name, const char *filename);
功能：
参数：

AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c);
功能：
参数：

void avformat_free_context(AVFormatContext *s);
功能：
参数：

int avformat_write_header(AVFormatContext *s, AVDictionary **options);
功能：
参数：

int av_write_trailer(AVFormatContext *s);
功能：
参数：

int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt);
功能：
参数：
*/

/*
设计思路：
建立MP4封装器线程，等候H.26X帧数据。每给对象喂进一个H.26X Frame, 就将该Frame放进容器。
设置视频时长X秒，每存够X秒的视频，就新建下一个视频。
*/

class Ffmpeg{
public:
	static Ffmpeg* getInstance();

	int enable();
	int disable();

	int setFrameQueueDepth(const unsigned int dequeDepth = 15);
	double setRecordSec(double _recordSec = 20.0);
	const char *setFileSavePath(const char *path);
	int setCodecpar(unsigned int _width = 1920, unsigned int _height = 1080, unsigned int _outFPS = 30);
	int sendH26xFrame(const Venc::stStreamPack_t *pstPacket);
	int requestIDR();

	bool isH265VPSFrame(unsigned char *dataBuf, unsigned dataSize);
	bool isH265SPSFrame(unsigned char *dataBuf, unsigned dataSize);
	bool isH265PPSFrame(unsigned char *dataBuf, unsigned dataSize);
	bool isH265IDRFrame(unsigned char *dataBuf, unsigned dataSize);
private:
	Ffmpeg();
	~Ffmpeg();
	Ffmpeg(const Ffmpeg&);
	Ffmpeg& operator=(const Ffmpeg&);

	bool bEnable;

	bool bRouteRunning;		// stream 线程的运行状态。
	pthread_t routeTid;		// 播放音频流线程的tid.
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	Queue<Venc::stStreamPack_t> frameQueue;
	//Queue<Venc::stStreamPack_t> frameQueue1{5};

	unsigned int outFPS = 30;
	unsigned int width = 1920;
	unsigned int height = 1080;
	double recordSec = 120.0;
	const char *fileSavePath = "/customer";

	// 类中创建线程，必须使用一个空格函数传递this 指针。
	void *route(void *arg);
	static void *__route(void *arg);
	
	int streamRouteCreate();
	int streamRouteDestroy();

	const char *getTimeString(char *timeStrBuf, const unsigned int timeStrSize);
	const char *getFileFullName(char *fileNameBuf, const unsigned int fileNameSize);
	
	int recvH26xFrame(Venc::stStreamPack_t *pstPacket);
};

#endif

