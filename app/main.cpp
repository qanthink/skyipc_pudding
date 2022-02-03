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

#include "spi.h"
//#include <linux/gpio.h>

using namespace std;

int main(int argc, const char *argv[])
{
	Spi *spi = Spi::getInstance();
	spi->GPIO_INIT();
	spi->fun();

	return 0;
}

