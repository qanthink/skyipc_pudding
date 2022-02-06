/*---------------------------------------------------------------- 
sigma star版权所有。
作者：
时间：2020.7.10
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
	pSpiPanel->panelFill(0, 0, PANEL_WIDTH, PANEL_HEIGHT, 0x00);
	pSpiPanel->panelDrawPoint(10, 10, 0xFFFF);
	pSpiPanel->panelDrawPoint(20, 20, 0xFFFF);
	pSpiPanel->panelDrawLine(30, 30, 50, 50, 0xFFFF);
	pSpiPanel->panelDrawRectangle(60, 60, 80, 70, 0xFFFF);
	pSpiPanel->panelDrawCircle(100, 100, 20, 0xFFFF);
	pSpiPanel->panelShowChar(130, 130, 'a', 0xFFFF, 0xCCCC, 16, 1);
	sleep(1);
	pSpiPanel->panelShowChar(130, 130, 'B', 0xFFFF, 0xCCCC, 16, 1);

	return 0;
}

