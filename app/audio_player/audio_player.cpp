/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.2.13
----------------------------------------------------------------*/

/*
*/

#define RUNDE_LICENSE 0
#define FREE_FRAME 999

#include "audio_player.h"

#include <thread>
#include <unistd.h>
#include <fstream>
#include <string.h>
#include <iostream>

using namespace std;

AudioPlayer* AudioPlayer::getInstance()
{
	static AudioPlayer audioPlayer;
	return &audioPlayer;
}

AudioPlayer::AudioPlayer()
{
}

AudioPlayer::~AudioPlayer()
{
}

/*
	功能：	播放PCM 文件，用户接口。
	返回：	
	注意：	
*/
int AudioPlayer::playPCM(const char *filePath)
{
	cout << "Call AudioPlayer::playPCM()." << endl;
	thread th(thPlayRoutePCM, filePath);
	th.detach();
	cout << "Call AudioPlayer::playPCM() end." << endl;
	return 0;
}

/*
	功能：	播放PCM 文件，内部实现，类内线程转换。
	返回：	
	注意：	
*/
int AudioPlayer::thPlayRoutePCM(const char *filePath)
{
	AudioPlayer *pThis = (AudioPlayer *)filePath;
	return pThis->playRoutePCM(filePath);
}

/*
	功能：	播放PCM 文件，内部实现。
	返回：	0, 成功；-1, 参数无效；-2, 文明打开失败。
	注意：	
*/
int AudioPlayer::playRoutePCM(const char *filePath)
{
	cout << "Call AudioPlayer::playRoutePMC()." << endl;
	if(NULL == filePath)
	{
		cerr << "Fail to call AudioPlayer::playRoutePMC(), argument has null value!" << endl;
		return -1;
	}

	ifstream ifs((const char *)filePath, ios::in);
	if(ifs.fail())
	{
		cerr << "Fail to open file: " << filePath << ". " << strerror(errno) << endl;
		return -2;
	}
	cout << "Success to open " << filePath << endl;

	int i = 0;
	while(!ifs.eof())
	{
		int readBytes = 0;
		unsigned int dataBufMaxSize = 1 * 1024;
		char dataBuf[dataBufMaxSize] = {0};
		
		//cout << "Ready to call read() in routeAo()." << endl;
		ifs.read(dataBuf, dataBufMaxSize);
		readBytes = ifs.gcount();
		if(!ifs)
		{
			cout << "error: only " << readBytes << " could be read";
			break;
		}
		cout << "readBytes = " << readBytes << endl;

#if 0 == RUNDE_LICENSE
		if(++i > FREE_FRAME)
		{
			break;
		}
#endif

		//cout << "Send pcm stream" << endl;
		AudioOut::getInstance()->sendStream(dataBuf, readBytes);
	}


	ifs.close();

	cout << "Call AudioPlayer::playRoutePMC() end." << endl;
	return 0;
}

/*
============================== WAV =====================================
============================== WAV =====================================
*/

/*
	功能：	播放WAV 文件，用户接口。
	返回：	
	注意：	
*/
int AudioPlayer::playWAV(const char *filePath)
{
	cout << "Call AudioPlayer::playWAV()." << endl;
	thread th(thPlayRouteWAV, filePath);
	th.detach();
	cout << "Call AudioPlayer::playWAV() end." << endl;
	return 0;
}

/*
	功能：	播放WAV 文件，内部实现，类内线程转换。
	返回：	
	注意：	
*/
int AudioPlayer::thPlayRouteWAV(const char *filePath)
{
	AudioPlayer *pThis = (AudioPlayer *)filePath;
	return pThis->playRouteWAV(filePath);
}

/*
	功能：	播放WAV 文件，内部实现。
	返回：	0, 成功；-1, 参数无效；-2, 文明打开失败。
	注意：	
*/
int AudioPlayer::playRouteWAV(const char *filePath)
{
	cout << "Call AudioPlayer::playRouteWAV()." << endl;
	if(NULL == filePath)
	{
		cerr << "Fail to call AudioPlayer::playRouteWAV(), argument has null value!" << endl;
		return -1;
	}

	ifstream ifs((const char *)filePath, ios::in);
	if(ifs.fail())
	{
		cerr << "Fail to open file: " << filePath << ". " << strerror(errno) << endl;
		return -2;
	}
	cout << "Success to open " << filePath << endl;

	int readBytes = 0;
	unsigned int dataBufMaxSize = 512;
	//unsigned int dataBufMaxSize = 540;
	//unsigned int dataBufMaxSize = 7170;
	char dataBuf[dataBufMaxSize] = {0};
	//unsigned int wavHeadBytes = 78;
	unsigned int wavHeadBytes = 44;
	
	if(!ifs.eof())
	{
		ifs.read(dataBuf, wavHeadBytes);
		if(!ifs)
		{
			cerr << "In AudioPlayer::playRouteWAV(): read fail." << endl;
		}
	}

	int i = 0;
	while(!ifs.eof())
	{
		//cout << "Ready to call read() in routeAo()." << endl;
		ifs.read(dataBuf, dataBufMaxSize);
		readBytes = ifs.gcount();
		if(!ifs)
		{
			cout << "error: only " << readBytes << " could be read";
			break;
		}
		cout << "readBytes = " << readBytes << endl;

#if 0 == RUNDE_LICENSE
		if(++i > FREE_FRAME)
		{
			break;
		}
#endif

		//cout << "Send pcm stream" << endl;
		AudioOut::getInstance()->sendStream(dataBuf, readBytes);
		usleep(2000 * 1000);
	}


	ifs.close();

	cout << "Call AudioPlayer::playRouteWAV() end." << endl;
	return 0;
}

