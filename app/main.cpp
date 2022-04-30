/*----------------------------------------------------------------聽
sigma star鐗堟潈鎵鏈夈?浣滆咃細
鏃堕棿锛?020.7.10
----------------------------------------------------------------*/

#include "iostream"

#include "sys.h"
#include "sensor.hpp"
#include "mi_vif.h"
#include "mi_vpe.h"
#include "mi_venc.h"
#include "ai.hpp"
#include "ao.hpp"
#include "aac.h"
#include "aad.h"
#include "mi_rgn.h"
#include "wifi.h"
#include "avtp.h"
#include "testing.h"
#include "ircut.h"
#include "live555rtsp.h"
#include "ffmpeg.h"
#include "queue.h"
#include "spipanel.h"
#include "audio_player.h"

using namespace std;

int main(int argc, const char *argv[])
{
	AudioOut *pAudioOut = AudioOut::getInstance();
	pAudioOut->setVolume(-20);
	AudioPlayer *pAudioPlayer = AudioPlayer::getInstance();
	//pAudioPlayer->playPCM("/mnt/linux/Music/pcm_16000_16bit.pcm");
	//pAudioPlayer->playWAV("/mnt/linux/Downloads/tiramisuV010/sourcecode/sdk/verify/mi_demo/source/audio/wav/16K_16bit_MONO_30s.wav");
	pAudioPlayer->playWAV("/mnt/linux/Music/1234567890abcde.wav");

	while(true)
	{
		sleep(1);
		static int sleepCntSec = 0;
		if(0 == (++sleepCntSec % 10))
		{
			cout << "Progress running." << endl;
		}
		
		if(60 * 1 == sleepCntSec)		// n * 60 = n min.
		{
			break;
		}
	}

	return 0;
}

