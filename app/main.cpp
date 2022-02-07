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

using namespace std;

int main(int argc, const char *argv[])
{
	SpiPanel *pSpiPanel = SpiPanel::getInstance();
	
	int fbColor = 0;
	srand((unsigned)time(NULL));
	fbColor = rand();

	//if(0)
	{
		pSpiPanel->panelFill(0, 0, PANEL_WIDTH, PANEL_HEIGHT, fbColor);
	}
	
	unsigned int fontColor = 0xFFFF;
	unsigned int backColor = 0x0000;
	
#if 1
	pSpiPanel->panelDrawPoint(10, 10, fontColor);
	pSpiPanel->panelDrawPoint(20, 10, fontColor);
	pSpiPanel->panelDrawLine(30, 10, 50, 20, fontColor);
	pSpiPanel->panelDrawRectangle(60, 10, 80, 20, fontColor);
	pSpiPanel->panelDrawCircle(100, 20, 10, fontColor);
	
	pSpiPanel->panelShowChar(10, 30, 'a', fontColor, backColor, 12, true);
	pSpiPanel->panelShowChar(30, 30, 'b', fontColor, backColor, 16, true);
	pSpiPanel->panelShowChar(50, 30, 'C', fontColor, backColor, 24, true);
	pSpiPanel->panelShowChar(80, 30, 'D', fontColor, backColor, 32, true);
	
	pSpiPanel->panelShowString(0, 60, "i love you0123456789", fontColor, backColor, 24, true);
	pSpiPanel->panelShowString(0, 90, "LOVE 0123456789", fontColor, backColor, 32, true);

	pSpiPanel->panelShowIntNum(10, 130, -9223372036854775807, fontColor, backColor, 16, true);
	pSpiPanel->panelShowFloatNum(10, 150, -123456789.0123456, fontColor, backColor, 16, true);
#endif

	cout << "sizeof(\"中\") = " << sizeof("中") << endl;
	pSpiPanel->panelShowChineseFont(10, 170, "我", fontColor, backColor, 12, true);
	pSpiPanel->panelShowChineseFont(30, 170, "爱", fontColor, backColor, 16, false);
	pSpiPanel->panelShowChineseFont(50, 170, "中", fontColor, backColor, 24, false);
	pSpiPanel->panelShowChineseFont(80, 170, "华", fontColor, backColor, 32, true);
	pSpiPanel->panelShowChineseText(120, 170, "我是中国人", fontColor, backColor, 24, true);
	pSpiPanel->panelShowPicture(0, 200, 40, 40, pPicQQImage);

	return 0;
}

