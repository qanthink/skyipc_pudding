/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
本程序基于ffmpeg 开源代码进行开发，请遵守ffmpeg 开源规则。
*/

#include "ffmpeg.h"
#include "iostream"
#include "errno.h"
#include "limits.h"
#include "unistd.h"

using namespace std;

Ffmpeg* Ffmpeg::getInstance()
{
	static Ffmpeg ffmpeg;
	return &ffmpeg;
}

Ffmpeg::Ffmpeg()
{
	enable();
}

Ffmpeg::~Ffmpeg()
{
	disable();
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Ffmpeg::enable()
{
	cout << "Call Ffmpeg::enable()." << endl;

	if(bEnable)
	{
		return 0;
	}

	setFrameQueueDepth(15);
	setFileSavePath("/mnt/linux/Downloads/videotest");
	setCodecpar(1920, 1080, 60);
	setRecordSec(30.0);
	streamRouteCreate();
	bEnable = true;

	cout << "Call Ffmpeg::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Ffmpeg::disable()
{
	cout << "Call Ffmpeg::disable()." << endl;

	bEnable = false;
	streamRouteDestroy();
	
	cout << "Call Ffmpeg::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Ffmpeg::setFrameQueueDepth(const unsigned int queueDepth)
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
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
void *Ffmpeg::route(void *arg)
{
	//cout << "Call Ffmpeg::route()" << endl;

	bRouteRunning = true;
	while(bEnable && bRouteRunning)
	{
		/* 准备输出文件 */
		const unsigned int fileNameSize = PATH_MAX;
		char fileNameBuf[fileNameSize] = {0};
		getFileFullName(fileNameBuf, fileNameSize);

		/* 准备FFMPEG的AVFormatContext */
		AVFormatContext *avOutFmtCtx = NULL;
		avformat_alloc_output_context2(&avOutFmtCtx, NULL, NULL, fileNameBuf);
		if(NULL == avOutFmtCtx)
		{
			cerr << "Fial to call avformat_alloc_output_context2()." << endl;
			return (void *)-1;
		}

		AVStream *avOutStream = NULL;
		avOutStream = avformat_new_stream(avOutFmtCtx, NULL);
		if(NULL == avOutStream)
		{
			cerr << "Fial to call avformat_new_stream()." << endl;
			avformat_free_context(avOutFmtCtx);
			return (void *)-2;
		}

		/* 设置编码器参数，图像宽高、色彩空间、时间基、色彩空间等 */
		{
			avOutStream->codecpar->width = width;
			avOutStream->codecpar->height = height;
			avOutStream->codecpar->bit_rate = 2000000;
			avOutStream->codecpar->profile = 1;
			avOutStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
			avOutStream->codecpar->codec_id = AV_CODEC_ID_HEVC;
			avOutStream->codecpar->codec_tag = 0;
			avOutStream->codecpar->format = AV_PIX_FMT_YUV420P;
			avOutStream->codecpar->color_range = AVCOL_RANGE_MPEG;
			avOutFmtCtx->streams[0]->time_base = {1, 90000};
		}

		#if 1	// debug
		cout << "========= dump avout information =========" << endl;
		av_dump_format(avOutFmtCtx, 0, fileNameBuf, 1);
		cout << "========= ====================== =========" << endl;
		#endif

		/* 打开输出文件并写入头信息。 */
		int ret = 0;
		ret = avio_open(&avOutFmtCtx->pb, fileNameBuf, AVIO_FLAG_WRITE);
		if(ret < 0)
		{
			cerr << "Fail to call avio_open(). Filename: " << fileNameBuf << endl;
			avformat_free_context(avOutFmtCtx);
			return (void *)ret;
		}
		
		ret = avformat_write_header(avOutFmtCtx, NULL);
		if(ret < 0)
		{
			cerr << "Fail to call avformat_write_header()." << endl;
			avformat_free_context(avOutFmtCtx);
			return (void *)ret;
		}

		double secCnt = 0.0;
		AVRational avInRational = {1, 1200000};	// 不知道为啥是12000000, 默认H.26X就是它吧。
		unsigned long int pts = 0;
		unsigned long int duration = 0;
		duration = (avInRational.den / outFPS);
		double perFrameSeconds = (double)1 / outFPS;
		cout << "perFrameSeconds = " << perFrameSeconds << endl;

		// 准备I帧。视频的第一帧必须为I帧。
		Venc *pVenc = Venc::getInstance();
		pVenc->requestIdr(Venc::vencMainChn, 1);
		bool isFirstFrame = true;
		
		/* 循环获取编码器输出的数据，封装进容器 */
		while(bEnable && bRouteRunning && (secCnt < recordSec))	// 秒计数器。
		{
			Venc::stStreamPack_t stStreamPack = {0};
			ret = recvH26xFrame(&stStreamPack);
			if(0 != ret)
			{
				cerr << "Fail to call Ffmpeg::recvH26xFrame()." << endl;
				continue;
			}

			// 如果是视频文件的第一帧的话，必须是I帧。
			if(isFirstFrame)
			{
				if(!isH265VPSFrame(stStreamPack.u8Pack, stStreamPack.u32Len))
				{
					continue;
				}
				isFirstFrame = false;
			}
			
			#if 0 //debug
			cout << "In stream timebase.den = " << avInRational.den << ", .num = " << avInRational.num << endl;
			cout << "Out stream timebase.den = " << avOutStream->time_base.den << ", .num = " << avOutStream->time_base.num << endl;
			#endif
			
			// 将H.26X数据放入AVPACKET中，设置PTS/DTS/Duration.
			AVPacket avPacket = {0};
			avPacket.pts = av_rescale_q_rnd(pts, avInRational, avOutStream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			// 2021.8.26 H.26X的MP/BS画质等级中，有I/B/P三种帧类型。此处如果对B帧解码时间计算错误，会导致花屏。干脆不计算。
			//avPacket.dts = av_rescale_q_rnd(dts, avInRational, avOutStream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			avPacket.duration = av_rescale_q(duration, avInRational, avOutStream->time_base);
			avPacket.pos = -1;
			pts += duration;
			secCnt += perFrameSeconds;
			//cout << "secCnt = " << secCnt << endl;
			
			avPacket.data = (uint8_t *)stStreamPack.u8Pack;
			avPacket.size = stStreamPack.u32Len;

			#if 0	// debug
			cout << "avPacket.size = " << avPacket.size << endl;
			cout << "avPacket.pts, avPacket.dts, avPacket.duration = " 
				<< avPacket.pts << ", " << avPacket.dts << ", " << avPacket.duration << endl;
			#endif
			// 将AVPACKET数据写入容器。
			ret = av_interleaved_write_frame(avOutFmtCtx, &avPacket);
			if(ret < 0)
			{
				cerr << "Fail to call av_interleaved_write_frame()." << endl;
				av_packet_unref(&avPacket);
				avio_closep(&avOutFmtCtx->pb);
				avformat_free_context(avOutFmtCtx);
			}
			av_packet_unref(&avPacket);
		}

		av_write_trailer(avOutFmtCtx);
		avio_closep(&avOutFmtCtx->pb);
		avformat_free_context(avOutFmtCtx);
	}

	cout << "Call Ffmpeg::route() end." << endl;
	return NULL;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
void *Ffmpeg::__route(void *arg)
{
	pthread_detach(pthread_self());

	Ffmpeg *pThis = (Ffmpeg *)arg;
	return pThis->route(NULL);
}

int Ffmpeg::streamRouteCreate()
{
	int ret = 0;
	int initVal = 0;
	bool bShared = false;

	ret = pthread_mutex_init(&mutex, NULL);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_init(3), ret = " << ret << ". " << endl;
		return ret;
	}
	cout << "Success to call pthread_mutex_init(3)." << endl;

	ret = pthread_cond_init(&cond, NULL);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_cond_init(3), ret = " << ret << ". " << endl;
	}
	cout << "Success to call pthread_cond_init(3)." << endl;
	
	ret = pthread_create(&routeTid, NULL, __route, this);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_create(3), ret = " << ret << ". " << strerror(errno) << endl;
		return ret;
	}
	cout << "Success to call pthread_create(3)." << endl;
}

int Ffmpeg::streamRouteDestroy()
{
	cout << "Call Ffmpeg::streamRouteDestroy()." << endl;
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
	
	if(-1 != routeTid)
	{
		routeTid = -1;
	}

	cout << "Call Ffmpeg::streamRouteDestroy() end." << endl;
	return 0;
}


const char *Ffmpeg::setFileSavePath(const char *path)
{
	const char *defaultPath = "/customer";
	
	if(NULL == path)
	{
		fileSavePath = defaultPath;
		return defaultPath;
	}
	else if(strlen(path) >= PATH_MAX)
	{
		fileSavePath = defaultPath;
		return defaultPath;
	}
	else
	{
		fileSavePath = path;
		return path;
	}

	return defaultPath;
}

const char *Ffmpeg::getTimeString(char *timeStrBuf, const unsigned int timeStrSize)
{
	time_t stTime = {0};
	stTime = time(NULL);
	if((time_t)-1 == stTime)
	{
		cerr << "Fail to call time(2), " << strerror(errno) << endl;
		return NULL;
	}
	
	struct tm *pst_tm = NULL;
	pst_tm = localtime(&stTime);
	if(NULL == pst_tm)
	{
		cerr << "Fail to call localtime(3)." << endl;
		return NULL;
	}
	
	//2021-01-01-18-59-59
	strftime(timeStrBuf, timeStrSize, "%Y-%m-%d-%H-%M-%S", pst_tm);
	
	//cout << "timeString = " << timeStrBuf << endl;

	return timeStrBuf;
}

const char *Ffmpeg::getFileFullName(char *fileNameBuf, const unsigned int fileNameSize)
{
	if(NULL == fileNameBuf || 0 == fileNameSize)
	{
		cerr << "Fail to call Ffmpeg::getFileFullName(). Point has null value or buff has zero length." << endl;
		return NULL;
	}
	
	const unsigned int timeStrSize = 128;
	char timeStrBuf[timeStrSize] = {0};
	getTimeString(timeStrBuf, timeStrSize);
	
	if(strlen(fileSavePath) + strlen(timeStrBuf) >= fileNameSize)
	{
		cerr << "File path length is out of range, max length = " << fileNameSize << endl;
		return NULL;
	}

	strncpy(fileNameBuf, fileSavePath, strlen(fileSavePath));
	if('/' != fileNameBuf[strlen(fileNameBuf)])
	{
		fileNameBuf[strlen(fileNameBuf)] = '/';
	}

	strncat(fileNameBuf, timeStrBuf, strlen(timeStrBuf));
	strncat(fileNameBuf, ".mp4", 4);
	
	cout << "fileNameBuf = " << fileNameBuf << endl;

	return NULL;
}

int Ffmpeg::setCodecpar(unsigned int _width, unsigned int _height, unsigned int _outFPS)
{
	width = _width;
	height = _height;
	outFPS = _outFPS;
	
	return 0;
}

double Ffmpeg::setRecordSec(double _recordSec)
{
	recordSec = _recordSec;
	return recordSec;
}

int Ffmpeg::sendH26xFrame(const Venc::stStreamPack_t *pstPacket)
{
	//cout << "Call Ffmpeg::sendH26xFrame()." << endl;
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

	//cout << "End of call Ffmpeg::sendH26xFrame()." << endl;	
	return 0;
}

int Ffmpeg::recvH26xFrame(Venc::stStreamPack_t *pstPacket)
{
	//cout << "Call Ffmpeg::recvH26xFrame()." << endl;
	int ret = 0;
	ret = pthread_mutex_lock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_lock(3), ret = " << ret << ". " << endl;
		return -1;
	}
	
	while(frameQueue.isEmpty() && bRouteRunning)
	{
	
		ret = pthread_cond_wait(&cond, &mutex);
		if(0 != ret)
		{
			cerr << "Fail to call pthread_cond_wait(3), ret = " << ret << ". " << endl;
			pthread_mutex_unlock(&mutex);
			return -1;
		}
	}

	frameQueue.pop(pstPacket);
	ret = pthread_mutex_unlock(&mutex);
	if(0 != ret)
	{
		cerr << "Fail to call pthread_mutex_unlock(3), ret = " << ret << ". " << strerror(errno) << endl;
		return -1;
	}

	//cout << "End of call Ffmpeg::recvH26xFrame()." << endl;	
	return 0;
}

int Ffmpeg::requestIDR()
{
	//cout << "Call Ffmpeg::requestIDR()." << endl;
	
	unsigned int u32Ret = 0;
	Venc *pVenc = Venc::getInstance();
	u32Ret = pVenc->requestIdr(Venc::vencMainChn, 1);
	if(0 != u32Ret)
	{
		cerr << "Fail to call Venc::requestIdr(). u32Ret = " << u32Ret << endl;
	}

	//cout << "End of call Ffmpeg::requestIDR()." << endl;	
	return u32Ret;
}

/*-----------------------------------------------------------------------------
描--述：判断是否为H265VPS帧。
参--数：
返回值：
说--明：判断帧类型的条件：data[0], data[1], data[2]为0；且data[3]为1；
		此时看data[4]data[5], H265用两个字节记录NAL.
		最左边第0位为校正位，必须为0; 第7-13位，14-15位，暂不关心。
		左边第1-6位为帧Type, 取值范围0-63. 我们需要关注。
		取值为十进制的19、20, 表示IDR帧；取值为32、33、34表示VPS, SPS, PPS.
		H.264的NAL:
		|0|1|2|3|4|5|6|7|
		|F|NRI|   Type  |
		H.265的NAL:
		|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|
		|F|    Type   |   LayerId   |TID|
+------------ - +---------------- - +
		一个VPS包含多个SPS, 一个SPS包含多个PPS.
-----------------------------------------------------------------------------*/
bool Ffmpeg::isH265VPSFrame(unsigned char *dataBuf, unsigned dataSize)
{
	if(NULL == dataBuf || dataSize < 5)
	{
		cerr << "Fail to call Ffmpeg::isH265VPSFrame(). Pointer has null value or data size is less than 4." << endl;
		return false;
	}

	bool isVPS = false;
	if(0x00 == dataBuf[0] && 0x00 == dataBuf[1] && 0x00 == dataBuf[2] && 0x01 == dataBuf[3] && ((32 == (dataBuf[4] & 0x7E) >> 1)))
	{
		isVPS =  true;
	}
	else
	{
		isVPS =  false;
	}

	#if 0	//debug
	if(isVPS)
	{
		cout << "=================VPS Frame Data======================" << endl;
		int i = 0;
		for(i = 0; i < dataSize / 8; ++i)
		{
			int j = 0;
			for(j = 0; j < 8; ++j)
			{
				printf("%2x ", dataBuf[i * 8 + j]);
			}
			//cout << endl;
		}
	}
	#endif

	return isVPS;
}

bool Ffmpeg::isH265IDRFrame(unsigned char *dataBuf, unsigned dataSize)
{
	if(NULL == dataBuf || dataSize < 5)
	{
		cerr << "Fail to call Ffmpeg::isH265IDRFrame(). Pointer has null value or data size is less than 4." << endl;
		return false;
	}

	bool isIDR = false;
	if(0x00 == dataBuf[0] && 0x00 == dataBuf[1] && 0x00 == dataBuf[2] && 0x01 == dataBuf[3] 
		&& ((19 == (dataBuf[4] & 0x7E) >> 1)) || (20 == (dataBuf[4] & 0x7E) >> 1))
	{
		isIDR =  true;
	}
	else
	{
		isIDR =  false;
	}

	#if 1	//debug
	if(isIDR)
	{
		cout << "=================IDR Frame Data======================" << endl;
		int i = 0;
		for(i = 0; i < dataSize / 8; ++i)
		{
			int j = 0;
			for(j = 0; j < 8; ++j)
			{
				printf("%2x ", dataBuf[i * 8 + j]);
			}
			//cout << endl;
		}
	}
	#endif

	return isIDR;
}

