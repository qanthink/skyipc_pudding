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
	pSpiPanel->panelFill(0, 0, PANEL_WIDTH, PANEL_HEIGHT, 0x0000);

	return 0;
}

