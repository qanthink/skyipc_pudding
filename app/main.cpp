/*----------------------------------------------------------------聽
sigma star鐗堟潈鎵€鏈夈€?浣滆€咃細
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
	
	int color = 0;
	srand((unsigned)time(NULL));
	color = rand();
	
	//if(0)
	{
		pSpiPanel->panelFill(0, 0, PANEL_WIDTH, PANEL_HEIGHT, color);
	}
	
	pSpiPanel->panelDrawPoint(10, 10, 0xFFFF);
	pSpiPanel->panelDrawPoint(20, 10, 0xFFFF);
	pSpiPanel->panelDrawLine(30, 10, 50, 20, 0xFFFF);
	pSpiPanel->panelDrawRectangle(60, 10, 80, 20, 0xFFFF);
	pSpiPanel->panelDrawCircle(100, 20, 10, 0xFFFF);
	
	pSpiPanel->panelShowChar(10, 30, 'a', 0xFFFF, 0xCCCC, 12, true);
	pSpiPanel->panelShowChar(30, 30, 'b', 0xFFFF, 0xCCCC, 16, true);
	pSpiPanel->panelShowChar(50, 30, 'C', 0xFFFF, 0xCCCC, 24, true);
	pSpiPanel->panelShowChar(80, 30, 'D', 0xFFFF, 0xCCCC, 32, true);
	
	pSpiPanel->panelShowString(0, 60, "abcdefghij0123456789", 0xFFFF, 0xCCCC, 24, true);
	pSpiPanel->panelShowString(0, 90, "ABCDE0123456789", 0xFFFF, 0xCCCC, 32, true);

	pSpiPanel->panelShowIntNum(10, 130, -9223372036854775807, 0xFFFF, 0xCCCC, 16, true);
	pSpiPanel->panelShowFloatNum(10, 150, -123456789.0123456, 0xFFFF, 0xCCCC, 16, true);
	pSpiPanel->PanelShowChineseFont(10, 170, "中", 0xFFFF, 0xCCCC, 12, true);
	pSpiPanel->PanelShowChineseFont(30, 170, "景", 0xFFFF, 0xCCCC, 16, false);
	pSpiPanel->PanelShowChineseFont(50, 170, "园", 0xFFFF, 0xCCCC, 24, false);
	pSpiPanel->PanelShowChineseFont(80, 170, "电", 0xFFFF, 0xCCCC, 32, true);
	pSpiPanel->PanelShowChineseText(120, 170, "中景园电子", 0xFFFF, 0xCCCC, 24, true);


	return 0;
}