int readWavHead(const char *filePath)
{
	int i; //用作循环计数

	unsigned char ch[100]; //用来存储wav文件的头信息

	FILE *fp;
	fp=fopen(filePath,"rb");//为读，打开一个wav文件

	//现在只能读取头文件，可是头文件后的数据要怎么读出来，求大神指教
	/**********输出wav文件的所有信息**********/
	printf("该wav文件的所有信息:");
	for(i=0;i<58;i++)
	{
	ch[i]=fgetc(fp); //每次读取一个字符，存在数组ch中
	if(i%16==0) //每行输出16个字符对应的十六进制数
	printf("\n");
	if(ch[i]<16) //对小于16的数，在前面加0，使其用8bit显示出来
	printf("0%x",ch[i]);
	else
	printf("%x ",ch[i]);
	}

	/*********RIFF WAVEChunk的输出*********/
	printf("\n\nRIFF WAVE Chunk信息:");
	//输出RIFF标志
	printf("\nRIFF标志:");
	for(i=0;i<4;i++)
	{
	printf("%x ",ch[i]);
	}
	//输出size大小
	printf("\nsize:ox");
	for(i=7;i>=4;i--) //低字节表示数值低位，高字节表示数值高位
	{
	if(ch[i]<16)
	printf("0%x",ch[i]);
	else
	printf("%x",ch[i]);
	}
	
	//输出WAVE标志
	printf("\nWAVE标志:");
	for(i=8;i<12;i++)
	{
	if(ch[i]<16)
	printf("0%x ",ch[i]);
	else
	printf("%x ",ch[i]);
	}

	/*******Format Chunk的输出*******/
	printf("\n\nFormat Chunk信息:");
	//输出fmt 标志
	printf("\nfmt 标志:");
	for(i=12;i<16;i++)
	{
	if(ch[i]<16)
	printf("0%x ",ch[i]);
	else
	printf("%x ",ch[i]);
	}

	//输出size段
	printf("\nsize:ox");
	for(i=19;i>15;i--)
	{
	if(ch[i]<16)
	printf("0%x",ch[i]);
	else
	printf("%x",ch[i]);
	}

	//输出编码方式
	printf("\n编码方式:ox");
	for(i=21;i>19;i--)
	{
	if(ch[i]<16)
	printf("0%x",ch[i]);
	else
	printf("%x",ch[i]);
	}

	//输出声道数目
	printf("\n声道数目:ox");
	for(i=23;i>21;i--)
	{
	if(ch[i]<16)
	printf("0%x",ch[i]);
	else
	printf("%x",ch[i]);
	}
	if(ch[i+1]==1) //1表示单声道，2表示双声道
	printf(" 单声道");

	else

	printf(" 双声道");

	//输出采样频率

	printf("\n采样频率:ox");

	for(i=27;i>23;i--)

	{
	if(ch[i]<16)

	printf("0%x",ch[i]);

	else

	printf("%x",ch[i]);

	}

	//输出每秒所需字节数

	printf("\n每秒所需字节数:ox");

	for(i=31;i>27;i--)

	{
	if(ch[i]<16)

	printf("0%x",ch[i]);

	else

	printf("%x",ch[i]);

	}

	//输出数据块对齐单位

	printf("\n数据块对齐单位:ox");

	for(i=33;i>31;i--)

	{
	if(ch[i]<16)

	printf("0%x",ch[i]);

	else

	printf("%x",ch[i]);

	}

	//输出每个采样所需bit数

	printf("\n每个采样所需bit数:ox");

	for(i=35;i>33;i--)

	{
	if(ch[i]<16)

	printf("0%x",ch[i]);

	else

	printf("%x",ch[i]);

	}

	//输出附加信息

	if(ch[16]==18) //若Format Chunk的size大小为18，则该模块的最后两个字节为附加信息

	{ //若为16，则无附加信息

	printf("\n附加信息:ox");

	for(i=37;i>35;i--)

	{
	if(ch[i]<16)

	printf("0%x",ch[i]);

	else

	printf("%x",ch[i]);

	}

	}

	/*******Fact Chunk的输出*******/

	printf("\n\nFact Chunk信息:");

	//输出fact标志

	printf("\nfact标志:");

	for(i=38;i<42;i++)

	{
	if(ch[i]<16)

	printf("0%x ",ch[i]);

	else

	printf("%x ",ch[i]);

	}

	//输出size

	printf("\nsize:ox");

	for(i=45;i>41;i--)

	{
	if(ch[i]<16)

	printf("0%x",ch[i]);

	else

	printf("%x",ch[i]);

	}

	//输出data段数据

	printf("\ndata段数据:");

	for(i=46;i<50;i++)

	{
	if(ch[i]<16)

	printf("0%x ",ch[i]);

	else

	printf("%x ",ch[i]);

	}

	/*******Data Chunk的输出*******/

	printf("\n\nData Chunk信息:");

	//输出data标志

	printf("\ndata标志:");

	for(i=50;i<54;i++)

	{
	if(ch[i]<16)

	printf("0%x ",ch[i]);

	else

	printf("%x ",ch[i]);

	}

	//输出数据大小

	printf("\n数据大小:ox");

	for(i=57;i>53;i--)

	{
	if(ch[i]<16)

	printf("0%x",ch[i]);

	else

	printf("%x",ch[i]);

	}

	printf("\n");

	fclose(fp);

}

